#include "core/pcg.h"
#include "baguatool.h"
#include "vertex_type.h"

namespace baguatool::core {

ProgramCallGraph::ProgramCallGraph() {}
ProgramCallGraph::~ProgramCallGraph() {}

int ProgramCallGraph::SetVertexBasicInfo(const vertex_t vertex_id, const int vertex_type, const char *vertex_name) {
  //
  // SetVertexAttributeNum("type", vertex_id, (igraph_real_t)vertex_type);
  SETVAN(&ipag_->graph, "type", vertex_id, (igraph_real_t)vertex_type);
  SETVAS(&ipag_->graph, "name", vertex_id, vertex_name);
  return 0;
}

int ProgramCallGraph::SetVertexDebugInfo(const vertex_t vertex_id, const int addr) {
  //
  SETVAN(&ipag_->graph, "addr", vertex_id, (igraph_real_t)addr);
  return 0;
}
}