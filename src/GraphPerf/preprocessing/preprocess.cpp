#include "preprocess.h"

#include "common/utils.h"
#include "core/vertex_type.h"

namespace baguatool::graph_perf {

void Preprocess::ReadFunctionGraphs(const char *dir_name, std::vector<core::ProgramAbstractionGraph *> &func_pag_vec) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  // Traverse the files
  std::vector<std::string>::iterator vect_iter = file_names.begin();
  std::vector<std::string>::iterator vect_end = file_names.end();

  for (; vect_iter != vect_end; vect_iter++) {
    std::string file_name = (*vect_iter);
    std::cout << file_name << std::endl;

    // Read a ProgramAbstractionGraph from each file
    core::ProgramAbstractionGraph *new_pag = new core::ProgramAbstractionGraph();
    new_pag->ReadGraphGML(file_name.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    func_pag_vec.push_back(new_pag);
  }
}

void ConnectCallerCallee(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  std::map<std::string, core::ProgramAbstractionGraph *> *func_name_2_pag =
      (std::map<std::string, core::ProgramAbstractionGraph *> *)extra;

  if (pag->GetVertexAttributeNum("type", vertex_id) == core::CALL_NODE) {
    core::ProgramAbstractionGraph *callee_pag =
        (*func_name_2_pag)[std::string(pag->GetVertexAttributeString("name", vertex_id))];
    void (*ConnectCallerCalleePointer)(core::ProgramAbstractionGraph *, int, void *) = &(ConnectCallerCallee);
    callee_pag->VertexTraversal(ConnectCallerCalleePointer, extra);

    // Get
    int vertex_count = pag->GetCurVertexId();

    pag->AddGraph(callee_pag);

    pag->AddEdge(vertex_id, vertex_count);
  }
}

core::ProgramAbstractionGraph *Preprocess::InterProceduralAnalysis(
    std::vector<core::ProgramAbstractionGraph *> &func_pag_vec) {
  // Search root node , "name == main"
  core::ProgramAbstractionGraph *root_pag = nullptr;

  std::vector<core::ProgramAbstractionGraph *>::iterator vect_iter = func_pag_vec.begin();
  std::vector<core::ProgramAbstractionGraph *>::iterator vect_end = func_pag_vec.end();

  std::map<std::string, core::ProgramAbstractionGraph *> func_name_2_pag;

  for (; vect_iter != vect_end; vect_iter++) {
    core::ProgramAbstractionGraph *pag = (*vect_iter);
    func_name_2_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    if (strcmp(pag->GetGraphAttributeString("name"), "main") == 0) {
      root_pag = pag;
      std::cout << "Find 'main'" << std::endl;
      // break;
    }
  }

  // DFS From root node
  void (*ConnectCallerCalleePointer)(core::ProgramAbstractionGraph *, int, void *) = &ConnectCallerCallee;
  root_pag->VertexTraversal(ConnectCallerCalleePointer, (void *)&func_name_2_pag);

  return root_pag;
}
}  // namespace baguatool::graph_perf