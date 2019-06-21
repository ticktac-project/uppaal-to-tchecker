#!/usr/bin/env bash

set -eu

echo "#### INSTALLING TChecker Sources"

git clone https://github.com/ticktac-project/tchecker.git ${TCHECKER_REPO} || true
cd ${TCHECKER_REPO} && git checkout ${TCHECKER_BRANCH}

(. ${TCHECKER_REPO}/ci-scripts/setenv-common.sh;
  ${TCHECKER_REPO}/ci-scripts/install-${TRAVIS_OS_NAME}.sh;
  .  ${TCHECKER_REPO}/ci-scripts/setenv-${TRAVIS_OS_NAME}.sh;
  ${TCHECKER_REPO}/ci-scripts/install-sources.sh)

