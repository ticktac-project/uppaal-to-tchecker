#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

K=10

function usage() {
    echo "Usage: $0 N";
    echo "       N number of processes";
}

if [ $# -eq 1 ]; then
    N=$1
else
    usage
    exit 1
fi

cat << EOF
//
// System parallel_bis${N}
//
const int N = ${N};

process Processes(int [1,N] pid) {
    clock x;
    state A, B,
            C { x <= 3 }; // labels access\$pid
    init A;
    trans
        A -> B { assign x = 0; },
        B -> A { guard x >= 1; },
        B -> C { guard x < 1; assign x = 0; },
        C -> A { guard x > 1; };
}

system Processes;
EOF
