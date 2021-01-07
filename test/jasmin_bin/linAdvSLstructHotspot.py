import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import os
import json
import baguatool as bgt

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
# analysis.setAnalysisMode("static")
# analysis.setAnalysisMode("dynamic")
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./linadvsl-main2d")  # , func="conspatch_")
# analysis.setStaticAnalysisMode("src", "./Makefile")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM", "LST_INS"])

nps = [64]
domain_boxes = [2048, 4096, 8192, 16384]
patch_sizes = [16, 32, 64, 128, 256, 512, 1024]
for np in nps:
    for domain_box in domain_boxes:
        for patch_size in patch_sizes:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --host gorgon3:22,gorgon4:22,gorgon5:20 ./linadvsl-main2d ./linadvsl_input/linadvsl-2d.input-p" + str(patch_size) + "-d" + str(domain_box) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size), save_file="LinAdvSL-linadv_2d.log")

domain_boxes = [4000, 6000, 8000, 10000, 12000]
patch_sizes = [40, 50, 100, 200, 400, 500]
for np in nps:
    for domain_box in domain_boxes:
        for patch_size in patch_sizes:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --host gorgon3:22,gorgon4:22,gorgon5:20 ./linadvsl-main2d ./linadvsl_input/linadvsl-2d.input-p" + str(patch_size) + "-d" + str(domain_box) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size), save_file="LinAdvSL-linadv_2d.log")

# set
analysis.setOutputDir(output_dir="linadvsl-baguatool-data")


# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg = analysis.getProgramStructureGraph()

#features = ["TOT_CYC", "LD_INS", "ST_INS", "L1_DCM"]
features = ["TOT_CYC"]#, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

# get performance data

for np in nps[-1:]:
    for domain_box in domain_boxes[-1:]:
        for patch_size in patch_sizes[-1:]:
            perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size), nprocs = np, dyn_features = features)
            # perf_data.show()
            psg.performanceDataEmbedding(perf_data)

# graph contraction
func_list = ["NumericalIntegratorComponent", "SweepingIntegratorComponent", "IntegratorComponent", "CopyIntegratorComponent", "InitializeIntegratorComponent", "DtIntegratorComponent" ]
psg.contraction(preserved_func_list = func_list)
#psg.contraction()


psg.show()


# loop_stack = []
# formulation = "T-linadvsl = "

# def formulationGen2(node):
#     global loop_stack
#     global formulation

#     if len(node.children) == 0:
#         formulation += "T-" + node.name + " + "

#     if node.type_name == "LOOP":
#         formulation += "N-" + node.name + " * ("
#         loop_stack.append(node.name)

#     for child in node.children:
#         formulationGen2(child)
    
#     if node.type_name == "LOOP":
#         formulation += ") + "
#         loop_stack.pop()

# def ff(str,num):
#     return str[:num] + str[num+1:]

# def standFormulation():
#     global formulation
#     index = 0
#     while index != -1:
#         index = formulation.find("+ ", index)
#         if index != -1:
#             if index == len(formulation) - 2 or formulation[index + 2] == ')':
#                 formulation = ff(formulation, index)
#                 formulation = ff(formulation, index)
#                 #print(formulation)
#             index += 1

# formulationGen2(psg.main_root)
# standFormulation()
# print(formulation)