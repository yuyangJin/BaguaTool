#!/bin/bash
BAGUA_DIR=/mnt/home/jinyuyang/MY_PROJECT/BaguaTool
# you can replace pthread_test with pthread_test_1
BIN=pthread_test_1

# Run as:
# sh ./hybrid_collect.sh 

# First phase, static analysis on binary, you should find your path to binary_analyzer
$BAGUA_DIR/build/bin/binary_analyzer $BIN

# Second phase, dynamic analysis on binary, you should find your path to libpthread_sampler.so
LD_PRELOAD=$BAGUA_DIR/build/lib/libpthread_sampler.so ./$BIN 1000000000

