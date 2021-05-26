import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import os
import json
import baguatool as bgt

analysis = bgt.Baguatool()

# # SETUP
# # set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")
# #analysis.setAnalysisMode("dynamic")

# # set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./zeusmp.x") 

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM"]) #, "LST_INS"])

nps = [16]
for np in nps:
    analysis.setExecutionCommand(cmd="srun -N 4 -n " + str(np) + " ./zeusmp.x", output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="zeusmp-baguatool-data")


# START ANALYSIS - 
# including static and dynamic parts
analysis.startAnalysis()

analysis.setAnalysisMode("dynamic")
analysis.setDynamicAnalysisMode("commdep")

analysis.startAnalysis()