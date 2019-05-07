#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

K=10

function usage() {
    echo "Usage: $0 N";
    echo "       $0 N K";
    echo "       N number of processes";
    echo "       K delay for mutex (default: $K)"
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
// System fischer_${N}_${K}
//
const int N = ${N};
const int K = ${K};

typedef int [1,N] pid_t;

int [0,N] id = 0;

process Process(const pid_t pid) {
    clock x;
    state
        A,
        req { x <= K },
        wait,
        cs; // labels= cs\$pid
    init A;
    trans
        A -> req { guard id == 0; assign x = 0; },
        req -> wait { guard x <= K; assign x = 0, id = pid; },
        wait -> req { guard id == 0; assign x = 0; },
        wait -> cs { guard x > K && id == pid; },
        cs -> A { assign id = 0; };
}
system Process;
EOF
