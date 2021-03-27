import sys
import os
proj_dir = os.environ['BAGUA_DIR']
sys.path.append(proj_dir + r"/src/Baguatool")
import json
import baguatool as bgt

# set npb executable file
npb_exec = 'cg.C.x'

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./" + npb_exec) 

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)
#analysis.setDynamicAnalysisMode("commdep", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM", "LST_INS"])


nps = [16]
for np in nps:
    analysis.setExecutionCommand(cmd="srun -n" + str(np) + " " + npb_exec, output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="NPB-" + npb_exec + "-baguatool-data")


# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg = analysis.getProgramStructureGraph()

# get performance data
features = ["TOT_CYC"]#, "TOT_INS"]#, "L2_DCM"]

for np in nps[-1:]:
    perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np), nprocs = np, dyn_features = features)
    psg.performanceDataEmbedding(perf_data)

# graph contraction
psg.contraction()

psg.nameSimplifying()

psg.show()

#feat = psg.get_common_features()
##print(feat)
#
#
ppg = analysis.transferToProgramPerformanceGraph(psg, nprocs=nps[0])

ppg.markProblematicNode(prob_threshold = 1.2)
#
ppg.show()