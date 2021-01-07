#!/bin/bash

mkdir -p output

#for np in "2 4 8 16"
#do 
mpirun -np ${np} ./newbinary ./input/linadvsl-2d.input-np${np}-p8 | tee ./output/np${np}-p8.log
#done 

#for p in "8 16 32 64"
#do
#  echo $p
#  mpirun -np 4 ./main2d ./input/linadvsl-2d.input-np4-p($p) | tee ./output/np4-p{$p}.log
#done

newbinary='newbinary_conspatch_time'
mpirun -np 2 ./$newbinary ./input/linadvsl-2d.input-np2-p4 | tee ./output/np2-p4.log
mpirun -np 2 ./$newbinary ./input/linadvsl-2d.input-np2-p8 | tee ./output/np2-p8.log
mpirun -np 2 ./$newbinary ./input/linadvsl-2d.input-np2-p16 | tee ./output/np2-p16.log
mpirun -np 2 ./$newbinary ./input/linadvsl-2d.input-np2-p32 | tee ./output/np2-p32.log
mpirun -np 2 ./$newbinary ./input/linadvsl-2d.input-np2-p64 | tee ./output/np2-p64.log
