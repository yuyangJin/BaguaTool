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

# # set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
# analysis.setStaticAnalysisFeature(features=["psg"])

# # set dynamic analysis mode as "sampling" / "instrumentation"
# # "sampling_freq" is sampling times per second
# analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

# #analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM"]) #, "LST_INS"])

nps = [32]
# for np in nps:
#     analysis.setExecutionCommand(cmd="srun -n " + str(np) + " ./zeusmp.x", output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="zeusmp-baguatool-data")


# # START ANALYSIS - 
# # including static and dynamic parts
# #analysis.startAnalysis()

# analysis.setAnalysisMode("dynamic")
# analysis.setDynamicAnalysisMode("commdep")

# #analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg = analysis.getProgramStructureGraph()


# get performance data
features = ["TOT_CYC"] #, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

for np in nps[-1:]:
    perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np), nprocs = np, dyn_features = features)
    psg.performanceDataEmbedding(perf_data)

    comm_dep = analysis.getCommDepData(dir_suffix = "_np" + str(np), nprocs = np)
    psg.commDepEmbedding(comm_dep)

# graph contraction
psg.contraction()
psg.convertToGraph()
psg.save(save_file="./zeusmp.32")


#node_file = "./zeusmp.16.nodes.json"
#edge_file = "./zeusmp.16.edges.json"
#print(node_file, edge_file)
#psg = analysis.getProgramStructureGraphFromJsonFile(nodes_file=node_file, edges_file=edge_file)

# np = 16
# comm_dep = analysis.getCommDepData(dir_suffix = "_np" + str(np), nprocs = np)
# psg.commDepEmbedding(comm_dep)

psg.show(save_fig="./zeusmp.32")

ppg = analysis.transferToProgramPerformanceGraph(psg, nprocs=np)


ppg.markProblematicNode(prob_threshold = 1.1)

ppg.show(save_fig="./zeusmp.32")