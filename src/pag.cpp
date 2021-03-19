#include "pag.h"

int ProgramAbstractionGraph::AddVertex(const int vertex_type, const char* vertex_name){
  if (this->cur_vertex_id >= igraph_vcount(&this->ipag_)){
    igraph_add_vertices(&this->ipag_, TRUNK_SIZE, 0);
  }
  // Add a new vertex
  igraph_integer_t new_vertex_id = this->cur_vertex_id ++;

  //igraph_integer_t new_vertex_id = igraph_vcount(&this->ipag_);
  printf("Add a vertex: %d %s\n", new_vertex_id, vertex_name);
  
  // Set basic attributes
  SETVAN(&this->ipag_, "type", new_vertex_id, (igraph_real_t)vertex_type);
  SETVAS(&this->ipag_, "name", new_vertex_id, vertex_name);

  // Return id of new vertex
  return (int) new_vertex_id;
}

int ProgramAbstractionGraph::AddEdge(const int src_vertex_id, const int dest_vertex_id){
  // Add a new edge
  printf("Add an edge: %d, %d\n", src_vertex_id, dest_vertex_id);
  igraph_add_edge(&this->ipag_, (igraph_integer_t)src_vertex_id, (igraph_integer_t)dest_vertex_id);
  igraph_integer_t new_edge_id = igraph_ecount(&this->ipag_);

  // Return id of new edge
  return (int) (new_edge_id - 1);
}

void ProgramAbstractionGraph::DeleteVertex(){

}

void ProgramAbstractionGraph::DeleteEdge(){}

void ProgramAbstractionGraph::QueryVertex(){}

void ProgramAbstractionGraph::QueryEdge(){}

void ProgramAbstractionGraph::QueryEdgeSrc(){}

void ProgramAbstractionGraph::QueryEdgeDest(){}

void ProgramAbstractionGraph::QueryEdgeOtherSide(){}

void ProgramAbstractionGraph::SetVertexAttribute(){}

void ProgramAbstractionGraph::SetEdgeAttribute(){}

void ProgramAbstractionGraph::GetVertexAttribute(){}

void ProgramAbstractionGraph::GetEdgeAttribute(){}

void ProgramAbstractionGraph::MergeVertices(){}

void ProgramAbstractionGraph::SplitVertex(){}

void ProgramAbstractionGraph::CopyVertex(){}

void ProgramAbstractionGraph::DumpGraph(const char* file_name) {
  // Check the number of vertices
  igraph_vs_t vs;
  igraph_vs_seq(&vs, this->cur_vertex_id, igraph_vcount(&this->ipag_) - 1);
  igraph_delete_vertices(&this->ipag_, vs);

  // Real dump
  FILE* outfile = fopen(file_name, "w");
  igraph_write_graph_gml(&this->ipag_, outfile, 0, "Bruce Jin");
  fclose(outfile);
}