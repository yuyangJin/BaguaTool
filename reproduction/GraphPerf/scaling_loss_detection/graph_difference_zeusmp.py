import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import os
import json
import baguatool as bgt

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./zeusmp.x") 

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
#analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
#analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

#analysis.setDynamicSamplingFeature(features=["TOT_CYC", "L1_DCM", "L2_DCM", "L3_DCM", "LST_INS"])

nps = [128]
#for np in nps:
#    analysis.setExecutionCommand(cmd="srun -n " + str(np) + " ./zeusmp.x", output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="zeusmp-np128-baguatool-data")

# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg_128 = analysis.getProgramStructureGraph()


# get performance data
features = ["TOT_CYC"] #, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

for np in nps[-1:]:
    perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np), nprocs = np, dyn_features = features)
    psg_128.performanceDataEmbedding(perf_data)

# graph contraction
psg_128.contraction()

psg_128.convertToGraph()
psg_128.save(save_file="./zeusmp_128")
#psg_128.show(save_fig="./zeusmp_128")

nps = [32]
#for np in nps:
#    analysis.setExecutionCommand(cmd="srun -n " + str(np) + " ./zeusmp.x", output_file_suffix="_np" + str(np) )

# set
analysis.setOutputDir(output_dir="zeusmp-np32-baguatool-data")

# START ANALYSIS
# including static and dynamic parts
#analysis.startAnalysis()

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg_32 = analysis.getProgramStructureGraph()


# get performance data
features = ["TOT_CYC"] #, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

for np in nps[-1:]:
    perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np), nprocs = np, dyn_features = features)
    psg_32.performanceDataEmbedding(perf_data)

# graph contraction
psg_32.contraction()


psg_32.convertToGraph()
psg_32.save(save_file="./zeusmp_32")
#psg_32.show(save_fig="./zeusmp_32")


visited = [] # List to keep track of visited nodes.
queue = []  #Initialize a queue
#node_pointer_psg_1 = 0 # Initialize a node pointer to psg32's root node
#node_pointer_psg_2 = 0 # Initialize a node pointer to psg128's root node

psg1_psg2_map = {}
psg1_psg2_map[0] = 0
#nodes = []
#edges = []
# G0 = G1 - G2
def maxinum_common_subgraph(psg_1, psg_2):
    global visited, queue, psg1_psg2_map
    visited.append(0)
    queue.append(0)
    nodes_1 = psg_1.nodes
    nodes_2 = psg_2.nodes
    edges_1 = psg_1.edges
    edges_2 = psg_2.edges

    while queue:
        src_node_1 = queue.pop(0)
        #print (s, end = " ") 
        if psg1_psg2_map.keys().__contains__(src_node_1):
            src_node_2 = psg1_psg2_map[src_node_1]
            if edges_1.keys().__contains__(src_node_1):
                for dest_node_1 in edges_1[src_node_1]:
                    if edges_2.keys().__contains__(src_node_2):
                        dest_node_1_entry_addr = nodes_1[dest_node_1][-2]
                        dest_node_1_exit_addr = nodes_1[dest_node_1][-1]
                        
                        for dest_node_2 in edges_2[src_node_2]:
                            if dest_node_1_entry_addr == nodes_2[dest_node_2][-2] and dest_node_1_exit_addr == nodes_2[dest_node_2][-1]:
                                psg1_psg2_map[dest_node_1] = dest_node_2
                                continue

        if edges_1.keys().__contains__(src_node_1):
            for dest_node_1 in edges_1[src_node_1]:
                if dest_node_1 not in visited:
                    visited.append(dest_node_1)
                    queue.append(dest_node_1)
    
    #print(psg1_psg2_map)

maxinum_common_subgraph(psg_128, psg_32)

def graph_difference(node, psg_2):
    if psg1_psg2_map.keys().__contains__(node.unique_id):
        node_2_id = psg1_psg2_map[node.unique_id]
        if node.performance_percentage > 0:
            print(node.unique_id, node.performance_percentage, psg_2.nodes[node_2_id][2])
            node.performance_percentage -= psg_2.nodes[node_2_id][2]


psg_128.BFS(psg_128.main_root, graph_difference, psg_32)

psg_128.convertToGraph()
psg_128.save(save_file="./zeusmp.32-zeusmp.8")
psg_128.show(save_fig="./zeusmp.32-zeusmp.8")

#ppg = analysis.transferToProgramPerformanceGraph(psg, nprocs=np)
#ppg.markProblematicNode(prob_threshold = 1.2)
#ppg.show()
