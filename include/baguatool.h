#ifndef BAGUATOOL_H_
#define BAGUATOOL_H_

#include <memory>
#include <vector>

// All public APIs are defined here.

namespace baguatool {

namespace core {
class PAGImpl;
class PAGVertex;
class ProgramAbstractionGraph {
 private:
  // PAGImpl* ipag_;
  std::unique_ptr<PAGImpl> ipag_;
  int cur_vertex_id;

 public:
  ProgramAbstractionGraph();
  ~ProgramAbstractionGraph();
  void GraphInit(const char* graph_name);
  int AddVertex();
  int AddEdge(const int src_vertex_id, const int dest_vertex_id);
  void AddGraph(ProgramAbstractionGraph* g);
  int SetVertexBasicInfo(const int vertex_id, const int vertex_type, const char* vertex_name);
  int SetVertexDebugInfo(const int vertex_id, const int entry_addr, const int exit_addr);
  void DeleteVertex();
  void DeleteEdge();
  void QueryVertex();
  void QueryEdge();
  int GetEdgeSrc(int edge_id);
  int GetEdgeDest(int edge_id);
  void QueryEdgeOtherSide();
  void SetVertexAttribute();
  void SetEdgeAttribute();
  void GetVertexAttribute();
  int GetVertexAttributeNum(const char* attr_name, int vertex_id);
  const char* GetVertexAttributeString(const char* attr_name, int vertex_id);
  void GetEdgeAttribute();
  const char* GetGraphAttributeString(const char* attr_name);
  void MergeVertices();
  void SplitVertex();
  void CopyVertex(int new_vertex_id, ProgramAbstractionGraph* g, int vertex_id);
  // TODO: do not expose inner igraph
  void DeleteVertices(PAGVertex* vs);
  void DeleteExtraTailVertices();
  void Dfs();
  void ReadGraphGML(const char* file_name);
  void DumpGraph(const char* file_name);
  void DumpGraphDot(const char* file_name);
  int GetCurVertexId();
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph*, int, void*), void* extra);
};
}

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
}  // graph_perf

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
  void DumpAllFunctionGraph();
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