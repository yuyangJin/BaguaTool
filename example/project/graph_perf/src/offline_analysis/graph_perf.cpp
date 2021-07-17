#include "graph_perf.h"

//#define IGNORE_SHARED_OBJ

// some functions are based on graph perf-specific sampler & static analyzer

namespace std {
template <typename T1, typename T2>
struct less<std::pair<T1, T2>> {
  bool operator()(const std::pair<T1, T2> &l, const std::pair<T1, T2> &r) const {
    if (l.first == r.first) return l.second > r.second;

    return l.first < r.first;
  }
};
}

namespace graph_perf {

using namespace baguatool;

std::map<type::thread_t, std::pair<type::call_path_t, type::thread_t>> created_tid_2_callpath_and_tid;

bool build_create_tid_to_callpath_and_tid_flag = false;
void build_create_tid_to_callpath_and_tid(core::PerfData *perf_data) {
  // Build created_tid_2_callpath_and_tid
  auto edge_data_size = perf_data->GetEdgeDataSize();
  for (unsigned long int i = 0; i < edge_data_size; i++) {
    // Value of pthread_create is recorded as (-1), Value of GOMP_parallel is recorded as (-2)
    auto value = perf_data->GetEdgeDataValue(i);
    if (value <= (type::perf_data_t)(-1)) {
      type::call_path_t src_call_path;
      perf_data->GetEdgeDataSrcCallPath(i, src_call_path);
      auto create_thread_id = perf_data->GetEdgeDataDestThreadId(i);
      auto thread_id = perf_data->GetEdgeDataSrcThreadId(i);
      std::pair<type::call_path_t, int> tmp(src_call_path, thread_id);
      created_tid_2_callpath_and_tid[create_thread_id] = tmp;
      // printf("map[%d] = < %llx , %d > \n", create_thread_id, src_call_path.top(), thread_id);
    }
  }
  build_create_tid_to_callpath_and_tid_flag = true;
}

void preprocess_call_path(std::stack<type::addr_t> &call_path) {
  std::stack<type::addr_t> tmp;
  while (!call_path.empty()) {
    type::addr_t addr = call_path.top();
    call_path.pop();
    if (type::IsValidAddr(addr)) {
      tmp.push(addr);
    }
  }
  bool exe_addr_exist = false;
  while (!tmp.empty()) {
    type::addr_t addr = tmp.top();
    tmp.pop();
    if (type::IsTextAddr(addr)) {
      call_path.push(addr);
      exe_addr_exist = true;
    } else if (exe_addr_exist == false) {
      call_path.push(addr);
    }
  }
  FREE_CONTAINER(tmp);
}

void delete_all_so_addr(std::stack<type::addr_t> &call_path) {
  std::stack<type::addr_t> tmp;
  while (!call_path.empty()) {
    type::addr_t addr = call_path.top();
    call_path.pop();
    if (type::IsTextAddr(addr)) {
      tmp.push(addr);
    }
  }

  while (!tmp.empty()) {
    type::addr_t addr = tmp.top();
    tmp.pop();
    call_path.push(addr);
  }
  FREE_CONTAINER(tmp);
}

GPerf::GPerf() { this->root_mpag = new core::ProgramAbstractionGraph(); }

GPerf::~GPerf() {
  for (auto &kv : created_tid_2_callpath_and_tid) {
    FREE_CONTAINER(kv.second.first);
  }
  FREE_CONTAINER(created_tid_2_callpath_and_tid);
}

void GPerf::ReadProgramAbstractionGraphMap(const char *file_name) {
  std::string file_name_str = std::string(file_name);
  ReadMap<int, type::addr_t>(this->hash_to_entry_addr, file_name_str);
}

void GPerf::ReadStaticControlFlowGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  std::string pag_map = dir_name + std::string(".map");
  ReadProgramAbstractionGraphMap(pag_map.c_str());
  // std::stringstream ss;
  //   ss << dir_name << ".map";
  // auto file_name = ss.str();

  // std::map<int, std::string> hash_2_func;
  // ReadMap<int, std::string>(hash_2_func, file_name);

  // Traverse the files
  for (const auto &hash_str : file_names) {
    // dbg(hash_str);

    // Read a ControlFlowGraph from each file
    core::ControlFlowGraph *func_cfg = new core::ControlFlowGraph();
    func_cfg->ReadGraphGML(hash_str.c_str());

    std::vector<std::string> split_vec, split_vec_1;
    split(hash_str, std::string("/"), split_vec);

    split(split_vec[split_vec.size() - 1], std::string("."), split_vec_1);
    // new_pag->DumpGraph((file_name + std::string(".bak")).c_str());
    int hash = strtoull(split_vec_1[0].c_str(), NULL, 10);

    FREE_CONTAINER(split_vec);
    FREE_CONTAINER(split_vec_1);

    if (this->hash_to_entry_addr.find(hash) != this->hash_to_entry_addr.end()) {
      type::addr_t entry_addr = this->hash_to_entry_addr[hash];
      // this->func_entry_addr_to_cfg[std::string(func_cfg->GetGraphAttributeString("name"))] = func_cfg;
      this->func_entry_addr_to_cfg[entry_addr] = func_cfg;
    }
  }
}

void GPerf::GenerateControlFlowGraphs(const char *dir_name) { this->ReadStaticControlFlowGraphs(dir_name); }

core::ControlFlowGraph *GPerf::GetControlFlowGraph(type::addr_t entry_addr) {
  return this->func_entry_addr_to_cfg[entry_addr];
}

std::map<type::addr_t, core::ControlFlowGraph *> &GPerf::GetControlFlowGraphs() { return this->func_entry_addr_to_cfg; }

void SetCallTypeAsStatic(core::ProgramCallGraph *pcg, type::edge_t edge_id, void *extra) {
  pcg->SetEdgeType(edge_id, type::STA_CALL_EDGE);  // static
}

void GPerf::ReadStaticProgramCallGraph(const char *binary_name) {
  // Get name of static program call graph's file
  std::string static_pcg_file_name = std::string(binary_name) + std::string(".pcg");

  // Read a ProgramCallGraph from each file
  this->pcg = new core::ProgramCallGraph();
  this->pcg->ReadGraphGML(static_pcg_file_name.c_str());
  this->pcg->EdgeTraversal(&SetCallTypeAsStatic, nullptr);
}

void GPerf::ReadDynamicProgramCallGraph(core::PerfData *perf_data, std::string &shared_obj_map_file_name) {
#ifdef IGNORE_SHARED_OBJ
  auto data_size = perf_data->GetVertexDataSize();
  // dbg(data_size);

  /** Optimization: First scan all call path and store <call_addr, callee_addr> pairs,
   * then AddEdgeWithAddr. It can reduce redundant graph query **/

  // AddEdgeWithAddr for each <call_addr, callee_addr> pair of each call path
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetVertexDataCallPath(i, call_path);

    if (!call_path.empty()) {
      call_path.pop();
    }

    while (!call_path.empty()) {
      // dbg(i, call_path.top());
      type::addr_t call_addr, callee_addr;
      // Get call fucntion address
      while (!call_path.empty()) {
        call_addr = call_path.top();
        call_path.pop();
        if (type::IsTextAddr(call_addr)) {
          break;
        }
      }
      // Get callee function address
      while (!call_path.empty()) {
        callee_addr = call_path.top();
        if (type::IsTextAddr(callee_addr)) {
          break;
        } else {
          call_path.pop();
        }
      }

      // if (call_addr == 0x408037 || callee_addr == 0x408b59) {
      //   dbg(call_addr, callee_addr);
      // }
      auto edge_id = this->pcg->AddEdgeWithAddr(call_addr, callee_addr);
      if (edge_id != -1) {
        this->pcg->SetEdgeType(edge_id, type::DYN_CALL_EDGE);  // dynamic
      }
    }
  }
#else
  auto data_size = perf_data->GetVertexDataSize();
  std::set<std::pair<type::addr_t, type::addr_t>> visited_call_callee;
  auto shared_obj_analysis = std::make_unique<baguatool::collector::SharedObjAnalysis>();
  // std::string somap_file_name = std::string("SOMAP-123729.TXT");
  shared_obj_analysis->ReadSharedObjMap(shared_obj_map_file_name);
  std::unordered_set<type::addr_t> addrs;

  /** Get debug info of shared object library addresses */

  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<unsigned long long> call_path;
    perf_data->GetVertexDataCallPath(i, call_path);

    if (!call_path.empty()) {
      call_path.pop();
    }

    preprocess_call_path(call_path);

    while (!call_path.empty()) {
      type::addr_t call_addr = call_path.top();
      call_path.pop();
      if (type::IsDynAddr(call_addr)) {
        addrs.insert(call_addr);
      }
    }
  }
  std::map<type::addr_t, type::addr_debug_info_t *> debug_info_map;
  shared_obj_analysis->GetDebugInfos(addrs, debug_info_map);

  /** Traverse all call-callee in the sampled data */

  int j = 0;
  for (unsigned long int i = 0; i < data_size; i++) {
    std::stack<type::addr_t> call_path;
    perf_data->GetVertexDataCallPath(i, call_path);

    if (!call_path.empty()) {
      call_path.pop();
    }

    preprocess_call_path(call_path);

    while (!call_path.empty()) {
      type::addr_t call_addr, callee_addr;

      /** Get call fucntion address */
      while (!call_path.empty()) {
        call_addr = call_path.top();
        call_path.pop();
        if (type::IsValidAddr(call_addr)) {
          break;
        }
      }
      /** Get callee function address */
      while (!call_path.empty()) {
        callee_addr = call_path.top();

        if (type::IsValidAddr(call_addr)) {
          if (!(type::IsDynAddr(call_addr) && debug_info_map.find(callee_addr) == debug_info_map.end())) {
            break;
          }
        }
        call_path.pop();
      }
      j++;

      std::pair<type::addr_t, type::addr_t> call_callee_pair = std::make_pair(call_addr, callee_addr);
      if (visited_call_callee.find(call_callee_pair) != visited_call_callee.end()) {
        continue;
      }
      dbg(call_addr, callee_addr);

      auto edge_id = this->pcg->AddEdgeWithAddr(call_addr, callee_addr);
      if (edge_id != -1) {
        this->pcg->SetEdgeType(edge_id, type::DYN_CALL_EDGE);  // dynamic
        dbg("find edge", edge_id);

      } else {
        type::vertex_t call_vertex = this->pcg->GetCallVertexWithAddr(call_addr);
        if (call_vertex != -1) {
          dbg("find call vertex", call_vertex);

          std::string &func_name = debug_info_map[callee_addr]->GetFuncName();

          type::vertex_t new_func_vertex_id = this->pcg->AddVertex();
          this->pcg->SetVertexBasicInfo(new_func_vertex_id, type::FUNC_NODE, func_name.c_str());
          this->pcg->SetVertexDebugInfo(new_func_vertex_id, callee_addr, callee_addr);

          type::edge_t new_func_edge_id = this->pcg->AddEdge(call_vertex, new_func_vertex_id);
          this->pcg->SetEdgeType(new_func_edge_id, type::DYN_CALL_EDGE);  // dynamic

          type::vertex_t new_call_vertex_id = this->pcg->AddVertex();
          this->pcg->SetVertexBasicInfo(new_call_vertex_id, type::CALL_NODE, func_name.c_str());
          this->pcg->SetVertexDebugInfo(new_call_vertex_id, callee_addr, callee_addr);

          type::edge_t new_call_edge_id = this->pcg->AddEdge(new_func_vertex_id, new_call_vertex_id);

          dbg(func_name, new_func_vertex_id);
          this->pcg->SetEdgeType(new_call_edge_id, type::DYN_CALL_EDGE);  // dynamic

          /** Build a program abstraction graph in the func_entry_addr_to_pag*/
          core::ProgramAbstractionGraph *new_pag = new core::ProgramAbstractionGraph();
          new_pag->GraphInit(func_name.c_str());
          new_func_vertex_id = new_pag->AddVertex();
          new_pag->SetVertexBasicInfo(new_func_vertex_id, type::FUNC_NODE, func_name.c_str());
          new_pag->SetVertexDebugInfo(new_func_vertex_id, callee_addr, callee_addr);

          new_call_vertex_id = new_pag->AddVertex();
          new_pag->SetVertexBasicInfo(new_call_vertex_id, type::CALL_NODE, "CALL_SO");
          new_pag->SetVertexDebugInfo(new_call_vertex_id, callee_addr, callee_addr);

          new_pag->AddEdge(new_func_vertex_id, new_call_vertex_id);

          this->func_entry_addr_to_pag[callee_addr] = new_pag;
          new_pag->SetGraphAttributeFlag("scanned", false);

        } else {
          dbg("not find call vertex");
        }
      }
      visited_call_callee.insert(std::make_pair(call_addr, callee_addr));
    }
  }
  dbg(j, visited_call_callee.size());

  for (auto kv : debug_info_map) {
    FREE_CONTAINER((*(kv.second)));
    delete kv.second;
  }
  FREE_CONTAINER(debug_info_map);
  FREE_CONTAINER(visited_call_callee);
#endif
}

void GPerf::GenerateProgramCallGraph(const char *binary_name, core::PerfData *perf_data,
                                     std::string &shared_obj_map_file_name) {
  this->ReadStaticProgramCallGraph(binary_name);
  this->ReadDynamicProgramCallGraph(perf_data, shared_obj_map_file_name);
}

core::ProgramCallGraph *GPerf::GetProgramCallGraph() { return this->pcg; }

void GPerf::ReadFunctionAbstractionGraphs(const char *dir_name) {
  // Get name of files in this directory
  std::vector<std::string> file_names;
  getFiles(std::string(dir_name), file_names);

  int dir_name_len = strlen(dir_name);
  char dir_name_copy[MAX_STR_LEN] = {0};
  for (int i = 0; i < dir_name_len - 1; i++) {
    dir_name_copy[i] = dir_name[i];
  }
  if (dir_name[dir_name_len - 1] != '/') {
    dir_name_copy[dir_name_len - 1] = dir_name[dir_name_len - 1];
  }

  std::string pag_map = dir_name_copy + std::string(".map");
  ReadProgramAbstractionGraphMap(pag_map.c_str());
  std::string(test_str) = std::string("test.txt");
  DumpMap<int, type::addr_t>(this->hash_to_entry_addr, test_str);

  // Traverse the files
  for (const auto &hash_str : file_names) {
    // dbg(hash_str);

    // Read a ProgramAbstractionGraph from each file
    core::ProgramAbstractionGraph *new_pag = new core::ProgramAbstractionGraph();
    new_pag->ReadGraphGML(hash_str.c_str());
    // func_entry_addr_to_pag[std::string(new_pag->GetGraphAttributeString("name"))] = new_pag;

    /**
     * Extract entry_addr from ./[bin_name]/[hash_str].pag and [bin_name].pag.map ([hash_str, entry_addr])
    */

    std::vector<std::string> split_vec, split_vec_1;
    split(hash_str, std::string("/"), split_vec);
    // dbg(split_vec.size());
    split(split_vec[split_vec.size() - 1], std::string("."), split_vec_1);
    int hash = strtoull(split_vec_1[0].c_str(), NULL, 10);

    FREE_CONTAINER(split_vec);
    FREE_CONTAINER(split_vec_1);
    // dbg(hash);
    if (this->hash_to_entry_addr.find(hash) != this->hash_to_entry_addr.end()) {
      type::addr_t entry_addr = this->hash_to_entry_addr[hash];
      // dbg(entry_addr);
      this->func_entry_addr_to_pag[entry_addr] = new_pag;
    }
  }
}

/** Intra-procedural Analysis **/

core::ProgramAbstractionGraph *GPerf::GetFunctionAbstractionGraph(type::addr_t func_entry_addr) {
  return this->func_entry_addr_to_pag[func_entry_addr];
}

std::map<type::addr_t, core::ProgramAbstractionGraph *> &GPerf::GetFunctionAbstractionGraphs() {
  return this->func_entry_addr_to_pag;
}

core::ProgramAbstractionGraph *GPerf::GetFunctionAbstractionGraphByAddr(type::addr_t addr) {
  for (auto &m : func_entry_addr_to_pag) {
    auto s_addr = m.second->GetVertexEntryAddr(0);
    auto e_addr = m.second->GetVertexExitAddr(0);
    // dbg(m.first, addr, s_addr, e_addr);
    if (addr >= s_addr - 4 && addr <= e_addr + 4) {
      return m.second;
    }
  }
  return nullptr;
}

void GPerf::IntraProceduralAnalysis() {}

/** Inter-procedural Analysis **/

// For recursive call, record function on call path
// std::map<type::addr_t, type::vertex_t> addr_2_vertex_call_stack;

typedef struct InterProceduralAnalysisArg {
  // std::map<std::string, core::ProgramAbstractionGraph *> *func_entry_addr_to_pag;
  std::map<type::addr_t, core::ProgramAbstractionGraph *> *func_entry_addr_to_pag;
  core::ProgramCallGraph *pcg;
} InterPAArg;

// FIXME: `void *` should not appear in cpp
void ConnectCallerCallee(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  InterPAArg *arg = (InterPAArg *)extra;
  std::map<type::addr_t, core::ProgramAbstractionGraph *> *func_entry_addr_to_pag = arg->func_entry_addr_to_pag;
  core::ProgramCallGraph *pcg = arg->pcg;

  // dbg(pag->GetGraphAttributeString("name"));
  int type = pag->GetVertexType(vertex_id);
  if (type == type::CALL_NODE || type == type::CALL_IND_NODE || type == type::CALL_REC_NODE) {
    type::addr_t addr = pag->GetVertexAttributeNum("saddr", vertex_id);
    // dbg("call_addr", vertex_id, addr);
    type::vertex_t call_vertex_id = pcg->GetCallVertexWithAddr(addr);
    // dbg(call_vertex_id);

    if (call_vertex_id == -1) {
      return;
    }

    type::addr_t callee_func_entry_addr = pcg->GetCalleeEntryAddr(call_vertex_id);

    if (!callee_func_entry_addr) {
      return;
    }
    // printf("callee_func_entry_addr: %llu ", callee_func_entry_addr);

    // if (callee_func_entry_addr) {
    for (type::addr_t entry_addr = callee_func_entry_addr - 4; entry_addr <= callee_func_entry_addr + 4; entry_addr++) {
      if (func_entry_addr_to_pag->find(entry_addr) != func_entry_addr_to_pag->end()) {
        core::ProgramAbstractionGraph *callee_pag = (*func_entry_addr_to_pag)[entry_addr];
        // printf("%s \n", callee_pag->GetGraphAttributeString("name"));

        if (!callee_pag->GetGraphAttributeFlag("scanned")) {
          void (*ConnectCallerCalleePointer)(core::ProgramAbstractionGraph *, int, void *) = &(ConnectCallerCallee);
          // dbg(callee_pag->GetGraphAttributeString("name"));
          callee_pag->SetGraphAttributeFlag("scanned", true);
          // printf("Enter %s\n", callee_pag->GetGraphAttributeString("name"));
          callee_pag->VertexTraversal(ConnectCallerCalleePointer, extra);
          // printf("Exit %s\n", callee_pag->GetGraphAttributeString("name"));
        }

        // Add Vertex to
        int vertex_count = pag->GetCurVertexNum();

        pag->AddGraph(callee_pag);

        pag->AddEdge(vertex_id, vertex_count);
      }
    }
    //}
  }
}

void GPerf::DynamicInterProceduralAnalysis(core::PerfData *pthread_data) {
  if (!build_create_tid_to_callpath_and_tid_flag) {
    build_create_tid_to_callpath_and_tid(pthread_data);
  }
  // Only focus on adding relationships between pthread_create and created functions

  // Query for each call path
  auto vertex_data_size = pthread_data->GetVertexDataSize();
  auto edge_data_size = pthread_data->GetEdgeDataSize();
  // dbg(vertex_data_size, edge_data_size);
  for (unsigned long int i = 0; i < edge_data_size; i++) {
    // Value of pthread_create is recorded as (-1)
    auto value = pthread_data->GetEdgeDataValue(i);
    if (value == (type::perf_data_t)(-1)) {
      std::stack<unsigned long long> src_call_path;
      pthread_data->GetEdgeDataSrcCallPath(i, src_call_path);

      // No need to get dest call path, because it is empty as we designed
      // std::stack<unsigned long long> dest_call_path;
      // pthread_data->GetEdgeDataDestCallPath(i, dest_call_path);

      // First add a pthread_create vertex to the graph.
      // if (!src_call_path.empty()) {
      //   src_call_path.pop();
      // }

      // type::vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, src_call_path);
      auto src_thread_id = pthread_data->GetEdgeDataSrcThreadId(i);
      type::vertex_t queried_vertex_id = GetVertexWithInterThreadAnalysis(src_thread_id, src_call_path);

      type::vertex_t pthread_create_vertex_id = -1;
      if (!src_call_path.empty()) {
        type::addr_t addr = src_call_path.top();

        pthread_create_vertex_id = root_pag->AddVertex();
        // dbg(pthread_create_vertex_id);
        root_pag->SetVertexBasicInfo(pthread_create_vertex_id, type::CALL_NODE, "pthread_create");
        root_pag->SetVertexDebugInfo(pthread_create_vertex_id, addr, addr);
        root_pag->SetVertexAttributeNum("id", pthread_create_vertex_id, pthread_create_vertex_id);
        // dbg(root_pag->GetCurVertexNum());
        root_pag->AddEdge(queried_vertex_id, pthread_create_vertex_id);
      } else {
        pthread_create_vertex_id = queried_vertex_id;
        // dbg(pthread_create_vertex_id);
      }
      FREE_CONTAINER(src_call_path);

      // Then find a corresponding pthread_join and create a new vertex for it.
      auto dest_thread_id = pthread_data->GetEdgeDataDestThreadId(i);

      type::vertex_t pthread_join_vertex_id = -1;
      for (unsigned long int j = i + 1; j < edge_data_size; j++) {
        // dbg(j, edge_data_size, dest_thread_id, pthread_data->GetEdgeDataSrcThreadId(j));

        if (dest_thread_id == pthread_data->GetEdgeDataSrcThreadId(j)) {
          std::stack<unsigned long long> src_call_path_join;
          pthread_data->GetEdgeDataSrcCallPath(j, src_call_path_join);
          if (src_call_path_join.size() > 1) {
            // dbg(src_call_path_join.size());
            FREE_CONTAINER(src_call_path_join);
            continue;
          }
          // dbg(src_call_path_join.size());

          // dbg(dest_thread_id, pthread_data->GetEdgeDataSrcThreadId(j));
          std::stack<unsigned long long> dest_call_path_join;
          pthread_data->GetEdgeDataDestCallPath(j, dest_call_path_join);

          // auto time_j = pthread_data->GetVertexDataValue(j);
          // auto create_thread_id_j = pthread_data->GetEdgeDataId(j);
          // auto thread_id_j = pthread_data->GetVertexDataThreadId(j);
          auto dest_thread_id_join = pthread_data->GetEdgeDataDestThreadId(j);
          // if (!dest_call_path_join.empty()) {
          //   dest_call_path_join.pop();
          // }
          // type::vertex_t queried_vertex_id_join = this->root_pag->GetVertexWithCallPath(0, dest_call_path_join);
          type::vertex_t queried_vertex_id_join =
              GetVertexWithInterThreadAnalysis(dest_thread_id_join, dest_call_path_join);

          if (!src_call_path.empty()) {
            type::addr_t addr_join = dest_call_path_join.top();

            // dbg(addr_join);
            pthread_join_vertex_id = root_pag->AddVertex();
            // dbg(pthread_join_vertex_id);
            root_pag->SetVertexBasicInfo(pthread_join_vertex_id, type::CALL_NODE, "pthread_join");
            root_pag->SetVertexDebugInfo(pthread_join_vertex_id, addr_join, addr_join);
            // dbg(root_pag->GetCurVertexNum());
            root_pag->SetVertexAttributeNum("id", pthread_join_vertex_id, pthread_join_vertex_id);

            root_pag->AddEdge(queried_vertex_id_join, pthread_join_vertex_id);
          } else {
            pthread_join_vertex_id = queried_vertex_id_join;
            // dbg(pthread_join_vertex_id);
          }

          auto join_value = pthread_data->GetEdgeDataValue(j);
          std::string metric = std::string("TOT_CYC");
          this->root_pag->GetGraphPerfData()->SetPerfData(
              pthread_join_vertex_id, metric, pthread_data->GetEdgeDataDestProcsId(j), dest_thread_id_join, join_value);
          // this->graph_perf_data->SetPerfData(pthread_join_vertex_id, perf_data->GetMetricName(), process_id,
          // thread_id, data);
          dbg(pthread_join_vertex_id, join_value);

          FREE_CONTAINER(dest_call_path_join);
          FREE_CONTAINER(src_call_path_join);
          break;
        }
      }

      // Thirdly, find the function launched by pthread_create and add edges.
      type::vertex_t func_vertex_id = -1;
      for (unsigned long int j = 0; j < vertex_data_size; j++) {
        // find the samples with same thread id as created thread id of this pthread_create
        auto func_thread_id = pthread_data->GetVertexDataThreadId(j);
        if (func_thread_id == dest_thread_id) {
          // dbg(func_thread_id, dest_thread_id);
          // Get the first addr of call path
          std::stack<type::addr_t> call_path;
          pthread_data->GetVertexDataCallPath(j, call_path);
          if (call_path.empty()) {
            continue;
          }

          // Use GetVertexWithCallPath function to pop address that over 0x7ff0000000, it must return 0 - root of pag
          this->root_pag->GetVertexWithCallPath(0, call_path);
          type::addr_t func_addr = call_path.top();

          FREE_CONTAINER(call_path);

          // Find the corresponding function with the func_addr
          auto func_pag = this->GetFunctionAbstractionGraphByAddr(func_addr);

          if (!func_pag) {
            dbg("func_pag is not found!");
          } else {
            // dbg(func_pag->GetGraphAttributeString("name"));

            /** Expand the graph, connect call&callee, inter-procedural analysis */
            // DFS From root node of function created by pthread_create
            InterPAArg *arg = new InterPAArg();
            arg->pcg = this->pcg;
            arg->func_entry_addr_to_pag = &(this->func_entry_addr_to_pag);
            func_pag->SetGraphAttributeFlag("scanned", true);
            func_pag->VertexTraversal(&ConnectCallerCallee, arg);
            delete arg;

            // Add the graph after call&callee connection
            func_vertex_id = this->root_pag->AddGraph(func_pag);
          }

          break;
        }
      }

      if (func_vertex_id == -1) {
        // dbg(func_vertex_id);
      } else {
        // dbg(pthread_create_vertex_id, func_vertex_id);
        this->root_pag->AddEdge(pthread_create_vertex_id, func_vertex_id);
        this->root_pag->SetVertexAttributeNum("wait", pthread_join_vertex_id, func_vertex_id);
        // dbg(func_vertex_id, pthread_join_vertex_id);
        // this->root_pag->AddEdge(func_vertex_id, pthread_join_vertex_id);
      }
    }

    // Sort the graph
    // TODO: support sorting from a specific vertex
    // this->root_pag->SortByAddr();
  }

  /** pthread_mutex_lock waiting events */

  for (unsigned long int i = 0; i < edge_data_size; i++) {
    // Value of pthread_create is recorded as (-1)
    auto value = pthread_data->GetEdgeDataValue(i);
    if (value >= (type::perf_data_t)(0)) {
      std::stack<unsigned long long> src_call_path;

      std::stack<unsigned long long> dest_call_path;
      pthread_data->GetEdgeDataSrcCallPath(i, src_call_path);
      pthread_data->GetEdgeDataDestCallPath(i, dest_call_path);
      type::thread_t src_thread_id = pthread_data->GetEdgeDataSrcThreadId(i);
      type::thread_t dest_thread_id = pthread_data->GetEdgeDataDestThreadId(i);
      type::vertex_t queried_vertex_id_src = GetVertexWithInterThreadAnalysis(src_thread_id, src_call_path);
      type::vertex_t queried_vertex_id_dest = GetVertexWithInterThreadAnalysis(dest_thread_id, dest_call_path);
      // dbg(src_thread_id, queried_vertex_id_src, dest_thread_id, queried_vertex_id_dest);
      if (-1 != queried_vertex_id_src && -1 != queried_vertex_id_dest) {
        // dbg(this->root_pag->GetVertexAttributeString("name", queried_vertex_id_src),
        //    this->root_pag->GetVertexAttributeString("name", queried_vertex_id_dest));
        // this->root_pag->AddEdge(pthread_create_vertex_id, func_vertex_id);
        this->root_pag->SetVertexAttributeNum("wait", queried_vertex_id_src, queried_vertex_id_dest);
        std::string metric = std::string("TOT_CYC");
        this->root_pag->GetGraphPerfData()->SetPerfData(queried_vertex_id_dest, metric,
                                                        pthread_data->GetEdgeDataDestProcsId(i), dest_thread_id, value);
      }
    }
  }

  //   std::stack<unsigned long long> src_call_path;
  //   pthread_data->GetEdgeDataSrcCallPath(i, src_call_path);
  //   if (!src_call_path.empty()) {
  //     src_call_path.pop();
  //   }
  //   auto time = pthread_data->GetEdgeDataValue(i);
  //   auto create_thread_id = pthread_data->GetEdgeDataSrcProcsId(i);
  //   auto thread_id = pthread_data->GetVertexDataThreadId(i);

  //   // TODO: Need to judge if it is in current thread
  //   // if thread_id == cur_thread

  //   if (time == (type::perf_data_t)(-1)) {
  //     type::vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, src_call_path);
  //     type::addr_t addr = src_call_path.top();
  //     dbg(addr);
  //     type::vertex_t pthread_create_vertex_id = root_pag->AddVertex();
  //     root_pag->SetVertexBasicInfo(pthread_create_vertex_id, type::CALL_NODE, "pthread_create");
  //     root_pag->SetVertexDebugInfo(pthread_create_vertex_id, addr, addr);
  //     root_pag->SetVertexAttributeNum("id", pthread_create_vertex_id, pthread_create_vertex_id);
  //     root_pag->AddEdge(queried_vertex_id, pthread_create_vertex_id);
  //     for (unsigned long int j = 0; j < data_size; j++) {
  //       if (j != i) {
  //         if (create_thread_id == pthread_data->GetVertexDataProcsId(j)) {
  //           dbg(create_thread_id, pthread_data->GetVertexDataProcsId(j));
  //           std::stack<unsigned long long> src_call_path_join;
  //           pthread_data->GetVertexDataCallPath(j, src_call_path_join);
  //           if (!src_call_path_join.empty()) {
  //             src_call_path_join.pop();
  //           }
  //           auto time_j = pthread_data->GetVertexDataValue(j);
  //           auto create_thread_id_j = pthread_data->GetVertexDataProcsId(j);
  //           auto thread_id_j = pthread_data->GetVertexDataThreadId(j);

  //           type::vertex_t queried_vertex_id_j = this->root_pag->GetVertexWithCallPath(0, src_call_path_join);
  //           type::addr_t addr_j = src_call_path_join.top();
  //           dbg(addr_j);
  //           type::vertex_t pthread_join_vertex_id = root_pag->AddVertex();
  //           dbg(pthread_join_vertex_id);
  //           root_pag->SetVertexBasicInfo(pthread_join_vertex_id, type::CALL_NODE, "pthread_join");
  //           root_pag->SetVertexDebugInfo(pthread_join_vertex_id, addr_j, addr_j);
  //           root_pag->SetVertexAttributeNum("id", pthread_join_vertex_id, pthread_join_vertex_id);
  //           root_pag->AddEdge(queried_vertex_id_j, pthread_join_vertex_id);
  //           FREE_CONTAINER(src_call_path_join);
  //           break;
  //         }
  //       }
  //     }
  //   }
  //   FREE_CONTAINER(src_call_path);
  // }
}

void GPerf::InterProceduralAnalysis(core::PerfData *pthread_data) {
  // Search root node , "name" is "main"
  // std::map<std::string, ProgramAbstractionGraph *> func_entry_addr_to_pag;

  for (auto &kv : this->func_entry_addr_to_pag) {
    printf("entry_addr: %llu, ", kv.first);
    // func_entry_addr_to_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->SetGraphAttributeFlag("scanned", false);
    printf("func_name: %s \n", pag->GetVertexAttributeString("name", 0));
    if (strcmp(pag->GetVertexAttributeString("name", 0), "main") == 0) {
      this->root_pag = pag;
      std::cout << "Find 'main'" << std::endl;
    }
  }
  if (!this->root_pag) {
    return;
  }

  // DFS From root node
  InterPAArg *arg = new InterPAArg();
  arg->pcg = this->pcg;
  arg->func_entry_addr_to_pag = &(this->func_entry_addr_to_pag);

  // void (*ConnectCallerCalleePointer)(Graph *, int, void *) = &ConnectCallerCallee;
  // dbg(this->root_pag->GetGraphAttributeString("name"));
  this->root_pag->VertexTraversal(&ConnectCallerCallee, arg);

  delete arg;

  // this->root_pag->VertexSortChild();

  this->DynamicInterProceduralAnalysis(pthread_data);

  // this->root_pag->VertexSortChild();

  for (auto &kv : this->func_entry_addr_to_pag) {
    // func_entry_addr_to_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->RemoveGraphAttribute("scanned");
  }

  return;
}  // function InterProceduralAnalysis

void GPerf::StaticInterProceduralAnalysis() {
  // Search root node , "name" is "main"
  // std::map<std::string, ProgramAbstractionGraph *> func_entry_addr_to_pag;

  for (auto &kv : this->func_entry_addr_to_pag) {
    // func_entry_addr_to_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->SetGraphAttributeFlag("scanned", false);
    if (strcmp(pag->GetVertexAttributeString("name", 0), "main") == 0) {
      this->root_pag = pag;
      std::cout << "Find 'main'" << std::endl;
      // break;
    }
  }

  // DFS From root node
  InterPAArg *arg = new InterPAArg();
  arg->pcg = this->pcg;
  arg->func_entry_addr_to_pag = &(this->func_entry_addr_to_pag);

  // void (*ConnectCallerCalleePointer)(Graph *, int, void *) = &ConnectCallerCallee;
  // dbg(this->root_pag->GetGraphAttributeString("name"));
  this->root_pag->VertexTraversal(&ConnectCallerCallee, arg);

  delete arg;

  // this->root_pag->VertexSortChild();

  for (auto &kv : this->func_entry_addr_to_pag) {
    // func_entry_addr_to_pag[std::string(pag->GetGraphAttributeString("name"))] = pag;
    auto pag = kv.second;
    pag->RemoveGraphAttribute("scanned");
  }

  return;
}  // function InterProceduralAnalysis

void GPerf::GenerateProgramAbstractionGraph(core::PerfData *perf_data) { this->InterProceduralAnalysis(perf_data); }

void GPerf::GenerateStaticProgramAbstractionGraph() { this->StaticInterProceduralAnalysis(); }

void GPerf::SetProgramAbstractionGraph(core::ProgramAbstractionGraph *pag) { this->root_pag = pag; }

core::ProgramAbstractionGraph *GPerf::GetProgramAbstractionGraph() { return this->root_pag; }

type::vertex_t GPerf::GetVertexWithInterThreadAnalysis(type::thread_t thread_id, type::call_path_t &call_path) {
  if (call_path.empty()) {
    return 0;
  } else {
    call_path.pop();
  }

  // dbg(thread_id, call_path.top());

  if (thread_id == 0) {
    if (call_path.empty()) {
      return 0;
    }
    //
    auto vertex_id = this->root_pag->GetVertexWithCallPath(0, call_path);
    // dbg(vertex_id);
    return vertex_id;
  } else {
    delete_all_so_addr(call_path);
  }

  // Get the parent thread and its id and call path
  std::pair<type::call_path_t, type::thread_t> next_pair = created_tid_2_callpath_and_tid[thread_id];
  type::call_path_t &parent_call_path = next_pair.first;
  type::thread_t parent_thread_id = next_pair.second;
  // dbg(parent_thread_id, parent_call_path.top());

  // Trace the pthread_create vertex that created this thread
  auto starting_vertex = GetVertexWithInterThreadAnalysis(parent_thread_id, parent_call_path);
  // dbg(starting_vertex);

  // Starting from the pthread_create vertex,
  if (call_path.empty()) {
    return starting_vertex;
  }

  auto detected_vertex = this->root_pag->GetVertexWithCallPath(starting_vertex, call_path);
  // dbg(detected_vertex);

  return detected_vertex;
}

void GPerf::DataEmbedding(core::PerfData *perf_data) {
  if (!build_create_tid_to_callpath_and_tid_flag) {
    build_create_tid_to_callpath_and_tid(perf_data);
  }

  // Query for each call path
  auto data_size = perf_data->GetVertexDataSize();
  for (unsigned long int i = 0; i < data_size; i++) {
    type::call_path_t call_path;
    perf_data->GetVertexDataCallPath(i, call_path);

    auto value = perf_data->GetVertexDataValue(i);
    auto process_id = perf_data->GetVertexDataProcsId(i);
    auto thread_id = perf_data->GetVertexDataThreadId(i);

    // type::vertex_t queried_vertex_id = this->root_pag->GetVertexWithCallPath(0, call_path);
    auto queried_vertex_id = GetVertexWithInterThreadAnalysis(thread_id, call_path);
    // dbg(queried_vertex_id);
    type::perf_data_t data = this->root_pag->GetGraphPerfData()->GetPerfData(
        queried_vertex_id, perf_data->GetMetricName(), process_id, thread_id);
    // dbg(data);
    data += value;
    this->root_pag->GetGraphPerfData()->SetPerfData(queried_vertex_id, perf_data->GetMetricName(), process_id,
                                                    thread_id, data);
    FREE_CONTAINER(call_path);
  }

}  // function Dataembedding

struct pthread_expansion_arg_t {
  core::ProgramAbstractionGraph *mpag;
  std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id;
  type::vertex_t src_vertex_id;
};

void in_pthread_expansion(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct pthread_expansion_arg_t *arg = (struct pthread_expansion_arg_t *)extra;
  core::ProgramAbstractionGraph *mpag = arg->mpag;
  std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id = arg->pag_vertex_id_2_mpag_vertex_id;

  type::vertex_t new_vertex_id = mpag->AddVertex();
  mpag->CopyVertex(new_vertex_id, pag, vertex_id);
  (*pag_vertex_id_2_mpag_vertex_id)[vertex_id] = new_vertex_id;

  if (arg->src_vertex_id >= 0) {
    mpag->AddEdge(arg->src_vertex_id, new_vertex_id);
  }
  arg->src_vertex_id = new_vertex_id;

  if (strcmp(pag->GetVertexAttributeString("name", vertex_id), "pthread_join") == 0) {
    if (pag->HasVertexAttribute("wait")) {
      type::vertex_t wait_vertex_id = pag->GetVertexAttributeNum("wait", vertex_id);

      if (wait_vertex_id > 0) {
        type::vertex_t last_vertex_id = pag->GetVertexAttributeNum("last", wait_vertex_id);
        // dbg(vertex_id, wait_vertex_id, last_vertex_id);
        mpag->AddEdge(last_vertex_id, new_vertex_id);
      }
    }
  }
}

void out_pthread_expansion(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct pthread_expansion_arg_t *arg = (struct pthread_expansion_arg_t *)extra;
  // core::ProgramAbstractionGraph *mpag = arg->mpag;
  std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id = arg->pag_vertex_id_2_mpag_vertex_id;

  type::vertex_t parent_vertex_id = pag->GetParentVertex(vertex_id);
  if (strcmp(pag->GetVertexAttributeString("name", parent_vertex_id), "pthread_create") == 0) {
    // dbg(vertex_id, arg->src_vertex_id);
    pag->SetVertexAttributeNum("last", vertex_id, arg->src_vertex_id);
    arg->src_vertex_id = (*pag_vertex_id_2_mpag_vertex_id)[parent_vertex_id];
  }
}

void add_unlock_to_lock_edge(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id =
      (std::map<type::vertex_t, type::vertex_t> *)extra;

  if (strcmp(pag->GetVertexAttributeString("name", vertex_id), "pthread_mutex_unlock") == 0) {
    if (pag->HasVertexAttribute("wait")) {
      type::vertex_t wait_vertex_id = pag->GetVertexAttributeNum("wait", vertex_id);

      if (wait_vertex_id > 0) {
        type::vertex_t dest_vertex_id = (*pag_vertex_id_2_mpag_vertex_id)[wait_vertex_id];

        // dbg(vertex_id, pag->GetVertexAttributeString("name", dest_vertex_id));
        pag->AddEdge(vertex_id, dest_vertex_id);
      }
    }
  }
}

void GPerf::GenerateMultiThreadProgramAbstractionGraph() {
  // this->root_mpag = new core::ProgramAbstractionGraph();
  this->root_mpag->GraphInit("Multi-thread Program Abstraction Graph");

  struct pthread_expansion_arg_t *arg = new (struct pthread_expansion_arg_t)();
  std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id =
      new std::map<type::vertex_t, type::vertex_t>();
  arg->mpag = this->root_mpag;
  arg->pag_vertex_id_2_mpag_vertex_id = pag_vertex_id_2_mpag_vertex_id;
  arg->src_vertex_id = -1;

  this->root_pag->DFS(0, in_pthread_expansion, out_pthread_expansion, arg);

  /** pthread_mutex_lock waiting */
  this->root_mpag->VertexTraversal(add_unlock_to_lock_edge, pag_vertex_id_2_mpag_vertex_id);

  FREE_CONTAINER(*pag_vertex_id_2_mpag_vertex_id);
  delete pag_vertex_id_2_mpag_vertex_id;
  delete arg;
}

struct group_thread_perf_data_arg_t {
  core::GraphPerfData *graph_perf_data;
  std::string metric;
  type::thread_t num_groups;
};

void group_thread_perf_data(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct group_thread_perf_data_arg_t *arg = (struct group_thread_perf_data_arg_t *)extra;
  core::GraphPerfData *pag_graph_perf_data = arg->graph_perf_data;
  type::thread_t num_groups = arg->num_groups;

  std::vector<type::procs_t> procs_vec;
  pag_graph_perf_data->GetMetricsPerfDataProcsNum(vertex_id, arg->metric, procs_vec);
  for (auto procs : procs_vec) {
    /** Get original process performance data */
    std::map<type::thread_t, type::perf_data_t> proc_perf_data;
    pag_graph_perf_data->GetProcsPerfData(vertex_id, arg->metric, procs, proc_perf_data);

    /** Erase original process performance data */
    pag_graph_perf_data->EraseProcsPerfData(vertex_id, arg->metric, procs);

    /** Grouping */
    std::map<type::thread_t, type::perf_data_t> group_proc_perf_data;
    for (auto &thread_perf_data : proc_perf_data) {
      type::thread_t group_id = thread_perf_data.first % num_groups;
      if (group_proc_perf_data.find(group_id) != group_proc_perf_data.end()) {
        group_proc_perf_data[group_id] += thread_perf_data.second;
      } else {
        group_proc_perf_data[group_id] = thread_perf_data.second;
      }
    }

    /** Record group process performance data */
    pag_graph_perf_data->SetProcsPerfData(vertex_id, arg->metric, procs, group_proc_perf_data);

    FREE_CONTAINER(proc_perf_data);
    FREE_CONTAINER(group_proc_perf_data);
  }
  FREE_CONTAINER(procs_vec);
}

void GPerf::OpenMPGroupThreadPerfData(std::string &metric, type::thread_t num_groups) {
  auto pag_graph_perf_data = this->root_pag->GetGraphPerfData();
  struct group_thread_perf_data_arg_t *arg = new (struct group_thread_perf_data_arg_t)();
  arg->metric = std::string(metric);
  arg->num_groups = num_groups;
  arg->graph_perf_data = pag_graph_perf_data;

  this->root_pag->VertexTraversal(&group_thread_perf_data, (void *)arg);

  delete arg;
}

bool openmp_seq_record_flag = false;
std::vector<type::vertex_t> openmp_seq;

struct openmp_expansion_arg_t {
  core::ProgramAbstractionGraph *mpag;
  type::vertex_t src_vertex_id;
  int num_threads;
};

void in_openmp_expansion(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct openmp_expansion_arg_t *arg = (struct openmp_expansion_arg_t *)extra;
  core::ProgramAbstractionGraph *mpag = arg->mpag;
  // std::map<type::vertex_t, type::vertex_t> *pag_vertex_id_2_mpag_vertex_id = arg->pag_vertex_id_2_mpag_vertex_id;

  if (openmp_seq_record_flag) {
    openmp_seq.push_back(vertex_id);
    return;
  }

  type::vertex_t new_vertex_id = mpag->AddVertex();
  mpag->CopyVertex(new_vertex_id, pag, vertex_id);
  //(*pag_vertex_id_2_mpag_vertex_id)[vertex_id] = new_vertex_id;

  if (arg->src_vertex_id >= 0) {
    mpag->AddEdge(arg->src_vertex_id, new_vertex_id);
  }
  arg->src_vertex_id = new_vertex_id;

  if (strcmp(pag->GetVertexAttributeString("name", vertex_id), "GOMP_parallel") == 0) {
    std::vector<baguatool::type::vertex_t> child_vec;
    pag->GetChildVertexSet(vertex_id, child_vec);
    if (child_vec.size() > 0) {
      openmp_seq_record_flag = true;
    }
    FREE_CONTAINER(child_vec);
  }
}

void out_openmp_expansion(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct openmp_expansion_arg_t *arg = (struct openmp_expansion_arg_t *)extra;
  core::ProgramAbstractionGraph *mpag = arg->mpag;
  auto pag_graph_perf_data = pag->GetGraphPerfData();
  auto mpag_graph_perf_data = mpag->GetGraphPerfData();

  if (openmp_seq_record_flag) {
    type::vertex_t parent_vertex_id = pag->GetParentVertex(vertex_id);
    if (strcmp(pag->GetVertexAttributeString("name", parent_vertex_id), "GOMP_parallel") == 0) {
      openmp_seq_record_flag = false;

      type::vertex_t end_vertex_id = mpag->AddVertex();
      mpag->CopyVertex(end_vertex_id, pag, vertex_id);
      mpag->SetVertexAttributeString("name", end_vertex_id, "GOMP_parallel.end");

      std::vector<type::vertex_t> *new_vertex_set;
      new_vertex_set = new std::vector<type::vertex_t>[arg->num_threads]();

      for (int i = 0; i < arg->num_threads; i++) {
        type::vertex_t last_new_vertex_id = arg->src_vertex_id;
        for (auto vertex_id : openmp_seq) {
          type::vertex_t new_vertex_id = mpag->AddVertex();
          mpag->CopyVertex(new_vertex_id, pag, vertex_id);
          if (last_new_vertex_id != -1) {
            mpag->AddEdge(last_new_vertex_id, new_vertex_id);
          }
          new_vertex_set[i].push_back(new_vertex_id);
          last_new_vertex_id = new_vertex_id;
        }
        mpag->AddEdge(last_new_vertex_id, end_vertex_id);
      }
      arg->src_vertex_id = end_vertex_id;

      /** Scatter performance data */
      int j = 0;
      for (auto vertex_id : openmp_seq) {
        std::vector<std::string> metrics;
        pag_graph_perf_data->GetVertexPerfDataMetrics(vertex_id, metrics);
        for (auto metric : metrics) {
          std::vector<type::procs_t> procs_vec;
          pag_graph_perf_data->GetMetricsPerfDataProcsNum(vertex_id, metric, procs_vec);
          for (auto procs_id : procs_vec) {
            std::map<type::thread_t, type::perf_data_t> proc_perf_data;
            pag_graph_perf_data->GetProcsPerfData(vertex_id, metric, procs_id, proc_perf_data);

            int i = 0;
            for (auto &thread_perf_data : proc_perf_data) {
              if (i >= arg->num_threads) {
                break;
              }
              mpag_graph_perf_data->SetPerfData(new_vertex_set[i][j], metric, procs_id, thread_perf_data.first,
                                                thread_perf_data.second);
              i++;
            }
            FREE_CONTAINER(proc_perf_data);
          }
          FREE_CONTAINER(procs_vec);
        }
        FREE_CONTAINER(metrics);
        j++;
      }

      for (int i = 0; i < arg->num_threads; i++) {
        FREE_CONTAINER(new_vertex_set[i]);
      }
      delete[] new_vertex_set;
      FREE_CONTAINER(openmp_seq);
    }
  }
}

void GPerf::GenerateOpenMPProgramAbstractionGraph(int num_threads) {
  this->root_mpag->GraphInit("OpenMP Program Abstraction Graph");

  struct openmp_expansion_arg_t *arg = new (struct openmp_expansion_arg_t)();
  arg->mpag = this->root_mpag;
  arg->src_vertex_id = -1;
  arg->num_threads = num_threads;

  this->root_pag->DFS(0, in_openmp_expansion, out_openmp_expansion, arg);

  // /** pthread_mutex_lock waiting */
  // this->root_mpag->VertexTraversal(add_unlock_to_lock_edge, pag_vertex_id_2_mpag_vertex_id);

  // FREE_CONTAINER(*pag_vertex_id_2_mpag_vertex_id);
  // delete pag_vertex_id_2_mpag_vertex_id;
  delete arg;
}

struct pre_order_traversal_t {
  std::vector<type::vertex_t> *seq;
};

void in_pre_order_traversal(core::ProgramAbstractionGraph *pag, int vertex_id, void *extra) {
  struct pre_order_traversal_t *arg = (struct pre_order_traversal_t *)extra;
  std::vector<type::vertex_t> *seq = arg->seq;

  seq->push_back(vertex_id);
}

void GPerf::GenerateMultiProcessProgramAbstractionGraph(int num_procs) {
  this->root_mpag->GraphInit("Multi-process Program Abstraction Graph");

  /** Pre-order traversal */
  struct pre_order_traversal_t *arg = new (struct pre_order_traversal_t)();
  std::vector<type::vertex_t> *pre_order_vertex_seq = new std::vector<type::vertex_t>();
  arg->seq = pre_order_vertex_seq;
  this->root_pag->DFS(0, in_pre_order_traversal, nullptr, arg);

  /** Build mpag and mpag's graph perf data */

  // First add a root vertex
  type::vertex_t root_vertex_id = this->root_mpag->AddVertex();
  this->root_mpag->SetVertexBasicInfo(root_vertex_id, type::FUNC_NODE, "root");
  this->root_mpag->SetVertexAttributeNum(
      "id", root_vertex_id, 0);  // I don't know why it turns to an unkown number if I don't set id to 0 manully.

  // Build for each process
  auto pag_graph_perf_data = this->root_pag->GetGraphPerfData();
  auto mpag_graph_perf_data = this->root_mpag->GetGraphPerfData();
  for (int i = 0; i < num_procs; i++) {
    type::vertex_t last_new_vertex_id = root_vertex_id;
    for (auto vertex_id : *pre_order_vertex_seq) {
      type::vertex_t new_vertex_id = root_mpag->AddVertex();
      this->root_mpag->CopyVertex(new_vertex_id, root_pag, vertex_id);
      if (last_new_vertex_id != -1) {
        this->root_mpag->AddEdge(last_new_vertex_id, new_vertex_id);
      }
      // Copy process i perf data of vertex in pag to process i perf data of new vertex in mpag
      std::vector<std::string> metrics;
      pag_graph_perf_data->GetVertexPerfDataMetrics(vertex_id, metrics);
      for (auto metric : metrics) {
        std::map<type::thread_t, type::perf_data_t> proc_perf_data;
        pag_graph_perf_data->GetProcsPerfData(vertex_id, metric, i, proc_perf_data);
        mpag_graph_perf_data->SetProcsPerfData(new_vertex_id, metric, i, proc_perf_data);
      }
      last_new_vertex_id = new_vertex_id;
    }
  }

  FREE_CONTAINER(*pre_order_vertex_seq);
  delete pre_order_vertex_seq;
  delete arg;
}

void GPerf::GenerateMultiProgramAbstractionGraph() {}

core::ProgramAbstractionGraph *GPerf::GetMultiProgramAbstractionGraph() { return root_mpag; }

}  // graph_perf