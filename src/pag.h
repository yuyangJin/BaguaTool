#ifndef PAG_H
#define PAG_H

#include <stdint.h>
#include <igraph.h>
#include "vertex_type.h"

#define TRUNK_SIZE 1000

class ProgramAbstractionGraph {
 private:
  igraph_t ipag_;
  int cur_vertex_id;

 public:
  ProgramAbstractionGraph(const char* graph_name){
      // open attributes
      igraph_set_attribute_table(&igraph_cattribute_table);
      // build an empty graph
      igraph_empty(&this->ipag_, 0, IGRAPH_DIRECTED);
      //
      SETGAS(&this->ipag_, "name", graph_name);
      //
      cur_vertex_id = 0;
  }

  ~ProgramAbstractionGraph() {
      igraph_destroy(&this->ipag_);
  }

  int AddVertex(const int vertex_type, const char* vertex_name);

  int AddEdge(const int src_vertex_id, const int dest_vertex_id);

  void DeleteVertex();

  void DeleteEdge();

  void QueryVertex();

  void QueryEdge();

  void QueryEdgeSrc();

  void QueryEdgeDest();

  void QueryEdgeOtherSide();

  void SetVertexAttribute();

  void SetEdgeAttribute();

  void GetVertexAttribute();

  void GetEdgeAttribute();

  void MergeVertices();

  void SplitVertex();

  void CopyVertex();

  void DumpGraph(const char* file_name);

};

#endif //PAG_H