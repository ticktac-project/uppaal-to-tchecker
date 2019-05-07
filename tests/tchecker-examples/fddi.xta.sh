#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

function usage() {
    echo "Usage: $0 N";
    echo "       $0 N TTRT SA TD";
    echo "";
    echo "       N number of processes";
    echo "       TTRT target token rotation timer (default: N*50)";
    echo "       SA synchronous allocation (default: 20)";
    echo "       TD token delay (default: 0)"
}

if [ $# -eq 1 ]; then
    N=$1
    TTRT=$(($N*50))
    SA=20
    TD=0
elif [ $# -eq 4 ]; then
    N=$1
    TTRT=$2
    SA=$3
    TD=$4
else
    usage
    exit 1
fi

if [ $N -lt 1 ]; then
    echo "The number of processes must be at least 1";
    exit 0
fi

cat << EOF
// Model of the FDDI protocol inspired from:
// The tool Kronos, C. Daws, A. Oliveiro, S. Tripakis and S. Yovine,
// Hybrid Systems III, 1996

// System fddi_${N}_${TTRT}_${SA}_${TD}

const int N = ${N};
const int TTRT = ${TTRT};
const int SA = ${SA};
const int TD = ${TD};

typedef int [1, N] pid_t;

chan TT[pid_t], RT[pid_t];

process Process(const pid_t pid) {
    clock trt;
    clock xA;
    clock xB;
    state
        q0,
        q1 { trt <= SA },
        q2 { trt <= SA },
        q3 { xA <= TTRT + SA },
        q4,
        q5 { trt <= SA },
        q6 { trt <= SA},
        q7 { xB <= TTRT + SA };
    init q0;
    trans
        q0 -> q1 { guard trt >= TTRT; sync TT[pid]; assign trt = 0, xB = 0;},
        q0 -> q2 { guard trt < TTRT; sync TT[pid]; assign trt = 0, xB = 0;},
        q1 -> q4 { guard trt == SA; sync RT[pid]; },
        q2 -> q3 { guard trt == SA; },
        q3 -> q4 { sync RT[pid]; },
        q4 -> q5 { guard trt >= TTRT; sync TT[pid]; assign trt = 0, xA = 0;},
        q4 -> q6 { guard trt < TTRT; sync TT[pid]; assign trt = 0, xA = 0;},
        q5 -> q0 { guard trt == SA; sync RT[pid]; },
        q6 -> q7 { guard trt == SA; },
        q7 -> q0 {sync RT[pid]; };
}

process Ring() {
    clock t;
    state
        q1 { t <= TD }, r1 $(for pid in `seq 2 $N`; do echo ", q${pid} { t <= TD }, r${pid}"; done);
    init q1;
    trans
        $(for pid in `seq 1 $(($N-1))`; do
            echo "q${pid} -> r${pid} { guard t == TD; sync TT[${pid}]; },";
            echo "r${pid} -> q$(($pid+1)) { sync RT[${pid}]; assign t = 0;
            },";
        done)

        $(echo "q${N} -> r${N} { guard t == TD; sync TT[N]; },")
        $(echo "r${N} -> q1 { sync RT[N]; assign t = 0; };")
}

system Process, Ring;
EOF