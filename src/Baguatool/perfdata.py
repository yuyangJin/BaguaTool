import os

class perfData(object):
    def __init__(self, binary_name, perf_data_dir, dir_suffix, nprocs, dyn_features):
        self.binary_name = binary_name
        self.dynamic_data_dir = perf_data_dir
        self.dir_suffix = dir_suffix
        self.perf_data_dir = perf_data_dir + "/" + dir_suffix + "/"
        self.nprocs = nprocs
        self.dyn_features = dyn_features
        self.data = []
        self.readPerfData()

        # ldmap {key: value} key is string("[low_addr] - [high_addr]"), value is file path
        if os.path.exists(self.perf_data_dir + "LDMAP" + str(0) + ".TXT"):
            self.ldmap = []
            self.readLdMap()

    
    def readPerfData(self):
        self.data = [[[] for i in range(self.nprocs)] for j in range(len(self.dyn_features))]
        #print (self.data)
        for feature_id in range(len(self.dyn_features)):
            for pid in range(self.nprocs):
                
                #parse_cmd = "$BAGUA_DIR/bin/parseCallPath.sh " + self.binary_name + " " + self.perf_data_dir + "./SAMPLE" + str(pid) + "-" + str(feature_id) + ".TXT"
                #os.system(parse_cmd)

                callpath_file_name = self.perf_data_dir + "SAMPLE" + str(pid) + "-" + str(feature_id) + ".TXT"
                
                with open(callpath_file_name) as f1:
                    callpath_lines = f1.readlines()
                f1.close()

                #with open(symb_file_name) as f2:
                #    symb_lines = f2.readlines()

                #dir_map = []
                #file_map = []

                dir_file_line_pointer = 0

                #symb_line_pointer = 0

                for callpath_line in callpath_lines:
                    callpath_struct = callpath_line.strip().split('|')
                    callpath_addr_str = callpath_struct[0]
                    callpath_count = callpath_struct[1]
                    callpath_addrs = callpath_addr_str.strip().split(' ')
                    while '' in callpath_addrs:
                        callpath_addrs.remove('')
                    #callpath_len = len(callpath_addrs)
                    tmp_list = []
                    for callpath_addr in callpath_addrs:
                        #symb_line = symb_lines[symb_line_pointer].strip().split('\n')
                        #tmp_list.append([callpath_addr, symb_line[0]])
                        tmp_list.append([callpath_addr])
                        #symb_line_pointer += 1
                    #self.data[pid].append(tmp_list)
                    self.data[feature_id][pid].append([tmp_list, callpath_count])

                #f2.close()
        #rm_cmd = "rm -f " + self.perf_data_dir + "./SAMPLE*-*-*"
        #os.system(rm_cmd)
    
    def readLdMap(self):
        self.ldmap = [ {} for i in range(self.nprocs)]
        for pid in range(self.nprocs):
            ldmap_file_name = self.perf_data_dir + "LDMAP" + str(pid) + ".TXT"

            with open(ldmap_file_name) as f:
                ldmap_lines = f.readlines()
            f.close()

            for ldmap_line in ldmap_lines:
                if ldmap_line.find(',') == -1:
                    ldmap_line_str_array = ldmap_line.strip().split(' ')
                    if ldmap_line_str_array[-1] != "0":
                        self.ldmap[pid][ldmap_line_str_array[0]] = ldmap_line_str_array[-1]



    def show(self):
        print(self.data)
