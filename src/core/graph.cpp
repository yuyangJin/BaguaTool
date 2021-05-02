#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include <map>
#include <string>

#include "core/graph.h"

#include "common/common.h"

#include "baguatool.h"

namespace baguatool::core {

Graph::Graph() {
  // ipag_ = new graph_t;
  ipag_ = std::make_unique<graph_t>();
  // open attributes
  igraph_set_attribute_table(&igraph_cattribute_table);
}

Graph::~Graph() {
  igraph_destroy(&ipag_->graph);
  // delete ipag_;
}

void Graph::GraphInit(const char *graph_name) {
  // build an empty graph
  igraph_empty(&ipag_->graph, 0, IGRAPH_DIRECTED);
  // set graph name
  SETGAS(&ipag_->graph, "name", graph_name);
  // set vertex number as 0
  cur_vertex_num = 0;
}

vertex_t Graph::AddVertex() {
  if (this->cur_vertex_num >= igraph_vcount(&ipag_->graph)) {
    igraph_add_vertices(&ipag_->graph, TRUNK_SIZE, 0);
  }
  // Add a new vertex
  igraph_integer_t new_vertex_id = this->cur_vertex_num++;

  // igraph_integer_t new_vertex_id = igraph_vcount(&ipag_.graph);
  // printf("Add a vertex: %d %s\n", new_vertex_id, vertex_name);

  // Set basic attributes

  // Return id of new vertex
  return (vertex_t)new_vertex_id;
}

edge_t Graph::AddEdge(const vertex_t src_vertex_id, const vertex_t dest_vertex_id) {
  // Add a new edge
  // printf("Add an edge: %d, %d\n", src_vertex_id, dest_vertex_id);
  igraph_add_edge(&ipag_->graph, (igraph_integer_t)src_vertex_id, (igraph_integer_t)dest_vertex_id);
  igraph_integer_t new_edge_id = igraph_ecount(&ipag_->graph);

  // Return id of new edge
  return (edge_t)(new_edge_id - 1);
}

void Graph::AddGraph(Graph *g) {
  dbg(g->GetCurVertexNum());
  g->DeleteExtraTailVertices();
  dbg(g->GetCurVertexNum());

  std::map<vertex_t, vertex_t> old_vertex_id_2_new_vertex_id;

  // Step over all vertices
  igraph_vs_t vs;
  igraph_vit_t vit;

  igraph_vs_all(&vs);
  igraph_vit_create(&g->ipag_->graph, vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    vertex_t vertex_id = (vertex_t)IGRAPH_VIT_GET(vit);
    printf("vertex %d", vertex_id);

    // Add new vertex (the copy of that in the input g) into this pag
    vertex_t new_vertex_id = this->AddVertex();

    old_vertex_id_2_new_vertex_id[vertex_id] = new_vertex_id;

    // copy all attributes of this vertex
    this->CopyVertex(new_vertex_id, g, vertex_id);

    IGRAPH_VIT_NEXT(vit);
  }
  printf("\n");

  igraph_vit_destroy(&vit);
  igraph_vs_destroy(&vs);

  // Step over all edges
  igraph_es_t es;
  igraph_eit_t eit;

  igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
  igraph_eit_create(&g->ipag_->graph, es, &eit);

  while (!IGRAPH_EIT_END(eit)) {
    // Get edge id
    edge_t edge_id = (edge_t)IGRAPH_EIT_GET(eit);
    printf("edge %d", edge_id);

    // Add new edge (the copy of that in the input g) into this pag
    edge_t new_edge_id = this->AddEdge(old_vertex_id_2_new_vertex_id[g->GetEdgeSrc(edge_id)],
                                       old_vertex_id_2_new_vertex_id[g->GetEdgeDest(edge_id)]);

    // copy all attributes of this vertex
    // this->CopyVertex(new_vertex_id, g, vertex_id);

    IGRAPH_EIT_NEXT(eit);
  }
  printf("\n");

  igraph_eit_destroy(&eit);
  igraph_es_destroy(&es);
  FREE_CONTAINER(old_vertex_id_2_new_vertex_id);
}

void Graph::DeleteVertex() { UNIMPLEMENTED(); }

void Graph::DeleteEdge() { UNIMPLEMENTED(); }

void Graph::QueryVertex() { UNIMPLEMENTED(); }

int Graph::QueryEdge(vertex_t src_id, vertex_t dest_id) {
  edge_t edge_id = -1;
  int ret = igraph_get_eid(&ipag_->graph, &edge_id, src_id, dest_id, IGRAPH_DIRECTED, /**error*/ 1);
  if (ret != IGRAPH_SUCCESS) {
    return -1;
  }
  return edge_id;
}

int Graph::GetEdgeSrc(edge_t edge_id) { return IGRAPH_FROM(&ipag_->graph, edge_id); }

int Graph::GetEdgeDest(edge_t edge_id) { return IGRAPH_TO(&ipag_->graph, edge_id); }

void Graph::QueryEdgeOtherSide() { UNIMPLEMENTED(); }

void Graph::SetGraphAttributeString(const char *attr_name, const char *value) {
  SETGAS(&ipag_->graph, attr_name, value);
}
void Graph::SetGraphAttributeNum(const char *attr_name, const int value) { SETGAN(&ipag_->graph, attr_name, value); }
void Graph::SetGraphAttributeFlag(const char *attr_name, const bool value) { SETGAB(&ipag_->graph, attr_name, value); }
void Graph::SetVertexAttributeString(const char *attr_name, vertex_t vertex_id, const char *value) {
  SETVAS(&ipag_->graph, attr_name, vertex_id, value);
}
void Graph::SetVertexAttributeNum(const char *attr_name, vertex_t vertex_id, const int value) {
  SETVAN(&ipag_->graph, attr_name, vertex_id, value);
}
void Graph::SetVertexAttributeFlag(const char *attr_name, vertex_t vertex_id, const bool value) {
  SETVAB(&ipag_->graph, attr_name, vertex_id, value);
}
void Graph::SetEdgeAttributeString(const char *attr_name, edge_t edge_id, const char *value) {
  SETEAS(&ipag_->graph, attr_name, edge_id, value);
}
void Graph::SetEdgeAttributeNum(const char *attr_name, edge_t edge_id, const int value) {
  SETEAN(&ipag_->graph, attr_name, edge_id, value);
}
void Graph::SetEdgeAttributeFlag(const char *attr_name, edge_t edge_id, const bool value) {
  SETEAB(&ipag_->graph, attr_name, edge_id, value);
}

const char *Graph::GetGraphAttributeString(const char *attr_name) {
  const char *ret_str = GAS(&ipag_->graph, attr_name);
  return ret_str;
}

const int Graph::GetGraphAttributeNum(const char *attr_name) {
  const int ret_num = GAN(&ipag_->graph, attr_name);
  return ret_num;
}

const bool Graph::GetGraphAttributeFlag(const char *attr_name) {
  const bool ret_flag = GAB(&ipag_->graph, attr_name);
  return ret_flag;
}

const char *Graph::GetVertexAttributeString(const char *attr_name, vertex_t vertex_id) {
  const char *ret_str = VAS(&ipag_->graph, attr_name, vertex_id);
  return ret_str;
}

const int Graph::GetVertexAttributeNum(const char *attr_name, vertex_t vertex_id) {
  const int ret_num = VAN(&ipag_->graph, attr_name, vertex_id);
  return ret_num;
}

const bool Graph::GetVertexAttributeFlag(const char *attr_name, vertex_t vertex_id) {
  const bool ret_flag = VAB(&ipag_->graph, attr_name, vertex_id);
  return ret_flag;
}

const char *Graph::GetEdgeAttributeString(const char *attr_name, edge_t edge_id) {
  const char *ret_str = EAS(&ipag_->graph, attr_name, edge_id);
  return ret_str;
}

const int Graph::GetEdgeAttributeNum(const char *attr_name, edge_t edge_id) {
  const int ret_num = EAN(&ipag_->graph, attr_name, edge_id);
  return ret_num;
}

const bool Graph::GetEdgeAttributeFlag(const char *attr_name, edge_t edge_id) {
  const bool ret_flag = EAB(&ipag_->graph, attr_name, edge_id);
  return ret_flag;
}

void Graph::RemoveGraphAttribute(const char *attr_name) { DELGA(&ipag_->graph, attr_name); }

void Graph::RemoveVertexAttribute(const char *attr_name) { DELVA(&ipag_->graph, attr_name); }

void Graph::RemoveEdgeAttribute(const char *attr_name) { DELEA(&ipag_->graph, attr_name); }

void Graph::MergeVertices() { UNIMPLEMENTED(); }

void Graph::SplitVertex() { UNIMPLEMENTED(); }

void Graph::CopyVertex(vertex_t new_vertex_id, Graph *g, vertex_t vertex_id) {
  igraph_vector_t gtypes, vtypes, etypes;
  igraph_strvector_t gnames, vnames, enames;
  int i;
  // igraph_vector_init(&gtypes, 0);
  igraph_vector_init(&vtypes, 0);
  // igraph_vector_init(&etypes, 0);
  // igraph_strvector_init(&gnames, 0);
  igraph_strvector_init(&vnames, 0);
  // igraph_strvector_init(&enames, 0);
  igraph_cattribute_list(&g->ipag_->graph, nullptr, nullptr, &vnames, &vtypes, nullptr, nullptr);
  /* Graph attributes */
  for (i = 0; i < igraph_strvector_size(&vnames); i++) {
    const char *vname = STR(vnames, i);
    printf("%s=", vname);
    if (VECTOR(vtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
      if (strcmp(vname, "id") == 0) {
        SETVAN(&ipag_->graph, vname, new_vertex_id, new_vertex_id);
        printf("%d", new_vertex_id);
      } else {
        SETVAN(&ipag_->graph, vname, new_vertex_id, VAN(&g->ipag_->graph, STR(vnames, i), vertex_id));
        igraph_real_printf(VAN(&g->ipag_->graph, STR(vnames, i), vertex_id));
      }

      // putchar(' ');
    } else {
      SETVAS(&ipag_->graph, vname, new_vertex_id, VAS(&g->ipag_->graph, STR(vnames, i), vertex_id));
      printf("\"%s\" ", VAS(&g->ipag_->graph, STR(vnames, i), vertex_id));
    }
  }
  printf("\n");
}

void Graph::DeleteVertices(vertex_set_t *vs) { igraph_delete_vertices(&ipag_->graph, vs->vertices); }

void Graph::DeleteExtraTailVertices() {
  // unnecessary to delete
  dbg(this->GetGraphAttributeString("name"), this->cur_vertex_num, igraph_vcount(&ipag_->graph));
  if (this->GetCurVertexNum() - 1 == igraph_vcount(&ipag_->graph)) {
    return;
  } else if (this->GetCurVertexNum() - 1 > igraph_vcount(&ipag_->graph)) {
    dbg("Error: The number of vertices is larger than pre-allocated gragh size");
    return;
  }

  // Check the number of vertices
  vertex_set_t vs;

  igraph_vs_seq(&vs.vertices, this->cur_vertex_num, igraph_vcount(&ipag_->graph) - 1);
  this->DeleteVertices(&vs);
  igraph_vs_destroy(&vs.vertices);
}

void Graph::Dfs() { UNIMPLEMENTED(); }

// void Graph::Dfs(igraph_dfshandler_t in_callback,
// igraph_dfshandler_t out_callback) {
//   igraph_dfs(&ipag_.graph, /*root=*/0, /*mode=*/ IGRAPH_OUT,
//                /*unreachable=*/ 1, /*order=*/ 0, /*order_out=*/ 0,
//                /*father=*/ 0, /*dist=*/ 0,
//                /*in_callback=*/ in_callback, /*out_callback=*/ out_callback,
//                /*extra=*/ 0);
// }

void Graph::ReadGraphGML(const char *file_name) {
  FILE *in_file = fopen(file_name, "r");
  igraph_read_graph_gml(&ipag_->graph, in_file);
  const char *graph_name = VAS(&ipag_->graph, "name", 0);
  SETGAS(&ipag_->graph, "name", graph_name);
  fclose(in_file);

  this->cur_vertex_num = igraph_vcount(&ipag_->graph);
}

void Graph::DumpGraph(const char *file_name) {
  this->DeleteExtraTailVertices();

  // Real dump
  FILE *out_file = fopen(file_name, "w");
  igraph_write_graph_gml(&ipag_->graph, out_file, 0, "Bruce Jin");
  fclose(out_file);
}

void Graph::DumpGraphDot(const char *file_name) {
  this->DeleteExtraTailVertices();

  // Real dump
  FILE *out_file = fopen(file_name, "w");
  igraph_write_graph_dot(&ipag_->graph, out_file);
  fclose(out_file);
}

void Graph::VertexTraversal(void (*CALL_BACK_FUNC)(Graph *, int, void *), void *extra) {
  igraph_vs_t vs;
  igraph_vit_t vit;
  // printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
  dbg(GetGraphAttributeString("name"));
  igraph_vs_all(&vs);
  igraph_vit_create(&ipag_->graph, vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    vertex_t vertex_id = (vertex_t)IGRAPH_VIT_GET(vit);
    dbg(vertex_id);

    // Call user-defined function
    (*CALL_BACK_FUNC)(this, vertex_id, extra);

    IGRAPH_VIT_NEXT(vit);
  }
  printf("\n");

  igraph_vit_destroy(&vit);
  igraph_vs_destroy(&vs);
  // printf("Function %s End\n", this->GetGraphAttributeString("name"));
}

int Graph::GetCurVertexNum() { return this->cur_vertex_num; }

void Graph::GetChildVertexSet(vertex_t vertex, std::vector<vertex_t> &neighbor_vertices) {
  // std::vector<vertex_t> neighbor_vertices;
  igraph_vector_t v;
  dbg(this->GetGraphAttributeString("name"), vertex);
  igraph_vector_init(&v, 0);
  igraph_neighbors(&ipag_->graph, &v, vertex, IGRAPH_OUT);
  long int neighbor_num = igraph_vector_size(&v);
  dbg(neighbor_num);
  for (long int i = 0; i < neighbor_num; i++) {
    neighbor_vertices.push_back((vertex_t)VECTOR(v)[i]);
  }
  igraph_vector_destroy(&v);
  // return neighbor_vertices;
}

}  // namespace baguatool::core