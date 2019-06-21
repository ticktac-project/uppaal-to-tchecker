#! /bin/bash

if test $# -ne 1; then
    echo 1>&2 "usage: $0 testname."
    exit 1
fi

TESTNAME="$1"

check_environment() {
    for var in $@; do
        eval "EXE=\${$var}"

        if test "x${EXE}" = "x"; then
            echo 1>&2 "missing ${var} envvar"
            exit 1
        fi
    done
}

check_executable() {
    for var in $@; do
        eval "EXE=\${$var}"

        if ! test -x ${EXE}; then
            echo 1>&2 "${EXE} is not executable";
            exit 1
        fi
    done
}

check_expected_file() {
    FILE="$1"
    EXPECTED="${SRCDIR}/${1}-expected"
    IS_ERR="$2"
    if test -f ${EXPECTED}; then
        if ! cmp -s ${FILE} ${EXPECTED}; then
            echo 1>&2 "${f} and ${EXPECTED} differs. try:"
	    echo 1>&2 "diff ${f} ${EXPECTED}"
            exit 1
        fi
    elif test "${IS_ERR}" = "yes" -a -s "${FILE}"; then
        echo 1>&2 "Something went wrong during test ${TESTNAME}."
        echo 1>&2 "Have a look to ${FILE}".
        exit 1
    fi
}

check_environment UTOT TCHECKER SRCDIR
check_executable UTOT TCHECKER

UTOT_OUTFILE="${TESTNAME}.tck.out"
UTOT_ERRFILE="${TESTNAME}.tck.err"
TCHECKER_OUTFILE="${TESTNAME}.out"
TCHECKER_ERRFILE="${TESTNAME}.err"
TCHECKER_CARDSFILE="${TESTNAME}.cards"

if ${UTOT} --sysname S ${SRCDIR}/${TESTNAME} > ${UTOT_OUTFILE} \
    2> ${UTOT_ERRFILE} && \
   test ! -s ${UTOT_ERRFILE} && \
   ${TCHECKER} explore -m ta -s bfs ${UTOT_OUTFILE} > ${TCHECKER_OUTFILE} \
    2> ${TCHECKER_ERRFILE} && \
    test ! -s ${TCHECKER_ERRFILE};
then
    cat > ${TCHECKER_CARDSFILE} << EOF
NBSTATE=$(grep -e '^[0-9]*:' ${TCHECKER_OUTFILE} | wc -l | tr -d " \t")
NBTRANS=$(grep -e '^[0-9]* -> [0-9]* ' ${TCHECKER_OUTFILE} | wc -l | tr -d " \t")
EOF
fi

for f in ${UTOT_OUTFILE} ${TCHECKER_OUTFILE} ${TCHECKER_CARDSFILE}; do
    check_expected_file $f no
done

for f in ${UTOT_ERRFILE} ${TCHECKER_ERRFILE}; do
    check_expected_file $f yes
done

exit 0
