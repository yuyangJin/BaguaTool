import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import os
import json
import baguatool as bgt

np_1 = sys.argv[1]
np_2 = sys.argv[2]

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")

np = np_1
# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./zeusmp.x") 

# set
analysis.setOutputDir(output_dir="zeusmp-baguatool-data")

# # OFFLINE PERFORMANCE ANALYSIS
# # get program structure graph
psg_1 = analysis.getProgramStructureGraph()


# get performance data
features = ["TOT_CYC"] #, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

perf_data = analysis.getPerformanceData( dir_suffix = "_np" + np, nprocs = int(np), dyn_features = features)
psg_1.performanceDataEmbedding(perf_data)

comm_dep = analysis.getCommDepData(dir_suffix = "_np" + np, nprocs = int(np))
psg_1.commDepEmbedding(comm_dep)

# graph contraction
psg_1.contraction()
psg_1.convertToGraph()
psg_1.save(save_file="./zeusmp_" + np)
#psg_1.show(save_fig="./zeusmp_" + np)

# set
np = np_2
analysis.setOutputDir(output_dir="zeusmp-baguatool-data")

psg_2 = analysis.getProgramStructureGraph()

# get performance data
features = ["TOT_CYC"] #, "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

perf_data = analysis.getPerformanceData( dir_suffix = "_np" + np, nprocs = int(np), dyn_features = features)
psg_2.performanceDataEmbedding(perf_data)

comm_dep = analysis.getCommDepData(dir_suffix = "_np" + np, nprocs = int(np))
psg_2.commDepEmbedding(comm_dep)

# graph contraction
psg_2.contraction()

psg_2.convertToGraph()
psg_2.save(save_file="./zeusmp_"+np)
#psg_2.show(save_fig="./zeusmp_"+np)


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

maxinum_common_subgraph(psg_1, psg_2)

def graph_difference(node, psg_2):
    if psg1_psg2_map.keys().__contains__(node.unique_id):
        node_2_id = psg1_psg2_map[node.unique_id]
        if node.performance_percentage > 0:
            print(node.unique_id, node.performance_percentage, psg_2.nodes[node_2_id][2])
            avg_tot_sampling_count = sum(psg_2.total_sampling_count) / len(psg_2.total_sampling_count)
            avg_sampling_count = psg_2.nodes[node_2_id][2] * avg_tot_sampling_count
            for i in range(len(node.performance_data["TOT_CYC"])):
                node.performance_data["TOT_CYC"][i] -= avg_sampling_count
            node.performance_percentage -= psg_2.nodes[node_2_id][2]


psg_1.BFS(psg_1.main_root, graph_difference, psg_2)

psg_1.convertToGraph()
psg_1.save(save_file="./zeusmp."+np_1 +"-zeusmp."+np_2)
psg_1.show(save_fig="./zeusmp."+np_1 +"-zeusmp."+np_2)

np = np_1

# get communication data
ppg_3 = analysis.transferToProgramPerformanceGraph(psg_1, nprocs = int(np))
ppg_3.markProblematicNode(prob_threshold = 0.01)
ppg_3.show(save_fig="./zeusmp."+np_1 +"-zeusmp."+np_2)
