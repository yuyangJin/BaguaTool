#ifndef PAG_H
#define PAG_H

#include <stdint.h>
#include <igraph.h>
#include <string>
#include "vertex_type.h"

#define TRUNK_SIZE 1000

class ProgramAbstractionGraph {
 private:
  igraph_t ipag_;
  int cur_vertex_id;

 public:
  ProgramAbstractionGraph(){
    // open attributes
    igraph_set_attribute_table(&igraph_cattribute_table);
  }

  ~ProgramAbstractionGraph() {
    igraph_destroy(&this->ipag_);
  }

  void GraphInit(const char* graph_name){
    // build an empty graph
    igraph_empty(&this->ipag_, 0, IGRAPH_DIRECTED);
    // set graph name
    SETGAS(&this->ipag_, "name", graph_name);
    // set vertex number as 0
    cur_vertex_id = 0;
  }

  igraph_t* GetGraph(){
    return &ipag_;
  }

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
  int GetVertexAttributeNum(const char * attr_name, int vertex_id);
  const char* GetVertexAttributeString(const char * attr_name, int vertex_id);

  void GetEdgeAttribute();

  const char* GetGraphAttributeString(const char * attr_name);

  void MergeVertices();

  void SplitVertex();

  void CopyVertex(int new_vertex_id, ProgramAbstractionGraph* g, int vertex_id);

  void DeleteVertices(igraph_vs_t vs);

  void DeleteExtraTailVertices();

  void Dfs();

  void ReadGraphGML(const char* file_name);

  void DumpGraph(const char* file_name);
  void DumpGraphDot(const char* file_name);

  int GetCurVertexId();

  void VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph*, int, void *), void * extra);

};

#endif //PAG_H