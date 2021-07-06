#ifndef GRAPHPERF_H_
#define GRAPHPERF_H_
#include <stdlib.h>
#include <map>
#include <string>
#include "baguatool.h"
#include "common.h"
#include "utils.h"

namespace graph_perf {

using namespace baguatool;

class GPerf {
 private:
  std::map<std::string, core::ControlFlowGraph*> func_cfg_map; /**<control-flow graphs for each function*/
  core::ProgramCallGraph* pcg;                                 /**<program call graph*/
  std::map<std::string, core::ProgramAbstractionGraph*>
      func_pag_map; /**<program abstraction graph extracted from control-flow graph (CFG) for each function */
  core::ProgramAbstractionGraph* root_pag;  /**<an overall program abstraction graph for a program */
  core::ProgramAbstractionGraph* root_mpag; /**<an overall multi-* program abstraction graph for a parallel program*/

 public:
  /** Constructor.
   */
  GPerf();
  /** Destructor.
   */
  ~GPerf();

  /** Read static control-flow graph of each function (Input)
   * @param dir_name - dictory name of all functions' CFG
   */
  void ReadStaticControlFlowGraphs(const char* dir_name);

  /** Geneate control-flow graph of each function through hybrid static-dynamic analysis. Dynamic module is disable.
   * @param dir_name - dictory name of all functions' CFG
   */
  void GenerateControlFlowGraphs(const char* dir_name);

  /** Get control-flow graph of a specific function.
   * @param func_name - function name
   * @return CFG of a specific function
   */
  core::ControlFlowGraph* GetControlFlowGraph(std::string func_name);

  /** Get control-flow graphs of all functions.
   * @return CFGs of all functions
   */
  std::map<std::string, core::ControlFlowGraph*>& GetControlFlowGraphs();

  /** Program Call Graph **/

  /** Read static program call graph from an input file.
   * @param file_name - file name of static program call graph
   */
  void ReadStaticProgramCallGraph(const char* file_name);

  /** Read dynamic program call graph. This function includes two phases: indirect call relationship analysis, as well
   * as pthread_create and its created function.
   * @param perf_data - performance data
  */
  void ReadDynamicProgramCallGraph(core::PerfData* perf_data);

  /** Generate complete program call graph through hybrid static-dynamic analysis.
   * @param binary_name - binary name
   * @param perf_data - performance data
   */
  void GenerateProgramCallGraph(const char* binary_name, core::PerfData* perf_data);

  /** Get complete program call graph.
   * @return complete program call graph
   */
  core::ProgramCallGraph* GetProgramCallGraph();

  /** Intra-procedural Analysis **/

  /** Perform intra-procedural analysis to abstract program structure from control-flow graph.
   * Not be implemented yet.
  */
  void IntraProceduralAnalysis();

  /** Read function abstraction graphs of all functions in a program from a directory.
   * @param dir_name - name of directory
  */
  void ReadFunctionAbstractionGraphs(const char* dir_name);

  /** Get function abstraction graph of a specific function.
   * @param func_name - name of a specific function
   * @return fucntion abstraction graph of the specific function
  */
  core::ProgramAbstractionGraph* GetFunctionAbstractionGraph(std::string func_name);

  /** Get function abstraction graph by address. (entry address of this function <= input address <= exit address of
   * this function)
   * @param addr - input address for identification
   * @return program abstraction graph of the identified function
  */
  core::ProgramAbstractionGraph* GetFunctionAbstractionGraphByAddr(type::addr_t addr);

  /** Get function abstraction graphs of all functions.
   * @return a map of function names and corresponding fucntion abstraction graphs
  */
  std::map<std::string, core::ProgramAbstractionGraph*>& GetFunctionAbstractionGraphs();

  /** Inter-procedural Analysis **/

  /** Perform dynamic inter-procedural analysis. Add pag of created function to pthread_create vertex.
   * @param pthread_data - data that contains pthread-related information
  */
  void DynamicInterProceduralAnalysis(core::PerfData* pthread_data);

  /** Perform inter-procedural analysis. Add pag of callee function to call vertex.
   * @param perf_data - performance data that contains runtime call relationship
  */
  void InterProceduralAnalysis(core::PerfData* perf_data);

  void StaticInterProceduralAnalysis();

  /** Generate an overall program abstraction graph through intra-procedural analysis and inter-procedural analysis.
   * param binary_name - name of binary for static analysis
   * @param perf_data - file name of performance data for dynamic analysis
  */
  void GenerateProgramAbstractionGraph(core::PerfData* perf_data);

  void GenerateStaticProgramAbstractionGraph();

  /** Set a input pag as program abstraction graph. This function is for reusing pag after intra-procedural and
   * inter-procedural analysis.
   * @param pag - input program abstraction graph
  */
  void SetProgramAbstractionGraph(core::ProgramAbstractionGraph* pag);

  /** Get program abstraction graph.
   * @return program abstraction graph
  */
  core::ProgramAbstractionGraph* GetProgramAbstractionGraph();

  /** DataEmbedding **/

  /** Get corresponding vertex through inter-thread analysis.
   * @param thread_id - thread id of a input call path
   * @param call_path - call path
   * @return id of the corresponding vertex of the input call path
  */
  type::vertex_t GetVertexWithInterThreadAnalysis(int thread_id, type::call_path_t& call_path);

  /** Embed data to graph.
   * @param perf_data - performance data
  */
  void DataEmbedding(core::PerfData* perf_data);

  // /** Get performance data on the graph (GraphPerfData)
  //  * @return GraphPerfData
  //  */
  // core::GraphPerfData* GetGraphPerfData();

  void GenerateMultiThreadProgramAbstractionGraph();

  void GenerateMultiProcessProgramAbstractionGraph(int num_procs);

  /** Generate multi-thread or multi-process program abstraction graph.
   *
  */
  void GenerateMultiProgramAbstractionGraph();

  /** Get multi-thread or multi-process program abstraction graph.
   * @return multi-thread or multi-process program abstraction graph
  */
  core::ProgramAbstractionGraph* GetMultiProgramAbstractionGraph();
};

}  // namespace graph_perf

#endif  // GRAPHPERF_H_
