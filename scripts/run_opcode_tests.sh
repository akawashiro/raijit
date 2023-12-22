#!/bin/bash -eux

cd $(git rev-parse --show-toplevel)
export PYTHONPATH=./build
python3 ./opcode_tests/identity.py
python3 ./opcode_tests/add.py
python3 ./opcode_tests/compare.py
python3 ./opcode_tests/load_global.py
python3 ./opcode_tests/fib.py
python3 ./opcode_tests/hello.py
python3 ./opcode_tests/use_if.py

export RAIJIT_TEST_MODE=1
# python3 ./opcode_tests/load_method.py
python3 ./opcode_tests/unary.py
python3 ./opcode_tests/opcodes.py
