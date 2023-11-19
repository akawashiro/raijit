#!/bin/bash -eux

if [ ! -d build ]
then
    cmake -G Ninja -S . -B build -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON
fi
cmake --build build
export PYTHONPATH=./build
python3 ./raytrace.py
