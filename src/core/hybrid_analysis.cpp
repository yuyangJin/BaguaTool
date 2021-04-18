#include "core/hybrid_analysis.h"

#include "common/utils.h"
#include "core/vertex_type.h"

namespace baguatool::core {

void HybridAnalysis::ReadStaticControlFlowGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  // Traverse the files
  for (const auto &fn : file_names) {
    dbg(fn);

    // Read a ControlFlowGraph from each file
    std::unique_ptr<baguatool::core::ControlFlowGraph> func_cfg = std::make_unique<ControlFlowGraph>();
    func_cfg->ReadGraphGML(fn.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    this->func_cfg_vec[fn] = std::move(func_cfg);
  }
}

void HybridAnalysis::GenerateControlFlowGraphs(const char *dir_name) { this->ReadStaticControlFlowGraphs(dir_name); }

std::unique_ptr<ControlFlowGraph> HybridAnalysis::GetControlFlowGraph(std::string func_name) {
  return std::move(this->func_cfg_vec[func_name]);
}

std::map<std::string, std::unique_ptr<ControlFlowGraph>> &HybridAnalysis::GetControlFlowGraphs() {
  return this->func_cfg_vec;
}

void HybridAnalysis::ReadStaticProgramCallGraph(std::string static_pcg_file_name) {
  // Get name of static program call graph's file

  // Read a ProgramCallGraph from each file
  this->pcg = std::make_unique<ProgramCallGraph>();
  pcg->ReadGraphGML(static_pcg_file_name.c_str());
}

void HybridAnalysis::ReadDynamicProgramCallGraph() {}

void HybridAnalysis::GenerateProgramCallGraph() {
  std::string str = std::string("tmp");
  this->ReadStaticProgramCallGraph(str);
}

std::unique_ptr<ProgramCallGraph> HybridAnalysis::GetProgramCallGraph() { return std::move(this->pcg); }

/** Intra-procedural Analysis **/

std::unique_ptr<ProgramAbstractionGraph> HybridAnalysis::GetFunctionAbstractionGraph(std::string func_name) {}

std::map<std::string, std::unique_ptr<ProgramAbstractionGraph>> &HybridAnalysis::GetFunctionAbstractionGraphs() {}

void HybridAnalysis::IntraProceduralAnalysis() {}

/** Inter-procedural Analysis **/

// FIXME: `void *` should not appear in cpp
// void ConnectCallerCallee(ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
//   std::map<std::string, ProgramAbstractionGraph *> *func_name_2_pag =
//       (std::map<std::string, ProgramAbstractionGraph *> *)extra;

//   if (pag->GetVertexAttributeNum("type", vertex_id) == CALL_NODE) {
//     ProgramAbstractionGraph *callee_pag =
//         (*func_name_2_pag)[std::string(pag->GetVertexAttributeString("name", vertex_id))];
//     void (*ConnectCallerCalleePointer)(ProgramAbstractionGraph *, int, void *) = &(ConnectCallerCallee);
//     callee_pag->VertexTraversal(ConnectCallerCalleePointer, extra);

//     // Get
//     int vertex_count = pag->GetCurVertexNum();

//     pag->AddGraph(callee_pag);

//     pag->AddEdge(vertex_id, vertex_count);
//   }
// }

void HybridAnalysis::InterProceduralAnalysis() {
  //   // Search root node , "name == main"
  //   ProgramAbstractionGraph *root_pag = nullptr;
  //   std::map<std::string, ProgramAbstractionGraph *> func_name_2_pag;

  //   for (auto pag : this->func_pag_vec) {
  //     func_name_2_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
  //     if (strcmp(pag->GetGraphAttributeString("name"), "main") == 0) {
  //       root_pag = pag;
  //       std::cout << "Find 'main'" << std::endl;
  //       // break;
  //     }
  //   }

  //   // DFS From root node
  //   void (*ConnectCallerCalleePointer)(Graph *, int, void *) = &ConnectCallerCallee;
  //   root_pag->VertexTraversal(ConnectCallerCalleePointer, (void *)&func_name_2_pag);

  //   return root_pag;
}

void HybridAnalysis::GenerateProgramAbstractionGraph() {}

}  // namespace baguatool::core