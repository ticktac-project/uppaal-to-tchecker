#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

function usage() {
    echo "Usage: $0 N";
    echo "       N number of trains";
}

if [ $# -eq 1 ]; then
    N=$1
else
    usage
    exit 1
fi

cat << EOF
//
// System train_gate_$N
//
const int N = ${N};

typedef int [1,N] pid_t;

chan go[pid_t], appr[pid_t], leave[pid_t], stop[pid_t];

process Gate() {
    int [1,N] buffer[N] = { 1$(for i in `seq 2 $N`; do echo -n ", 1"; done) };
    int [0,N-1] head = 0;
    int [0,N] length = 0;
    state Free, Occ, Transient;
    commit Transient;
    init Free;
    trans
        Free -> Occ {
            select pid : pid_t;
            guard length > 0 && buffer[head] == pid;
            sync go[pid];
        },
        Free -> Occ {
            select pid : pid_t;
            guard length == 0;
            sync appr[pid];
            assign buffer[ (head+length) % N] = pid, length++;
        },
        Occ -> Transient {
            select pid : pid_t;
            sync appr[pid];
            assign buffer[(head+length)%N]=pid, length++;
        },
        Occ -> Free {
            select pid : pid_t;
            guard length>0&&buffer[head]==pid;
            sync leave[pid];
            assign head=(head+1)%N, length--;
        },
        Transient -> Occ {
            select pid : pid_t;
            guard length>0&&buffer[(head+length-1) % N] ==  pid;
            sync stop[pid];
        };
}

process Trains(const pid_t pid) {
    clock x;
    state
        Safe,
        Appr { x <= 20 },
        Stop,
        Start { x <= 15 },
        Cross { x <= 5 }; // labels:cross\$pid
    init Safe;
    trans
        Safe -> Appr { sync appr[pid]; assign x = 0; },
        Appr -> Cross { guard x >= 10; assign x = 0; },
        Appr -> Stop { guard x <= 10; sync stop[pid]; },
        Stop -> Start { sync go[pid]; assign x = 0; },
        Start -> Cross { guard x >= 7; assign x = 0; },
        Cross -> Safe { guard x >= 3; sync leave[pid]; };
}

EOF


for i in $(eval echo "{1..${N}}"); do
    echo "T${i} := Trains($i);"
done

echo -n "system Gate"
for i in $(eval echo "{1..${N}}"); do
    echo -n ",T${i}"
done
echo ";"

echo -n "IO Gate { go[1], appr[1], leave[1], stop[1]"
for i in $(eval echo "{1..${N}}"); do
    echo -n ", go[$i], appr[$i], leave[$i], stop[$i]"
done
echo "}"

for i in $(eval echo "{1..${N}}"); do
    echo -n "IO T${i} { go[$i], appr[$i], leave[$i], stop[$i] }"
done
