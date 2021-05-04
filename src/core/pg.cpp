#include "core/pg.h"
#include <stack>
#include "baguatool.h"
#include "common/common.h"
#include "vertex_type.h"

namespace baguatool::core {

ProgramGraph::ProgramGraph() {}
ProgramGraph::~ProgramGraph() {}

void ProgramGraph::VertexTraversal(void (*CALL_BACK_FUNC)(ProgramGraph *, int, void *), void *extra) {
  // this->DeleteExtraTailVertices();
  igraph_vs_t vs;
  igraph_vit_t vit;
  // printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
  // dbg(this->GetGraphAttributeString("name"));

  igraph_vs_seq(&vs, 0, this->cur_vertex_num - 1);
  igraph_vit_create(&ipag_->graph, vs, &vit);
  while (!IGRAPH_VIT_END(vit)) {
    // Get vector id
    vertex_t vertex_id = (vertex_t)IGRAPH_VIT_GET(vit);
    // printf("Traverse %d\n", vertex_id);
    // dbg(vertex_id);

    // Call user-defined function
    (*CALL_BACK_FUNC)(this, vertex_id, extra);

    IGRAPH_VIT_NEXT(vit);
  }
  printf("\n");

  igraph_vit_destroy(&vit);
  igraph_vs_destroy(&vs);
  // printf("Function %s End\n", this->GetGraphAttributeString("name"));
}

int ProgramGraph::SetVertexBasicInfo(const vertex_t vertex_id, const int vertex_type, const char *vertex_name) {
  //
  // SetVertexAttributeNum("type", vertex_id, (igraph_real_t)vertex_type);
  SETVAN(&ipag_->graph, "type", vertex_id, (igraph_real_t)vertex_type);
  SETVAS(&ipag_->graph, "name", vertex_id, vertex_name);
  return 0;
}

int ProgramGraph::SetVertexDebugInfo(const vertex_t vertex_id, const int entry_addr, const int exit_addr) {
  //
  SETVAN(&ipag_->graph, "saddr", vertex_id, (igraph_real_t)entry_addr);
  SETVAN(&ipag_->graph, "eaddr", vertex_id, (igraph_real_t)exit_addr);
  return 0;
}

int ProgramGraph::GetVertexType(vertex_t vertex) {
  return this->GetVertexAttributeNum("type", vertex);
}  // function GetVertexType

vertex_t ProgramGraph::GetChildVertexWithAddr(vertex_t root_vertex, unsigned long long addr) {
  // std::vector<vertex_t> children = GetChildVertexSet(root_vertex);
  std::vector<vertex_t> children;
  GetChildVertexSet(root_vertex, children);
  if (0 == children.size()) {
    return -1;
  }

  for (auto &child : children) {
    dbg(child);
    unsigned long long int s_addr = GetVertexAttributeNum("saddr", child);
    unsigned long long int e_addr = GetVertexAttributeNum("eaddr", child);
    dbg(addr, s_addr, e_addr);
    int type = GetVertexType(child);
    if (type == CALL_NODE || type == CALL_REC_NODE || type == CALL_IND_NODE) {
      if (addr >= s_addr - 4 && addr <= e_addr + 4) {
        return child;
      }
    } else {
      if (addr >= s_addr - 4 && addr <= e_addr + 4) {
        return child;
      }
    }
  }

  FREE_CONTAINER(children);
  // std::vector<vertex_t>().swap(children);

  // Not found
  return -1;
}  // function GetChildVertexWithAddr

vertex_t ProgramGraph::GetVertexWithCallPath(vertex_t root_vertex, std::stack<unsigned long long> &call_path_stack) {
  // if call path stack is empty, it means the call path points to current vertex, so return it.
  if (call_path_stack.empty()) {
    return root_vertex;
  }

  // Get the top addr of the stack
  unsigned long long addr = call_path_stack.top();

  dbg(addr);

  if (addr > 0x40000000) {
    call_path_stack.pop();
    return GetVertexWithCallPath(root_vertex, call_path_stack);
  }

  // Find the CALL vertex of current addr, addr is from calling context
  vertex_t found_vertex = root_vertex;
  vertex_t child_vertex = -1;
  while (1) {
    child_vertex = GetChildVertexWithAddr(found_vertex, addr);
    dbg(child_vertex);

    // if child_vertex is not found
    if (-1 == child_vertex) {
      // vertex_t new_vertex = this->AddVertex();
      // this->AddEdge(root_vertex, new_vertex);
      // this->SetVertexBasicInfo();
      // found_vertex = new_vertex;

      break;
    }
    found_vertex = child_vertex;

    // If found_vertex is FUNC_NODE or LOOP_NODE, then continue searching child_vertex
    auto found_vertex_type = GetVertexType(found_vertex);
    if (FUNC_NODE != found_vertex_type && LOOP_NODE != found_vertex_type && BB_NODE != found_vertex_type) {
      break;
    }
  }

  if (-1 == child_vertex) {
    return found_vertex;
  }

  // if find the corresponding child vertex, then pop a addr from the stack.
  call_path_stack.pop();

  // From the found_vertex, recursively search vertex with current call path
  return GetVertexWithCallPath(found_vertex, call_path_stack);

}  // function GetVertexWithCallPath

// void
typedef struct CallVertexWithAddrArg {
  unsigned long long addr;  // input
  vertex_t vertex_id;       // output
  bool find_flag = false;   // find flag
} CVWAArg;

void CallVertexWithAddr(ProgramGraph *pg, int vertex_id, void *extra) {
  CVWAArg *arg = (CVWAArg *)extra;
  if (arg->find_flag) {
    return;
  }
  unsigned long long addr = arg->addr;
  if (pg->GetVertexAttributeNum("type", vertex_id) == CALL_NODE ||
      pg->GetVertexAttributeNum("type", vertex_id) == CALL_IND_NODE ||
      pg->GetVertexAttributeNum("type", vertex_id) == CALL_REC_NODE) {
    unsigned long long s_addr = pg->GetVertexAttributeNum("saddr", vertex_id);
    unsigned long long e_addr = pg->GetVertexAttributeNum("eaddr", vertex_id);
    dbg(addr, s_addr, e_addr);
    if (addr >= s_addr - 4 && addr <= e_addr + 4) {
      arg->vertex_id = vertex_id;
      arg->find_flag = true;
      return;
    }
  }
  return;
}

vertex_t ProgramGraph::GetCallVertexWithAddr(unsigned long long addr) {
  CVWAArg *arg = new CVWAArg();
  arg->addr = addr;
  this->VertexTraversal(&CallVertexWithAddr, arg);
  vertex_t vertex_id = arg->vertex_id;
  delete arg;
  return vertex_id;
}

void FuncVertexWithAddr(ProgramGraph *pg, int vertex_id, void *extra) {
  CVWAArg *arg = (CVWAArg *)extra;
  if (arg->find_flag) {
    return;
  }
  unsigned long long addr = arg->addr;
  if (pg->GetVertexAttributeNum("type", vertex_id) == FUNC_NODE) {
    unsigned long long s_addr = pg->GetVertexAttributeNum("saddr", vertex_id);
    unsigned long long e_addr = pg->GetVertexAttributeNum("eaddr", vertex_id);
    if (addr >= s_addr - 4 && addr <= e_addr + 4) {
      arg->vertex_id = vertex_id;
      arg->find_flag = true;
      return;
    }
  }
  return;
}

vertex_t ProgramGraph::GetFuncVertexWithAddr(unsigned long long addr) {
  CVWAArg *arg = new CVWAArg();
  arg->addr = addr;
  this->VertexTraversal(&FuncVertexWithAddr, arg);
  vertex_t vertex_id = arg->vertex_id;
  delete arg;
  return vertex_id;
}

int ProgramGraph::AddEdgeWithAddr(unsigned long long call_addr, unsigned long long callee_addr) {
  vertex_t call_vertex = GetCallVertexWithAddr(call_addr);
  vertex_t callee_vertex = GetFuncVertexWithAddr(callee_addr);
  if (!this->QueryEdge(call_vertex, callee_vertex)) {
    edge_t edge_id = this->AddEdge(call_vertex, callee_vertex);
    return edge_id;
  }
  return -1;
}

const char *ProgramGraph::GetCalleeVertex(vertex_t vertex_id) {
  // dbg(GetVertexAttributeString("name", vertex_id));
  std::vector<vertex_t> children;
  GetChildVertexSet(vertex_id, children);
  if (0 == children.size()) {
    return nullptr;
  }

  for (auto &child : children) {
    if (GetVertexType(child) == FUNC_NODE) {
      // dbg(GetVertexAttributeString("name", child));
      return GetVertexAttributeString("name", child);
    }
  }

  FREE_CONTAINER(children);
  // std::vector<vertex_t>().swap(children);

  // Not found
  return nullptr;
}

struct compare_addr {
  compare_addr(const std::vector<unsigned long long> &v) : _v(v) {}
  bool operator()(const size_t i, const size_t j) const { return _v[i] < _v[j]; }
  const std::vector<unsigned long long> &_v;
};

void SortChild(ProgramGraph *pg, int vertex_id, void *extra) {
  std::vector<vertex_t> children;
  pg->GetChildVertexSet(vertex_id, children);

  if (0 == children.size()) {
    return;
  }

  std::vector<vertex_t> children_id = children;

  std::vector<unsigned long long> children_s_addr;
  for (auto &child : children) {
    unsigned long long s_addr = pg->GetVertexAttributeNum("saddr", child);
    children_s_addr.push_back(s_addr);
  }

  // Sort by s_addr
  std::sort(children_id.begin(), children_id.end(), compare_addr(children_s_addr));
  dbg(children, children_id);

  // Swap vertex, children is original sequence, children_id is sorted sequence

  int num_children = children.size();
  std::map<vertex_t, vertex_t> vertex_id_to_tmp_vertex_id;
  std::map<vertex_t, std::vector<edge_t>> vertex_id_to_tmp_edge_id_vec;
  for (int i = 0; i < num_children; i++) {
    if (children[i] != children_id[i]) {
      // Copy attributes except "id"
      vertex_t tmp_vertex_id = pg->AddVertex();
      pg->CopyVertex(tmp_vertex_id, pg, children[i]);
      // If children_id[i] is covered, use tmp_vertex in vertex_id_to_tmp_vertex_id
      if (vertex_id_to_tmp_vertex_id.count(children_id[i]) > 0) {
        pg->CopyVertex(children[i], pg, vertex_id_to_tmp_vertex_id[children_id[i]]);
        // pg->SetVertexAttributeNum("id", children[i], children_id[i]);
      } else {
        pg->CopyVertex(children[i], pg, children_id[i]);
      }
      vertex_id_to_tmp_vertex_id[children[i]] = tmp_vertex_id;

      // Get and record children of children[i]
      std::vector<vertex_t> children_children;
      pg->GetChildVertexSet(children[i], children_children);
      vertex_id_to_tmp_edge_id_vec[children[i]] = children_children;
      // Delete all edges of children[i]
      for (auto &child_child : children_children) {
        dbg(children[i], child_child);
        pg->DeleteEdge(children[i], child_child);
      }
      if (vertex_id_to_tmp_edge_id_vec.count(children_id[i]) > 0) {
        std::vector<vertex_t> &tmp_children_children = vertex_id_to_tmp_edge_id_vec[children_id[i]];
        for (auto &child_child : tmp_children_children) {
          dbg(children[i], child_child);
          pg->AddEdge(children[i], child_child);
        }
      } else {
        // Get children of children_id[i]
        std::vector<vertex_t> children_id_children;
        pg->GetChildVertexSet(children_id[i], children_id_children);
        dbg(children_id[i], children_id_children);

        for (auto &child_child : children_id_children) {
          dbg(children[i], child_child);
          pg->AddEdge(children[i], child_child);
        }
        FREE_CONTAINER(children_id_children);
      }
    }
  }

  // for (auto& vertex: vertex_id_to_tmp_vertex_id) {
  //   dbg(pg->GetCurVertexNum() - 1);
  //   pg->DeleteVertex(pg->GetCurVertexNum() - 1);
  // }

  for (auto &vertex : vertex_id_to_tmp_vertex_id) {
    dbg(vertex.second);
    pg->DeleteVertex(vertex.second);
  }

  FREE_CONTAINER(children);
  FREE_CONTAINER(children_id);
  FREE_CONTAINER(children_s_addr);
  FREE_CONTAINER(vertex_id_to_tmp_vertex_id);
  for (auto &item : vertex_id_to_tmp_edge_id_vec) {
    FREE_CONTAINER(item.second);
  }
  FREE_CONTAINER(vertex_id_to_tmp_edge_id_vec);

  return;
}

void ProgramGraph::VertexSortChild() {
  this->VertexTraversal(&SortChild, nullptr);
  return;
}
}