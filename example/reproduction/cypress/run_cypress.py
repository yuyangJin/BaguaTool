#!/usr/bin/env python

from __future__ import print_function
import os

npb_path = '/mnt/home/jinyuyang/BaguaTool/reproduction/cypress/experiments/'
out_path = '/mnt/home/jinyuyang/BaguaTool/reproduction/cypress/experiments/NPB_trace/'

#bms = ['bt', 'cg', 'ep', 'ft', 'mg', 'sp', 'lu', 'is'] #, 'dt']
bms = ['cg']
np1 = [8, 32]#, 16, 32, 64]#, 128, 256] #, 512, 1024, 2048, 4096]
np2 = [9, 36]#, 16, 36, 64]#, 121, 256] #, 529, 1024, 2025, 4096]
scale1 = ['C']#'A','B','C','D']
scale2 = ['C']#'A','B','C','D']

work_path = out_path + '/%s/%s/%d'
copy_bc_cmd = 'cp /mnt/home/jinyuyang/BaguaTool/reproduction/cypress/experiments/%s ' + work_path
sub_cmd = 'LD_PRELOAD=/mnt/home/jinyuyang/BaguaTool/reproduction/cypress/mpi_tracer/mpi_tracer.so srun -n {1} ./{0}'
cypress_cmd = 'python3.5 ./npb_cypress.py {0} {1}'
compare_cmd = 'du -h '+ work_path + ' && du -h /mnt/home/jinyuyang/BaguaTool/reproduction/cypress/experiments/NPB_trace_cypress/%s/%s/%d/*'

for bm in bms:
    if bm in ('bt', 'sp'):
        nps = np2
    else:
        nps = np1
    if bm in ('is', 'dt'):
        mpi_comp = 'mpicxx'
    else:
        mpi_comp = 'mpif90'
    if bm in ('is'):
	scales = scale2
    else:
	scales = scale1
    for scale in scales:
	for np in nps:
        	print('\n==========' + bm + ' ' + scale + ' ' + str(np) + '==========')

        	name = bm + '.' + scale + '.' + str(np)

                print('cypress run')
                os.system(cypress_cmd.format(bm, np))
                
                print("intra compress comparison")
                os.system(compare_cmd % (scale, bm, np, scale, bm, np))
