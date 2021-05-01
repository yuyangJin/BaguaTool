#include "core/hybrid_analysis.h"

#include "core/graph_perf_data.h"

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
    ControlFlowGraph *func_cfg = new ControlFlowGraph();
    func_cfg->ReadGraphGML(fn.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    this->func_cfg_map[std::string(func_cfg->GetGraphAttributeString("name"))] = func_cfg;
  }
}

void HybridAnalysis::GenerateControlFlowGraphs(const char *dir_name) { this->ReadStaticControlFlowGraphs(dir_name); }

ControlFlowGraph *HybridAnalysis::GetControlFlowGraph(std::string func_name) { return this->func_cfg_map[func_name]; }

std::map<std::string, ControlFlowGraph *> &HybridAnalysis::GetControlFlowGraphs() { return this->func_cfg_map; }

void HybridAnalysis::ReadStaticProgramCallGraph(const char *binary_name) {
  // Get name of static program call graph's file
  std::string static_pcg_file_name = std::string(binary_name) + std::string(".pcg");

  // Read a ProgramCallGraph from each file
  this->pcg = new ProgramCallGraph();
  this->pcg->ReadGraphGML(static_pcg_file_name.c_str());
}

void HybridAnalysis::ReadDynamicProgramCallGraph(std::string perf_data_file) {
  PerfData *perf_data = new PerfData();
  perf_data->Read(perf_data_file.c_str());
  auto data_size = perf_data->GetSize();

  /** Optimization: First scan all call path and store <call_addr, callee_addr> pairs,
   * then AddEdgeWithAddr. It can reduce redundant graph query **/

  // AddEdgeWithAddr for each <call_addr, callee_addr> pair of each call path
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetCallPath(i, call_path);
    auto count = perf_data->GetSamplingCount(i);
    // auto process_id = perf_data->GetProcessId(i);
    // auto thread_id = perf_data->GetThreadId(i, process_id);

    while (!call_path.empty()) {
      auto call_addr = call_path.top();
      call_path.pop();
      auto callee_addr = call_path.top();
      this->pcg->AddEdgeWithAddr(call_addr, callee_addr);
    }
  }
}

void HybridAnalysis::GenerateProgramCallGraph(const char *binary_name) {
  this->ReadStaticProgramCallGraph(binary_name);
}

ProgramCallGraph *HybridAnalysis::GetProgramCallGraph() { return this->pcg; }

void HybridAnalysis::ReadFunctionAbstractionGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  // Traverse the files
  for (const auto &fn : file_names) {
    dbg(fn);

    // Read a ProgramAbstractionGraph from each file
    ProgramAbstractionGraph *new_pag = new ProgramAbstractionGraph();
    new_pag->ReadGraphGML(fn.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    func_pag_map[std::string(new_pag->GetGraphAttributeString("name"))] = new_pag;
  }
}

/** Intra-procedural Analysis **/

ProgramAbstractionGraph *HybridAnalysis::GetFunctionAbstractionGraph(std::string func_name) {
  return this->func_pag_map[func_name];
}

std::map<std::string, ProgramAbstractionGraph *> &HybridAnalysis::GetFunctionAbstractionGraphs() {
  return this->func_pag_map;
}

void HybridAnalysis::IntraProceduralAnalysis() {}

/** Inter-procedural Analysis **/

typedef struct InterProceduralAnalysisArg {
  std::map<std::string, ProgramAbstractionGraph *> *func_pag_map;
  ProgramCallGraph *pcg;
} InterPAArg;

// FIXME: `void *` should not appear in cpp
void ConnectCallerCallee(ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  InterPAArg *arg = (InterPAArg *)extra;
  std::map<std::string, ProgramAbstractionGraph *> *func_name_2_pag = arg->func_pag_map;
  ProgramCallGraph *pcg = arg->pcg;

  // dbg(pag->GetGraphAttributeString("name"));
  int type = pag->GetVertexType(vertex_id);
  if (type == CALL_NODE || type == CALL_IND_NODE || type == CALL_REC_NODE) {
    int addr = pag->GetVertexAttributeNum("saddr", vertex_id);
    // dbg(vertex_id, addr);
    vertex_t call_vertex_id = pcg->GetCallVertexWithAddr(addr);
    // dbg(call_vertex_id);

    // ProgramAbstractionGraph *callee_pag =
    //     (*func_name_2_pag)[std::string(pag->GetVertexAttributeString("name", vertex_id))];
    auto callee_func_name = pcg->GetCalleeVertex(call_vertex_id);
    // free(callee_func_name);

    string callee_func_name_str = std::string(callee_func_name);

    // dbg(callee_func_name_str);

    if (callee_func_name) {
      ProgramAbstractionGraph *callee_pag = (*func_name_2_pag)[callee_func_name_str];

      if (!callee_pag->GetGraphAttributeFlag("scanned")) {
        void (*ConnectCallerCalleePointer)(ProgramAbstractionGraph *, int, void *) = &(ConnectCallerCallee);
        // dbg(callee_pag->GetGraphAttributeString("name"));
        callee_pag->VertexTraversal(ConnectCallerCalleePointer, extra);
        callee_pag->SetGraphAttributeFlag("scanned", true);
      }

      // Add Vertex to
      int vertex_count = pag->GetCurVertexNum();

      pag->AddGraph(callee_pag);

      pag->AddEdge(vertex_id, vertex_count);
    }
  }
}

void HybridAnalysis::InterProceduralAnalysis() {
  // Search root node , "name" is "main"
  // std::map<std::string, ProgramAbstractionGraph *> func_name_2_pag;

  for (auto &kv : this->func_pag_map) {
    // func_name_2_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->SetGraphAttributeFlag("scanned", false);
    if (strcmp(kv.first.c_str(), "main") == 0) {
      this->root_pag = pag;
      std::cout << "Find 'main'" << std::endl;
      // break;
    }
  }

  // DFS From root node
  InterPAArg *arg = new InterPAArg();
  arg->pcg = this->pcg;
  arg->func_pag_map = &(this->func_pag_map);

  // void (*ConnectCallerCalleePointer)(Graph *, int, void *) = &ConnectCallerCallee;
  dbg(this->root_pag->GetGraphAttributeString("name"));
  this->root_pag->VertexTraversal(&ConnectCallerCallee, arg);

  for (auto &kv : this->func_pag_map) {
    // func_name_2_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->RemoveGraphAttribute("scanned");
  }
  delete arg;

  return;
}  // function InterProceduralAnalysis

void HybridAnalysis::GenerateProgramAbstractionGraph() { this->InterProceduralAnalysis(); }

void HybridAnalysis::SetProgramAbstractionGraph(ProgramAbstractionGraph *pag) { this->root_pag = pag; }

ProgramAbstractionGraph *HybridAnalysis::GetProgramAbstractionGraph() { return this->root_pag; }

void HybridAnalysis::DataEmbedding(PerfData *perf_data) {
  this->graph_perf_data = new GraphPerfData();

  // Query for each call path
  auto data_size = perf_data->GetSize();
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetCallPath(i, call_path);
    if (!call_path.empty()) {
      call_path.pop();
    }
    auto count = perf_data->GetSamplingCount(i);
    auto process_id = perf_data->GetProcessId(i);
    auto thread_id = perf_data->GetThreadId(i);

    vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, call_path);
    dbg(queried_vertex_id);
    perf_data_t data =
        this->graph_perf_data->GetPerfData(queried_vertex_id, perf_data->GetMetricName(), process_id, thread_id);
    data += count;
    this->graph_perf_data->SetPerfData(queried_vertex_id, perf_data->GetMetricName(), process_id, thread_id, data);
  }

}  // function Dataembedding

GraphPerfData *HybridAnalysis::GetGraphPerfData() { return this->graph_perf_data; }

// typedef struct ReducePerfDataArg {
//   std::string op;

// } RPDArg;

// void ReducePerfData(ProgramAbstractionGraph *pag, void* extra) {

//   perf_data_t data

// }

// void HybridAnalysis::ReduceVertexPerfData(std::string& op) {
//   // Reduce each vertex's perf data

//   pag->VertexTraversal(&ReducePerfData, arg);

// }

}  // namespace baguatool::core