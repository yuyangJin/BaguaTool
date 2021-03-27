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
analysis.setStaticAnalysisMode(mode="binary", binary="./rotadv-main2d")  # , func="conspatch_")
# analysis.setStaticAnalysisMode("src", "./Makefile")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM", "LST_INS"])

nps = [32]
#domain_boxes = [128, 256, 512, 1024]
patch_size_xs = [4, 16, 64, 128, 512]
patch_size_ys = [4, 16, 64, 128, 512]
#domain_boxes = [1024]
#patch_sizes = [128]
for np in nps:
    for patch_size_x in patch_size_xs:
        for patch_size_y in patch_size_ys:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --hosts gorgon3:16,gorgon4:16 ./rotadv-main2d ./rotadv_input/rotation-2d.input-px" + str(patch_size_x) + "-py" + str(patch_size_y) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-psx" + str(patch_size_x) + "-pxy" + str(patch_size_y), save_file = "RotAdv-rotation_2d.log")

patch_size_xs = [8, 32, 256, 1024]
patch_size_ys = [8, 32, 256, 1024]
#domain_boxes = [1024]
#patch_sizes = [128]
for np in nps:
    for patch_size_x in patch_size_xs:
        for patch_size_y in patch_size_ys:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --hosts gorgon3:16,gorgon4:16 ./rotadv-main2d ./rotadv_input/rotation-2d.input-px" + str(patch_size_x) + "-py" + str(patch_size_y) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-psx" + str(patch_size_x) + "-pxy" + str(patch_size_y), save_file = "RotAdv-rotation_2d.log")

patch_size_xs = [4, 16, 64, 128, 512]
patch_size_ys = [8, 32, 256, 1024]
#domain_boxes = [1024]
#patch_sizes = [128]
for np in nps:
    for patch_size_x in patch_size_xs:
        for patch_size_y in patch_size_ys:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --hosts gorgon3:16,gorgon4:16 ./rotadv-main2d ./rotadv_input/rotation-2d.input-px" + str(patch_size_x) + "-py" + str(patch_size_y) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-psx" + str(patch_size_x) + "-pxy" + str(patch_size_y), save_file = "RotAdv-rotation_2d.log")

patch_size_xs = [8, 32, 256, 1024]
patch_size_ys = [4, 16, 64, 128, 512]
# patch_size_xs = [8]
# patch_size_ys = [512]
for np in nps:
    for patch_size_x in patch_size_xs:
        for patch_size_y in patch_size_ys:
            # set commands for execution
            # # set suffix of output file for performance recording
            analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " --hosts gorgon3:16,gorgon4:16 ./rotadv-main2d ./rotadv_input/rotation-2d.input-px" + str(patch_size_x) + "-py" + str(patch_size_y) + "-n" + str(np), output_file_suffix="_np" + str(np) + "-psx" + str(patch_size_x) + "-pxy" + str(patch_size_y), save_file = "RotAdv-rotation_2d.log")



# set
analysis.setOutputDir(output_dir="rotadv-baguatool-data")

# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg = analysis.getProgramStructureGraph()

#features = ["TOT_CYC", "LD_INS", "ST_INS", "L1_DCM"]
features = ["TOT_CYC"]#, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

# get performance data

patch_size_xs = [4, 16, 64, 128, 512]
patch_size_ys = [4, 16, 64, 128, 512]

for np in nps[-1:]:
    for patch_size_x in patch_size_xs[-1:]:
        for patch_size_y in patch_size_ys[-1:]:
            perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np) + "-psx" + str(patch_size_x) + "-pxy" + str(patch_size_y), nprocs = 1, dyn_features = features)
            # perf_data.show()
            psg.performanceDataEmbedding(perf_data)

# graph contraction
#func_list = ["NumericalIntegratorComponent", "SweepingIntegratorComponent", "IntegratorComponent", "CopyIntegratorComponent", "InitializeIntegratorComponent", "DtIntegratorComponent" ]
#psg.contraction(preserved_func_list=func_list)
psg.contraction()

psg.show()
