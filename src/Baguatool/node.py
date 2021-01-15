import copy
typeDict = {k: v for (k, v) in zip([-10, -9, -8, -7, -6, -5, -4, -1, 1], ['LOOP_END', 'ADDR', 'ADDR2FUNC', 'CALL_IND', 'CALL_REC', 'CALL', 'FUNCTION', 'LOOP', 'MPI'])}

class Node(object):
    def __init__(self,
                 uid,
                 node_id,
                 node_type,
                 #firstType,
                 #lastType,
                 child_id,
                 num_children,
                 #dirID,
                 #fileID,
                 entry_addr,
                 exit_addr,
                 name,
                 #lineNum,
                 #exitLineNum,
                 #sampleCount,
                 #sumTime
                 performance_percentage = 0.0,
                 performance_data={}
                 ):
        self.unique_id = uid
        self.id = node_id
        self.type = node_type
        self.type_name = typeDict[node_type]
        self.child_id = child_id
        #self.firstType = firstType
        #self.lastType = lastType
        self.num_children = num_children
        self.children = [None] * num_children
        #self.dirID = dirID
        #self.fileID = fileID
        self.entry_addr = hex(entry_addr)
        self.exit_addr = hex(exit_addr)
        self.name = name
        #self.lineNum = lineNum
        #self.exitLineNum = exitLineNum
        #self.sumTime = sumTime
        self.sampling_count = [0]
        self.performance_percentage = performance_percentage
        self.all_procs_percentage = []
        self.performance_data = performance_data
        self.all_data = {}
        self.has_calculated_performance = False
        self.appended_feature_data_to_perf_data = False
        # sumTime only
        #self.sumTimeList = [sumTime]
        #self.sumTimeSecList = []
        #self.pred = 0
        #self.crossPreds = []
        self.converted_perfdata_to_json = False
        self.got_node_with_func_name = 0
        self.got_node_with_loop_name = 0
        self.got_node_with_unique_id = 0

        self.trimmed = False
        self.removed = False
        self.contracted = False

        self.generated_nodes_edges_groups = False

        self.inst_count = []
        self.ECM = []
        self.ECMmodel = None

        self.built_ecm_model = False

        self.bfs_flag = False

        self.merged = False
        self.merged_removed = False

        self.user_removed = False
        self.user_trimmed = False
        self.user_contracted = False

        self.comm_dep = []
        self.comm_time = [0]

        self.group_flag = False

class PNode(object):
    def __init__(self,
                 unique_id,
                 node,
                 node_type,
                 total_sampling_count,
                 pid
                 ):
        self.unique_id = unique_id
        self.id = node.unique_id
        self.type = node_type
        self.type_name = typeDict[node_type]
        self.entry_addr = node.entry_addr
        self.exit_addr = node.exit_addr
        self.name = node.name
        if node.performance_data.keys().__contains__("TOT_CYC"):
            perf_data = node.performance_data["TOT_CYC"]
        else:
            perf_data = [0]
        


        if len(perf_data) > pid:
            self.sampling_count = perf_data[pid]
        else:
            self.sampling_count = 0

        if pid == 0:
            #self.all_procs_perf_data = copy.deepcopy(perf_data)
            self.all_procs_perf_data = perf_data

        if len(node.comm_time) > pid:
            self.comm_time = node.comm_time[pid]
        else:
            self.comm_time = 0

        # # TODO: this version is not accurate 
        # if pid == 0:
        #     self.performance_percentage = node.performance_percentage
        # elif perf_data[0] == 0:
        #     self.performance_percentage = 0.0
        # else:
        #     self.performance_percentage = node.performance_percentage * self.sampling_count / perf_data[0]

        self.all_procs_percentage = node.all_procs_percentage

        self.performance_percentage = self.sampling_count / total_sampling_count[pid]

        self.children = []

        self.loop_pair_node = None

        self.generated_nodes_edges_groups = False

        self.group_flag = False

        self.bfs_flag = False

        self.comm_dep = []

        if pid == 0 and len(node.comm_dep) > 0:
            self.comm_dep = node.comm_dep