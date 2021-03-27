import sys
import json
import os
import copy
import numpy as npy

sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import baguatool as bgt

analysis = bgt.Baguatool()

prog = sys.argv[1]
np = sys.argv[2]

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
# analysis.setAnalysisMode("static")
# analysis.setAnalysisMode("dynamic")
analysis.setAnalysisMode("static")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./experiments/" + prog + ".C." + np) #, func="conspatch_")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"  
# "sampling_freq" is sampling times per second
# analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#nps = [1, 2, 4, 8]
#for np in nps:
# set commands for execution 
# # set suffix of output file for performance recording
#np = 8
#analysis.setExecutionCommand(cmd = "srun -n " + np + " ./" + prog + ".C." + np , output_file_suffix = "_np-" + np )

# set 
analysis.setOutputDir(output_dir = "NPB-baguatool-data/" + prog.upper())

# START ANALYSIS
# including static and dynamic parts
analysis.startAnalysis()

# OFFLINE PERFORMANCE ANALYSIS
# get program structure graph
psg = analysis.getProgramStructureGraph()

# get performance data
#np = 8
#perf_data = analysis.getPerformanceData(dir_suffix = "_np-" + np, nprocs = int(np), dyn_features = ["TOT_CYC", "LD_INS", "ST_INS", "L1_DCM"])
#perf_data.show()
#psg.performanceDataEmbedding(perf_data)

# get communication data
#comm_data = analysis.getCommunicationData()
# analysis.get

# graph contraction
#psg.contraction()

#psg.show()

compressed_data = {}
# read performancedata
for pid in range(int(np)):
    input_file = "./experiments/NPB_trace/C/" + prog + "/" + np + "/MPI" + str(pid) + ".TXT"
    with open(input_file, "r") as f:
        lines = f.readlines()
    
    # intra-process compression
    for i in range(len(lines)):
        line = lines[i]
        line_split = line.strip().split('|')
        mpi_name = line_split[0].strip()
        callpath_line = line_split[1].strip().split(' ')
        callstack = []
        for addr in callpath_line:
            callstack.append([addr])
        info = line_split[2].strip()

        if len(callstack) != 0:
            callstack.pop()
            node = bgt.getNodeWithCallstack(psg.main_root, callstack, psg.roots_of_all_functions)
            if node != None:
                #print(node.unique_id)
                if compressed_data.__contains__(node.unique_id):
                    if compressed_data[node.unique_id].__contains__(pid):
                        if len(compressed_data[node.unique_id][pid]) == 0:
                            compressed_data[node.unique_id][pid].append([info, 1])
                        else:
                            if compressed_data[node.unique_id][pid][-1][0] == info:
                                compressed_data[node.unique_id][pid][-1][1] += 1
                            else:
                                compressed_data[node.unique_id][pid].append([info, 1])
                        # if compressed_data[node.unique_id][pid].__contains__(info):
                        #     compressed_data[node.unique_id][pid][info].append(i)
                        # else:
                        #     compressed_data[node.unique_id][pid][info] = [i]
                    else:
                        compressed_data[node.unique_id][pid] = [[info, 1]]
                else:
                    compressed_data[node.unique_id] = {pid : [[info, 1]]}

output_dir = "./experiments/NPB_trace_cypress/C/" + prog + "/" + np + "/"
os.system("mkdir -p " + output_dir)
output_file = output_dir + "MPI-intra.json"

with open(output_file, "w") as f:
    json.dump(compressed_data,f)

# for unique_id_index in compressed_data.keys():
#     for pid_index in compressed_data[unique_id_index].keys():
#         for info_index in compressed_data[unique_id_index][pid_index].keys():
#             if len(compressed_data[unique_id_index][pid_index][info_index]) :
#                 X = npy.arange(0, len(compressed_data[unique_id_index][pid_index][info_index]), 1)
#                 Y = compressed_data[unique_id_index][pid_index][info_index]
#                 model = npy.polyfit(X, Y, 4)
#                 formulation = npy.poly1d(model)
#                 #err
#                 compressed_data[unique_id_index][pid_index][info_index] = [formulation]

# output_file = output_dir + "MPI-intra2.json"

# with open(output_file, "w") as f:
#     json.dump(compressed_data, f)

def listAdd(l, num):
    if len(l) == 0:
        l.append(num)
        return
    last_num = 0
    if l[-1].find('-') == -1:
        last_num = int(l[-1])
    else:
        last_num = int(l[-1].strip().split('-')[-1])
    if int(num) == last_num + 1:
        tmp_index = l[-1].find('-')
        if tmp_index == -1:
            l[-1] += "-" + num
        else:
            l[-1] = l[-1].strip().split('-')[0] + "-" + num
    

# intra-process compression
for unique_id_index in compressed_data.keys():
    tmp_info_dict = {}
    for pid_index in compressed_data[unique_id_index].keys():
        info_key = ""
        for tmp_info_key in compressed_data[unique_id_index][pid_index]:
            info_key += tmp_info_key[0] + "|" + str(tmp_info_key[1])
        if tmp_info_dict.__contains__(info_key):
            listAdd(tmp_info_dict[info_key], str(pid_index))
        else:
            tmp_info_dict[info_key] = [str(pid_index)]
    compressed_data[unique_id_index] = copy.deepcopy(tmp_info_dict)

output_file = output_dir + "MPI-inter.json"

with open(output_file, "w") as f:
    json.dump(compressed_data, f)