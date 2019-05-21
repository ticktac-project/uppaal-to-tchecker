#!/bin/bash

UTOT=${UTOT:-utot}
TCHECKER=${TCHECKER:-tchecker}

if test $# -lt 2;
then
    echo 2>&1 "usage: $0 sh-script output-prefix [args...]"
    exit 1
fi

SCRIPT=$1
PREFIX=$2
shift 2

GENERATED_FILE="${PREFIX}.out"
TCHECKER_INPUT="${PREFIX}.tck"
TCHECKER_RESULT="${PREFIX}.res"
TCHECKER_CARDS="${PREFIX}.cards"

case ${SCRIPT/.sh/} in
    *.xta) XTAMODE=yes ;;
    *) XTAMODE=no ;;
esac

${SCRIPT} $* > ${GENERATED_FILE} || exit 1

if test ${XTAMODE} = "yes";
then
    ${UTOT} -e --xta ${GENERATED_FILE} ${TCHECKER_INPUT} || exit 1
else
    cp ${GENERATED_FILE} ${TCHECKER_INPUT} || exit 1
fi

${TCHECKER} explore -m ta -s bfs ${TCHECKER_INPUT} > ${TCHECKER_RESULT} \
    || exit 1

cat > ${TCHECKER_CARDS} << EOF
NBSTATE=$(grep -e '^[0-9]*:' ${TCHECKER_RESULT} | wc -l)
NBTRANS=$(grep -e '^[0-9]* -> [0-9]* ' ${TCHECKER_RESULT} | wc -l)
EOF









