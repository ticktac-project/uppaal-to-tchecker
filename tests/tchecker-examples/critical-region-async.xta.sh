#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Critical region, inspired from:
# "Fully Symbolic Model Checking for Timed Automata", Georges Morbé,
# Florian Pigorsch and Christoph Scholl, CAV 2011
# Model without global variable, compatible with asynchronous zone graph

if [ $# -eq 1 ];
then
  NPROCS=$1            # Number of processes
  T=10
else if [ $# -eq 2 ];
then
  NPROCS=$1
  T=$2
else  
	echo "Usage: $0 <# stations>";
	echo "       $0 <# stations> <t>";
	echo " ";
	echo "where:    <# stations> is the number of stations in the network";
	echo "          <t>          is the limit on process";
	echo " ";
	echo "The values must satisfy: #stations >= 1";
	echo " ";
	echo "The values in 'Fully Symbolic Model Checking for Timed Automata' \
CAV, 2011, were t=10";
	exit 0
  fi
fi


if [ ! $NPROCS -ge 1 ];
then
    echo "*** The number of processes must be at least 1"
    exit 0
fi


# Model
cat << EOF
//
// System critical_region_async_${NPROCS}_${T}
//
const int NPROCS = ${NPROCS};
const int T = ${T};
typedef int[1,NPROCS] pid_t;

chan ch_enter[pid_t];
chan ch_exit[pid_t];
chan id_eq_0, id_lt_NPROCS, id_eq_NPROCS;

int [0,NPROCS] id = 0;

process ID() {
    state l;
    init l;
    trans
        l->l { guard id == 0; sync id_eq_0; },
        l->l { guard id < NPROCS; sync id_lt_NPROCS; },
        l->l { guard id == NPROCS; sync id_eq_NPROCS; },
        l->l { select pid : pid_t; guard id == pid; sync ch_enter[pid];
               assign id = 0; },
        l->l { select pid : pid_t; sync ch_exit[pid]; assign id = pid; };
}

process Counter() {
    state I, C;
    init I;
    trans
        I->C { sync id_eq_0; assign id = 1; },
        C->C { sync id_lt_NPROCS; assign id++; },
        C->C { sync id_eq_NPROCS; assign id = 1; };
}

process Arbiter(const pid_t pid) {
    state req, ack;
    init req;
    trans
        req->ack { sync ch_enter[pid]; },
        ack->req { sync ch_exit[pid]; };
}

process ProdCell(const pid_t pid) {
    clock x;
    state not_ready,
          testing { x <= T },
          requesting,
          critical { x <= 2*T },
          testing2 { x <=  T },
          safe, // labels safe\${pid}
          error; // labels error\${pid}
    init not_ready;
    trans
    not_ready->testing { guard x <= 2*T; assign x = 0; },
    testing->not_ready { guard x >= T; assign x = 0; },
    testing->requesting { guard x <= T-1; },
    requesting->critical { sync ch_enter[pid]; assign x = 0; },
    critical->error { guard x >= 2*T; },
    critical->testing2 { guard x <= T-1; sync ch_exit[pid]; assign x = 0; },
    testing2->error { guard x >= T; },
    testing2->safe { guard x <= T-1; };
}
system ID, Counter, Arbiter, ProdCell;
EOF

