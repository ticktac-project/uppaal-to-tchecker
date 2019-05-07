#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Generates a TChecker model for the dining philosophers, with a timeout
# to release the left fork if the right fork cannot be obtained in time,
# in order to avoid deadlocks. Inspired from:
# D. Lugiez, P. Niebert and S. Zennou, "A partial order semantics approach
# to the clock explosion problem of timed automata", TCS 2005

# Checks command line arguments
if [ $# -eq 1 ];
then
    N=$1        # number of philosophers
    TIMEOUT=3   # time-out to acquire the second fork
    EAT=10      # time required by a philosopher to eat
    SLOW=0      # time in-between releasing the left fork and the right fork
else
    if [ $# -ge 4 ];
    then
	N=$1        # number of philosophers
	TIMEOUT=$2  # time-out to acquire the second fork
	EAT=$3      # time required by a philosopher to eat
	SLOW=$4     # time in-between releasing the left fork and the right fork
    else
	echo "Usage: $0 <# philosophers>";
	echo "       $0 <# philosophers> <timeout> <eat> <slow>";
	echo " ";
	echo "where:   <# philosophers>  is the number of philosophers";
	echo "         <timeout>         is the timeout to release the left fork";
	echo "         <eat>             is the duration of the dinner";
	echo "         <slow>            is the delay to release forks";
	echo " ";
	echo "By default, we choose timeout=3, eat=10, slow=0";
	exit 1
    fi
fi

if [ "$N" -lt 2 ]; then
    echo "ERROR: number of philosophers should be >= 2";
    exit 1;
fi

if [ "$TIMEOUT" -lt 0 ]; then
    echo "ERROR: timeout should be >= 0"
    exit 1;
fi

if [ "$EAT" -lt 0 ]; then
    echo "ERROR: duration of dinner should be >= 0"
    exit 1;
fi

if [ "$SLOW" -lt 0 ]; then
    echo "ERROR: slowness should be >= 0"
    exit 1;
fi

cat <<EOF
//
// System dining_philosophers_${N}_${TIMEOUT}_${EAT}_${SLOW}
//
const int N = ${N};
const int TIMEOUT = ${TIMEOUT};
const int EAT = ${EAT};
const int SLOW = ${SLOW};

typedef int[1,N] fork_t;

chan take[fork_t], release[fork_t];

process Fork(const fork_t id) {
    state free, taken;
    init free;
    trans
       free->taken { sync take[id]?; },
       taken->free { sync release[id]?; };
}

typedef int [1,N] pid_t;

process Philosopher(const pid_t p) {
    const int LEFT = ((p == 1)? N :p - 1);
    const int RIGHT = p;
    clock x;
    state
        idle,
        acq { x <= TIMEOUT },
        eat { x <= EAT }, // labels eating\${p}
        rel { x <= SLOW };
    init idle;
    trans
        idle->acq { sync take[LEFT]!; assign x = 0; },
        acq->idle { guard x >= TIMEOUT; sync release[LEFT]!; },
        acq->eat { guard x <= TIMEOUT; sync take[RIGHT]!; assign x = 0; },
        eat->rel { guard x >= EAT; sync release[RIGHT]!; assign x = 0; },
        rel->idle { sync release[LEFT]!; };
}

system Fork, Philosopher;
EOF