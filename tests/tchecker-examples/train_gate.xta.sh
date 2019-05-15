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

system Gate, Trains;

/*
# Gate process
echo "# Gate
process:Gate
int:$N:1:$N:1:buffer
int:1:0:$((N-1)):0:head
int:1:0:$N:0:length
location:Gate:Free{initial:}
location:Gate:Occ{}
location:Gate:Transient:{committed:}"

for pid in `seq 1 $N`; do
    echo "edge:Gate:Free:Occ:go$pid{provided:length>0&&buffer[head]==$pid}
edge:Gate:Free:Occ:appr$pid{provided:length==0 : do:buffer[(head+length)%$N]=$pid;length=length+1}
edge:Gate:Occ:Transient:appr$pid{do:buffer[(head+length)%$N]=$pid;length=length+1}
edge:Gate:Occ:Free:leave$pid{provided:length>0&&buffer[head]==$pid : do:head=(head+1)%$N;length=length-1}
edge:Gate:Transient:Occ:stop$pid{provided:length>0&&buffer[(head+length-1)%$N]==$pid}"
done

echo ""

# Train processes

for pid in `seq 1 $N`; do
    echo "# Train $pid
process:Train$pid
clock:1:x$pid
location:Train$pid:Safe{initial:}
location:Train$pid:Appr{invariant:x$pid<=20}
location:Train$pid:Stop{}
location:Train$pid:Start{invariant:x$pid<=15}
location:Train$pid:Cross{invariant:x$pid<=5 : labels:cross$pid}
edge:Train$pid:Safe:Appr:appr{do:x$pid=0}
edge:Train$pid:Appr:Cross:tau{provided:x$pid>=10 : do:x$pid=0}
edge:Train$pid:Appr:Stop:stop{provided:x$pid<=10}
edge:Train$pid:Stop:Start:go{do:x$pid=0}
edge:Train$pid:Start:Cross:tau{provided:x$pid>=7 : do:x$pid=0}
edge:Train$pid:Cross:Safe:leave{provided:x$pid>=3}

sync:Train$pid@appr:Gate@appr$pid
sync:Train$pid@stop:Gate@stop$pid
sync:Train$pid@go:Gate@go$pid
sync:Train$pid@leave:Gate@leave$pid
"
done*/

EOF