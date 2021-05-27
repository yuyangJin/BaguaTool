#ifndef GRAPHPERF_H_
#define GRAPHPERF_H_
#include <map>
#include <string>
#include "baguatool.h"
#include "core/cfg.h"
#include "core/pag.h"
#include "core/pcg.h"

// class GPerf {
//  private:
//   std::map<std::string, core::ControlFlowGraph*> func_cfg_map; /**<control-flow graphs for each function*/
//   core::ProgramCallGraph* pcg;                                 /**<program call graph*/
//   std::map<std::string, core::ProgramAbstractionGraph*>
//       func_pag_map; /**<program abstraction graph extracted from control-flow graph (CFG) for each function */
//   core::ProgramAbstractionGraph* root_pag;  /**<an overall program abstraction graph for a program */
//   core::ProgramAbstractionGraph* root_mpag; /**<an overall multi-* program abstraction graph for a parallel program*/
//   core::GraphPerfData* graph_perf_data;     /**<performance data in a graph*/

//  public:
//   /** Constructor.
//    */
//   GPerf();
//   /** Destructor.
//    */
//   ~GPerf();

//   /** Control Flow Graph of Each Function **/

//   void ReadStaticControlFlowGraphs(const char* dir_name);
//   void GenerateControlFlowGraphs(const char* dir_name);
//   core::ControlFlowGraph* GetControlFlowGraph(std::string func_name);
//   std::map<std::string, core::ControlFlowGraph*>& GetControlFlowGraphs();

//   /** Program Call Graph **/

//   void ReadStaticProgramCallGraph(const char* static_pcg_file_name);
//   void ReadDynamicProgramCallGraph(std::string perf_data_file_name);
//   void GenerateProgramCallGraph(const char*);
//   core::ProgramCallGraph* GetProgramCallGraph();

//   /** Intra-procedural Analysis **/

//   core::ProgramAbstractionGraph* GetFunctionAbstractionGraph(std::string func_name);
//   std::map<std::string, ProgramAbstractionGraph*>& GetFunctionAbstractionGraphs();
//   void IntraProceduralAnalysis();
//   void ReadFunctionAbstractionGraphs(const char* dir_name);

//   /** Inter-procedural Analysis **/

//   void InterProceduralAnalysis();
//   void GenerateProgramAbstractionGraph();
//   void SetProgramAbstractionGraph(core::ProgramAbstractionGraph*);
//   core::ProgramAbstractionGraph* GetProgramAbstractionGraph();

//   /** DataEmbedding **/
//   void DataEmbedding(core::PerfData*);
//   core::GraphPerfData* GetGraphPerfData();
//   type::perf_data_t ReduceVertexPerfData(std::string& metric, std::string& op);
//   void ConvertVertexReducedDataToPercent(std::string& metric, perf_data_t total, std::string& new_metric);

//   void GenerateMultiProgramAbstractionGraph();
//   ProgramAbstractionGraph* GetMultiProgramAbstractionGraph();

//   void PthreadAnalysis(PerfData* pthread_data);

// }

#endif  // GRAPHPERF_H_
