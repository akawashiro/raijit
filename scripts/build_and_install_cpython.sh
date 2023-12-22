#! /bin/bash

set -eux

ROOT_DIR=$(git rev-parse --show-toplevel)
CPYTHON_SRC_DIR=${ROOT_DIR}/cpython-src
CPYTHON_INSTALL_DIR=${ROOT_DIR}/cpython-install

rm -rf ${CPYTHON_INSTALL_DIR}

if [ ! -d ${CPYTHON_SRC_DIR} ]; then
    git clone https://github.com/python/cpython.git ${CPYTHON_SRC_DIR}
fi
cd ${CPYTHON_SRC_DIR}
git checkout 3.12
./configure --prefix=${CPYTHON_INSTALL_DIR} --enable-shared
bear -- make -j10 install
