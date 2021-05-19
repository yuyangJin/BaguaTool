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

/** @brief Wrapper class of igraph. Provide basic graph operations.
    @author Yuyang Jin, PACMAN, Tsinghua University
    @date March 2021
    */
class Graph {
 protected:
  std::unique_ptr<graph_t> ipag_; /**<igraph_t wrapper struct */
  int cur_vertex_num;             /**<initial the number of vertices in this graph */

 public:
  /** Constructor. Create an graph and enable graph attributes.
   */
  Graph();

  /** Destructor. Destroy graph.
   */
  ~Graph();

  /** Initialize the graph. Build a directed graph with zero vertices and zero edges. Set name of the graph with the
   * input parameter.
   * @param graph_name - name of the graph
   */
  void GraphInit(const char* graph_name);

  /** Create a vertex in the graph.
   * @return id of the new vertex
   */
  vertex_t AddVertex();

  /** Create an edge in the graph.
   * @param src_vertex_id - id of the source vertex of the edge
   * @param dest_vertex_id - id of the destination vertex of the edge
   * @return id of the new edge
   */
  edge_t AddEdge(const vertex_t src_vertex_id, const vertex_t dest_vertex_id);

  /** Append a graph to the graph. Copy all the vertices and edges (and all their attributes) of a graph to this graph.
   * @param g - the graph to be appended
   */
  void AddGraph(Graph* g);

  /** Delete a vertex.
   * @param vertex_id - id of vertex to be removed
   */
  void DeleteVertex(vertex_t vertex_id);

  /** Delete a set of vertex.
   * @param vs - set of vertices to be deleted
   */
  void DeleteVertices(vertex_set_t* vs);

  /** Delete extra vertices at the end of vertices. (No need to expose to developers)
   */
  void DeleteExtraTailVertices();

  /** Delete an edge.
   * @param src_vertex_id - id of the source vertex of the edge to be removed
   * @param dest_vertex_id - id of the destination vertex of the edge to be removed
   */
  void DeleteEdge(vertex_t src_vertex_id, vertex_t dest_vertex_id);

  /** Swap two vertices. Swap all attributes except for "id"
   * @param vertex_id_1 - id of the first vertex
   * @param vertex_id_2 - id of the second vertex
   */
  void SwapVertex(vertex_t vertex_id_1, vertex_t vertex_id_2);

  /** Query a vertex. (Not implement yet.)
   */
  void QueryVertex();

  /** Query an edge with source and destination vertex ids.
   * @param src_vertex_id - id of the source vertex of the edge
   * @param dest_vertex_id - id of the destination vertex of the edge
   * @return id of the queried edge
   */
  edge_t QueryEdge(vertex_t src_vertex_id, vertex_t dest_vertex_id);

  /** Get the source vertex of the input edge.
   * @param edge_id - input edge id
   * @return id of the source vertex
   */
  vertex_t GetEdgeSrc(edge_t edge_id);

  /** Get the destination vertex of the input edge.
   * @param edge_id - input edge id
   * @return id of the destination vertex
   */
  vertex_t GetEdgeDest(edge_t edge_id);

  /** Get the other side vertex of the input edge. (Not implement yet.)
   * @param edge_id - input edge id
   * @param vertex_id - input vertex id
   * @return id of the vertex in the other side of the input edge
   */
  void GetEdgeOtherSide();

  /** Set a string graph attribute
   * @param attr_name - name of the graph attribute
   * @param value - the (new) value of the graph attribute
   */
  void SetGraphAttributeString(const char* attr_name, const char* value);

  /** Set a numeric graph attribute
   * @param attr_name - name of the graph attribute
   * @param value - the (new) value of the graph attribute
   */
  void SetGraphAttributeNum(const char* attr_name, const int value);

  /** Set a boolean graph attribute as flag
   * @param attr_name - name of the graph attribute
   * @param value - the (new) value of the graph attribute
   */
  void SetGraphAttributeFlag(const char* attr_name, const bool value);

  /** Set a string vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @param value - the (new) value of the vertex attribute
   */
  void SetVertexAttributeString(const char* attr_name, vertex_t vertex_id, const char* value);

  /** Set a numeric vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @param value - the (new) value of the vertex attribute
   */
  void SetVertexAttributeNum(const char* attr_name, vertex_t vertex_id, const int value);

  /** Set a boolean vertex attribute as flag
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @param value - the (new) value of the vertex attribute
   */
  void SetVertexAttributeFlag(const char* attr_name, vertex_t vertex_id, const bool value);

  /** Set a string edge attribute
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeString(const char* attr_name, edge_t edge_id, const char* value);

  /** Set a numeric edge attribute
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeNum(const char* attr_name, edge_t edge_id, const int value);

  /** Set a boolean edge attribute as flag
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeFlag(const char* attr_name, edge_t edge_id, const bool value);

  /** Get a string graph attribute
   * @param attr_name - name of the graph attribute
   * @return the (new) value of the graph attribute
   */
  const char* GetGraphAttributeString(const char* attr_name);

  /** Get a numeric graph attribute
   * @param attr_name - name of the graph attribute
   * @return the (new) value of the graph attribute
   */
  const int GetGraphAttributeNum(const char* attr_name);

  /** Get a flag graph attribute
   * @param attr_name - name of the graph attribute
   * @return the (new) value of the graph attribute
   */
  const bool GetGraphAttributeFlag(const char* attr_name);

  /** Get a string vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @return value - the (new) value of the vertex attribute
   */
  const char* GetVertexAttributeString(const char* attr_name, vertex_t vertex_id);

  /** Get a numeric vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @return value - the (new) value of the vertex attribute
   */
  const int GetVertexAttributeNum(const char* attr_name, vertex_t vertex_id);

  /** Get a flag vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @return value - the (new) value of the vertex attribute
   */
  const bool GetVertexAttributeFlag(const char* attr_name, vertex_t vertex_id);

  /** Get a string edge attribute
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const char* GetEdgeAttributeString(const char* attr_name, edge_t edge_id);

  /** Get a numeric edge attribute
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const int GetEdgeAttributeNum(const char* attr_name, edge_t edge_id);

  /** Get a flag edge attribute
   * @param attr_name - name of the edge attribute
   * @param edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const bool GetEdgeAttributeFlag(const char* attr_name, edge_t edge_id);

  /** Remove a graph attribute
   * @param attr_name - name of the graph attribute
   */
  void RemoveGraphAttribute(const char* attr_name);

  /** Remove a vertex attribute
   * @param attr_name - name of the vertex attribute
   */
  void RemoveVertexAttribute(const char* attr_name);

  /** Remove an edge attribute
   * @param attr_name - name of the edge attribute
   */
  void RemoveEdgeAttribute(const char* attr_name);

  void MergeVertices();
  void SplitVertex();

  /** Copy a vertex to the designated vertex. All attributes (include "id") are copied.
   * @param new_vertex_id - id of the designated vertex
   * @param g - graph that contains the vertex to be copied
   * @param vertex_id - id of the vertex to be copied
   */
  void DeepCopyVertex(vertex_t new_vertex_id, Graph* g, vertex_t vertex_id);

  /** Copy a vertex to the designated vertex. All attributes, except "id", are copied.
   * @param new_vertex_id - id of the designated vertex
   * @param g - graph that contains the vertex to be copied
   * @param vertex_id - id of the vertex to be copied
   */
  void CopyVertex(vertex_t new_vertex_id, Graph* g, vertex_t vertex_id);
  // TODO: do not expose inner igraph

  /** Depth-First Search, not implement yet.
   */
  void Dfs();

  /** Read a graph from a GML format file.
   * @param
   */
  void ReadGraphGML(const char* file_name);
  void DumpGraphGML(const char* file_name);
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

typedef struct VERTEX_DATA_STRUCT VDS;
typedef struct EDGE_DATA_STRUCT EDS;

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 256
#endif

typedef unsigned long long int addr_t;
typedef double perf_data_t;

class PerfData {
 private:
  VDS* vertex_perf_data = nullptr;
  unsigned long int vertex_perf_data_space_size = 0;
  unsigned long int vertex_perf_data_count = 0;
  EDS* edge_perf_data = nullptr;
  unsigned long int edge_perf_data_space_size = 0;
  unsigned long int edge_perf_data_count = 0;
  FILE* perf_data_fp = nullptr;
  std::ifstream perf_data_in_file;
  bool has_open_output_file = false;
  char file_name[MAX_LINE_LEN] = {0};
  // TODO: design a method to make metric_name portable
  std::string metric_name = std::string("TOT_CYC");

 public:
  PerfData();
  ~PerfData();
  void Read(const char*);
  void Dump(const char*);
  unsigned long int GetVertexDataSize();
  void SetMetricName(std::string& metric_name);
  std::string& GetMetricName();

  int QueryVertexData(addr_t* call_path, int call_path_len, int procs_id, int thread_id);
  int QueryEdgeData(addr_t* call_path, int call_path_len, addr_t* out_call_path, int out_call_path_len, int procs_id,
                    int out_procs_id, int thread_id, int out_thread_id);

  void RecordVertexData(addr_t* call_path, int call_path_len, int procs_id, int thread_id, perf_data_t value);
  void RecordEdgeData(addr_t* call_path, int call_path_len, addr_t* out_call_path, int out_call_path_len, int procs_id,
                      int out_procs_id, int thread_id, int out_thread_id, perf_data_t value);

  void GetVertexDataCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  void GetEdgeDataSrcCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  void GetEdgeDataDestCallPath(unsigned long int data_index, std::stack<unsigned long long>&);

  perf_data_t GetVertexDataValue(unsigned long int data_index);
  perf_data_t GetEdgeDataValue(unsigned long int data_index);

  int GetVertexDataProcsId(unsigned long int data_index);
  int GetEdgeDataSrcProcsId(unsigned long int data_index);
  int GetEdgeDataDestProcsId(unsigned long int data_index);

  int GetVertexDataThreadId(unsigned long int data_index);
  int GetEdgeDataSrcThreadId(unsigned long int data_index);
  int GetEdgeDataDestThreadId(unsigned long int data_index);
};

class GraphPerfData {
 private:
  json j_perf_data;

 public:
  GraphPerfData();
  ~GraphPerfData();

  void Read(std::string&);
  void Dump(std::string&);

  void SetPerfData(vertex_t vertex_id, std::string& metric, int procs_id, int thread_id, perf_data_t value);
  perf_data_t GetPerfData(vertex_t vertex_id, std::string& metric, int procs_id, int thread_id);

  bool HasMetric(vertex_t vertex_id, std::string& metric);
  void GetVertexPerfDataMetrics(vertex_t, std::vector<std::string>&);
  int GetMetricsPerfDataProcsNum(vertex_t vertex_id, std::string& metric);
  int GetProcsPerfDataThreadNum(vertex_t vertex_id, std::string& metric, int procs_id);
  void GetProcsPerfData(vertex_t vertex_id, std::string& metric, int procs_id,
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