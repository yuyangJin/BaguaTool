import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import os
import json
import baguatool as bgt
import models as mds
import random
analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./linadvsl-main2d", func="conspatch_")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg", "asm"])

# set output data directory
analysis.setOutputDir(output_dir="linadvsl-baguatool-data")

# START ANALYSIS
# including static and dynamic parts
analysis.startAnalysis()

# OFFLINE PERFORMANCE ANALYSIS
# get program structure graph
psg = analysis.getProgramStructureGraph()

# get assembly instruction counts
asm_data = analysis.getAsembleInstructionData()

features = ["TOT_CYC", "LD_INS", "ST_INS", "L1_DCM", "TOT_INS"]#, "L2_DCM"]

nps = [1]
domain_boxes = [2048, 4096, 8192]
patch_sizes = [16, 32, 64, 128, 256, 512, 1024]

X_input_parameter = []
X_input_parameter_new = []
# get training performance data
for np in nps:
    for domain_box in domain_boxes:
        for patch_size in patch_sizes:
            X_input_parameter.append([domain_box, patch_size])
            X_input_parameter_new.append([domain_box, patch_size])
            perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size), nprocs = np, dyn_features = features)
            psg.performanceDataEmbedding(perf_data)

# graph contraction
psg.contraction()
psg.show()

domain_boxes = [4000, 6000, 8000, 10000, 12000]
patch_sizes = [40, 50, 100, 200, 400, 500]

# get validation performance data
for np in nps:
    for domain_box in domain_boxes:
        for patch_size in patch_sizes:
            X_input_parameter_new.append([domain_box, patch_size])
            perf_data = analysis.getPerformanceData( dir_suffix = "_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size), nprocs = np, dyn_features = features)
            psg.performanceDataEmbedding(perf_data)

domain_boxes = [2048, 4096, 8192]
patch_sizes = [16, 32, 64, 128, 256, 512, 1024]

def nodeModeling(node):
    # establish a hierarchical performance model for each hotspot node
    if (node.type_name == "LOOP" or node.type_name == "CALL" ) and node.performance_percentage > 0.0:

        # firstly, get train dataset (X = [domain_box, patch_size], Y = [L1_DCM, L2_DCM, L3_DCM, LST_INS])
        app_layer_train_dataset = []
        y_tot_cyc = []
        attr_loop_times_train_dataset = []
        attr_cache_charact_train_dataset = []
        #X_input_parameter = [] 
        for np in nps:
            for domain_box in domain_boxes:
                for patch_size in patch_sizes:
                    index = "_np" + str(np) + "-db" + str(domain_box) + "-ps" + str(patch_size)

                    for i in range(len(node.all_data[index][features[0]])):
                        y_tot_cyc.append(node.all_data[index][features[0]][i])
                    y_ld_ins = node.all_data[index][features[1]]
                    y_st_ins = node.all_data[index][features[2]]
                    y_l1_dcm = node.all_data[index][features[3]]
                    y_tot_ins = node.all_data[index][features[4]]

                    total_asm_ins = 1
                    if node.type_name == "LOOP":
                        total_asm_ins = sum(node.inst_count)
                        for i in range(len(y_l1_dcm)):
                            attr_loop_times_train_dataset.append(y_tot_ins[i] / total_asm_ins)
                            attr_cache_charact_train_dataset.append( y_l1_dcm[i] / (100 * max(1, (y_ld_ins[i] + y_st_ins[i]))))

        # establish black-box application-related performance models
        if node.type_name == "LOOP":
            node.app_model_1, node.pred_1 = mds.multivariatePolynomialFitting(X_input_parameter, attr_loop_times_train_dataset, X_input_parameter_new)
            node.app_model_2, node.pred_2 = mds.multivariatePolynomialFitting(X_input_parameter, attr_cache_charact_train_dataset, X_input_parameter_new)
        elif node.type_name == "CALL":
            node.app_model_1, node.pred_1 = mds.multivariatePolynomialFitting(X_input_parameter, y_tot_cyc, X_input_parameter_new)

        # output actual performance (time) of all data
        y_tot_cyc_1 = []
        for index in list(node.all_data.keys()):
            for i in range(len(node.all_data[index][features[0]])):
                y_tot_cyc_1.append(node.all_data[index][features[0]][i])
        print(node.unique_id, node.type_name, X_input_parameter, X_input_parameter_new, y_tot_cyc_1)

def platform_predict(ECM, node):
    predicted_time = []
    for i in range(len(list(node.pred_1))):
        predicted_time.append(max(0, 15 * list(node.pred_1)[i] * 1e7 / (2.3*1e9) * (list(node.pred_2)[i] * ECM[0] + (1 - list(node.pred_2)[i]) * ECM[3])))
    return predicted_time

def nodePerformancePrediction(node):
    # predict performance for each hotspot node
    if (node.type_name == "LOOP" ) and node.performance_percentage > 0.0:
        node.predicted_time = platform_predict(node.ECM, node)
        print(node.unique_id, node.type_name, node.predicted_time) 
    #if (node.type_name == "CALL") and node.performance_percentage > 0.0:
    #    node.predicted_time = list(node.pred_1)
    #    print(node.unique_id, node.type_name, node.predicted_time) 

# it is a graph-based algorithm to predict program's performance with node's predicted performance
def programPerformancePrediction(node):
    total_time = []
    if (node.type_name == "LOOP" or node.type_name == "CALL" ) and node.performance_percentage > 0.0:
        total_time = node.predicted_time

    # recursively merge node's predicted performance
    for child in node.children:
        tmp_total_time = programPerformancePrediction(child)
        if total_time == []:
            total_time = tmp_total_time
        else:
            for i in range(len(tmp_total_time)):
                total_time[i] += tmp_total_time[i]

    return total_time

#################################################
# Graph-based Hierarchical Performance Modeling #
#################################################

# First, use unique id of hotspot function to get this "node"
func_node = bgt.getNodeWithUniqueId(psg.main_root, 910754, random.randint(0,100000))
print(func_node.type_name, func_node.name)

# Second, establish white-box platform-related performance models
psg.asembleInstructionDataEmbedding(asm_data, func_node)
psg.buildECMModel(func_node)

# Third, establish black-box application-related performance models of "node"'s children and children's children and ...
psg.BFS(func_node, nodeModeling)

# Forth, predict performance of "node"'s children and children's children and ...
psg.BFS(func_node, nodePerformancePrediction)

# Finally, use graph-based algorithm to gather all children's predicted performance to get predicted performance of program or a certain node
program_time = programPerformancePrediction(func_node)
print(program_time)