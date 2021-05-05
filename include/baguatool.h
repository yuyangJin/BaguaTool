#ifndef BAGUATOOL_H_
#define BAGUATOOL_H_
#include <fstream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <stack>
#include <string>
#include <vector>
#include "../src/common/tprintf.h"

using namespace nlohmann;

// All public APIs are defined here.

namespace baguatool {

namespace core {

class graph_t;
class vertex_set_t;
typedef int vertex_t;
typedef int edge_t;

class Graph {
 protected:
  std::unique_ptr<graph_t> ipag_;
  int cur_vertex_num;

 public:
  Graph();
  ~Graph();
  void GraphInit(const char* graph_name);
  vertex_t AddVertex();
  void SwapVertex(vertex_t vertex_id_1, vertex_t vertex_id_2);
  edge_t AddEdge(const vertex_t src_vertex_id, const vertex_t dest_vertex_id);
  void AddGraph(Graph* g);

  void DeleteVertex(vertex_t vertex_id);
  void DeleteEdge(vertex_t src_id, vertex_t dest_id);

  void QueryVertex();
  int QueryEdge(vertex_t, vertex_t);

  int GetEdgeSrc(edge_t edge_id);
  int GetEdgeDest(edge_t edge_id);
  void QueryEdgeOtherSide();

  void SetGraphAttributeString(const char* attr_name, const char* value);
  void SetGraphAttributeNum(const char* attr_name, const int value);
  void SetGraphAttributeFlag(const char* attr_name, const bool value);
  void SetVertexAttributeString(const char* attr_name, vertex_t vertex_id, const char* value);
  void SetVertexAttributeNum(const char* attr_name, vertex_t vertex_id, const int value);
  void SetVertexAttributeFlag(const char* attr_name, vertex_t vertex_id, const bool value);
  void SetEdgeAttributeString(const char* attr_name, edge_t edge_id, const char* value);
  void SetEdgeAttributeNum(const char* attr_name, edge_t edge_id, const int value);
  void SetEdgeAttributeFlag(const char* attr_name, edge_t edge_id, const bool value);

  const char* GetGraphAttributeString(const char* attr_name);
  const int GetGraphAttributeNum(const char* attr_name);
  const bool GetGraphAttributeFlag(const char* attr_name);
  const char* GetVertexAttributeString(const char* attr_name, vertex_t vertex_id);
  const int GetVertexAttributeNum(const char* attr_name, vertex_t vertex_id);
  const bool GetVertexAttributeFlag(const char* attr_name, vertex_t vertex_id);
  const char* GetEdgeAttributeString(const char* attr_name, edge_t edge_id);
  const int GetEdgeAttributeNum(const char* attr_name, edge_t edge_id);
  const bool GetEdgeAttributeFlag(const char* attr_name, edge_t edge_id);

  void RemoveGraphAttribute(const char* attr_name);
  void RemoveVertexAttribute(const char* attr_name);
  void RemoveEdgeAttribute(const char* attr_name);

  void MergeVertices();
  void SplitVertex();
  void DeepCopyVertex(vertex_t new_vertex_id, Graph* g, vertex_t vertex_id);
  void CopyVertex(vertex_t new_vertex_id, Graph* g, vertex_t vertex_id);
  // TODO: do not expose inner igraph
  void DeleteVertices(vertex_set_t* vs);
  void DeleteExtraTailVertices();
  void Dfs();
  void ReadGraphGML(const char* file_name);
  void DumpGraph(const char* file_name);
  void DumpGraphDot(const char* file_name);
  int GetCurVertexNum();
  void VertexTraversal(void (*CALL_BACK_FUNC)(Graph*, vertex_t, void*), void* extra);
  void GetChildVertexSet(vertex_t, std::vector<vertex_t>&);
  void PreOrderTraversal(vertex_t root, std::vector<vertex_t>& pre_order_vertex_vec);
};

class ProgramGraph : public Graph {
 private:
 public:
  ProgramGraph();
  ~ProgramGraph();
  int SetVertexBasicInfo(const vertex_t vertex_id, const int vertex_type, const char* vertex_name);
  int SetVertexDebugInfo(const vertex_t vertex_id, const int entry_addr, const int exit_addr);
  int GetVertexType(vertex_t);
  vertex_t GetChildVertexWithAddr(vertex_t root_vertex, unsigned long long addr);
  vertex_t GetVertexWithCallPath(vertex_t, std::stack<unsigned long long>&);
  vertex_t GetCallVertexWithAddr(unsigned long long addr);
  vertex_t GetFuncVertexWithAddr(unsigned long long addr);
  int AddEdgeWithAddr(unsigned long long call_addr, unsigned long long callee_addr);
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramGraph*, int, void*), void* extra);
  const char* GetCalleeVertex(vertex_t);
  void VertexSortChild();
};

class ProgramAbstractionGraph : public ProgramGraph {
 private:
 public:
  ProgramAbstractionGraph();
  ~ProgramAbstractionGraph();
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph*, int, void*), void* extra);
};

class ControlFlowGraph : public ProgramGraph {
 private:
 public:
  ControlFlowGraph();
  ~ControlFlowGraph();
};

class ProgramCallGraph : public ProgramGraph {
 private:
 public:
  ProgramCallGraph();
  ~ProgramCallGraph();
};

typedef struct SAMPLER_STRUCT SaStruct;

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 256
#endif

typedef unsigned long long int addr_t;
typedef double perf_data_t;

class PerfData {
 private:
  SaStruct* sampler_data = nullptr;
  unsigned long int sampler_data_space_size = 0;
  unsigned long int sampler_data_size = 0;
  FILE* sampler_data_fp = nullptr;
  std::ifstream sampler_data_in;
  bool has_open_output_file = false;
  char file_name[MAX_LINE_LEN] = {0};
  // TODO: design a method to make metric_name portable
  std::string metric_name = std::string("TOT_CYC");

 public:
  PerfData();
  ~PerfData();
  int Query(addr_t* call_path, int call_path_len, int process_id, int thread_id);
  void Record(addr_t* call_path, int call_path_len, int process_id, int thread_id, perf_data_t count);
  void Read(const char*);
  void Dump(const char*);
  unsigned long int GetSize();
  void SetMetricName(std::string& metric_name);
  std::string& GetMetricName();
  void GetCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  perf_data_t GetSamplingCount(unsigned long int data_index);
  int GetProcessId(unsigned long int data_index);
  int GetThreadId(unsigned long int data_index);
};

class GraphPerfData {
 private:
  json j_perf_data;

 public:
  GraphPerfData();
  ~GraphPerfData();

  void Read(std::string&);
  void Dump(std::string&);

  void SetPerfData(vertex_t vertex_id, std::string& metric, int process_id, int thread_id, perf_data_t value);
  perf_data_t GetPerfData(vertex_t vertex_id, std::string& metric, int process_id, int thread_id);

  bool HasMetric(vertex_t vertex_id, std::string& metric);
  void GetVertexPerfDataMetrics(vertex_t, std::vector<std::string>&);
  int GetMetricsPerfDataProcsNum(vertex_t vertex_id, std::string& metric);
  int GetProcsPerfDataThreadNum(vertex_t vertex_id, std::string& metric, int process_id);
  void GetProcsPerfData(vertex_t vertex_id, std::string& metric, int process_id,
                        std::vector<perf_data_t>& proc_perf_data);

};  // class GraphPerfData

class HybridAnalysis {
 private:
  std::map<std::string, ControlFlowGraph*> func_cfg_map;
  ProgramCallGraph* pcg;
  std::map<std::string, ProgramAbstractionGraph*> func_pag_map;
  ProgramAbstractionGraph* root_pag;
  ProgramAbstractionGraph* root_mpag;
  GraphPerfData* graph_perf_data;

 public:
  HybridAnalysis();
  ~HybridAnalysis();

  /** Control Flow Graph of Each Function **/

  void ReadStaticControlFlowGraphs(const char* dir_name);
  void GenerateControlFlowGraphs(const char* dir_name);
  ControlFlowGraph* GetControlFlowGraph(std::string func_name);
  std::map<std::string, ControlFlowGraph*>& GetControlFlowGraphs();

  /** Program Call Graph **/

  void ReadStaticProgramCallGraph(const char* static_pcg_file_name);
  void ReadDynamicProgramCallGraph(std::string perf_data_file_name);
  void GenerateProgramCallGraph(const char*);
  ProgramCallGraph* GetProgramCallGraph();

  /** Intra-procedural Analysis **/

  ProgramAbstractionGraph* GetFunctionAbstractionGraph(std::string func_name);
  std::map<std::string, ProgramAbstractionGraph*>& GetFunctionAbstractionGraphs();
  void IntraProceduralAnalysis();
  void ReadFunctionAbstractionGraphs(const char* dir_name);

  /** Inter-procedural Analysis **/

  void InterProceduralAnalysis();
  void GenerateProgramAbstractionGraph();
  void SetProgramAbstractionGraph(ProgramAbstractionGraph*);
  ProgramAbstractionGraph* GetProgramAbstractionGraph();

  /** DataEmbedding **/
  void DataEmbedding(PerfData*);
  GraphPerfData* GetGraphPerfData();
  perf_data_t ReduceVertexPerfData(std::string& metric, std::string& op);
  void ConvertVertexReducedDataToPercent(std::string& metric, perf_data_t total, std::string& new_metric);

  void GenerateMultiProgramAbstractionGraph();
  ProgramAbstractionGraph* GetMultiProgramAbstractionGraph();

  void PthreadAnalysis(PerfData* pthread_data);

};  // class HybridAnalysis

}  // namespace core

namespace graph_perf {

class Preprocess {
 private:
 public:
  Preprocess() {}
  ~Preprocess() {}

  void ReadFunctionGraphs(const char* dir_name, std::vector<core::ProgramAbstractionGraph*>& func_pag_vec);

  core::ProgramAbstractionGraph* InterProceduralAnalysis(std::vector<core::ProgramAbstractionGraph*>& func_pag_vec);

  // void ConnectCallerCallee(ProgramAbstractionGraph* pag, int vertex_id, void* extra);
};
}  // namespace graph_perf

namespace graph_sd {

class StaticAnalysisImpl;
class StaticAnalysis {
 private:
  std::unique_ptr<StaticAnalysisImpl> sa;

 public:
  StaticAnalysis(char* binary_name);

  ~StaticAnalysis();
  void IntraProceduralAnalysis();
  void InterProceduralAnalysis();
  void CaptureProgramCallGraph();
  void DumpAllControlFlowGraph();
  void DumpProgramCallGraph();
  void GetBinaryName();
};  // class StaticAnalysis

typedef unsigned long long int addr_t;

class LongLongVec;
class SamplerImpl;
class Sampler {
 private:
  std::unique_ptr<SamplerImpl> sa;

 public:
  Sampler();
  ~Sampler();

  void SetSamplingFreq(int freq);
  void Setup();
  void AddThread();
  void RemoveThread();
  void UnsetOverflow();
  void SetOverflow(void (*FUNC_AT_OVERFLOW)(int));
  void Start();
  void Stop();
  int GetOverflowEvent(LongLongVec* overflow_vector);
  int GetBacktrace(addr_t* call_path, int max_call_path_depth);
};  // class Sampler

// static void* resolve_symbol(const char* symbol_name, int config);

}  // namespace graph_sd
}  // namespace baguatool

#endif