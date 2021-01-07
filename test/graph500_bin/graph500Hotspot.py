import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")

import baguatool as bgt

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
# analysis.setAnalysisMode("static")
# analysis.setAnalysisMode("dynamic")
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./graph500_reference_bfs") #, func="conspatch_")
# analysis.setStaticAnalysisMode("binary", "./slice_halo_gpu")
# analysis.setStaticAnalysisMode("src", "./Makefile")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"  
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=5000000)

#nps = [1, 2, 4, 8]
#for np in nps:
# set commands for execution 
# # set suffix of output file for performance recording
np = 4
analysis.setExecutionCommand(cmd = "mpirun -np " + str(np) + " ./graph500_reference_bfs 22" , output_file_suffix = "_np-" + str(np) )

# set 
analysis.setOutputDir(output_dir = "graph500_reference-baguatool-data/bfs")

# START ANALYSIS
# including static and dynamic parts
# analysis.startAnalysis()

# OFFLINE PERFORMANCE ANALYSIS
# get program structure graph
psg = analysis.getProgramStructureGraph()

# get performance data
#np = 8
perf_data = analysis.getPerformanceData("_np-" + str(np),1)
#perf_data.show()
psg.performanceDataEmbedding(perf_data)

# get communication data
#comm_data = analysis.getCommunicationData()
# analysis.get

# graph contraction
psg.contraction()

psg.show()
