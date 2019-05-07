#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Generates a TChecker model for the Cornell Single Sifn-On (CorSSO)
# protocol inspired from:
# "SAT-based model-checking for region automata", Fang Yu and Bow-Yaw Wang,
# International Journal of Foundations of Computer Science, 17(4), 2006

# Checks command line arguments
if [ $# -eq 1 ];
then
    N=$1        # number of processes
    TA=2        # min time to acquire a certificate
    TE=10       # max time to acquire all certificates
    LTH="1 2"   # numbers of required certificates for each policy
    NTH=2       # size of TH (seen as an array)
else
    if [ $# -ge 4 ];
    then
	N=$1        # number of processes
	TA=$2       # min time to acquire a certificate
	TE=$3       # max time to acquire all certificates
	shift
	shift
	shift
	LTH=$*      # number of required certificates for each policy
	NTH=$#      # size of TH (seen as an array)
    else
	echo "Usage: $0 <# processes>";
	echo "       $0 <# processes> <TA> <TE> <TH1> ... <THn>";
	echo " ";
	echo "where:   <# processes>  is the number of processes";
	echo "         <TA>           is the min time to acquire a certificate";
	echo "         <TE>           is the max time to acquire a certificate";
	echo "         <THi>          is the number of certificates for policy i";
	echo " ";
	echo "By default, we choose TA=2, TE=10, TH=1 2";
	exit 1
    fi
fi

# Computes the maximum number of required certificates
MAXTH=0
for n in $LTH; do
    if [ "$n" -gt "$MAXTH" ];
    then
	let MAXTH=$n
    fi
done

declare -a TH
let i=0
for th in $LTH; do
    TH[i]=$th
    i=$(($i+1))
done

# Model

cat << EOF
//
// System CorSSO_${N}_${TA}_${TE}_$(echo ${LTH} | tr " " "_")
//

const int N=${N};
const int MAXTH=${MAXTH};
const int NTH=${NTH};
const int TE=${TE};
const int TA=${TA};
const int TH[NTH] = { $(echo ${LTH} | tr " " ",") };

typedef int[1,N] pid_t;

process P(pid_t pid) {
    clock x, y;
    int [0,MAXTH+1] a = 0;
    int [0,MAXTH+1] p = 0;

    state auth,
          access; // has label access\${pid}
    init auth;

    trans
        auth->auth{
            guard p > 0 && x > TA && a < MAXTH;
            assign a++, x = 0;
        },
        auth->auth{
            select j : int[0,NTH-1];
            guard p == 0;
            assign p = j+1, a = 0, x = 0, y = 0;
        },
        auth->access{
            select j : int[0,NTH-1];
            guard y < TE && p == j+1 && a >= TH[j];
        },
        access->auth{
            assign p = 0;
        };
}

system P;
EOF

