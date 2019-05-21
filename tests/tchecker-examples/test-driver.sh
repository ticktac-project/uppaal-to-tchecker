#! /bin/bash

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

check_environment UTOT TCHECKER SRCDIR

TESTNAME="$1"
shift
TESTPARAMS="$@"

APPLY_TCHECKER_SH="${SRCDIR}/apply-tchecker.sh"
TCHECKER_GENERATOR_SCRIPT="${SRCDIR}/${TESTNAME}.sh"
UTOT_GENERATOR_SCRIPT="${SRCDIR}/${TESTNAME}.xta.sh"

check_executable UTOT TCHECKER APPLY_TCHECKER_SH \
                 TCHECKER_GENERATOR_SCRIPT \
                 UTOT_GENERATOR_SCRIPT

export UTOT TCHECKER

PREFIX1=$(echo "$TESTNAME $TESTPARAMS" | tr " " "_")-direct
${APPLY_TCHECKER_SH} ${TCHECKER_GENERATOR_SCRIPT} ${PREFIX1} \
                     ${TESTPARAMS} \
  || exit 1

PREFIX2=$(echo "$TESTNAME $TESTPARAMS" | tr " " "_")-utot
${APPLY_TCHECKER_SH} ${UTOT_GENERATOR_SCRIPT} ${PREFIX2} \
                     ${TESTPARAMS} \
  || exit 1

cmp -s ${PREFIX1}.cards ${PREFIX2}.cards

exit $?
