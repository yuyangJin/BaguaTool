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
analysis.setDynamicAnalysisMode("commdep")

#nps = [1, 2, 4, 8]
#for np in nps:
# set commands for execution 
# # set suffix of output file for performance recording
#np = 8
analysis.setExecutionCommand(cmd = "srun -n " + np + " ./" + prog + ".C." + np , output_file_suffix = "_np-" + np )

# set 
analysis.setOutputDir(output_dir = "NPB-so-baguatool-data/" + prog.upper())

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

# get communication dependence
comm_dep = analysis.getCommDepData(dir_suffix = "_np-" + np, nprocs = int(np))
psg.commDepEmbedding(comm_dep)

# graph contraction
#psg.contraction()

psg.show()
