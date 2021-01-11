from node import *
from graphvizloc import *
import sys

pnode = None
unique_id = 0

class PPG(object):
    def __init__(self, psg, nprocs):
        global pnode
        global unique_id
        self.psg = psg
        self.ppg_file = psg.psg_file[:-2] + "p" + psg.psg_file[-1]
        psg_root = psg.main_root
        self.main_root = PNode(unique_id, psg_root, psg_root.type, 0)
        unique_id += 1
        
        self.psg_node_to_ppg_node_map = dict()
        pnode = self.main_root
        self.buildPPG(psg_root, 0)

        self.num_node_per_column = unique_id - 1
        #print(self.num_node_per_column)
        for pid in range(1, nprocs):
            pnode = self.main_root
            self.buildPPG(psg_root, pid)
        
        self.comm_dep_edges = []
        
        self.addInterProcessCommDepEdge()
        #print(self.comm_dep_edges)

        self.problematic_nodes = []

    def buildPPG(self, node, pid):
        global pnode
        global unique_id

        if node != self.psg.main_root:
            new_pnode = PNode(unique_id, node, node.type, pid)
            unique_id += 1
            pnode.children.append(new_pnode)
            pnode = new_pnode

        if pid == 0:
            self.psg_node_to_ppg_node_map[node] = pnode

        if node.type_name == "LOOP":
            loop_start_pnode = pnode

        for child in node.children:
            self.buildPPG(child, pid)

        # if node.type_name == "LOOP":
        #     loop_end_pnode = PNode(unique_id, node, -10, pid)
        #     unique_id += 1
        #     loop_end_pnode.performance_percentage = 0
        #     loop_end_pnode.loop_pair_node = loop_start_pnode
        #     loop_start_pnode.loop_pair_node = loop_end_pnode
        #     pnode.children.append(loop_end_pnode)
        #     pnode = loop_end_pnode
        
    def transferPSGCommDepToPPGCommDep(self, pnode):
        if pnode.type_name == "MPI" and len(pnode.comm_dep) > 0:
            for i in range(len(pnode.comm_dep)):
                
                comm_dep = pnode.comm_dep[i]
                #print(comm_dep)
                src_pnode = self.psg_node_to_ppg_node_map[comm_dep[0][0]]
                dest_pnode = self.psg_node_to_ppg_node_map[comm_dep[1][0]]
                if pnode == dest_pnode:
                    src_pid = comm_dep[0][1]
                    dest_pid = comm_dep[1][1]
                    edge_weight = comm_dep[2]
                    if edge_weight > 0 :
                        src_pnode_uid = self.num_node_per_column * src_pid + src_pnode.unique_id
                        dest_pnode_uid = self.num_node_per_column * dest_pid + dest_pnode.unique_id
                        self.comm_dep_edges.append([src_pnode_uid, dest_pnode_uid, edge_weight])

    def addCommDepInList(self, pnode):
        if pnode.type_name == "MPI":
            self.comm_dep = []
            for i in range(len(self.comm_dep_edges)):
                comm_dep = self.comm_dep_edges[i]
                # pnode is dest node of comm_dep
                if pnode.unique_id == comm_dep[1]:
                    self.comm_dep.append(comm_dep)

    def addInterProcessCommDepEdge(self):
        # add comm_dep to list
        self.BFS(self.main_root.children[0], self.transferPSGCommDepToPPGCommDep)
        #print(self.comm_dep_edges)
        # add comm_dep to each ppg node
        self.BFS(self.main_root, self.addCommDepInList)


    def BFS(self, node, func, *args, **kwargs):
        sys.setrecursionlimit(100000)
        std_flag = not node.bfs_flag
        self.doBFS(std_flag, node, func, *args, **kwargs)
    
    def doBFS(self, std_flag, node, func, *args, **kwargs):
        if node.bfs_flag == std_flag:
            return
        node.bfs_flag = not node.bfs_flag
        
        func(node, *args, **kwargs)

        for child in node.children:
            self.doBFS(std_flag, child, func, *args, **kwargs)

    def listProblematicNode(self, pnode, prob_threshold):
        #print(pnode.unique_id)
        if pnode.type_name != "LOOP_END" and len(pnode.all_procs_perf_data) > 1 and pnode.performance_percentage > 0.001:
            perf_data = pnode.all_procs_perf_data
            avg_perf = sum(perf_data) / len(perf_data)
            for pid in range(len(perf_data)):
                if avg_perf == 0:
                    if perf_data[pid] > 0:
                        self.problematic_nodes.append(self.num_node_per_column * pid + pnode.unique_id)
                else:
                    if perf_data[pid] / avg_perf > prob_threshold:
                        self.problematic_nodes.append(self.num_node_per_column * pid + pnode.unique_id)
                        #print(self.num_node_per_column * pid + unique_id)
                
    def markProblematicNodeInList(self, pnode):
        if pnode.unique_id in self.problematic_nodes:
            pnode.group_flag = True
            #print("set " + str(pnode.unique_id) + "'s group_flag as True")

    def markProblematicNode(self, prob_threshold = 100):
        # traverse PSG to list problematic node 
        self.BFS(self.main_root.children[0], self.listProblematicNode, prob_threshold)

        # mark group_flag as true for problrmatic node in the list
        self.BFS(self.main_root, self.markProblematicNodeInList)

    def show(self, save_fig = ""):
        #printGraph(self.main_root,0)
        if save_fig == "":
            output = GraphvizOutput(self.ppg_file, self.main_root, edge_list=self.comm_dep_edges)
        else:    
            output = GraphvizOutput(self.ppg_file, self.main_root, edge_list=self.comm_dep_edges, output_file = save_fig + ".ppg")
        output.done()