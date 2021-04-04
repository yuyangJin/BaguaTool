#!/bin/bash
prog=$1
./static_analysis ${prog}
./preprocess ${prog}.pag/

dot -Tpdf -o root.pdf ./root.dot

#

LD_PRELOAD=./libsampler_test.so ${prog}
