#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

L=808  # lambda
S=26   # sigma

function usage() {
    echo "Usage: $0 N";
    echo "       $0 N L S";
    echo "       N number of processes";
    echo "       L (lambda) delay for full communication";
    echo "       S (sigma) delay for collision detection";
}

if [ $# -eq 1 ]; then
    N=$1
elif [ $# -eq 3 ]; then
    N=$1
    L=$2
    S=$3
else
    usage
    exit 1
fi

# Model
cat << EOF
//
// System csmacd_${N}_${L}_$S
//
const int N = ${N};
const int L = ${L};
const int S = ${S};

typedef int[1,N] pid_t;

chan begin, busy, end;
chan cdi[pid_t];

process Bus() {
    int [1,N+1] j = 1;
    clock y;
    state Idle,
          Active,
          Collision { y < S },
          Loop;
    commit Loop;
    init Idle;
    trans
        Idle->Active { sync begin!; assign y = 0; },
        Active->Collision { guard y < S; sync begin!; assign y = 0; },
        Active->Active { guard y >= S; sync busy!; },
        Active->Idle { sync end!; assign y = 0; },
        Collision->Loop { guard y < S; assign j = 1; },
        Loop->Idle { guard j==N+1 && y<S; assign y=0, j=1; },
        Loop->Loop { select pid : pid_t; guard j == pid; sync cdi[pid]!;
        assign j++; };
}

process Station(const pid_t pid) {
    clock x;
    state Wait,
          Start { x <= L },
          Retry { x < 2*S };
    init Wait;
    trans
    Wait->Start { sync begin?; assign x = 0; },
    Wait->Retry { sync busy?; assign x = 0; },
    Wait->Wait { sync cdi[pid]?; assign x = 0; },
    Wait->Retry { sync cdi[pid]?; assign x = 0; },
    Start->Wait { guard x == L; sync end?; assign x = 0; },
    Start->Retry { guard x  < S; sync cdi[pid]?; assign x = 0; },
    Retry->Start { guard x < 2*S; sync begin?; assign x = 0; },
    Retry->Retry { guard x < 2*S; sync busy?; assign x = 0; },
    Retry->Retry { guard x < 2*S; sync cdi[pid]?; assign x = 0; };
}

system Bus, Station;
EOF
