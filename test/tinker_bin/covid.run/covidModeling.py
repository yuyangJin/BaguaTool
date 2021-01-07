import sys
sys.path.append(r"/home/jinyuyang/workspace/BaguaTool/src/Baguatool")
import baguatool as bgt

analysis = bgt.Baguatool()

# SETUP
# set analysis mode as "static" / "dynamic" / "static+dynamic"
analysis.setAnalysisMode("static+dynamic")

# set static analysis mode as "binary" (binary analysis) / "src" (source code analysis)
analysis.setStaticAnalysisMode(mode="binary", binary="./dynamic")

# set static analysis feature as "psg" (program structure graph) / "asm" (asemble instruction analysis)
analysis.setStaticAnalysisFeature(features=["psg"])

# set dynamic analysis mode as "sampling" / "instrumentation"
# "sampling_freq" is sampling times per second
analysis.setDynamicAnalysisMode("sampling", sampling_freq=100)

nps = [16]
for np in nps:
    # set commands for execution
    # # set suffix of output file for performance recording
    # np = 4
    #analysis.setExecutionCommand(cmd="mpirun -np " + str(np) + " ./moleculardynamics-main2d ./moleculardynamics-2d.input", output_file_suffix="_np-" + str(np))
    analysis.setExecutionCommand(cmd="srun -n " + str(np) + " -w gorgon[7,8] ./dynamic stmv 100 2.0 1.0 2 300", output_file_suffix="_np-" + str(np))

# set
#analysis.setOutputDir(output_dir="moleculardynamics-baguatool-data")
analysis.setOutputDir(output_dir="stmv-baguatool-data")


# START ANALYSIS
# including static and dynamic parts
# analysis.startAnalysis()

# OFFLINE PERFORMANCE ANALYSIS
# get program structure graph
psg = analysis.getProgramStructureGraph()

# get performance data
#np = 1
perf_data = analysis.getPerformanceData("_np-" + str(np), 1, dyn_features = ["TOT_CYC"])
# perf_data.show()
psg.performanceDataEmbedding(perf_data)

# get communication data
#comm_data = analysis.getCommunicationData()
# analysis.get

# graph contraction
psg.contraction()

psg.show()
