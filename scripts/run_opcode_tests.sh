#!/bin/bash -eux

cd $(git rev-parse --show-toplevel)
ROOT_DIR=$(pwd)
PYTHON_BINARY=${ROOT_DIR}/cpython-install/bin/python3.12

export PYTHONPATH=${ROOT_DIR}/build
export LD_LIBRARY_PATH=${ROOT_DIR}/cpython-install/lib
${PYTHON_BINARY} ./opcode_tests/add.py
${PYTHON_BINARY} ./opcode_tests/compare.py
${PYTHON_BINARY} ./opcode_tests/load_global.py
${PYTHON_BINARY} ./opcode_tests/hello.py
${PYTHON_BINARY} ./opcode_tests/fib.py

export RAIJIT_TEST_MODE=1
${PYTHON_BINARY} ./opcode_tests/identity.py
${PYTHON_BINARY} ./opcode_tests/use_if.py
${PYTHON_BINARY} ./opcode_tests/unary.py
${PYTHON_BINARY} ./opcode_tests/load_method.py
${PYTHON_BINARY} ./opcode_tests/binary.py
${PYTHON_BINARY} ./opcode_tests/inplace.py
${PYTHON_BINARY} ./opcode_tests/map.py
${PYTHON_BINARY} ./opcode_tests/misc.py
${PYTHON_BINARY} ./opcode_tests/for.py
