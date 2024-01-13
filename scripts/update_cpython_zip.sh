#!/bin/bash -eux

cd $(dirname $0)/..

if [[ ! -d cpython-install ]]; then
    echo "cpython-install directory not found."
    exit 1
fi

rm -rf cpython-install.zip
zip -r cpython-install.zip cpython-install
