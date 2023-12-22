#!/bin/bash -eux

cd $(git rev-parse --show-toplevel)
ROOT_DIR=$(pwd)
PYTHON_BINARY=${ROOT_DIR}/cpython-install/bin/python3.12

export PYTHONPATH=${ROOT_DIR}/build
export LD_LIBRARY_PATH=${ROOT_DIR}/cpython-install/lib
${PYTHON_BINARY} ./opcode_tests/identity.py
${PYTHON_BINARY} ./opcode_tests/add.py
${PYTHON_BINARY} ./opcode_tests/compare.py
${PYTHON_BINARY} ./opcode_tests/load_global.py
${PYTHON_BINARY} ./opcode_tests/fib.py
${PYTHON_BINARY} ./opcode_tests/hello.py
${PYTHON_BINARY} ./opcode_tests/use_if.py

export RAIJIT_TEST_MODE=1
# python3 ./opcode_tests/load_method.py
${PYTHON_BINARY} ./opcode_tests/unary.py
${PYTHON_BINARY} ./opcode_tests/opcodes.py
