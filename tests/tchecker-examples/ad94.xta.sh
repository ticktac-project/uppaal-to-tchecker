#!/bin/sh

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

cat << EOF
chan a,b,c,d;

process P() {
    clock x, y;
    state l0,l1,l2,
            l3; // has green label
    init l0;

    trans
        l0->l1 { sync a; assign y = 0; },
        l1->l2 { guard y == 1; sync b; },
        l1->l3 { guard x < 1; sync c; },
        l2->l3 { guard x < 1; sync c; },
        l3->l1 { guard y < 1; sync a; assign y = 0; },
        l3->l3 { guard x > 1; sync d; };
}

system P;

IO P {a,b,c,d}
EOF


