#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Generates a TChecker model for a Leader election protocol over a ring inspired
# from "Fully Symbolic Timed Model Checking using Constraint Matrix Diagrams"
# R. Ehlers, D. Fass, M. Gerke and H-J Peter, RTSS 2010.
# Model without global variable, compatible with asynchronous zone graph

### Checks command line arguments
if [ ! $# -eq 2 ];
then
    echo "Usage: $0 <# processes> <timeout>"
    exit 0;
fi

NPROCS=$1
TIMEOUT=$2 # 4 in RTSS10 paper

if [ $NPROCS -le 1 ]; # Number of processes at least 2
then
    echo "*** Invalid number of processes: $N";
    echo "The number of processes must be at least 2";
    exit 0;
fi

cat << EOF
//
// System leader_election_async_${NPROCS}_${TIMEOUT}
//

const int NPROCS = ${NPROCS};
const int TIMEOUT = ${TIMEOUT};

typedef int[1,NPROCS] pid_t;

//chan read_out;
chan finished[pid_t], lock[pid_t], unlock[pid_t], read_out[pid_t];
pid_t in[pid_t] = { 1 $(for p in `seq 2 $NPROCS`; do printf ", ${p}"; done) };
pid_t out[pid_t] = { 1 $(for p in `seq 2 $NPROCS`; do printf ", ${p}"; done) };

process Candidates(const pid_t p) {
    const int prev = ((p == 1)?NPROCS:(p - 1));
    clock x;

    state
        eval { x <= 1},
        initial { x <= 0},
        leader,
        read { x <= 1 },
        release { x <= 1 },
        wait_prev { x <= 0},
        wait_succ,
        wait_succ_read_out;
    init initial;
    trans
        eval -> eval { sync read_out[p]; },
        initial -> initial { sync read_out[p]; },
        leader -> leader { sync read_out[p]; },
        read -> read { sync read_out[p]; },
        release -> release { sync read_out[p]; },
        wait_prev -> wait_prev { sync read_out[p]; },
        wait_succ -> wait_succ { sync read_out[p]; },
        wait_succ_read_out -> wait_succ_read_out { sync read_out[p]; },
        eval -> leader { guard in[p] == p; sync finished[p]; },
        eval -> wait_succ { guard in[p] != p; assign out[p] = in[p]; },
        initial -> read { sync lock[prev]; assign x = 0; },
        initial -> wait_succ_read_out { sync lock[p]; },
        read -> release { sync read_out[prev]; assign in[p] = out[prev]; },
        release -> eval { sync unlock[prev]; },
        wait_prev -> read { sync lock[prev]; assign x = 0; },
        wait_succ -> wait_succ_read_out { sync lock[p]; },
        wait_succ_read_out -> wait_prev { sync unlock[p]; assign x = 0; };
}

process Stopwatch() {
    clock z;
    state
        error, // labels error
        initial,
        safe;
    init initial;
    trans
        initial -> error { guard z > TIMEOUT; },
        initial -> safe { select p : pid_t; guard z <= TIMEOUT; sync finished[p]; };
}
EOF

for i in $(eval echo "{1..$NPROCS}"); do
    echo "C${i} := Candidates($i);"
done

echo -n "system Stopwatch"
for i in $(eval echo "{1..$NPROCS}"); do
    echo -n ",C${i}"
done
echo ";"

echo -n "IO Stopwatch { finished[1]"
for i in $(eval echo "{2..$NPROCS}"); do
    echo -n ", finished[$i]"
done
echo "}"

echo "IO C1 { read_out[1], read_out[$NPROCS], finished[1], lock[1], lock[$NPROCS], unlock[1], unlock[$NPROCS] }"
for i in $(eval echo "{2..$NPROCS}"); do
    pred=$(($i-1))
    echo "IO C$i { read_out[$i], read_out[$pred], finished[$i], lock[$i], lock[$pred], unlock[$i], unlock[$pred] }"
done
