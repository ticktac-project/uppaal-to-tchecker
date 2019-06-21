#!/usr/bin/env bash

set -euvx

mkdir -p ${TCHECKER_REPO}/build || true

pushd ${TCHECKER_REPO}/build
 cmake -DTCK_ENABLE_COVREACH_TESTS=OFF -DTCK_ENABLE_EXPLORE_TESTS=OFF \
       -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} ..
 make -j ${NB_CPUS}
 make install
popd

mkdir build || true
cd build
cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      ..

make -j ${NB_CPUS}

CTEST_OUTPUT_ON_FAILURE=1 make test

