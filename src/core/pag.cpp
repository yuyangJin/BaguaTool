#include "core/pag.h"

#include <cstring>
#include <map>

#include <vector>
#include "baguatool.h"
#include "igraph.h"

namespace baguatool::core {

ProgramAbstractionGraph::ProgramAbstractionGraph() {
  // // ipag_ = new PAG_graph_t;
  // ipag_ = std::make_unique<PAG_graph_t>();
  // // open attributes
  // igraph_set_attribute_table(&igraph_cattribute_table);
}

ProgramAbstractionGraph::~ProgramAbstractionGraph() {
  // igraph_destroy(&ipag_->graph);
  // // delete ipag_;
}

// int ProgramAbstractionGraph::SetVertexBasicInfo(const type::vertex_t vertex_id, const int vertex_type,
//                                                 const char *vertex_name) {
//   //
//   // SetVertexAttributeNum("type", vertex_id, (igraph_real_t)vertex_type);
//   SETVAN(&ipag_->graph, "type", vertex_id, (igraph_real_t)vertex_type);
//   SETVAS(&ipag_->graph, "name", vertex_id, vertex_name);
//   return 0;
// }

// int ProgramAbstractionGraph::SetVertexDebugInfo(const type::vertex_t vertex_id, const int entry_addr, const int
// exit_addr)
// {
//   //
//   SETVAN(&ipag_->graph, "s_addr", vertex_id, (igraph_real_t)entry_addr);
//   SETVAN(&ipag_->graph, "e_addr", vertex_id, (igraph_real_t)exit_addr);
//   return 0;
// }

// int ProgramAbstractionGraph::GetVertexType(type::vertex_t vertex) {
//   return this->GetVertexAttributeNum("type", vertex);
// } // function GetVertexType

// type::vertex_t ProgramAbstractionGraph::GetChildVertexWithAddress(type::vertex_t root_vertex, unsigned long long
// addr) {
//   std::vector<type::vertex_t> children = GetChildVertexSet(root_vertex);
//   if (0 == children.size()) {
//     return -1;
//   }

//   for (auto child : children) {
//     unsigned long long int s_addr = GetVertexAttributeNum("s_addr", child);
//     unsigned long long int e_addr = GetVertexAttributeNum("e_addr", child);
//     if (addr >= s_addr && addr <= e_addr) {
//       return child;
//     }
//   }

//   std::vector<type::vertex_t>().swap(children);

//   // Not found
//   return -1;
// }  // function GetChildVertexWithAddress

// type::vertex_t ProgramAbstractionGraph::GetVertexWithCallPath(type::vertex_t root_vertex, std::stack<unsigned long
// long>&
// call_path_stack ) {
//   // if call path stack is empty, it means the call path points to current vertex, so return it.
//   if (call_path_stack.empty()) {
//     return root_vertex;
//   }

//   // Get the top addr of the stack
//   unsigned long long addr = call_path_stack.top();

//   // while (addr > 0x40000000) {
//   //   call_path_stack.pop();
//   //   addr = call_path_stack.top();
//   // }

//   // Find the CALL vertex of current addr, addr is from calling context
//   type::vertex_t found_vertex = root_vertex;
//   while (1) {
//     type::vertex_t child_vertex = GetChildVertexWithAddress(found_vertex, addr);

//     found_vertex = child_vertex;
//     // if child_vertex is not found
//     if (-1 == child_vertex) {
//       //type::vertex_t new_vertex = this->AddVertex();
//       //this->AddEdge(root_vertex, new_vertex);
//       //this->SetVertexBasicInfo();
//       //found_vertex = new_vertex;
//       break;
//     }

//     // If found_vertex is FUNC_NODE or LOOP_NODE, then continue searching child_vertex
//     auto found_vertex_type = GetVertexType(found_vertex);
//     if (FUNC_NODE != found_vertex_type && LOOP_NODE != found_vertex_type && BB_NODE != found_vertex_type) {
//       break;
//     }
//   }

//   if (-1 == found_vertex) {
//     return root_vertex;
//   }

//   // if find the corresponding child vertex, then pop a addr from the stack.
//   call_path_stack.pop();

//   // From the found_vertex, recursively search vertex with current call path
//   return GetVertexWithCallPath(found_vertex, call_path_stack);

// }  // function GetVertexWithCallPath

void ProgramAbstractionGraph::VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *),
                                              void *extra) {
  igraph_vs_t vs;
  igraph_vit_t vit;
  // printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
  igraph_vs_all(&vs);
  igraph_vit_create(&ipag_->graph, vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    type::vertex_t vertex_id = (type::vertex_t)IGRAPH_VIT_GET(vit);
    // printf("Traverse %d\n", vertex_id);

    // Call user-defined function
    (*CALL_BACK_FUNC)(this, vertex_id, extra);

    IGRAPH_VIT_NEXT(vit);
  }
  // printf("\n");

  igraph_vit_destroy(&vit);
  igraph_vs_destroy(&vs);
  // printf("Function %s End\n", this->GetGraphAttributeString("name"));
}

struct pag_dfs_call_back_t {
  void (*IN_CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *);
  void (*OUT_CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *);
  ProgramAbstractionGraph* g;
  void* extra;
};

igraph_bool_t pag_in_callback(const igraph_t *graph, igraph_integer_t vid, igraph_integer_t dist, void *extra) {
  struct pag_dfs_call_back_t* extra_wrapper = (struct pag_dfs_call_back_t*) extra;
  if (extra_wrapper->IN_CALL_BACK_FUNC != nullptr){ 
    (*(extra_wrapper->IN_CALL_BACK_FUNC))(extra_wrapper->g, vid, extra_wrapper->extra);
  }
  return 0;
}

igraph_bool_t pag_out_callback(const igraph_t *graph, igraph_integer_t vid, igraph_integer_t dist, void *extra) {
  struct pag_dfs_call_back_t* extra_wrapper = (struct pag_dfs_call_back_t*) extra;
  if (extra_wrapper->OUT_CALL_BACK_FUNC != nullptr){ 
    (*(extra_wrapper->OUT_CALL_BACK_FUNC))(extra_wrapper->g, vid, extra_wrapper->extra);
  }
  return 0;
}

void ProgramAbstractionGraph::DFS(type::vertex_t root, void (*IN_CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *), void (*OUT_CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *),
                                              void *extra) {
  // ProgramAbstractionGraph* new_graph = new ProgramAbstractionGraph();
  // new_graph->GraphInit();
  struct pag_dfs_call_back_t* extra_wrapper = new (struct pag_dfs_call_back_t)();
  extra_wrapper->IN_CALL_BACK_FUNC = IN_CALL_BACK_FUNC;
  extra_wrapper->OUT_CALL_BACK_FUNC = OUT_CALL_BACK_FUNC;
  extra_wrapper->g = this;
  extra_wrapper->extra = extra;

  igraph_dfs(&(this->ipag_->graph), /*root=*/root, /*neimode=*/IGRAPH_OUT,
             /*unreachable=*/1, /*order=*/0, /*order_out=*/0,
             /*father=*/0, /*dist=*/0,
             /*in_callback=*/pag_in_callback, /*out_callback=*/pag_out_callback, /*extra=*/extra_wrapper);
             ///*in_callback=*/0, /*out_callback=*/0, /*extra=*/extra_wrapper);
}

struct hot_spot_t {
  char* metric_name;
  //bool preserve;
};

void in_func(baguatool::core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  //struct hot_spot_t* extra_ = (struct hot_spot_t*)extra;
  //extra_->preserve = false;

  pag->SetVertexAttributeFlag("preserve", vertex_id, false);
}

void out_func(baguatool::core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct hot_spot_t* extra_ = (struct hot_spot_t*)extra;
  const char* metric_name = extra_->metric_name;

  bool preserve_flag = false;
  type::perf_data_t data = strtod(pag->GetVertexAttributeString(std::string(metric_name).c_str(), (type::vertex_t)vertex_id), NULL);
  if (data > 0.0) {
    preserve_flag = true;
  } else {
    std::vector<type::vertex_t> children;
    pag->GetChildVertexSet(vertex_id, children);

    for (auto &child: children) {
      bool child_preserve_flag = pag->GetVertexAttributeFlag("preserve", child);
      preserve_flag |= child_preserve_flag;
    }
  }

  dbg(vertex_id, metric_name, strtod(pag->GetVertexAttributeString(std::string(metric_name).c_str(), (type::vertex_t)vertex_id), NULL), preserve_flag); //, extra_->preserve);

  pag->SetVertexAttributeFlag("preserve", vertex_id, preserve_flag);
}

void ProgramAbstractionGraph::PreserveHotVertices(char* metric_name) {
  struct hot_spot_t* arg = new (struct hot_spot_t)();
  arg->metric_name = metric_name;
  //arg->preserve = false;

  this->DFS(0, in_func, out_func, arg);
}


}  // namespace baguatool::core