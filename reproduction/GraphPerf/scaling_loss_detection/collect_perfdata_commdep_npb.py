import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")

import baguatool as bgt

analysis = bgt.Baguatool()

prog = sys.argv[1]
np = sys.argv[2]

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
# analysis.setAnalysisMode("static")
# analysis.setAnalysisMode("dynamic")
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./" + prog + ".C." + np) #, func="conspatch_")
# analysis.setStaticAnalysisMode("binary", "./slice_halo_gpu")
# analysis.setStaticAnalysisMode("src", "./Makefile")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"  
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)
#analysis.setDynamicAnalysisMode("commdep")

#nps = [1, 2, 4, 8]
#for np in nps:
# set commands for execution 
# # set suffix of output file for performance recording
#np = 8
analysis.setExecutionCommand(cmd = "srun -N 6 -n " + np + " ./" + prog + ".C." + np , output_file_suffix = "_np-" + np )

# set 
analysis.setOutputDir(output_dir = "NPB-so-baguatool-data/" + prog.upper())

# START ANALYSIS
# including static and dynamic parts
analysis.startAnalysis()

analysis.setAnalysisMode("dynamic")
analysis.setDynamicAnalysisMode("commdep")

analysis.startAnalysis()