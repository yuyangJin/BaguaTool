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
  g->DeleteExtraTailVertices();

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

void Graph::SetVertexAttribute() { UNIMPLEMENTED(); }

void Graph::SetEdgeAttribute() { UNIMPLEMENTED(); }

void Graph::GetVertexAttribute() { UNIMPLEMENTED(); }

int Graph::GetVertexAttributeNum(const char *attr_name, vertex_t vertex_id) {
  int ret_str = VAN(&ipag_->graph, attr_name, vertex_id);
  return ret_str;
}

const char *Graph::GetVertexAttributeString(const char *attr_name, vertex_t vertex_id) {
  const char *ret_str = VAS(&ipag_->graph, attr_name, vertex_id);
  return ret_str;
}

void Graph::GetEdgeAttribute() { UNIMPLEMENTED(); }

const char *Graph::GetGraphAttributeString(const char *attr_name) {
  // igraph_vector_t gtypes, vtypes, etypes;
  // igraph_strvector_t gnames, vnames, enames;
  // long int i;
  // igraph_vector_init(&gtypes, 0);
  // igraph_vector_init(&vtypes, 0);
  // igraph_vector_init(&etypes, 0);
  // igraph_strvector_init(&gnames, 0);
  // igraph_strvector_init(&vnames, 0);
  // igraph_strvector_init(&enames, 0);
  // igraph_cattribute_list(&ipag_.graph, &gnames, &gtypes, &vnames, &vtypes,
  //                          &enames, &etypes);
  // /* Graph attributes */
  // for (i = 0; i < igraph_strvector_size(&gnames); i++) {
  //     printf("%s=", STR(gnames, i));
  //     if (VECTOR(gtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
  //         igraph_real_printf(GAN(&ipag_.graph, STR(gnames, i)));
  //         putchar(' ');
  //     } else {
  //         printf("\"%s\" ", GAS(&ipag_.graph, STR(gnames, i)));
  //     }
  // }
  // printf("\n");

  const char *ret_str = GAS(&ipag_->graph, attr_name);
  return ret_str;
}

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
  printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
  igraph_vs_all(&vs);
  igraph_vit_create(&ipag_->graph, vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    vertex_t vertex_id = (vertex_t)IGRAPH_VIT_GET(vit);
    printf("Traverse %d\n", vertex_id);

    // Call user-defined function
    (*CALL_BACK_FUNC)(this, vertex_id, extra);

    IGRAPH_VIT_NEXT(vit);
  }
  printf("\n");

  igraph_vit_destroy(&vit);
  igraph_vs_destroy(&vs);
  printf("Function %s End\n", this->GetGraphAttributeString("name"));
}

int Graph::GetCurVertexNum() { return this->cur_vertex_num; }

std::vector<vertex_t> Graph::GetChildVertexSet(vertex_t vertex) {
  std::vector<vertex_t> neighbor_vertices;
  igraph_vector_t v;
  igraph_neighbors(&ipag_->graph, &v, vertex, IGRAPH_OUT);
  long int neighbor_num = igraph_vector_size(&v);
  for (long int i = 0; i < neighbor_num; i++) {
    neighbor_vertices.push_back((vertex_t)VECTOR(v)[i]);
  }
  igraph_vector_destroy(&v);
  return neighbor_vertices;
}

}  // namespace baguatool::core