#include "graph_perf.h"
#include <stdlib.h>
#include "common/utils.h"
#include "core/graph_perf_data.h"
#include "core/vertex_type.h"

namespace baguatool::graph_perf {

GPerf::GPerf() { this->graph_perf_data = new core::GraphPerfData(); }

GPerf::~GPerf() { delete this->graph_perf_data; }

void GPerf::ReadStaticControlFlowGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  // Traverse the files
  for (const auto &fn : file_names) {
    dbg(fn);

    // Read a ControlFlowGraph from each file
    core::ControlFlowGraph *func_cfg = new core::ControlFlowGraph();
    func_cfg->ReadGraphGML(fn.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    this->func_cfg_map[std::string(func_cfg->GetGraphAttributeString("name"))] = func_cfg;
  }
}

void GPerf::GenerateControlFlowGraphs(const char *dir_name) { this->ReadStaticControlFlowGraphs(dir_name); }

core::ControlFlowGraph *GPerf::GetControlFlowGraph(std::string func_name) { return this->func_cfg_map[func_name]; }

std::map<std::string, core::ControlFlowGraph *> &GPerf::GetControlFlowGraphs() { return this->func_cfg_map; }

void GPerf::ReadStaticProgramCallGraph(const char *binary_name) {
  // Get name of static program call graph's file
  std::string static_pcg_file_name = std::string(binary_name) + std::string(".pcg");

  // Read a ProgramCallGraph from each file
  this->pcg = new core::ProgramCallGraph();
  this->pcg->ReadGraphGML(static_pcg_file_name.c_str());
}

void GPerf::ReadDynamicProgramCallGraph(std::string perf_data_file) {
  core::PerfData *perf_data = new core::PerfData();
  perf_data->Read(perf_data_file.c_str());
  auto data_size = perf_data->GetVertexDataSize();

  /** Optimization: First scan all call path and store <call_addr, callee_addr> pairs,
   * then AddEdgeWithAddr. It can reduce redundant graph query **/

  // AddEdgeWithAddr for each <call_addr, callee_addr> pair of each call path
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetVertexDataCallPath(i, call_path);
    auto value = perf_data->GetVertexDataValue(i);
    // auto process_id = perf_data->GetVertexDataProcsId(i);
    // auto thread_id = perf_data->GetVertexDataThreadId(i, process_id);

    while (!call_path.empty()) {
      auto call_addr = call_path.top();
      call_path.pop();
      auto callee_addr = call_path.top();
      this->pcg->AddEdgeWithAddr(call_addr, callee_addr);
    }
  }
}

void GPerf::GenerateProgramCallGraph(const char *binary_name) { this->ReadStaticProgramCallGraph(binary_name); }

core::ProgramCallGraph *GPerf::GetProgramCallGraph() { return this->pcg; }

void GPerf::ReadFunctionAbstractionGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  // Traverse the files
  for (const auto &fn : file_names) {
    dbg(fn);

    // Read a ProgramAbstractionGraph from each file
    core::ProgramAbstractionGraph *new_pag = new core::ProgramAbstractionGraph();
    new_pag->ReadGraphGML(fn.c_str());
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    func_pag_map[std::string(new_pag->GetGraphAttributeString("name"))] = new_pag;
  }
}

/** Intra-procedural Analysis **/

core::ProgramAbstractionGraph *GPerf::GetFunctionAbstractionGraph(std::string func_name) {
  return this->func_pag_map[func_name];
}

std::map<std::string, core::ProgramAbstractionGraph *> &GPerf::GetFunctionAbstractionGraphs() {
  return this->func_pag_map;
}

void GPerf::IntraProceduralAnalysis() {}

/** Inter-procedural Analysis **/

typedef struct InterProceduralAnalysisArg {
  std::map<std::string, core::ProgramAbstractionGraph *> *func_pag_map;
  core::ProgramCallGraph *pcg;
} InterPAArg;

// FIXME: `void *` should not appear in cpp
void ConnectCallerCallee(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  InterPAArg *arg = (InterPAArg *)extra;
  std::map<std::string, core::ProgramAbstractionGraph *> *func_name_2_pag = arg->func_pag_map;
  core::ProgramCallGraph *pcg = arg->pcg;

  // dbg(pag->GetGraphAttributeString("name"));
  int type = pag->GetVertexType(vertex_id);
  if (type == type::CALL_NODE || type == type::CALL_IND_NODE || type == type::CALL_REC_NODE) {
    int addr = pag->GetVertexAttributeNum("saddr", vertex_id);
    // dbg(vertex_id, addr);
    type::vertex_t call_vertex_id = pcg->GetCallVertexWithAddr(addr);
    // dbg(call_vertex_id);

    // ProgramAbstractionGraph *callee_pag =
    //     (*func_name_2_pag)[std::string(pag->GetVertexAttributeString("name", vertex_id))];
    auto callee_func_name = pcg->GetCallee(call_vertex_id);
    // free(callee_func_name);

    string callee_func_name_str = std::string(callee_func_name);

    // dbg(callee_func_name_str);

    if (callee_func_name) {
      core::ProgramAbstractionGraph *callee_pag = (*func_name_2_pag)[callee_func_name_str];

      if (!callee_pag->GetGraphAttributeFlag("scanned")) {
        void (*ConnectCallerCalleePointer)(core::ProgramAbstractionGraph *, int, void *) = &(ConnectCallerCallee);
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

void GPerf::InterProceduralAnalysis() {
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
  // dbg(this->root_pag->GetGraphAttributeString("name"));
  this->root_pag->VertexTraversal(&ConnectCallerCallee, arg);

  for (auto &kv : this->func_pag_map) {
    // func_name_2_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->RemoveGraphAttribute("scanned");
  }
  delete arg;

  this->root_pag->VertexSortChild();

  return;
}  // function InterProceduralAnalysis

void GPerf::GenerateProgramAbstractionGraph() { this->InterProceduralAnalysis(); }

void GPerf::SetProgramAbstractionGraph(core::ProgramAbstractionGraph *pag) { this->root_pag = pag; }

core::ProgramAbstractionGraph *GPerf::GetProgramAbstractionGraph() { return this->root_pag; }

void GPerf::DataEmbedding(core::PerfData *perf_data) {
  // Query for each call path
  auto data_size = perf_data->GetVertexDataSize();
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetVertexDataCallPath(i, call_path);
    if (!call_path.empty()) {
      call_path.pop();
    }
    auto value = perf_data->GetVertexDataValue(i);
    auto process_id = perf_data->GetVertexDataProcsId(i);
    auto thread_id = perf_data->GetVertexDataThreadId(i);

    type::vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, call_path);
    // dbg(queried_vertex_id);
    type::perf_data_t data =
        this->graph_perf_data->GetPerfData(queried_vertex_id, perf_data->GetMetricName(), process_id, thread_id);
    data += value;
    this->graph_perf_data->SetPerfData(queried_vertex_id, perf_data->GetMetricName(), process_id, thread_id, data);
  }

}  // function Dataembedding

core::GraphPerfData *GPerf::GetGraphPerfData() { return this->graph_perf_data; }

type::perf_data_t ReduceOperation(std::vector<type::perf_data_t> &perf_data, int num, string &op) {
  if (num == 0) {
    return 0.0;
  }
  if (!strcmp(op.c_str(), "AVG")) {
    type::perf_data_t avg = 0.0;
    for (int i = 0; i < num; i++) {
      avg += perf_data[i];
    }
    avg /= (type::perf_data_t)num;
    return avg;
  } else {
    return perf_data[0];
  }
}

typedef struct ReducePerfDataArg {
  // input
  core::GraphPerfData *graph_perf_data = nullptr;
  std::string metric;
  std::string op;
  // output
  type::perf_data_t total_reduced_data = 0.0;
} RPDArg;

void ReducePerfData(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  RPDArg *arg = (RPDArg *)extra;
  core::GraphPerfData *graph_perf_data = arg->graph_perf_data;
  std::string metric(arg->metric);
  std::string op(arg->op);

  int num_procs = graph_perf_data->GetMetricsPerfDataProcsNum(vertex_id, metric);

  std::vector<type::perf_data_t> im_reduced_data;

  for (int i = 0; i < num_procs; i++) {
    // type::perf_data_t* procs_perf_data = nullptr;
    std::vector<type::perf_data_t> procs_perf_data;
    graph_perf_data->GetProcsPerfData(vertex_id, metric, i, procs_perf_data);
    // int num_thread = graph_perf_data->GetProcsPerfDataThreadNum(vertex_id, metric, i);
    int num_thread = procs_perf_data.size();

    if (num_thread > 0) {
      im_reduced_data.push_back(ReduceOperation(procs_perf_data, num_thread, op));
      // dbg(im_reduced_data[i]);
    } else {
      im_reduced_data[i] = 0.0;
    }

    FREE_CONTAINER(procs_perf_data);
  }

  type::perf_data_t reduced_data = ReduceOperation(im_reduced_data, num_procs, op);
  // dbg(reduced_data);

  FREE_CONTAINER(im_reduced_data);

  pag->SetVertexAttributeString(std::string(metric + std::string("_") + op).c_str(), (type::vertex_t)vertex_id,
                                std::to_string(reduced_data).c_str());

  arg->total_reduced_data += reduced_data;
}

// Reduce each vertex's perf data
type::perf_data_t GPerf::ReduceVertexPerfData(std::string &metric, std::string &op) {
  RPDArg *arg = new RPDArg();
  arg->graph_perf_data = this->graph_perf_data;
  arg->metric = std::string(metric);
  arg->op = std::string(op);
  arg->total_reduced_data = 0.0;

  this->root_pag->VertexTraversal(&ReducePerfData, arg);

  type::perf_data_t total = arg->total_reduced_data;
  delete arg;
  return total;
}

typedef struct PerfDataToPercentArg {
  // input
  std::string new_metric;
  std::string metric;
  type::perf_data_t total;
  // output
  //...
} PDTPArg;

void PerfDataToPercent(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  PDTPArg *arg = (PDTPArg *)extra;
  std::string new_metric(arg->new_metric);
  std::string metric(arg->metric);
  type::perf_data_t total = arg->total;

  type::perf_data_t data = strtod(pag->GetVertexAttributeString(metric.c_str(), (type::vertex_t)vertex_id), NULL);
  type::perf_data_t percent = data / total;
  pag->SetVertexAttributeString(new_metric.c_str(), (type::vertex_t)vertex_id, std::to_string(percent).c_str());
}

// convert vertex's reduced data to percent
void GPerf::ConvertVertexReducedDataToPercent(std::string &metric, type::perf_data_t total, std::string &new_metric) {
  PDTPArg *arg = new PDTPArg();
  arg->new_metric = std::string(new_metric);
  arg->metric = std::string(metric);
  arg->total = total;

  this->root_pag->VertexTraversal(&PerfDataToPercent, arg);

  delete arg;
}

void GPerf::GenerateMultiProgramAbstractionGraph() {
  root_mpag = new core::ProgramAbstractionGraph();
  root_mpag->GraphInit("Multi-process Program Abstraction Graph");

  std::vector<type::vertex_t> pre_order_vertex_seq;
  root_pag->PreOrderTraversal(0, pre_order_vertex_seq);

  type::vertex_t last_new_vertex_id = -1;
  for (auto &vertex_id : pre_order_vertex_seq) {
    type::vertex_t new_vertex_id = root_mpag->AddVertex();
    root_mpag->CopyVertex(new_vertex_id, root_pag, vertex_id);
    if (last_new_vertex_id != -1) {
      root_mpag->AddEdge(last_new_vertex_id, new_vertex_id);
    }
    last_new_vertex_id = new_vertex_id;
  }
}

core::ProgramAbstractionGraph *GPerf::GetMultiProgramAbstractionGraph() { return root_mpag; }

void GPerf::PthreadAnalysis(core::PerfData *pthread_data) {
  // Query for each call path
  auto data_size = pthread_data->GetVertexDataSize();
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    pthread_data->GetVertexDataCallPath(i, call_path);
    if (!call_path.empty()) {
      call_path.pop();
    }
    auto time = pthread_data->GetVertexDataValue(i);
    auto create_thread_id = pthread_data->GetVertexDataProcsId(i);
    auto thread_id = pthread_data->GetVertexDataThreadId(i);

    // TODO: Need to judge if it is in current thread
    // if thread_id == cur_thread

    if (time == (type::perf_data_t)(-1)) {
      type::vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, call_path);
      type::addr_t addr = call_path.top();
      dbg(addr);
      type::vertex_t pthread_vertex_id = root_pag->AddVertex();
      root_pag->SetVertexBasicInfo(pthread_vertex_id, type::CALL_NODE, "pthread_create");
      root_pag->SetVertexDebugInfo(pthread_vertex_id, addr, addr);
      root_pag->SetVertexAttributeNum("id", pthread_vertex_id, pthread_vertex_id);
      root_pag->AddEdge(queried_vertex_id, pthread_vertex_id);
      for (unsigned long int j = 0; j < data_size; j++) {
        if (j != i) {
          if (create_thread_id == pthread_data->GetVertexDataProcsId(j)) {
            dbg(create_thread_id, pthread_data->GetVertexDataProcsId(j));
            std::stack<unsigned long long> call_path_j;
            pthread_data->GetVertexDataCallPath(j, call_path_j);
            if (!call_path_j.empty()) {
              call_path_j.pop();
            }
            auto time_j = pthread_data->GetVertexDataValue(j);
            auto create_thread_id_j = pthread_data->GetVertexDataProcsId(j);
            auto thread_id_j = pthread_data->GetVertexDataThreadId(j);

            type::vertex_t queried_vertex_id_j = this->root_pag->GetVertexWithCallPath(0, call_path_j);
            type::addr_t addr_j = call_path_j.top();
            dbg(addr_j);
            type::vertex_t pthread_join_vertex_id = root_pag->AddVertex();
            dbg(pthread_join_vertex_id);
            root_pag->SetVertexBasicInfo(pthread_join_vertex_id, type::CALL_NODE, "pthread_join");
            root_pag->SetVertexDebugInfo(pthread_join_vertex_id, addr_j, addr_j);
            root_pag->SetVertexAttributeNum("id", pthread_join_vertex_id, pthread_join_vertex_id);
            root_pag->AddEdge(queried_vertex_id_j, pthread_join_vertex_id);
            FREE_CONTAINER(call_path_j);
            break;
          }
        }
      }
    }
    FREE_CONTAINER(call_path);
  }

  root_pag->VertexSortChild();
}

}  // graph_perf