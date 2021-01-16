import os
import time

from perfdata import *
from psg import *
from asemble import *
from commdep import *
from ppg import *
import json
#from models import *


class Baguatool(object):
    def __init__(self):
        self.analysis_mode = "init"
        self.static_analysis_mode = "init"
        self.binary_name = "init"
        self.funcs = []
        self.static_analysis_feature = []
        self.sampling_count = 1000

        self.dynamic_analysis_mode = "init"
        self.save_file = ""
        self.command_lines = []
        self.output_file_suffix = []
        self.output_dir = ""
        #print("Baguatool init")

    def setAnalysisMode(self, analysis_mode):
        if analysis_mode != "static" and analysis_mode != "dynamic" and analysis_mode != "static+dynamic":
            print("Unknown analysis mode")
            return
        self.analysis_mode = analysis_mode
        print(self.analysis_mode)

    def setStaticAnalysisMode(self, mode="", binary=""):
        if self.analysis_mode != "static" and self.analysis_mode != "static+dynamic":
            print("Please set analysis mode as 'static' or 'static+dynamic' ")
            return
        if mode != "binary" and mode != "src":
            print("Unknown static analysis mode")
            return
        self.static_analysis_mode = mode
        self.binary_name = binary
    
    def setStaticAnalysisFeature(self, features=["psg"], funcs = []):
        if self.analysis_mode != "static" and self.analysis_mode != "static+dynamic":
            print("Please set analysis mode as 'static' or 'static+dynamic' ")
            return
        self.static_analysis_feature = features
        self.funcs = funcs


    def setOutputDir(self, output_dir=""):
        if output_dir != "":
            self.output_dir = output_dir
        else:
            now = int(round(time.time() * 1000))
            nowstr = time.strftime('%Y%m%d-%H%M%S', time.localtime(now / 1000))
            self.output_dir = "baguatool-data-" + self.binary_name.strip().split('/')[-1] + "-" + nowstr + ""


    def startAnalysis(self):
        if self.analysis_mode == "init":
            print("Please set analysis mode")
        elif self.analysis_mode == "static":
            self.startStaticAnalysis()
        elif self.analysis_mode == "dynamic":
            self.startDynamicAnalysis()
        elif self.analysis_mode == "static+dynamic":
            self.startStaticAnalysis()
            self.startDynamicAnalysis()
        else:
            print("Unknown analysis mode")

    def startStaticAnalysis(self):
        static_data_mkdir_cmd = "mkdir -p ./" + self.output_dir + "/static_data"
        os.system(static_data_mkdir_cmd)
        if self.static_analysis_mode == "init":
            print("Please set static analysis mode")
        elif self.static_analysis_mode == "binary":
            self.staticBinaryAnalysis()
        elif self.static_analysis_mode == "src":
            self.staticSrcAnalysis()
        else:
            print("Unknown static analysis mode")

    def staticBinaryAnalysis(self):
        if "psg" in self.static_analysis_feature:
            # Execute binary analysis commands
            binary_analysis_psg_cmd = "$BAGUA_DIR/bin/psg-addr " + self.binary_name 
            os.system(binary_analysis_psg_cmd)
            psg_mv_cmd = "mv " + self.binary_name + ".psg ./" + self.output_dir + "/static_data/"
            os.system(psg_mv_cmd)
            #print(self.static_analysis_feature)
        if "asm" in self.static_analysis_feature:
            binary_analysis_asm_cmd = "$BAGUA_DIR/bin/instruction_psg_counter " + self.binary_name
            for func in self.funcs:
                binary_analysis_asm_cmd += " \"" + func + "\""
            os.system(binary_analysis_asm_cmd)
            asm_mv_cmd = "mv " + self.binary_name + ".asm ./" + self.output_dir + "/static_data/"
            os.system(asm_mv_cmd)
        #print("staticBinaryAnalysis")

    def staticSrcAnalysis(self):
        # Makefile transformation

        # Execute src analysis commands
        print("staticSrcAnalysis")

    def setDynamicAnalysisMode(self, dynamic_analysis_mode, sampling_freq=100):
        if self.analysis_mode != "dynamic" and self.analysis_mode != "static+dynamic":
            print("Please set analysis mode as 'dynamic' or 'static+dynamic' ")
            return
        if dynamic_analysis_mode != "sampling" and dynamic_analysis_mode != "instrumentation" and dynamic_analysis_mode != "commdep":
            print("Unknown dynamic analysis mode")
            return
        self.dynamic_analysis_mode = dynamic_analysis_mode
        self.sampling_count = 2.3 * 1e9 / sampling_freq

    def setExecutionCommand(self, cmd="", output_file_suffix="", save_file=""):
        self.save_file = save_file
        if cmd != "":
            self.command_lines.append(cmd)
            self.output_file_suffix.append(output_file_suffix)

    

    def startDynamicAnalysis(self):
        dyn_data_mkdir_cmd = "mkdir -p ./" + self.output_dir + "/dynamic_data"
        os.system(dyn_data_mkdir_cmd)
        if self.dynamic_analysis_mode == "init":
            print("Please set dynamic analysis mode")
        elif self.dynamic_analysis_mode == "sampling":
            self.dynamicSamplingAnalysis()
        elif self.dynamic_analysis_mode == "instrumentation":
            self.dynamicInstrumentationAnalysis()
        elif self.dynamic_analysis_mode == "commdep":
            self.dynamicCommDepAnalysis()
        else:
            print("Unknown dynamic analysis mode")

    def dynamicSamplingAnalysis(self):
        #
        #sampling_count = 1000
        index = 0
        for command_line in self.command_lines:
            sampling_cmd = "SAMPLE_COUNT=" + \
               str(int(self.sampling_count)) + \
               " CYC_SAMPLE_COUNT=23000000 INS_SAMPLE_COUNT=10000000 CM_SAMPLE_COUNT=100000 LD_PRELOAD=$BAGUA_DIR/bin/libdynco_multiPMU_lib_omp.so " + command_line
            # sampling_cmd = "SAMPLE_COUNT=" + \
            #     str(int(self.sampling_count)) + \
            #     " LD_PRELOAD=$BAGUA_DIR/bin/libdynco.so " + command_line
            print(sampling_cmd)
            os.system(sampling_cmd)
            #sampling_cmd = "SAMPLE_COUNT=" + \
            #   str(int(self.sampling_count)) + \
            #   " CYC_SAMPLE_COUNT=23000000 INS_SAMPLE_COUNT=10000000 CM_SAMPLE_COUNT=100000 LD_PRELOAD=$BAGUA_DIR/bin/libdynco_multiPMU_2.so " + command_line
            #print(sampling_cmd)
            #os.system(sampling_cmd)
            perf_data_dir = "./" + self.output_dir + "/dynamic_data/" + self.output_file_suffix[index]
            perf_data_file_mkdir_cmd = "mkdir -p " + perf_data_dir
            perf_data_file_mv_cmd = "mv SAMPLE* " + perf_data_dir
            ldmap_file_mv_cmd = "mv LDMAP* " + perf_data_dir
            print(perf_data_file_mkdir_cmd)
            os.system(perf_data_file_mkdir_cmd)
            print(perf_data_file_mv_cmd)
            os.system(perf_data_file_mv_cmd)
            print(ldmap_file_mv_cmd)
            os.system(ldmap_file_mv_cmd)
            if self.save_file != "":
                save_file_mv_cmd = "mv " + self.save_file + " " + perf_data_dir
                print(save_file_mv_cmd)
                os.system(save_file_mv_cmd)
            index += 1
    
    def dynamicCommDepAnalysis(self):
        index = 0
        for command_line in self.command_lines:
            commdep_cmd = "LD_PRELOAD=$BAGUA_DIR/bin/libcommdep.so " + command_line
            print(commdep_cmd)
            os.system(commdep_cmd)

            comm_dep_dir = "./" + self.output_dir + "/dynamic_data/" + self.output_file_suffix[index]
            comm_dep_file_mkdir_cmd = "mkdir -p " + comm_dep_dir
            comm_dep_file_mv_cmd = "mv MPI* " + comm_dep_dir

            print(comm_dep_file_mkdir_cmd)
            os.system(comm_dep_file_mkdir_cmd)
            print(comm_dep_file_mv_cmd)
            os.system(comm_dep_file_mv_cmd)

            if self.save_file != "":
                save_file_mv_cmd = "mv " + self.save_file + " " + comm_dep_dir
                print(save_file_mv_cmd)
                os.system(save_file_mv_cmd)
            index += 1

    def dynamicInstrumentationAnalysis(self):
        for func in self.funcs:
            instrument_cmd = "$BAGUA_DIR/bin/loop_instrument " + \
                self.binary_name + " " + func
            os.system(instrument_cmd)
            self.execution_binary_name = "newbinary"
        # print("dynamicInstrumentationAnalysis")

    def getProgramStructureGraph(self):
        # Read program structure graph from [binary].psg
        psg_file = "./" + self.output_dir + "/static_data/" + self.binary_name.split("/")[-1] + ".psg"
        self.psg = PSG(psg_file = psg_file)
        print("Already read " + psg_file)
        return self.psg

    def getAsembleInstructionData(self):
        # Read program structure graph from [binary].asm
        asm_file = "./" + self.output_dir + "/static_data/" + self.binary_name + ".asm"
        self.asm_data = asembleInstructionData(asm_file, self.funcs)
        return self.asm_data

    def getPerformanceData(self, dir_suffix, nprocs, dyn_features):
        # Read performance data from SAMPLE[pid].TXT
        perf_data = perfData(self.binary_name, "./" + self.output_dir + "/dynamic_data/", dir_suffix, nprocs, dyn_features)
        return perf_data
 
    def getCommDepData(self, dir_suffix, nprocs):
        cd_enter_cmd = "cd " + "./" + self.output_dir + "/dynamic_data/" + dir_suffix 
        comm_dep_edge_analysis_cmd = "$BAGUA_DIR/bin/commDepDetectApproxi " + str(nprocs) + " " + self.binary_name.strip().split('/')[-1] + ".cdp"
        cd_back_cmd = "cd - "
        print(cd_enter_cmd, comm_dep_edge_analysis_cmd, cd_back_cmd)
        print(cd_enter_cmd, os.system(cd_enter_cmd))
        print(comm_dep_edge_analysis_cmd, os.system(comm_dep_edge_analysis_cmd))
        print(cd_back_cmd, os.system(cd_back_cmd))

        comm_dep_data = commDep("./" + self.output_dir + "/dynamic_data/", self.binary_name.strip().split('/')[-1], dir_suffix, nprocs)
        return comm_dep_data

    def transferToProgramPerformanceGraph(self, psg, nprocs = 1):
        ppg = PPG(psg, nprocs)
        return ppg


    def getProgramStructureGraphFromJsonFile(self, nodes_file="", edges_file=""):
        nodes = {}
        edges = {}
        with open(nodes_file, 'r') as input_file: 
            nodes = json.load(input_file)
        #input_file.close()
        with open(edges_file, 'r') as input_file: 
            edges = json.load(input_file)
        #input_file.close()
        self.psg = PSG(nodes=nodes, edges=edges)
        print("Already read " + nodes_file + " and " + edges_file)
        return self.psg