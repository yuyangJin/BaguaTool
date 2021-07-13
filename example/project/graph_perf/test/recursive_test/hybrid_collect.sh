#!/bin/bash
GPERF_DIR=/home/pairshoe/BaguaTool/build/example/project/graph_perf
BIN=recursive_test_1

# Run as:
# sh ./hybrid_collect.sh 

# First phase, static analysis on binary, you should find your path to binary_analyzer
$GPERF_DIR/binary_analyzer $BIN

# Second phase, dynamic analysis on binary, you should find your path to libpthread_sampler.so
LD_PRELOAD=$GPERF_DIR/libpthread_sampler.so ./$BIN 10