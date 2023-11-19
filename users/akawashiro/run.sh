#!/bin/bash -eux

if [ ! -d build ]
then
    cmake -G Ninja -S . -B build -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON
fi
cmake --build build
export PYTHONPATH=./build
export RAIJIT_TEST_MODE=1
# gdb --ex run --args python3 use_jit.py
python3 use_jit.py
