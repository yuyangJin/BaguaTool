#include "pag.h"
#include <cstring>
#include <map>

int ProgramAbstractionGraph::AddVertex(){
  if (this->cur_vertex_id >= igraph_vcount(&this->ipag_)){
    igraph_add_vertices(&this->ipag_, TRUNK_SIZE, 0);
  }
  // Add a new vertex
  igraph_integer_t new_vertex_id = this->cur_vertex_id ++;

  //igraph_integer_t new_vertex_id = igraph_vcount(&this->ipag_);
  //printf("Add a vertex: %d %s\n", new_vertex_id, vertex_name);
  
  // Set basic attributes


  // Return id of new vertex
  return (int) new_vertex_id;
}

int ProgramAbstractionGraph::AddEdge(const int src_vertex_id, const int dest_vertex_id){
  // Add a new edge
  //printf("Add an edge: %d, %d\n", src_vertex_id, dest_vertex_id);
  igraph_add_edge(&this->ipag_, (igraph_integer_t)src_vertex_id, (igraph_integer_t)dest_vertex_id);
  igraph_integer_t new_edge_id = igraph_ecount(&this->ipag_);

  // Return id of new edge
  return (int) (new_edge_id - 1);
}

void ProgramAbstractionGraph::AddGraph(ProgramAbstractionGraph* g) {
  g->DeleteExtraTailVertices();

  std::map<int, int> old_vertex_id_2_new_vertex_id;


  // Step over all vertices
  igraph_vs_t vs;
  igraph_vit_t vit;

  igraph_vs_all(&vs);
  igraph_vit_create(g->GetGraph(), vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    int vertex_id = (int) IGRAPH_VIT_GET(vit);
    printf("vertex %d", vertex_id);

    // Add new vertex (the copy of that in the input g) into this pag
    int new_vertex_id = this->AddVertex();

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
  igraph_eit_create(g->GetGraph(), es, &eit);

    while (!IGRAPH_EIT_END(eit)) {
    // Get edge id
    int edge_id = (int) IGRAPH_EIT_GET(eit);
    printf("edge %d", edge_id);

    // Add new edge (the copy of that in the input g) into this pag
    int new_edge_id = this->AddEdge(old_vertex_id_2_new_vertex_id[g->GetEdgeSrc(edge_id)], old_vertex_id_2_new_vertex_id[g->GetEdgeDest(edge_id)]);

    // copy all attributes of this vertex 
    //this->CopyVertex(new_vertex_id, g, vertex_id);

    IGRAPH_EIT_NEXT(eit);
  }
  printf("\n");

  igraph_eit_destroy(&eit);
  igraph_es_destroy(&es);

}

int ProgramAbstractionGraph::SetVertexBasicInfo(const int vertex_id, const int vertex_type, const char* vertex_name){
  //
  SETVAN(&this->ipag_, "type", vertex_id, (igraph_real_t)vertex_type);
  SETVAS(&this->ipag_, "name", vertex_id, vertex_name);
}

int ProgramAbstractionGraph::SetVertexDebugInfo(const int vertex_id, const int entry_addr, const int exit_addr){
  //
  SETVAN(&this->ipag_, "s_addr", vertex_id, (igraph_real_t)entry_addr);
  SETVAN(&this->ipag_, "e_addr", vertex_id, (igraph_real_t)exit_addr);
}

void ProgramAbstractionGraph::DeleteVertex(){

}

void ProgramAbstractionGraph::DeleteEdge(){}

void ProgramAbstractionGraph::QueryVertex(){}

void ProgramAbstractionGraph::QueryEdge(){}

int ProgramAbstractionGraph::GetEdgeSrc(int edge_id){
  return IGRAPH_FROM(&this->ipag_, edge_id);
}

int ProgramAbstractionGraph::GetEdgeDest(int edge_id){
  return IGRAPH_TO(&this->ipag_, edge_id);
}

void ProgramAbstractionGraph::QueryEdgeOtherSide(){}

void ProgramAbstractionGraph::SetVertexAttribute(){}

void ProgramAbstractionGraph::SetEdgeAttribute(){}

void ProgramAbstractionGraph::GetVertexAttribute(){}

int ProgramAbstractionGraph::GetVertexAttributeNum(const char * attr_name, int vertex_id) {
  int ret_str = VAN(&this->ipag_, attr_name, vertex_id);
  return ret_str;
}

const char* ProgramAbstractionGraph::GetVertexAttributeString(const char * attr_name, int vertex_id) {
  const char* ret_str = VAS(&this->ipag_, attr_name, vertex_id);
  return ret_str;
}

void ProgramAbstractionGraph::GetEdgeAttribute(){}

const char* ProgramAbstractionGraph::GetGraphAttributeString(const char * attr_name) {
  // igraph_vector_t gtypes, vtypes, etypes;
  // igraph_strvector_t gnames, vnames, enames;
  // long int i;
  // igraph_vector_init(&gtypes, 0);
  // igraph_vector_init(&vtypes, 0);
  // igraph_vector_init(&etypes, 0);
  // igraph_strvector_init(&gnames, 0);
  // igraph_strvector_init(&vnames, 0);
  // igraph_strvector_init(&enames, 0);
  // igraph_cattribute_list(&this->ipag_, &gnames, &gtypes, &vnames, &vtypes,
  //                          &enames, &etypes);
  // /* Graph attributes */
  // for (i = 0; i < igraph_strvector_size(&gnames); i++) {
  //     printf("%s=", STR(gnames, i));
  //     if (VECTOR(gtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
  //         igraph_real_printf(GAN(&this->ipag_, STR(gnames, i)));
  //         putchar(' ');
  //     } else {
  //         printf("\"%s\" ", GAS(&this->ipag_, STR(gnames, i)));
  //     }
  // }
  // printf("\n");

  const char* ret_str = GAS(&this->ipag_, attr_name);
  return ret_str;
}

void ProgramAbstractionGraph::MergeVertices(){}

void ProgramAbstractionGraph::SplitVertex(){}

void ProgramAbstractionGraph::CopyVertex(int new_vertex_id, ProgramAbstractionGraph* g, int vertex_id){
  igraph_vector_t gtypes, vtypes, etypes;
  igraph_strvector_t gnames, vnames, enames;
  int i;
  // igraph_vector_init(&gtypes, 0);
  igraph_vector_init(&vtypes, 0);
  // igraph_vector_init(&etypes, 0);
  // igraph_strvector_init(&gnames, 0);
  igraph_strvector_init(&vnames, 0);
  // igraph_strvector_init(&enames, 0);
  igraph_cattribute_list(&g->ipag_, nullptr, nullptr, &vnames, &vtypes,
                            nullptr, nullptr);
  /* Graph attributes */
  for (i = 0; i < igraph_strvector_size(&vnames); i++) {
    const char* vname = STR(vnames, i);
    printf("%s=", vname);
    if (VECTOR(vtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
      
      if (strcmp(vname, "id") == 0) {
        SETVAN(&this->ipag_, vname, new_vertex_id, new_vertex_id);
        printf("%d", new_vertex_id);
      } else {
        SETVAN(&this->ipag_, vname, new_vertex_id, VAN(g->GetGraph(), STR(vnames, i), vertex_id));
        igraph_real_printf(VAN(g->GetGraph(), STR(vnames, i), vertex_id));
      }
      
      //putchar(' ');
    } else {
      SETVAS(&this->ipag_, vname, new_vertex_id, VAS(g->GetGraph(), STR(vnames, i), vertex_id));
      printf("\"%s\" ", VAS(g->GetGraph(), STR(vnames, i), vertex_id));
    }
  }
  printf("\n");
}

void ProgramAbstractionGraph::DeleteVertices(igraph_vs_t vs) {
  igraph_delete_vertices(&this->ipag_, vs);
}


void ProgramAbstractionGraph::DeleteExtraTailVertices() {
  // Check the number of vertices
  igraph_vs_t vs;
  igraph_vs_seq(&vs, this->cur_vertex_id, igraph_vcount(&this->ipag_) - 1);
  this->DeleteVertices(vs);
  igraph_vs_destroy(&vs);
}


void ProgramAbstractionGraph::Dfs() {

}

// void ProgramAbstractionGraph::Dfs(igraph_dfshandler_t in_callback, igraph_dfshandler_t out_callback) {
//   igraph_dfs(&this->ipag_, /*root=*/0, /*mode=*/ IGRAPH_OUT,
//                /*unreachable=*/ 1, /*order=*/ 0, /*order_out=*/ 0, 
//                /*father=*/ 0, /*dist=*/ 0,
//                /*in_callback=*/ in_callback, /*out_callback=*/ out_callback, /*extra=*/ 0);
// }

void ProgramAbstractionGraph::ReadGraphGML(const char* file_name) {
  FILE* in_file = fopen(file_name, "r");
  igraph_read_graph_gml(&this->ipag_, in_file);
  const char* graph_name = VAS(&this->ipag_, "name", 0);
  SETGAS(&this->ipag_, "name", graph_name);
  fclose(in_file);

  this->cur_vertex_id = igraph_vcount(&this->ipag_);
}

void ProgramAbstractionGraph::DumpGraph(const char* file_name) {
  this->DeleteExtraTailVertices();

  // Real dump
  FILE* out_file = fopen(file_name, "w");
  igraph_write_graph_gml(&this->ipag_, out_file, 0, "Bruce Jin");
  fclose(out_file);
}

void ProgramAbstractionGraph::DumpGraphDot(const char* file_name) {
  this->DeleteExtraTailVertices();

  // Real dump
  FILE* out_file = fopen(file_name, "w");
  igraph_write_graph_dot(&this->ipag_, out_file);
  fclose(out_file);
}


void ProgramAbstractionGraph::VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph* , int, void *), void *extra) {
  igraph_vs_t vs;
  igraph_vit_t vit;
  printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
  igraph_vs_all(&vs);
  igraph_vit_create(this->GetGraph(), vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    int vertex_id = (int) IGRAPH_VIT_GET(vit);
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

int ProgramAbstractionGraph::GetCurVertexId() {
  return this->cur_vertex_id;
}