import sys
import os
proj_dir = os.environ['BAGUA_DIR']
sys.path.append(proj_dir + r"/src/Baguatool")
import json
import baguatool as bgt

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./miniVite") 

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM", "LST_INS"])

# export OMP_PLACES=sockets
# export OMP_PROC_BIND=true
nps = [2]
for np in nps:
    analysis.setExecutionCommand(cmd="/opt/openmpi-3.0.0/bin/mpiexec -bind-to socket -np " + str(np) + " ./miniVite -l -w -n 100000", output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="miniVite-baguatool-data")


# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg = analysis.getProgramStructureGraph()

nthreads = 12

# get performance data
features = ["TOT_CYC"]#, "TOT_INS"]#, "L2_DCM"]

for np in nps[-1:]:
    perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np), nprocs = np, dyn_features = features, nthreads = nthreads)
    psg.performanceDataEmbedding(perf_data, nthreads)

# graph contraction
psg.contraction(["distLouvainMethod"])

psg.show()


psg.markUnderOmpStart()

ppg = analysis.transferToProgramPerformanceGraph(psg, nprocs=2)

ppg.addThreadParallelism(nthreads)

#ppg.markProblematicNode(prob_threshold = 1.2)

ppg.show()
