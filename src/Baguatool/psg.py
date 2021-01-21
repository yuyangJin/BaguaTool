from node import *
import copy
import collections
import json
import random
import sys
from functools import reduce
import os
from graphvizloc import *

uniqId = 0
percentage_sum = 0.0
#recursive_flag = False
recursive_function_stack = collections.OrderedDict()
queried_so_addr_map = collections.OrderedDict()
queried_so_addr_func_map = collections.OrderedDict()
#all_perf_data_config = {}

class PSG(object):
    # Initilize elements of class PSG
    def __init__(self, psg_file = "", nodes = {}, edges = {}):
        #print("psg init")
        self.x = 1
        self.y = 1
        self.psg_file = psg_file
        self.main_root = None
        self.funcs_num = 0
        self.roots_of_all_functions = []
        self.total_sampling_count = []
        self.total_comm_time = 0
        #self.perf_data_config = {}
        #self.perf_data_config_file = ""
        #self.all_perf_data_config = {}
        self.binary_name = ""

        self.ldmap_flag = False

        self.nodes = nodes
        self.edges = edges

        sys.setrecursionlimit(100000)
        if psg_file != "":
            self.readPSG()
            self.BFS(self.main_root, sortPSG)
        else:
            self.main_root = generatePSGWithNodesEdges(str(0), self.edges, self.nodes)
        
    def has_cycles(self):
        global revisited
        revisited = False
        def check_revisit(node):
            global revisited
            if not revisited:
                for child in node.children:
                    revisited |= child.bfs_flag == node.bfs_flag
        self.BFS(self.main_root, check_revisit)
        return revisited

    # read Program Structure Graph from psg_file
    def readPSG(self):
        with open(self.psg_file) as f:
            self.funcs_num = int(f.readline().strip())
            for funcs_ in range(self.funcs_num):
                root = readGraph(f)
                self.roots_of_all_functions.append(root)
        self.main_root = self.roots_of_all_functions[0]
        # print(self.psg_file)

    def BFS(self, node, func, *args, **kwargs):
        std_flag = not node.bfs_flag
        self.doBFS(std_flag, node, func, *args, **kwargs)
    
    def doBFS(self, std_flag, node, func, *args, **kwargs):
        if node.bfs_flag == std_flag:
            return
        node.bfs_flag = not node.bfs_flag
        
        func(node, *args, **kwargs)


        for child in node.children:
            self.doBFS(std_flag, child, func, *args, **kwargs)
        
    def deleteSharedObjectLibLeaf(self, node):
        has_user_defined_func_addr_flag = False
        for child in node.children:
            flag = self.deleteSharedObjectLibLeaf(child)
            has_user_defined_func_addr_flag |= flag
            #merge performance data
            print(child.unique_id, child.performance_percentage)
            print(node.unique_id, node.performance_percentage)
            if flag == False:
                node.performance_percentage += child.performance_percentage
                # for k, v in child.performance_data.items():
                #     if len(node.performance_data[k]) == 1:
                #         node.performance_data[k][0] += v[0]
                #         for i in range(1, len(v)):
                #             node.performance_data[k].append(v[i])
                #     for i in range(len(v)):
                #         node.performance_data[k][i] += v[i]
            print(node.unique_id, node.performance_percentage)
        
        if int(node.entry_addr, 16) < int('4f0000000000', 16) or has_user_defined_func_addr_flag == True:
            #has_user_defined_func_addr_flag = True
            #node_perf_data = {}
            return True
        
        if has_user_defined_func_addr_flag == False:
            node.removed = True
            #print(node.unique_id, node.performance_data)
            return has_user_defined_func_addr_flag
        
        return has_user_defined_func_addr_flag
            
    
    def nameSimplifying(self):
        # 去掉<...>之间的内容
        def simplify(node):
            start = node.name.find('<')
            end = node.name.rfind('>')
            if start == -1 or end == -1: return
            new_name = node.name[:start + 1] + '...' + node.name[end:]
            node.name = new_name
        
        self.BFS(self.main_root, simplify)

    # Embed performance data to Program Structure Graph
    def performanceDataEmbedding(self, perf_data, nthreads=1):
        self.binary_name = perf_data.binary_name
        global percentage_sum
        #global all_perf_data_config

        if os.path.exists(perf_data.perf_data_dir + "LDMAP" + str(0) + ".TXT"):
            self.ldmap_flag = True        

        self.BFS(self.main_root, clearPerfData)
        #print(perf_data.data)
        for j in range(len(perf_data.data)):
            #clear sampling count and appended_feature_data_to_perf_data flag
            
            # This loop is for traversing all processes in performance data
            for i in range(len(perf_data.data[j])):
                total_sampling_count = 0
                real_total_sampling_count = 0
                # This loop is for traversing all [call stack, sampling count] lines of one process's performance data
                #for callstack_line in perf_data.data[j][i]:
                for k, callstack_line in enumerate(perf_data.data[j][i]):
                    callstack = callstack_line[0]
                    cur_sampling_count = int(callstack_line[1])
                    #if cur_sampling_count <= 1:
                    #    continue
                    real_total_sampling_count += cur_sampling_count
                    if len(callstack) != 0:
                        callstack.pop()
                        if len(callstack) != 0:
                            if self.ldmap_flag == True:
                                callstack.pop()
                            # Invoke "getNodeWithCallstack" to embed one [call stack, sampling count] line to corresponding vertex
                            node = getNodeWithCallstack(self.main_root, callstack, self.roots_of_all_functions)
                            #print(node.name, cur_sampling_count)
                            if node != None:
                                #print(node.name, cur_sampling_count)
                                if len(node.sampling_count) != len(perf_data.data[j]):
                                    node.sampling_count = [0 for i in range(len(perf_data.data[j]))]
                                    #print(node.sampling_count)
                                #print(node.sampling_count)
                                node.sampling_count[i] += cur_sampling_count

                                # thread level
                                if j == 0 and nthreads > 1:
                                    callpath_thread_id = perf_data.thread_data[j][i][k][2]
                                    if len(node.thread_sampling_count) != len(perf_data.data[j]) or len(node.thread_sampling_count[0]) != nthreads + 1:
                                        node.thread_sampling_count = [[0] * (nthreads + 1)] * len(perf_data.data[j])
                                    node.thread_sampling_count[i][callpath_thread_id] += cur_sampling_count
                            #else:
                                #print(callstack, cur_sampling_count, "not found")
                            # Record total sampling count
                            total_sampling_count += cur_sampling_count
                # only count for TOT_CYC
                if j == 0:
                    self.total_sampling_count.append(total_sampling_count)
                    #print(total_sampling_count, real_total_sampling_count)
                    

            
            self.BFS(self.main_root, appendFeatureDataToPerfData, perf_data.dyn_features[j])
            if j == 0:
                #print(self.total_sampling_count)
                self.BFS(self.main_root, updatePercentageOfNode, self.total_sampling_count)
            #appendFeatureDataToPerfData(self.main_root, perf_data.dyn_features[j], self.main_root.appended_feature_data_to_perf_data)
            self.BFS(self.main_root, clearSamplingCount)
        
        self.BFS(self.main_root, appendPerfDataToAllData, perf_data.dir_suffix)
        for i in range(len(perf_data.data[0])):
            if self.ldmap_flag == True:
                self.BFS(self.main_root, updateNameofAddrNode, perf_data.ldmap[i])
            else:
                #print("here")
                self.BFS(self.main_root, updateNameofAddrNodeNonLib, self.binary_name)
        self.BFS(self.main_root, sortPSG)
        #appendPerfDataToAllData(self.main_root, perf_data.dir_suffix, self.main_root.appended_feature_data_to_perf_data)
        

        
        #clearPerfData(self.main_root, self.main_root.appended_feature_data_to_perf_data)
        
        # self.perf_data_config_file = perf_data.dynamic_data_dir + "./tmp_perf_data.json"
        
        # if os.path.exists(self.perf_data_config_file):
        #     with open(self.perf_data_config_file, "r") as f:
        #         all_perf_data_config = json.load(f)

        # self.convertNodePerfDataToJson(self.main_root, self.main_root.converted_perfdata_to_json)
        # all_perf_data_config[perf_data.dir_suffix] = self.perf_data_config

        # with open(self.perf_data_config_file, "w") as f:
        #     json.dump(all_perf_data_config, f)

        # print(all_perf_data_config.keys())

        print("data embed")

    # def commDepEmbedding(self, comm_dep):
    #     # This loop is for traversing all processes in performance data
    #     for i in range(len(comm_dep.data)):
    #         total_comm_time = 0.0
    #         #real_total_comm_time = 0
    #         # This loop is for traversing all [call stack, sampling count] lines of one process's performance data
    #         for callstack_line in comm_dep.data[i]:
    #             callstack = callstack_line[0]
    #             #cur_comm_count = int(callstack_line[2])
    #             cur_comm_time = float(callstack_line[3]) * 1e-3
    #             #if cur_comm_count <= 1:
    #             #    continue
    #             #real_total_comm_time += cur_comm_count
    #             if len(callstack) != 0:
    #                 callstack.pop()
    #                 # Invoke "getNodeWithCallstack" to embed one [call stack, sampling count] line to corresponding vertex
    #                 node = getNodeWithCallstack(self.main_root, callstack, self.roots_of_all_functions)
    #                 #print(node.name, cur_comm_count)
    #                 if node != None:
    #                     #print(node.name, cur_comm_count)
    #                     if len(node.comm_dep) != len(comm_dep.data):
    #                         node.comm_dep = [[] for i in range(len(comm_dep.data))]
    #                         node.comm_time = [0 for i in range(len(comm_dep.data))]
    #                         #print(node.sampling_count)
    #                     #print(node.sampling_count)
    #                     node.comm_time[i] += cur_comm_time
                        
    #                     #print(cur_comm_time)
    #                     node.comm_dep[i].append(callstack_line[1:])
    #                 #else:
    #                     #print(callstack, cur_comm_count, "not found")
    #                 # Record total sampling count
    #                 total_comm_time += cur_comm_time
    #                 #print(cur_comm_time)
    #         self.total_comm_time = total_comm_time
    #         total_comm_time = 0
    #         #print(total_comm_time, real_total_comm_time)
    #         #updatePercentageOfNode(self.main_root, total_sampling_count)
    #         #print(percentage_sum)
    #         #self.BFS(self.main_root, updateNameofAddrNode, comm_dep.ldmap[i])

    def commDepEmbedding(self, comm_dep):
        
        for comm_dep_edge in comm_dep.p2p_data:
            src_call_path = comm_dep_edge[0][0]
            dest_call_path = comm_dep_edge[1][0]
            src_pid = comm_dep_edge[0][1]
            dest_pid = comm_dep_edge[1][1]
            edge_weight = comm_dep_edge[2]
            src_node = None
            dest_node = None
            if len(src_call_path) != 0:
                src_call_path.pop()
                #print(src_call_path)
                # Invoke "getNodeWithCallstack" to embed one [call stack, sampling count] line to corresponding vertex
                src_node = getNodeWithCallstack(self.main_root, src_call_path, self.roots_of_all_functions)
                #if src_node != None:
                    #print("FOUND NODE:", node.unique_id, node.name, src_call_path)
                #    node.comm_dep.append(comm_dep_edge)
            if len(dest_call_path) != 0:
                dest_call_path.pop()
                # Invoke "getNodeWithCallstack" to embed one [call stack, sampling count] line to corresponding vertex
                dest_node = getNodeWithCallstack(self.main_root, dest_call_path, self.roots_of_all_functions)
                #if dest_node != None:
                    #print("FOUND NODE:", node.unique_id, node.name)
                #    node.comm_dep.append(comm_dep_edge)
            if src_node != None and dest_node != None:
                src_node.comm_dep.append([[src_node, src_pid], [dest_node, dest_pid], edge_weight])
                dest_node.comm_dep.append([[src_node, src_pid], [dest_node, dest_pid], edge_weight])



    # def convertNodePerfDataToJson(self, node, stand_flag):
    #     if node.converted_perfdata_to_json != stand_flag:
    #         return
    #     node.converted_perfdata_to_json = not stand_flag

    #     self.perf_data_config[node.unique_id] = node.performance_data

    #     for child in node.children:
    #         self.convertNodePerfDataToJson(child, stand_flag)

    # def readPerformanceDataFromJsonFile(self, file):
    #     if os.path.exists(file):
    #         with open(file, "r") as f:
    #             self.perf_data_config = json.load(f)
    #     else:
    #         print("No such file: ", file)
    #         return
    #     self.embedJsonToNode(self.main_root)

    # def embedJsonToNode(self, node):
    #     if embedded_json_to_node:
    #         return
    #     embedded_json_to_node = True

    #     for child in node.children:



    # Embed asemble instruction data to Program Structure Graph
    # def asembleInstructionDataEmbedding(self, asm_data, node):
    #     for i in range(len(asm_data.data)):
    #         #for asm_line in asm_data.data[i]:
    #         asm_line = asm_data.data[i]
    #         #print()
    #         #print(asm_line)
    #         func_name = asm_line[0]
    #         #func_node = getNodeWithFuncName(self.main_root, func_name)
    #         func_node = node
    #         if func_node != None:
    #             for loop_asm_line in asm_line[1:]:
    #                 #print(loop_asm_line)
    #                 loop_name = loop_asm_line[0]
    #                 loop_asm = loop_asm_line[1]
    #                 loop_node = getNodeWithLoopName(func_node, loop_name, random.randint(0,100000))
    #                 if loop_node != None:
    #                     loop_node.inst_count = loop_asm
    #                 else:
    #                     print("L", loop_name, " not found")
    #         else:
    #             print("F", func_name, " not found")

    #     #print("Designing a .asm file now.")




    def asembleInstructionDataEmbedding(self, asm_data, node):
        self.BFS(node, recursiveAsembleDataEmbedding, asm_data)
        self.BFS(node, noneFunc)

    def contraction(self, preserved_func_list=[]):
        #self.BFS(self.main_root, updatePercentageOfNode, self.total_sampling_count)
        #clear contracted flag
        self.BFS(self.main_root, clearContractionFlag)

        mergeSameAddrFuncNode(self.main_root)
        
        markRemoveFlagOnGrpah(self.main_root)
        graphContraction(self.main_root)
        if len(preserved_func_list) != 0:
            # markRemoveFlagOnGrpahForUser(self.main_root, preserved_func_list)
            markPreservedSubgraph(self.main_root, preserved_func_list, False)
            graphContractionForUser(self.main_root)
        
        #clear contracted flag
        self.BFS(self.main_root, clearContractionFlag)

        self.deleteSharedObjectLibLeaf(self.main_root)
        graphContraction(self.main_root)

        #self.BFS(self.main_root, updatePercentageOfNode, self.total_sampling_count)

    # Build a ECM Model with asemble instruction data for each vertex
    def buildECMModel(self, node):
        #buildNodeECMModel(self.main_root)
        buildNodeECMModel(node)
        #print("build ECM Model with asemble instruction data")

    def convertToGraph(self):
        global uniqId
        self.BFS(self.main_root, self.savePSGasGraph)
        uniqId = 0

    # Show the Program Structure Graph
    def show(self, save_fig=""):
        global uniqId
        uniqId = 0
        #printGraph(self.main_root,0)
        if save_fig == "":
            output = GraphvizOutput(self.psg_file, self.main_root, nodes = self.nodes, edges = self.edges, total_sample_count = self.total_sampling_count[0], total_comm_time = self.total_comm_time)
        else:
            output = GraphvizOutput(self.psg_file, self.main_root, nodes = self.nodes, edges = self.edges, total_sample_count = self.total_sampling_count[0], total_comm_time = self.total_comm_time, output_file = save_fig + ".psg")
        output.done()

    def save(self, save_file=""):
        if save_file == "":
            node_output_file = self.psg_file + ".nodes.json"
            edge_output_file = self.psg_file + ".edges.json"
        else:
            node_output_file = save_file + ".nodes.json"
            edge_output_file = save_file + ".edges.json"
        with open(node_output_file, "w") as outfile: 
            json.dump(self.nodes, outfile)
        outfile.close()
        with open(edge_output_file, "w") as outfile: 
            json.dump(self.edges, outfile)
        outfile.close()

    def savePSGasGraph(self, node):
        self.nodes[node.unique_id] = [node.type, node.name, node.performance_percentage, node.performance_data, node.entry_addr, node.exit_addr]
        for child in node.children:
            if self.edges.keys().__contains__(node.unique_id):
                self.edges[node.unique_id].append(child.unique_id)
            else:
                self.edges[node.unique_id] = [child.unique_id]

    def markUnderOmpStart(self):
        assert not self.has_cycles()
        # DFS
        def dfs(node, omp_father):
            node.under_omp_start = omp_father

            is_omp_start = node.name in OMP_START_LIST or omp_father
            for child in node.children:
                dfs(child, is_omp_start)

        dfs(self.main_root, False)


def recursiveAsembleDataEmbedding(node, asm_data):
    if node.type_name == "FUNCTION":
        func_name = node.name
        asm_line = asm_data.data[func_name]
        for k,v in asm_line.items():
            #print(loop_asm_line)
            loop_name = k
            loop_asm = v
            loop_node = getNodeWithLoopName(node, loop_name, random.randint(0,100000))
            if loop_node != None:
                loop_node.inst_count = loop_asm
            else:
                print("L", loop_name, " not found")


# Recursively read the vertex and build a Graph

def readGraph(f):
    global uniqId
    types = [int, int, int, int, int, int, str]
    #args = list(map(lambda x: x[0](x[1]), zip(types, f.readline().decode('utf-8').strip().split(' '))))
    line = f.readline()
    #print(line)
    zips = zip(types, line.strip().split(' '))
    args = list(map(lambda x: x[0](x[1]), zips))
    args.insert(0, uniqId)
    uniqId += 1
    node = Node(*args)
    for i in range(len(node.children)):
        node.children[i] = readGraph(f)
    return node

def getNodeArgs(uid, edges, nodes):
    if edges.keys().__contains__(uid):
        num_children = len(edges[uid])
    else:
        num_children = 0
    if nodes.keys().__contains__(uid):
        node_type = nodes[uid][0]
        name = nodes[uid][1]
        performance_percentage = nodes[uid][2]
        performance_data = nodes[uid][3]
        entry_addr = int(nodes[uid][4], 16)
        exit_addr = int(nodes[uid][5], 16)
    else:
        node_type = -10
        name = ""
        performance_percentage = 0
        performance_data = {}
        entry_addr = 0
        exit_addr = 0
    args = [uid, -1, node_type, -1, num_children, entry_addr, exit_addr, name, performance_percentage, performance_data]
    return args

def generatePSGWithNodesEdges(uid, edges, nodes):
    
    args = getNodeArgs(uid, edges, nodes)
    node = Node(*args)

    #print(args)
    # TODO: recursive call    
    if edges.keys().__contains__(uid):
        for i in range(len(edges[uid])):
            child_uid = edges[uid][i]
            node.children[i] = generatePSGWithNodesEdges(str(child_uid), edges, nodes)

    return node
    

# Print Graph


def printGraph(node, depth):
    if node.removed == True: #and node.sampling_count == 0:
        return
    print(' ' * depth, end='')
    print([node.unique_id, node.type, node.entry_addr, node.exit_addr, node.performance_data])
    for child in node.children:
        printGraph(child, depth + 1)

def copyOneNode(node):
    global uniqId
    newnode = Node(uniqId, node.id, node.type, node.child_id, 0, int(node.entry_addr, 16), int(node.exit_addr, 16), node.name)
    uniqId += 1
    return newnode

def copyNode(node):
    global uniqId
    newnode = Node(uniqId, node.id, node.type, node.child_id, node.num_children, int(node.entry_addr, 16), int(node.exit_addr, 16), node.name)
    uniqId += 1
    for i in range(len(node.children)):
        newnode.children[i] = copyNode(node.children[i])
    return newnode

def getSoLibWithAddr(addr, ldmap):
    for k, v in ldmap.items():
        k_array = k.strip().split('-')
        low_addr = int(k_array[0], 16)
        high_addr = int(k_array[-1], 16)
        #print(hex(addr), hex(low_addr), hex(high_addr))
        if addr > low_addr and addr < high_addr:
            if addr < int('4f0000000000', 16):
                return v, addr
            return v, addr - low_addr
    return None, 0

def getNodeWithSoAddr(node, pop_addr, ldmap):
    global queried_so_addr_map
    global uniqId
    found_node = None
    if queried_so_addr_map.keys().__contains__(pop_addr):
        found_node = copyOneNode(queried_so_addr_map[pop_addr])
    else:
        solib_name, query_addr = getSoLibWithAddr(pop_addr, ldmap)
        if solib_name == None:
            node_name = "Unknown Addr"
        else:
            #print(hex(query_addr), solib_name)
            addr2func_cmd = "addr2line -e " + solib_name + " -fC " + str(query_addr) + " | head -n 1"
            tmp = os.popen(addr2func_cmd)
            node_name = tmp.read()
        found_node = Node(uniqId, -1, -9, node.num_children, 0, pop_addr, pop_addr, node_name)
        uniqId += 1
        queried_so_addr_map[pop_addr] = found_node
    
    node.children.append(found_node)
    found_node.child_id = node.num_children
    node.num_children += 1
    return found_node
    

# Identify the corresponding vertex of "callstack" from "node"
def getNodeWithCallstack(node, callstack, roots_of_all_functions):
    global uniqId
    global recursive_function_stack
    # If "callstack" is empty, then the corresponding node is found, return "node"
    if len(callstack) == 0:
        return node

    # Get the first addr of current call stack
    pop_addr = int(callstack[-1][0], 16)
    # Pop the first addr of current call stack
    callstack.pop()

    found_node = None

    # If the addr is less than '40000', skip it and pop the next addr
    if pop_addr < int('400000', 16):
        return getNodeWithCallstack(node, callstack, roots_of_all_functions)
    # If the addr is large than '4f0000000000', it is an addr in shared library
    #elif pop_addr > int('4f0000000000', 16):
    #    found_node = getNodeWithSoAddr(node, pop_addr)
    else:
        # Invoke "getNodeWithAddr" to identify the corresponding 
        found_node = getNodeWithAddr(node, pop_addr)

    # Node is not found, create a new node then return it
    if found_node == None:
        addr_node = Node(uniqId, -1, -9, node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
        node.children.append(addr_node)
        node.num_children += 1
        uniqId += 1
        return getNodeWithCallstack(addr_node, callstack, roots_of_all_functions)

    # If indirect call node is found
    if found_node.type_name == "CALL_IND" or (found_node.type_name == "CALL" and len(found_node.children) == 0) or (found_node.type_name == "ADDR" and len(found_node.children) == 0):
    #if found_node.type_name == "CALL_IND" or len(found_node.children) == 0:
        if len(callstack) != 0:
            #First read the next line
            pop_addr = int(callstack[-1][0], 16)
            #callstack.pop()

            if pop_addr >= int('400000', 16):
                if hasAttachedCallee(found_node, pop_addr) == False:

                    #Find the tree root of indirect callee
                    indCalleeNode = getTreeRootWithAddr(pop_addr, roots_of_all_functions)

                    if indCalleeNode == None:
                        #print(hex(pop_addr))
                        addr_node = Node(uniqId, -1, -9, found_node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
                        found_node.children.append(addr_node)
                        found_node.num_children += 1
                        uniqId += 1
                        return getNodeWithCallstack(found_node, callstack, roots_of_all_functions)
                    #print(node.unique_id , "invokes", indCalleeNode.id, ":", indCalleeNode.name)
                    
                    #attach the copy of the callee function to the indirect call
                    new_node = copyNode(indCalleeNode)
                    found_node.name = new_node.name
                    found_node.children.append(new_node)
                    #indCalleeNode.parent = found_node
                    indCalleeNode.childID = found_node.num_children
                    found_node.num_children += 1
                    
                    return getNodeWithCallstack(found_node, callstack, roots_of_all_functions)

    # Recursively invoke "getNodeWithCallstack" with call stack that pops top addr from "found_node"
    return getNodeWithCallstack(found_node, callstack, roots_of_all_functions)

# def getNodeWithCallstack(node, callstack, roots_of_all_functions):
#     global uniqId
#     global recursive_function_stack
#     # If "callstack" is empty, then the corresponding node is found, return "node"
#     if len(callstack) == 0:
#         return node

#     # Get the first addr of current call stack
#     pop_addr = int(callstack[-1][0], 16)
#     # Pop the first addr of current call stack
#     callstack.pop()

#     found_node = None

#     # If addr is less than '40000', we record as -1 in the file, skip it and pop the next LineNum
#     if pop_addr < int('400000', 16):
#         return getNodeWithCallstack(node, callstack, roots_of_all_functions)

#     # Invoke "getNodeWithAddr" to identi
#     found_node = getNodeWithAddr(node, pop_addr)

#     # Node is not found, so return the last level node
#     if found_node == None:
#         # if len(callstack) != 0:
#         #     pop_addr = int(callstack[-1][0], 16)
#         #     #callstack.pop()
#         #     addr_node = Node(uniqId, -1, -9, node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
#         #     node.children.append(addr_node)
#         #     node.num_children += 1
#         #     uniqId += 1
#         #     return getNodeWithCallstack(node, callstack, roots_of_all_functions)
#         # else:
#         #     return node
#         #pop_addr = int(callstack[-1][0], 16)
#         #callstack.pop()
#         addr_node = Node(uniqId, -1, -9, node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
#         node.children.append(addr_node)
#         node.num_children += 1
#         uniqId += 1
#         return getNodeWithCallstack(addr_node, callstack, roots_of_all_functions)

#     # If indirect call node is found
#     if found_node.type_name == "CALL_IND" or (found_node.type_name == "CALL" and len(found_node.children) == 0) or (found_node.type_name == "ADDR" and len(found_node.children) == 0):   
#     #if found_node.type_name == "CALL_IND" or len(found_node.children) == 0:
#         if len(callstack) != 0:
#             #First read the next line
#             pop_addr = int(callstack[-1][0], 16)
#             callstack.pop()

#             if pop_addr >= int('400000', 16):
#                 if hasAttachedCallee(found_node, pop_addr) == False:

#                     #Find the tree root of indirect callee
#                     indCalleeNode = getTreeRootWithAddr(pop_addr, roots_of_all_functions)

#                     if indCalleeNode == None:
#                         #print(hex(pop_addr))
#                         addr_node = Node(uniqId, -1, -9, found_node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
#                         found_node.children.append(addr_node)
#                         found_node.num_children += 1
#                         uniqId += 1
#                         return getNodeWithCallstack(addr_node, callstack, roots_of_all_functions)
#                     #print(node.unique_id , "invokes", indCalleeNode.id, ":", indCalleeNode.name)
                    
#                     #expand computation node for the callee function
#                     #sortByLineNum(indCalleeNode);
#                     #expandComputeNode(indCalleeNode);
#                     #sortByLineNum(indCalleeNode);
                    
#                     #attach the copy of the callee function to the indirect call
#                     new_node = copyNode(indCalleeNode)
#                     found_node.name = new_node.name
#                     found_node.children.append(new_node)
#                     #indCalleeNode.parent = found_node
#                     indCalleeNode.childID = found_node.num_children
#                     found_node.num_children += 1
                    
#                     return getNodeWithCallstack(new_node, callstack, roots_of_all_functions)

#     # if(found_node.type == CALL_REC || found_node . type >= 0 || found_node . type == COMPUTE_NODE){

#     # If "found_node" is a leaf node, stop detect and return it.
#     # if len(found_node.children) == 0 and found_node.type_name != "CALL_REC":
#     #     # addr_node = Node(uniqId, -1, -9, found_node.num_children, 0, pop_addr, pop_addr, "ADDRESS")
#     #     # found_node.children.append(addr_node)
#     #     # found_node.num_children += 1
#     #     # uniqId += 1
#     #     # return getNodeWithCallstack(addr_node, callstack, roots_of_all_functions)
#     #     return found_node

#     # Recursively invoke "getNodeWithCallstack" with call stack that pops top addr from "found_node"
#     return getNodeWithCallstack(found_node, callstack, roots_of_all_functions)

# Identify the corresponding vertex of "pop_addr" from "node"
def getNodeWithAddr(node, pop_addr):
    # Traverse all children of node
    for child in node.children:
        # If "pop_addr" is in range of "child"'s addr, then perform further detection from this "child"
        if int(child.entry_addr, 16) <= pop_addr and pop_addr <= int(child.exit_addr, 16):
            # If "child" is a leaf node, then stop detection and return it.
            if child.type_name == "CALL_REC":
                #print (list(recursive_function_stack.keys()))
                if len(list(recursive_function_stack.keys())) > 0:
                    checkRecursiveCall(child)
                return child
            if len(child.children) == 0:
                return child
            # If "child" is an MPI or CALL or CALL_REC node, then stop detection and return it.
            if child.type >= 0 or child.type_name == "CALL" or child.type_name == "CALL_IND": # or child.type_name == "CALL_REC":
                return child
            # If "child" is a LOOP or FUNCTION node, perform deeper detection from "child"
            elif child.type_name == "LOOP" or child.type_name == "FUNCTION":
                if child.type_name == "FUNCTION":
                    recursive_function_stack[child.name] = child

                found_node = getNodeWithAddr(child, pop_addr)
                # If detection find deeper node, then return "found_node", otherwise, return "child"
                if found_node:
                    return found_node
                else:
                    return child
            else:
                return child

    #for child in node.children:
    #    return getNodeWithAddr(child, pop_addr)

    return None



def checkRecursiveCall(node):
    if node.name in list(recursive_function_stack.keys()):
        #print(node.name,node.type_name)
        if hasAttachedCalleeWithFuncName(node, node.name) == False:
            #print("here")
            # add recursive edge
            callee_node = recursive_function_stack.get(node.name)
            node.children.append(callee_node)
            node.num_children += 1
            # if recursive call is found delete all functions after this call
            index = list(recursive_function_stack.keys()).index(node.name)
            while len(list(recursive_function_stack.keys())) > index:
                recursive_function_stack.popitem()


# This function is for indirect call

def hasAttachedCalleeWithFuncName(node, func_name):
    for child in node.children:
        if func_name == child.name :
            return True
    return False

def hasAttachedCallee(node, pop_addr):
    for child in node.children:
        if int(child.entry_addr, 16) <= pop_addr and int(child.exit_addr, 16) >= pop_addr :
            return True
    return False


def getNodeWithFuncName(node, func_name):
    if node.got_node_with_func_name:
        return
    node.got_node_with_func_name = True

    #print(node.type_name,node.name)
    if node.type_name == "FUNCTION" and node.name == func_name:
        print("FOUND")
        return node
    for child in node.children:
        func_node = getNodeWithFuncName(child, func_name)
        if func_node != None:
            return func_node
    return None

def getNodeWithUniqueId(node, unique_id, stand_flag):
    if node.got_node_with_unique_id == stand_flag:
        return
    node.got_node_with_unique_id = stand_flag

    if node.unique_id == unique_id:
        return node
    for child in node.children:
        unique_node = getNodeWithUniqueId(child, unique_id, stand_flag)
        if unique_node != None:
            return unique_node
    return None


def getNodeWithLoopName(node, loop_name, stand_flag):
    if node.got_node_with_loop_name == stand_flag:
        return
    node.got_node_with_loop_name = stand_flag
    
    if node.type_name == "LOOP" and node.name == loop_name:
        return node
    for child in node.children:
        loop_node = None
        if child.type_name != "FUNCTION" and child.type_name != "CALL" and child.type_name != "CALL_IND":
            loop_node = getNodeWithLoopName(child, loop_name, stand_flag)
        if loop_node != None:
            return loop_node
    return None


def getTreeRootWithAddr(pop_addr, roots_of_all_functions):
    for node in roots_of_all_functions:
        if int(node.exit_addr, 16) >= pop_addr and int(node.entry_addr, 16) <= pop_addr:
            return node


def updatePercentageOfNode(node, total_sampling_count):
    #global percentage_sum

    average_percentage = 0.0
    for i in range(len(node.performance_data["TOT_CYC"])):
        tmp = float(node.performance_data["TOT_CYC"][i]) / float(total_sampling_count[i])
        average_percentage += tmp
        node.all_procs_percentage.append(tmp)
    average_percentage /= len(node.performance_data["TOT_CYC"])

    
    
    node.performance_percentage = round(average_percentage, 5)
    #if abs(node.performance_percentage) > 0:
    #    print(node.unique_id, node.performance_percentage)
    #    print(node.performance_data["TOT_CYC"], total_sampling_count)
    #percentage_sum += node.performance_percentage


# def updatePercentageOfNode(node, pid, total_sampling_count):
#     global percentage_sum
    
#     node.performance_percentage = round(node.sampling_count[pid] / total_sampling_count, 5)
#     percentage_sum += node.performance_percentage

# def appendPerfDataToAllData(node, suffix, stand_flag):
#     if node.appended_feature_data_to_perf_data != stand_flag:
#         return
#     node.appended_feature_data_to_perf_data = not stand_flag

#     node.all_data[suffix] = node.performance_data
#     for child in node.children:
#         appendPerfDataToAllData(child, suffix, stand_flag)

def appendPerfDataToAllData(node, suffix):
    node.all_data[suffix] = node.performance_data

# def appendFeatureDataToPerfData(node, feature, stand_flag):
#     if node.appended_feature_data_to_perf_data != stand_flag:
#         return
#     node.appended_feature_data_to_perf_data = not stand_flag

#     node.performance_data[feature] = node.sampling_count
#     for child in node.children:
#         appendFeatureDataToPerfData(child, feature, stand_flag)

def appendFeatureDataToPerfData(node, feature):
    node.performance_data[feature] = []
    for i in range(len(node.sampling_count)):
        node.performance_data[feature].append(node.sampling_count[i])

# def clearSamplingCount(node, stand_flag):
#     if node.appended_feature_data_to_perf_data != stand_flag:
#         return
#     node.appended_feature_data_to_perf_data = stand_flag

#     node.sampling_count = [0]
#     for child in node.children:
#         clearSamplingCount(child, stand_flag)

def clearSamplingCount(node):
    node.sampling_count = [0]

# def clearPerfData(node, stand_flag):
#     if node.appended_feature_data_to_perf_data != stand_flag:
#         return
#     node.appended_feature_data_to_perf_data = stand_flag

#     node.performance_data = {}
#     for child in node.children:
#         clearPerfData(child, stand_flag)

def clearPerfData(node):
    node.performance_data = {}

def updateNameofAddrNode(node, ldmap):
    global queried_so_addr_func_map
    global uniqId
    if node.type_name == "ADDR" or (node.type_name == "ADDR2FUNC" and node.name == "Unknown Addr"):
        addr = int(node.entry_addr, 16)
        if queried_so_addr_func_map.keys().__contains__(addr):
            node.name = queried_so_addr_func_map[addr]
        else:
            solib_name, base_addr = getSoLibWithAddr(addr, ldmap)
            if solib_name == None:
                node_name = "Unknown Addr"
            else:
                #print(hex(base_addr), solib_name)
                addr2func_cmd = "addr2line -e " + solib_name + " -fC " + hex(base_addr) + " | head -n 1"
                tmp = os.popen(addr2func_cmd)
                node_name = tmp.read().strip().split('(')[0]
                #print(hex(base_addr), solib_name, node_name)
                queried_so_addr_func_map[addr] = node_name
            node.name = node_name
        node.type = -8
        node.type_name = "ADDR2FUNC"

def updateNameofAddrNodeNonLib(node, binary_name):
    global queried_so_addr_func_map
    #print(node.entry_addr, node.type_name)
    
    if node.type_name == "ADDR":
        addr = int(node.entry_addr, 16)
        #print(hex(addr))
        if queried_so_addr_func_map.keys().__contains__(addr):
            node.name = queried_so_addr_func_map[addr]
            node.type = -8
            node.type_name = "ADDR2FUNC"
        else:
            if addr < int('4f0000000000', 16):
                addr2func_cmd = "addr2line -e " + binary_name +  " -fC " + hex(addr) + " | head -n 1"
                tmp = os.popen(addr2func_cmd)
                node_name = tmp.read().strip().split('(')[0]
                #print(hex(addr), binary_name, node_name)
                queried_so_addr_func_map[addr] = node_name
                node.name = node_name
                node.type = -8
                node.type_name = "ADDR2FUNC"


def mergeSameAddrFuncNode(node):
    # Return true if this node should not be merged
    if node.merged:
        return 
    node.merged = True
    funcname_node_map = {}

    for child in node.children:
        if child.type_name == "ADDR2FUNC" and len(child.children) == 0:
            if funcname_node_map.keys().__contains__(child.name):
                funcname_node_map[child.name].performance_percentage += child.performance_percentage
                child.performance_percentage = 0
                child.merged_removed = True
            else:
                funcname_node_map[child.name] = child
        else:
            mergeSameAddrFuncNode(child)
            


def markRemoveFlagOnGrpah(node):
    # Return true if this node should not be removed
    #if node:
    #    return False
    if node.trimmed:
        return not node.removed
    if node.merged_removed:
        return False

    node.trimmed = True

    reserved = False
    for child in node.children:
        reserved |= markRemoveFlagOnGrpah(child)
    
    #if node.type_name == "MPI": # or node.sampling_count[0] > 0 :
    if node.performance_percentage > 0.00001 or len(node.comm_dep) > 0:
        node.removed = False
        node.trimmed = True
        
        return True
    
    node.removed = not reserved
    return reserved
    #if node.type_name == "MPI" or 

def graphContraction(node):
    #global all_perf_data_config
    if node.contracted:
        return
    node.contracted = True
    #print(node.children)
    i = 0
    while i < len(node.children):
        child = node.children[i]
        if child.removed == True or child.merged_removed == True:
            node.children.pop(i)
        else:
            graphContraction(child)
            i += 1

def markPreservedSubgraph(node, preserved_func_list, father_has_preserved_func):
    # 如果一个node自己包含preserved_func_list的内容，则其本身及其所有子节点都被保留
    # 如果一个node需要被保留，则其父节点都需要被保留
    if node.user_trimmed:
        return not node.user_removed
    node.user_trimmed = True


    find_func = False
    for func in preserved_func_list:
        if node.name.find(func) != -1:
            find_func = True
            break
    reserved = find_func or father_has_preserved_func
    child_reserved = False
    for child in node.children:
        if not child.user_trimmed:
            child_reserved |= markPreservedSubgraph(child, preserved_func_list, reserved)

    node.user_removed = not (reserved or child_reserved)
    return reserved or child_reserved

def markRemoveFlagOnGrpahForUser(node, preserved_func_list):
    # Return true if this node should not be removed
    #if node:
    #    return False
    if node.user_trimmed:
        return not node.user_removed

    node.user_trimmed = True

    reserved = False
    for child in node.children:
        reserved |= markRemoveFlagOnGrpahForUser(child, preserved_func_list)
    
    for func in preserved_func_list:
        if node.name.find(func) != -1:
            node.user_removed = False
            node.user_trimmed = True
        
            return True
    
    node.user_removed = not reserved
    return reserved

def graphContractionForUser(node):
    #global all_perf_data_config
    if node.user_contracted:
        return
    node.user_contracted = True
    #print(node.children)
    i = 0
    while i < len(node.children):
        child = node.children[i]
        if child.user_removed == True:
            node.children.pop(i)
        else:
            graphContractionForUser(child)
            i += 1


def buildNodeECMModel(node):
    if node.built_ecm_model:
        return 
    node.built_ecm_model = True
    
    if len(node.inst_count)!= 0:

        # First phase: get asemble instruction number
        load_num = node.inst_count[0]
        store_num = node.inst_count[1]
        add_num = node.inst_count[2]
        sub_num = node.inst_count[3]
        mul_num = node.inst_count[4]
        imul_num = node.inst_count[5]
        div_num = node.inst_count[6]
        idiv_num = node.inst_count[7]
        other_num = node.inst_count[8]

        # Second phase: calculate LD/ST latency (cycles)
        reg_l1 = max(2 * load_num, 4 * store_num)
        l1_l2 = 2 * load_num + 4 * store_num
        l2_l3 = 2 * load_num + 4 * store_num
        l3_mem = (load_num + 2 * store_num) * 64 * 2.3 / (2133 * 64 / 8 * 2 / 1024)

        # Third phase: calculate non-LD/ST total cycles
        non_ldst_cycle = 1 * max(add_num, sub_num) + 3 * mul_num + 64 * div_num + 1 * other_num
        
        # Forth phase: calculate ECM Models
        ECM = []
        ECM.append(max(non_ldst_cycle, reg_l1))
        ECM.append(max(non_ldst_cycle, reg_l1 + l1_l2))
        ECM.append(max(non_ldst_cycle, reg_l1 + l1_l2 + l2_l3))
        ECM.append(max(non_ldst_cycle, reg_l1 + l1_l2 + l2_l3 + l3_mem))
        node.ECM = ECM
        node.ECMmodel = str(ECM[0]) + " | " + str(ECM[1]) + " | " + str(ECM[2]) + " | " + str(round(ECM[3],2))

    for child in node.children:
        buildNodeECMModel(child)


def getEntryAddr(node):
    return int(node.entry_addr, 16)

def sortPSG(node):
    if len(node.children) > 1:
        node.children.sort(key = getEntryAddr)


# do nothing
def noneFunc(node):
    return 

def clearContractionFlag(node):
    node.trimmed = False
    node.removed = False
    node.contracted = False