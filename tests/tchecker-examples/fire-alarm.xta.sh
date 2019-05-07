#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Generates a TChecker model for the simplified Fire Alarm model in
# Figure 1 of:
# "Timed Automata with Disjoint Activity", Marco Muniz, Bernd Westphal
# and Andreas Podelski, FORMATS 2012.

# Checks command line arguments
if [ $# -eq 1 ];
then
    NSENSORS=$1                  # Number of sensors (processes)
    WINSIZE=10                   # Window size
    DURATION=$(($NSENSORS * 50)) # Cycle duration
else
    if [ $# -eq 3 ];
    then
	NSENSORS=$1 # Number of sensors (processes)
	WINSIZE=$2  # Window size
        DURATION=$3 # Cycle duration
    else
	echo "Usage: $0 <# sensors>";
	echo "       $0 <# sensors> <window size> <cycle duration>";
	echo " ";
	echo "where:    <# sensors>      is the number of sensor processes";
	echo "          <window size>    is the delay allocated for ack.";
	echo "          <cycle duration> is the duration of the whole cycle";
	echo " ";
	echo "Default values: window size=10, cycle duration=50*<# sensors>";
	exit 0;
    fi
fi

cat << EOF
//
// system fire_alarm_${NSENSORS}_${WINSIZE}_${DURATION}
//
const int NSENSORS = ${NSENSORS};
const int WINSIZE = ${WINSIZE};
const int DURATION = ${DURATION};

chan alive, ack;

process Sensor(const int [1, NSENSORS] pid) {
    const int WINSTART = 2 * pid * WINSIZE - WINSIZE;
    const int WINEND = 2 * pid * WINSIZE;
    const int WINSEND = WINSTART + 5;
    clock x;
    state
        ini { x <= WINSTART },
        wait { x <= WINSEND },
        sent { x <= WINEND },
        fin { x <= DURATION };
    init ini;
    trans
        ini -> wait { guard x >= WINSTART; },
        wait -> sent { sync alive?; },
        sent -> fin { sync ack?; },
        sent -> fin { guard x >= WINEND; },
        fin -> ini { guard x >= DURATION; assign x = 0; };
}

process CentralProcess() {
    state I;
    init I;
    trans
        I -> I { sync ack!; },
        I -> I { sync alive!; };
}

system CentralProcess, Sensor;
EOF
