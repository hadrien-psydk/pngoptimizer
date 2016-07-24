#!/bin/bash
# Build and run all unit tests

# Set current directory to the one containing this script
cd "$(dirname "$0")"

make -C chustd_ut
make -C poeng_ut

make -C chustd_ut run
make -C poeng_ut run
