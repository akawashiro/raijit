#!/bin/bash -eux

cd $(git rev-parse --show-toplevel)
ROOT_DIR=$(pwd)
PYTHON_BINARY=${ROOT_DIR}/cpython-install/bin/python3.12
export PYTHONPATH=./build
export LD_LIBRARY_PATH=${ROOT_DIR}/cpython-install/lib
export RAIJIT_TEST_MODE=1

${PYTHON_BINARY} ./app_tests/nbody_test.py
