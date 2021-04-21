#ifndef BAGUATOOL_H_
#define BAGUATOOL_H_
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include "../src/common/tprintf.h"

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
  edge_t AddEdge(const vertex_t src_vertex_id, const vertex_t dest_vertex_id);
  void AddGraph(Graph* g);

  void DeleteVertex();
  void DeleteEdge();

  void QueryVertex();
  int QueryEdge(vertex_t, vertex_t);

  int GetEdgeSrc(edge_t edge_id);
  int GetEdgeDest(edge_t edge_id);
  void QueryEdgeOtherSide();

  void SetVertexAttribute();
  void SetEdgeAttribute();
  void GetVertexAttribute();

  int GetVertexAttributeNum(const char* attr_name, vertex_t vertex_id);
  const char* GetVertexAttributeString(const char* attr_name, vertex_t vertex_id);
  void GetEdgeAttribute();

  const char* GetGraphAttributeString(const char* attr_name);

  void MergeVertices();
  void SplitVertex();
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
  std::vector<vertex_t> GetChildVertexSet(vertex_t);
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
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramGraph *, int, void *), void *extra);
};

class ProgramAbstractionGraph : public ProgramGraph {
 private:
 public:
  ProgramAbstractionGraph();
  ~ProgramAbstractionGraph();
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *), void *extra);
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

class PerfData {
 private:
  SaStruct* sampler_data = nullptr;
  unsigned long int sampler_data_size = 0;
  FILE* sampler_data_fp = nullptr;
  std::ifstream sampler_data_in;
  bool has_open_output_file = false;
  char* file_name = nullptr;

 public:
  PerfData();
  ~PerfData();
  // int SetAttribute();
  int Query();
  void Record();
  void Read(std::string&);
  void Dump();
  unsigned long int GetSize();
  void GetCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  int GetSamplingCount(unsigned long int data_index);
};

class HybridAnalysis {
 private:
  std::map<std::string, std::unique_ptr<ControlFlowGraph>> func_cfg_vec;
  std::unique_ptr<ProgramCallGraph> pcg;
  std::map<std::string, std::unique_ptr<ProgramAbstractionGraph>> func_pag_vec;
  std::unique_ptr<ProgramAbstractionGraph> pag;

 public:
  HybridAnalysis() {}
  ~HybridAnalysis() {}

  /** Control Flow Graph of Each Function **/

  void ReadStaticControlFlowGraphs(const char* dir_name);

  void GenerateControlFlowGraphs(const char* dir_name);

  std::unique_ptr<ControlFlowGraph> GetControlFlowGraph(std::string func_name);

  std::map<std::string, std::unique_ptr<ControlFlowGraph>>& GetControlFlowGraphs();

  /** Program Call Graph **/

  void ReadStaticProgramCallGraph(std::string static_pcg_file_name);

  void ReadDynamicProgramCallGraph(std::string perf_data_file_name);

  void GenerateProgramCallGraph();

  std::unique_ptr<ProgramCallGraph> GetProgramCallGraph();

  /** Intra-procedural Analysis **/

  std::unique_ptr<ProgramAbstractionGraph> GetFunctionAbstractionGraph(std::string func_name);

  std::map<std::string, std::unique_ptr<ProgramAbstractionGraph>>& GetFunctionAbstractionGraphs();

  void IntraProceduralAnalysis();

  /** Inter-procedural Analysis **/

  void InterProceduralAnalysis();

  void GenerateProgramAbstractionGraph();

  void Dataembedding(std::unique_ptr<ProgramAbstractionGraph>, std::unique_ptr<PerfData>);

  // void ConnectCallerCallee(ProgramAbstractionGraph* pag, int vertex_id, void* extra);
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
};

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
  void SetOverflow(void (*FUNC_AT_OVERFLOW)(int));
  void Start();
  void Stop();
  int GetOverflowEvent(LongLongVec* overflow_vector);
  void GetBacktrace(char* call_path_str, int max_call_path_str_len);
  // int my_backtrace(unw_word_t *buffer, int max_depth);
  // static void papi_handler(int EventSet, void *address, long_long overflow_vector, void *context);
};

}  // namespace graph_sd
}  // namespace baguatool

#endif