#!/bin/bash -eux

cd $(dirname $0)/..
docker build . --network=host -f ./ci/ubuntu-Dockerfile
