#!/bin/bash -eux

cd $(git rev-parse --show-toplevel)
export PYTHONPATH=./build
python3 ./app_tests/nbody_test.py
