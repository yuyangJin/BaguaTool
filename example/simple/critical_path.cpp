#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include "baguatool.h"
#include "dbg.h"

std::queue<baguatool::type::vertex_t> path;
std::queue<baguatool::type::vertex_t> critical_path;
std::map<baguatool::type::vertex_t, baguatool::type::perf_data_t> path_max_value;
baguatool::type::perf_data_t critical_path_value;
std::vector<baguatool::type::vertex_t> visited_vertices;

// void in_critical_path(baguatool::core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
//   //pag

//   //path_value
//   std::cout << vertex_id << " " ;
// }

// void out_critical_path(baguatool::core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {

// }

void traversal(baguatool::core::ProgramAbstractionGraph* pag, int vertex_id) {
  std::cout << vertex_id << " ";
  /************** in **************/
  path_max_value[vertex_id] = strtod(pag->GetVertexAttributeString("CYCAVGPERCENT", vertex_id), NULL);
  /********************************/

  std::vector<baguatool::type::vertex_t>::iterator it =
      std::find(visited_vertices.begin(), visited_vertices.end(), vertex_id);
  if (visited_vertices.end() == it) {
    // Not visited
    visited_vertices.push_back(vertex_id);
    std::vector<baguatool::type::vertex_t> children;
    pag->GetChildVertexSet(vertex_id, children);
    for (auto child : children) {
      traversal(pag, child);
    }
    /************** out **************/

    baguatool::type::perf_data_t max = 0;
    for (auto child : children) {
      if (path_max_value[child] > max) {
        max = path_max_value[child];
      }
    }
    path_max_value[vertex_id] += max;
    std::cout << vertex_id << " " << max << std::endl;
    /*********************************/
  } else {
  }
}

int main(int argc, char** argv) {
  const char* graph_name = argv[1];
  baguatool::core::ProgramAbstractionGraph* mpag = new baguatool::core::ProgramAbstractionGraph();
  mpag->ReadGraphGML(graph_name);
  // mpag -> DFS(0, in_critical_path, out_critical_path, nullptr);
  traversal(mpag, 0);
  std::cout << std::endl;
  std::cout << path_max_value[0] << std::endl;
}