#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Variant of the classical model of Fischer's protocol where the shared
# variable id is modeled as a process (required from asynchrnous zone graph)

# Check parameters

K=10

function usage() {
    echo "Usage: $0 N [K]";
    echo "       N number of processes";
    echo "       K delay for mutex (default: ${K})"
}

if [ $# -eq 1 ]; then
    N=$1
elif [ $# -eq 2 ]; then
    N=$1
    K=$2
else
    usage
    exit 1
fi

cat << EOF
//
// System fischer_async_${N}_${K}
//

const int N = ${N};
const int K = ${K};

typedef int [1,N] pid_t;

chan id_is[N+1], id_to[N+1];

process Process(const pid_t pid) {
    clock x;
    state
        A,
        req { x <= K },
        wait,
        cs; // labels= cs\$pid
    init A;
    trans
    A -> req { sync id_is[0]?; assign x = 0; },
    req -> wait { guard x <= K; sync id_to[pid]!; assign x = 0; },
    wait -> req { sync id_is[0]?; assign x = 0; },
    wait -> cs { guard x > K; sync id_is[pid]?; },
    cs -> A { sync id_to[0]!; };
}

process ID() {
    int [0,N] id = 0;
    state l;
    init l;
    trans
        l -> l { select i : int [0,N]; guard id == i; sync id_is[i]!; },
        l -> l { select i : int [0,N]; sync id_to[i]?; assign id = i; };
}

system ID, Process;
EOF

