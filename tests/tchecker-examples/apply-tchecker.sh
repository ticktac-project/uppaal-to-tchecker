#!/bin/bash

UTOT=utot
TCHECKER=tchecker

if test $# = 0;
then
    echo 2>&1 "usage: $0 sh-script [args...]"
    exit 1
fi

SCRIPT=$1
shift

case ${SCRIPT/.sh/} in
    *.xta)
        OUTPUTFILE=${SCRIPT/.xta.sh/_xta}
        XTAMODE=yes
    ;;
    *)
        OUTPUTFILE=${SCRIPT/.sh/}
        XTAMODE=no
    ;;
esac

if test $# -gt 0;
then
    OUTPUTFILE=${OUTPUTFILE}_$(echo $* | tr " " "_")
fi

${SCRIPT} $* > ${OUTPUTFILE} || exit 1

if test ${XTAMODE} = "yes";
then
    TCHECKER_INPUT=${OUTPUTFILE/.xta/}
    ${UTOT} -e --xta ${OUTPUTFILE} ${TCHECKER_INPUT} || exit 1
else
    TCHECKER_INPUT=${OUTPUTFILE}
fi

TCHECKER_RESULT=${TCHECKER_INPUT}.res

${TCHECKER} explore -m ta -s bfs ${TCHECKER_INPUT} > ${TCHECKER_RESULT}

echo "RESULT=${TCHECKER_RESULT}"
echo "NBSTATE= $(grep -e '^[0-9]*:' ${TCHECKER_RESULT} | wc -l)"
echo "NBTRANS= $(grep -e '^[0-9]* -> [0-9]* ' ${TCHECKER_RESULT} | wc -l)"









