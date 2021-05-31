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

namespace type {
class graph_t;                         /**<igragh-graph wrapper */
class vertex_set_t;                    /**<igraph_vs_t wrapper */
typedef int vertex_t;                  /**<vertex id type (int)*/
typedef int edge_t;                    /**<edge id type (int)*/
typedef unsigned long long int addr_t; /**<address type (unsigned long long int)*/
typedef double perf_data_t;            /**<performance data type (double) */
typedef std::stack<type::addr_t> call_path_t;
}

namespace core {

// class type::graph_t;
// class baguatool::type::vertex_set_t;
// typedef int type::vertex_t;
// typedef int type::edge_t;

/** @brief Wrapper class of igraph. Provide basic graph operations.
    @author Yuyang Jin, PACMAN, Tsinghua University
    @date March 2021
    */
class Graph {
 protected:
  std::unique_ptr<type::graph_t> ipag_; /**<igraph_t wrapper struct */
  int cur_vertex_num;                   /**<initial the number of vertices in this graph */

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
  type::vertex_t AddVertex();

  /** Create an edge in the graph.
   * @param src_vertex_id - id of the source vertex of the edge
   * @param dest_vertex_id - id of the destination vertex of the edge
   * @return id of the new edge
   */
  type::edge_t AddEdge(const type::vertex_t src_vertex_id, const type::vertex_t dest_vertex_id);

  /** Append a graph to the graph. Copy all the vertices and edges (and all their attributes) of a graph to this graph.
   * @param g - the graph to be appended
   * @return id of entry vertex of the appended new graph (not id in old graph)
   */
  type::vertex_t AddGraph(Graph* g);

  /** Delete a vertex.
   * @param vertex_id - id of vertex to be removed
   */
  void DeleteVertex(type::vertex_t vertex_id);

  /** Delete a set of vertex.
   * @param vs - set of vertices to be deleted
   */
  void DeleteVertices(baguatool::type::vertex_set_t* vs);

  /** Delete extra vertices at the end of vertices. (No need to expose to developers)
   */
  void DeleteExtraTailVertices();

  /** Delete an edge.
   * @param src_vertex_id - id of the source vertex of the edge to be removed
   * @param dest_vertex_id - id of the destination vertex of the edge to be removed
   */
  void DeleteEdge(type::vertex_t src_vertex_id, type::vertex_t dest_vertex_id);

  /** Swap two vertices. Swap all attributes except for "id"
   * @param vertex_id_1 - id of the first vertex
   * @param vertex_id_2 - id of the second vertex
   */
  void SwapVertex(type::vertex_t vertex_id_1, type::vertex_t vertex_id_2);

  /** Query a vertex. (Not implement yet.)
   */
  void QueryVertex();

  /** Query an edge with source and destination vertex ids.
   * @param src_vertex_id - id of the source vertex of the edge
   * @param dest_vertex_id - id of the destination vertex of the edge
   * @return id of the queried edge
   */
  type::edge_t QueryEdge(type::vertex_t src_vertex_id, type::vertex_t dest_vertex_id);

  /** Get the source vertex of the input edge.
   * @param edge_id - input edge id
   * @return id of the source vertex
   */
  type::vertex_t GetEdgeSrc(type::edge_t edge_id);

  /** Get the destination vertex of the input edge.
   * @param edge_id - input edge id
   * @return id of the destination vertex
   */
  type::vertex_t GetEdgeDest(type::edge_t edge_id);

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
  void SetVertexAttributeString(const char* attr_name, type::vertex_t vertex_id, const char* value);

  /** Set a numeric vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @param value - the (new) value of the vertex attribute
   */
  void SetVertexAttributeNum(const char* attr_name, type::vertex_t vertex_id, const int value);

  /** Set a boolean vertex attribute as flag
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @param value - the (new) value of the vertex attribute
   */
  void SetVertexAttributeFlag(const char* attr_name, type::vertex_t vertex_id, const bool value);

  /** Set a string edge attribute
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeString(const char* attr_name, type::edge_t edge_id, const char* value);

  /** Set a numeric edge attribute
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeNum(const char* attr_name, type::edge_t edge_id, const int value);

  /** Set a boolean edge attribute as flag
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @param value - the (new) value of the edge attribute
   */
  void SetEdgeAttributeFlag(const char* attr_name, type::edge_t edge_id, const bool value);

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
  const char* GetVertexAttributeString(const char* attr_name, type::vertex_t vertex_id);

  /** Get a numeric vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @return value - the (new) value of the vertex attribute
   */
  const int GetVertexAttributeNum(const char* attr_name, type::vertex_t vertex_id);

  /** Get a flag vertex attribute
   * @param attr_name - name of the vertex attribute
   * @param vertex_id - the vertex id
   * @return value - the (new) value of the vertex attribute
   */
  const bool GetVertexAttributeFlag(const char* attr_name, type::vertex_t vertex_id);

  /** Get a string edge attribute
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const char* GetEdgeAttributeString(const char* attr_name, type::edge_t edge_id);

  /** Get a numeric edge attribute
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const int GetEdgeAttributeNum(const char* attr_name, type::edge_t edge_id);

  /** Get a flag edge attribute
   * @param attr_name - name of the edge attribute
   * @param type::edge_t - the edge id
   * @return value - the (new) value of the edge attribute
   */
  const bool GetEdgeAttributeFlag(const char* attr_name, type::edge_t edge_id);

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
  void DeepCopyVertex(type::vertex_t new_vertex_id, Graph* g, type::vertex_t vertex_id);

  /** Copy a vertex to the designated vertex. All attributes, except "id", are copied.
   * @param new_vertex_id - id of the designated vertex
   * @param g - graph that contains the vertex to be copied
   * @param vertex_id - id of the vertex to be copied
   */
  void CopyVertex(type::vertex_t new_vertex_id, Graph* g, type::vertex_t vertex_id);
  // TODO: do not expose inner igraph

  /** Depth-First Search, not implement yet.
   */
  void Dfs();

  /** Read a graph from a GML format file.
   * @param file_name - name of input file
   */
  void ReadGraphGML(const char* file_name);

  /** Dump the graph as a GML format file.
   * @param file_name - name of output file
   */
  void DumpGraphGML(const char* file_name);

  /** Dump the graph as a dot format file.
   * @param file_name - name of output file
   */
  void DumpGraphDot(const char* file_name);

  /** Get the number of vertices.
   * @return the number of vertices
   */
  int GetCurVertexNum();

  /** Get a set of child vertices.
   * @param vertex_id - id of a vertex
   * @param child_vec - a vector that stores the id of child vertices
   */
  void GetChildVertexSet(type::vertex_t vertex_id, std::vector<type::vertex_t>& child_vec);

  /** [Graph Algorithm] Traverse all vertices and execute CALL_BACK_FUNC when accessing each vertex.
   * @param CALL_BACK_FUNC - callback function when a vertex is accessed. The input parameters of this function contain
   * a pointer to the graph being traversed, id of the accessed vertex, and an extra pointer for developers to pass more
   * parameters.
   * @param extra - a pointer for developers to pass more parameters as the last parameter of CALL_BACK_FUNC
   */
  void VertexTraversal(void (*CALL_BACK_FUNC)(Graph*, type::vertex_t, void*), void* extra);

  /** [Graph Algorithm] Perform Pre-order traversal on the graph.
   * @param root_vertex_id - id of the starting vertex
   * @param pre_order_vertex_vec - a vector that stores the accessing sequence (id) of vertex
   */
  void PreOrderTraversal(type::vertex_t root_vertex_id, std::vector<type::vertex_t>& pre_order_vertex_vec);
};

class ProgramGraph : public Graph {
 private:
 public:
  /** Default Constructor
   */
  ProgramGraph();

  /** Default Destructor
   */
  ~ProgramGraph();

  /** Set basic information of a vertex, including type and name.
   * @param vertex_id - id of the target vertex
   * @param vertex_type - type of the target vertex
   * @param vertex_name - name of the target vertex
   * @return 0 is success
   */
  int SetVertexBasicInfo(const type::vertex_t vertex_id, const int vertex_type, const char* vertex_name);

  /** Set debug information of a vertex, including address or line number (need to extend).
   * @param vertex_id - id of the target vertex
   * @param entry_addr - entry address of the target vertex
   * @param exit_addr - exit address of the target vertex
   * @return 0 is success
   */
  int SetVertexDebugInfo(const type::vertex_t vertex_id, const int entry_addr, const int exit_addr);

  /** Get the type of a vertex.
   * @param vertex_id - id of the target vertex
   * @return type of the vertex
   */
  int GetVertexType(type::vertex_t vertex_id);

  /** Get the entry address of a specific vertex
   * @param vertex_id - id of the specific vertex
   * @return entry address of the specific vertex
   */
  type::addr_t GetVertexEntryAddr(type::vertex_t vertex_id);

  /** Get the exit address of a specific vertex
   * @param vertex_id - id of the specific vertex
   * @return exit address of the specific vertex
   */
  type::addr_t GetVertexExitAddr(type::vertex_t vertex_id);

  /** Identify the vertex corresponding to the address from a specific starting vertex.
   * @param root_vertex_id - id of the starting vertex
   * @param addr - address
   * @return id of the identified vertex
   */
  type::vertex_t GetChildVertexWithAddr(type::vertex_t root_vertex_id, type::addr_t addr);

  /** Identify the vertex corresponding to the call path from a specific starting vertex.
   * @param root_vertex_id - id of the starting vertex
   * @param call_path - call path
   * @return id of the identified vertex
   */
  type::vertex_t GetVertexWithCallPath(type::vertex_t root_vertex_id, std::stack<unsigned long long>& call_path);

  /** Identify the call vertex corresponding to the address from all vertices.
   * @param addr - address
   * @return id of the identified vertex
   */
  type::vertex_t GetCallVertexWithAddr(unsigned long long addr);

  /** Identify the function vertex corresponding to the address from all vertices.
   * @param addr - address
   * @return id of the identified vertex
   */
  type::vertex_t GetFuncVertexWithAddr(unsigned long long addr);

  /** Add a new edge between call vertex and callee function vertex through their addresses.
   * @param call_addr - address of the call instruction
   * @param callee_addr - address of the callee function
   * @return
   */
  int AddEdgeWithAddr(unsigned long long call_addr, unsigned long long callee_addr);

  /** Get name of a vertex's callee vertex
   * @param vertex_id - id of vertex
   * @return name of the callee vertex
   */
  const char* GetCallee(type::vertex_t vertex_id);

  /** Sort vertices by entry addresses
   */
  void VertexSortChild();

  /** [Graph Algorithm] Traverse all vertices and execute CALL_BACK_FUNC when accessing each vertex.
   * @param CALL_BACK_FUNC - callback function when a vertex is accessed. The input parameters of this function contain
   * a pointer to the graph being traversed, id of the accessed vertex, and an extra pointer for developers to pass more
   * parameters.
   * @param extra - a pointer for developers to pass more parameters as the last parameter of CALL_BACK_FUNC
   */
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramGraph*, int, void*), void* extra);
};

class ProgramAbstractionGraph : public ProgramGraph {
 private:
 public:
  /** Default Constructor
   */
  ProgramAbstractionGraph();
  /** Default Destructor
   */
  ~ProgramAbstractionGraph();
  /** [Graph Algorithm] Traverse all vertices and execute CALL_BACK_FUNC when accessing each vertex.
   * @param CALL_BACK_FUNC - callback function when a vertex is accessed. The input parameters of this function contain
   * a pointer to the graph being traversed, id of the accessed vertex, and an extra pointer for developers to pass more
   * parameters.
   * @param extra - a pointer for developers to pass more parameters as the last parameter of CALL_BACK_FUNC
   */
  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph*, int, void*), void* extra);
};

class ControlFlowGraph : public ProgramGraph {
 private:
 public:
  /** Default Constructor
   */
  ControlFlowGraph();
  /** Default Destructor
   */
  ~ControlFlowGraph();
};

class ProgramCallGraph : public ProgramGraph {
 private:
 public:
  /** Default Constructor
   */
  ProgramCallGraph();
  /** Default Destructor
   */
  ~ProgramCallGraph();
};

typedef struct VERTEX_DATA_STRUCT VDS;
typedef struct EDGE_DATA_STRUCT EDS;

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 256
#endif

class PerfData {
 private:
  VDS* vertex_perf_data = nullptr;                   /**<pointer to vertex type performance data */
  unsigned long int vertex_perf_data_space_size = 0; /**<pre-allocate space size of vertex type performance data*/
  unsigned long int vertex_perf_data_count = 0;      /**<amount of recorded vertex type performance data*/
  EDS* edge_perf_data = nullptr;                     /**<pointer to edge type performance data */
  unsigned long int edge_perf_data_space_size = 0;   /**<pre-allocate space size of edge type performance data*/
  unsigned long int edge_perf_data_count = 0;        /**<amount of recorded edge type performance data*/
  FILE* perf_data_fp = nullptr;                      /**<file handler for output */
  std::ifstream perf_data_in_file;                   /**<file handler for input */
  bool has_open_output_file = false;                 /**<flag for record whether output file has open or not*/
  char file_name[MAX_LINE_LEN] = {0};                /**<file name for output */
  // TODO: design a method to make metric_name portable
  std::string metric_name = std::string("TOT_CYC"); /**<metric name */

 public:
  /** Default Constructor
   */
  PerfData();
  /** Default Destructor
   */
  ~PerfData();

  /** Read vertex type and edge type performance data (Input)
   * @param file_name - name of input file
   */
  void Read(const char* file_name);

  /** Dump vertex type and edge type performance data (Output)
   * @param file_name - name of output file
   */
  void Dump(const char* file_name);

  /** Get size of recorded vertex type performance data
   * @return size of recorded vertex type performance data
   */
  unsigned long int GetVertexDataSize();

  /** Get size of recorded edge type performance data
   * @return size of recorded edge type performance data
   */
  unsigned long int GetEdgeDataSize();

  /** Set metric name for the performance data
   * @param metric_name - name of the metric
   */
  void SetMetricName(std::string& metric_name);

  /** Get metric name of the performance data
   * @return name of the metric
   */
  std::string& GetMetricName();

  /** Query a piece of vertex type performance data by (call path, call path length, process id, thread id)
   * @param call_path - call path
   * @param call_path_len - depth of the call path
   * @param procs_id - process id
   * @param thread_id - thread id
   * @return index of the queried piece of data
   */
  int QueryVertexData(baguatool::type::addr_t* call_path, int call_path_len, int procs_id, int thread_id);

  /** Query a piece of edge type performance data by (call path, call path length, process id, thread id)
   * @param call_path - call path of source
   * @param call_path_len - depth of the call path of source
   * @param out_call_path - call path of destination
   * @param out_call_path_len - depth of the call path of destination
   * @param procs_id - process id of source
   * @param out_procs_id - process id of destination
   * @param thread_id - thread id of source
   * @param out_thread_id - thread id of destination
   * @return index of the queried piece of data
   */
  int QueryEdgeData(baguatool::type::addr_t* call_path, int call_path_len, baguatool::type::addr_t* out_call_path,
                    int out_call_path_len, int procs_id, int out_procs_id, int thread_id, int out_thread_id);

  /** Record a piece of vertex type performance data
   * @param call_path - call path
   * @param call_path_len - depth of the call path
   * @param procs_id - process id
   * @param thread_id - thread id
   * @param value of this piece of data
   */
  void RecordVertexData(baguatool::type::addr_t* call_path, int call_path_len, int procs_id, int thread_id,
                        baguatool::type::perf_data_t value);

  /** Record a piece of edge type performance data
   * @param call_path - call path of source
   * @param call_path_len - depth of the call path of source
   * @param out_call_path - call path of destination
   * @param out_call_path_len - depth of the call path of destination
   * @param procs_id - process id of source
   * @param out_procs_id - process id of destination
   * @param thread_id - thread id of source
   * @param out_thread_id - thread id of destination
   * @param value of this piece of data
   */
  void RecordEdgeData(baguatool::type::addr_t* call_path, int call_path_len, baguatool::type::addr_t* out_call_path,
                      int out_call_path_len, int procs_id, int out_procs_id, int thread_id, int out_thread_id,
                      baguatool::type::perf_data_t value);

  /** Query call path of a piece of vertex type performance data through index
   * @param data_index - index of the piece of data
   * @param call_path - call path of this piece of data
   */
  void GetVertexDataCallPath(unsigned long int data_index, std::stack<unsigned long long>& call_path);

  /** Query source call path of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @param call_path - call path of the queried piece of data
   */
  void GetEdgeDataSrcCallPath(unsigned long int data_index, std::stack<unsigned long long>& call_path);

  /** Query destination call path of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @param call_path - call path of the queried piece of data
   */
  void GetEdgeDataDestCallPath(unsigned long int data_index, std::stack<unsigned long long>& call_path);

  /** Query value of a piece of vertex type performance data through index
   * @param data_index - index of the piece of data
   * @return value of the queried piece of data
   */
  baguatool::type::perf_data_t GetVertexDataValue(unsigned long int data_index);

  /** Query value of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @return value of the queried piece of data
   */
  baguatool::type::perf_data_t GetEdgeDataValue(unsigned long int data_index);

  /** Query process id of a piece of vertex type performance data through index
   * @param data_index - index of the piece of data
   * @return process id of the queried piece of data
   */
  int GetVertexDataProcsId(unsigned long int data_index);

  /** Query source process id of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @return process id of the queried piece of data
   */
  int GetEdgeDataSrcProcsId(unsigned long int data_index);

  /** Query destination process id of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @return process id of the queried piece of data
   */
  int GetEdgeDataDestProcsId(unsigned long int data_index);

  /** Query thread id of a piece of vertex type performance data through index
   * @param data_index - index of the piece of data
   * @return thread id of the queried piece of data
   */
  int GetVertexDataThreadId(unsigned long int data_index);

  /** Query source thread id of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @return thread id of the queried piece of data
   */
  int GetEdgeDataSrcThreadId(unsigned long int data_index);

  /** Query destination thread id of a piece of edge type performance data through index
   * @param data_index - index of the piece of data
   * @return thread id of the queried piece of data
   */
  int GetEdgeDataDestThreadId(unsigned long int data_index);
};

class GraphPerfData {
 private:
  json j_perf_data; /**<[json format] Record performance data in a graph*/

 public:
  /** Default Constructor
   */
  GraphPerfData();
  /** Default Destructor
   */
  ~GraphPerfData();

  /** Read json file (Input)
   * @param file_name - name of input file
   */
  void Read(std::string& file_name);

  /** Dump as json file (Output)
   * @param file_name - name of output file
   */
  void Dump(std::string& file_name);

  /** Record a piece of data of a specific vertex, including metric, process id, thread id, and value.
   * @param vertex_id - id of the specific vertex
   * @param metric - metric name of this value
   * @param procs_id - process id
   * @param thread_id - thread id
   * @param value - value
   */
  void SetPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id, int thread_id,
                   baguatool::type::perf_data_t value);

  /** Query the value of a piece of data of a specific vertex through metric, process id, thread id.
   * @param vertex_id - id of the specific vertex
   * @param metric - metric name of this value
   * @param procs_id - process id
   * @param thread_id - thread id
   * @return value
   */
  baguatool::type::perf_data_t GetPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id, int thread_id);

  /** Query if a specific vertex has the input metric
   * @param vertex_id - id of the specific vertex
   * @param metric - metric name
   * @return true for existance, false for
   */
  bool HasMetric(type::vertex_t vertex_id, std::string& metric);

  /** Query metric list of
   */
  void GetVertexPerfDataMetrics(type::vertex_t vertex_id, std::vector<std::string>&);
  int GetMetricsPerfDataProcsNum(type::vertex_t vertex_id, std::string& metric);
  int GetProcsPerfDataThreadNum(type::vertex_t vertex_id, std::string& metric, int procs_id);
  void GetProcsPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id,
                        std::vector<baguatool::type::perf_data_t>& proc_perf_data);

};  // class GraphPerfData

// class HybridAnalysis {
//  private:
//   std::map<std::string, ControlFlowGraph*> func_cfg_map; /**<control-flow graphs for each function*/
//   ProgramCallGraph* pcg;                                 /**<program call graph*/
//   std::map<std::string, ProgramAbstractionGraph*>
//       func_pag_map; /**<program abstraction graph extracted from control-flow graph (CFG) for each function */
//   ProgramAbstractionGraph* root_pag;  /**<an overall program abstraction graph for a program */
//   ProgramAbstractionGraph* root_mpag; /**<an overall multi-* program abstraction graph for a parallel program*/
//   GraphPerfData* graph_perf_data;     /**<performance data in a graph*/

//  public:
//   HybridAnalysis();
//   ~HybridAnalysis();

//   /** Control Flow Graph of Each Function **/

//   void ReadStaticControlFlowGraphs(const char* dir_name);
//   void GenerateControlFlowGraphs(const char* dir_name);
//   ControlFlowGraph* GetControlFlowGraph(std::string func_name);
//   std::map<std::string, ControlFlowGraph*>& GetControlFlowGraphs();

//   /** Program Call Graph **/

//   void ReadStaticProgramCallGraph(const char* static_pcg_file_name);
//   void ReadDynamicProgramCallGraph(std::string perf_data_file_name);
//   void GenerateProgramCallGraph(const char*);
//   ProgramCallGraph* GetProgramCallGraph();

//   /** Intra-procedural Analysis **/

//   ProgramAbstractionGraph* GetFunctionAbstractionGraph(std::string func_name);
//   std::map<std::string, ProgramAbstractionGraph*>& GetFunctionAbstractionGraphs();
//   void IntraProceduralAnalysis();
//   void ReadFunctionAbstractionGraphs(const char* dir_name);

//   /** Inter-procedural Analysis **/

//   void InterProceduralAnalysis();
//   void GenerateProgramAbstractionGraph();
//   void SetProgramAbstractionGraph(ProgramAbstractionGraph*);
//   ProgramAbstractionGraph* GetProgramAbstractionGraph();

//   /** DataEmbedding **/
//   void DataEmbedding(PerfData*);
//   GraphPerfData* GetGraphPerfData();
//   baguatool::type::perf_data_t ReduceVertexPerfData(std::string& metric, std::string& op);
//   void ConvertVertexReducedDataToPercent(std::string& metric, baguatool::type::perf_data_t total, std::string&
//   new_metric);

//   void GenerateMultiProgramAbstractionGraph();
//   ProgramAbstractionGraph* GetMultiProgramAbstractionGraph();

//   void PthreadAnalysis(PerfData* pthread_data);

// };  // class HybridAnalysis

}  // namespace core

namespace graph_perf {

// class Preprocess {
//  private:
//  public:
//   Preprocess() {}
//   ~Preprocess() {}

//   void ReadFunctionGraphs(const char* dir_name, std::vector<core::ProgramAbstractionGraph*>& func_pag_vec);

//   core::ProgramAbstractionGraph* InterProceduralAnalysis(std::vector<core::ProgramAbstractionGraph*>& func_pag_vec);

//   // void ConnectCallerCallee(ProgramAbstractionGraph* pag, int vertex_id, void* extra);
// };

class GPerf {
 private:
  std::map<std::string, core::ControlFlowGraph*> func_cfg_map; /**<control-flow graphs for each function*/
  core::ProgramCallGraph* pcg;                                 /**<program call graph*/
  std::map<std::string, core::ProgramAbstractionGraph*>
      func_pag_map; /**<program abstraction graph extracted from control-flow graph (CFG) for each function */
  core::ProgramAbstractionGraph* root_pag;  /**<an overall program abstraction graph for a program */
  core::ProgramAbstractionGraph* root_mpag; /**<an overall multi-* program abstraction graph for a parallel program*/
  core::GraphPerfData* graph_perf_data;     /**<performance data in a graph*/

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
   * @param file_name - file name of performance data
  */
  void ReadDynamicProgramCallGraph(std::string file_name);

  /** Generate complete program call graph through hybrid static-dynamic analysis.
   * @param binary_name - binary name
   * @param perf_data_file_name - input file name of performance data
   */
  void GenerateProgramCallGraph(const char* binary_name, const char* perf_data_file_name);

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

  /** Generate an overall program abstraction graph through intra-procedural analysis and inter-procedural analysis.
   * param binary_name - name of binary for static analysis
   * @param perf_data - file name of performance data for dynamic analysis
  */
  void GenerateProgramAbstractionGraph(core::PerfData* perf_data);

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

  /** Get performance data on the graph (GraphPerfData)
   * @return GraphPerfData
   */
  core::GraphPerfData* GetGraphPerfData();

  /** Reduce performance data of each process and thread of a speific metric for each vertex (GraphPerfData)
   * @param metric - a specfic metric
   * @param op - reduce operation
   * @return what????
   */
  baguatool::type::perf_data_t ReduceVertexPerfData(std::string& metric, std::string& op);

  /** Convert performance data to percentage format for each vertex (total value as 100 percent).
   * @param metric - a specfic metric
   * @param total - total value
   * @param new_metric - new metric for record percentage format data
   */
  void ConvertVertexReducedDataToPercent(std::string& metric, baguatool::type::perf_data_t total,
                                         std::string& new_metric);

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

namespace collector {

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

// typedef unsigned long long int baguatool::type::addr_t;

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
  int GetBacktrace(baguatool::type::addr_t* call_path, int max_call_path_depth);
};  // class Sampler

// static void* resolve_symbol(const char* symbol_name, int config);

}  // namespace graph_sd
}  // namespace baguatool

#endif