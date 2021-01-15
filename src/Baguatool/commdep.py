import os

class commDep(object):
    def __init__(self, dynamic_data_dir, binary_name, dir_suffix, nprocs):
        self.dynamic_data_dir = dynamic_data_dir
        self.dir_suffix = dir_suffix
        self.comm_dep_dir = dynamic_data_dir + "/" + dir_suffix + "/"
        self.binary_name = binary_name
        self.nprocs = nprocs
        self.p2p_data = [] # [ [[callpath, pid],[callpath, pid], time] ]
        self.coll_data = [] 
        self.readP2PCommDep()
        #self.readCollCommDep()

    def readP2PCommDep(self):
        p2p_comm_dep_file_name = self.comm_dep_dir + self.binary_name + ".cdp"
        with open(p2p_comm_dep_file_name) as f1:
            p2p_comm_dep_lines = f1.readlines()
        f1.close()
        for p2p_comm_dep_line in p2p_comm_dep_lines:
            p2p_comm_dep_split = p2p_comm_dep_line.strip().split(',')
            p2p_dest = p2p_comm_dep_split[0]
            p2p_dest_call_path_str = p2p_dest.strip().split('|')[1].strip().split(' ')
            p2p_dest_call_path = []
            for i in range(len(p2p_dest_call_path_str)):
                p2p_dest_call_path.append([p2p_dest_call_path_str[i]])
            p2p_dest_pid = int(p2p_dest.strip().split('|')[2])

            p2p_src = p2p_comm_dep_split[1]
            p2p_src_call_path_str = p2p_src.strip().split('|')[1].strip().split(' ')
            p2p_src_call_path = []
            for i in range(len(p2p_src_call_path_str)):
                p2p_src_call_path.append([p2p_src_call_path_str[i]])
            p2p_src_pid = int(p2p_src.strip().split('|')[2])
            
            p2p_weight = int(eval(p2p_comm_dep_split[2]))
            self.p2p_data.append([[p2p_src_call_path, p2p_src_pid], [p2p_dest_call_path, p2p_dest_pid], p2p_weight])


    def readCollCommDep(self):
        self.coll_data = [[] for i in range(self.nprocs)]

        for pid in range(self.nprocs):
            comm_dep_file_name = self.comm_dep_dir + "MPID" + str(pid) + ".TXT"

            with open(comm_dep_file_name) as f1:
                comm_dep_lines = f1.readlines()
            f1.close()

            dir_file_line_pointer = 0

            for comm_dep_line in comm_dep_lines:
                comm_dep_struct = comm_dep_line.strip().split('|')
                comm_dep_addr_str = comm_dep_struct[0]
                comm_dep_info = comm_dep_struct[1].strip()
                comm_dep_count = comm_dep_struct[2].strip()
                comm_dep_time = comm_dep_struct[3].strip()
                comm_dep_addrs = comm_dep_addr_str.strip().split(' ')[1:]
                comm_dep_type = comm_dep_addr_str.strip().split(' ')[0]
                while '' in comm_dep_addrs:
                    comm_dep_addrs.remove('')

                comm_dep_len = len(comm_dep_addrs)

                tmp_list = []
                for comm_dep_addr in comm_dep_addrs:
                    tmp_list.append([comm_dep_addr])

                self.coll_data[pid].append([tmp_list, comm_dep_type, comm_dep_count, comm_dep_time, comm_dep_info])