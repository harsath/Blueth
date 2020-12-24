#!/bin/bash
set -e
set -u
COMPILER_OPTIONS="-Wall -O2 -g"
CC=gcc-9 && CXX=g++-9 cmake -D CMAKE_CXX_FLAGS="${COMPILER_OPTIONS}" .. && make && ./run_test.sh
