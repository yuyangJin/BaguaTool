import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")

import baguatool as bgt

analysis = bgt.Baguatool()

prog = sys.argv[1]
#np = sys.argv[2]

analysis.setAnalysisMode("static+dynamic")

np = str(32)
# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./" + prog + ".C." + np)
# set 
analysis.setOutputDir(output_dir = "NPB-so-baguatool-data/" + prog.upper())

# OFFLINE PERFORMANCE ANALYSIS
# get program structure graph
psg_32 = analysis.getProgramStructureGraph()

# get performance data
#np = 8
perf_data = analysis.getPerformanceData(dir_suffix = "_np-" + np, nprocs = int(np), dyn_features = ["TOT_CYC"]) #, "LD_INS", "ST_INS", "L1_DCM"])
#perf_data.show()
psg_32.performanceDataEmbedding(perf_data)


# graph contraction
psg_32.contraction()
psg_32.convertToGraph()
psg_32.save(save_file="./" + prog + ".C." + np)
psg_32.show(save_fig="./" + prog + ".C." + np)


np = str(8)

analysis.setStaticAnalysisMode(mode="binary", binary="./" + prog + ".C." + np)
analysis.setOutputDir(output_dir="NPB-so-baguatool-data/" + prog.upper())

psg_8 = analysis.getProgramStructureGraph()

# get performance data
#np = 8
perf_data = analysis.getPerformanceData(dir_suffix = "_np-" + np, nprocs = int(np), dyn_features = ["TOT_CYC"]) #, "LD_INS", "ST_INS", "L1_DCM"])
#perf_data.show()
psg_8.performanceDataEmbedding(perf_data)
# graph contraction
psg_8.contraction()
psg_8.convertToGraph()
psg_8.save(save_file="./" + prog + ".C." + np)
psg_8.show(save_fig="./" + prog + ".C." + np)



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

maxinum_common_subgraph(psg_32, psg_8)

def graph_difference(node, psg_2):
    if psg1_psg2_map.keys().__contains__(node.unique_id):
        node_2_id = psg1_psg2_map[node.unique_id]
        if node.performance_percentage > 0:
            print(node.unique_id, node.performance_percentage, psg_2.nodes[node_2_id][2])
            node.performance_percentage -= psg_2.nodes[node_2_id][2]


psg_32.BFS(psg_32.main_root, graph_difference, psg_8)

psg_32.convertToGraph()
psg_32.save(save_file="./" + prog + ".C.32-" + prog + ".C.8")
#psg_32.show(save_fig="./" + prog + ".C.32-" + prog + ".C.8")

# get communication data
#comm_dep = analysis.getCommDepData(dir_suffix = "_np-" + np, nprocs = int(np))
#psg.commDepEmbedding(comm_dep)


#ppg = analysis.transferToProgramPerformanceGraph(psg, nprocs = int(np))

#ppg.markProblematicNode(prob_threshold = 1.1)

#ppg.show()