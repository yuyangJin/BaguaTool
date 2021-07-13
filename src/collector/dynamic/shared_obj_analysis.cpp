#include "shared_obj_analysis.h"

namespace baguatool::type {
struct addr_debug_info_t {
  type::addr_t addr;
  std::string file_name;
  std::string func_name;
  int line_num;

  type::addr_t GetAddress() { return this->addr; }
  std::string& GetFileName() { return this->file_name; }
  std::string& GetFuncName() { return this->func_name; }
  int GetLineNum() { return this->line_num; }

  void SetAddress(type::addr_t addr) { this->addr = addr; }
  void SetFileName(std::string& file_name) { this->file_name = std::string(file_name); }
  void SetFuncName(std::string& func_name) { this->func_name = std::string(func_name); }
  void SetLineNum(int line_num) { this->line_num = line_num; }
};
}

namespace baguatool::collector {

SharedObjAnalysis::SharedObjAnalysis() {}

SharedObjAnalysis::~SharedObjAnalysis() {}

void SharedObjAnalysis::CollectSharedObjMap() {
  type::procs_t pid = getpid();
  std::ifstream fin;
  std::string map_file_name = std::string("/proc/") + to_string(pid) + std::string("/maps");
  fin.open(map_file_name, std::ios_base::in);
  if (!fin.is_open()) {
    std::cout << "Failed to open" << map_file_name << std::endl;
  }

  std::string line;

  while (getline(fin, line)) {
    std::vector<std::string> line_vec, line_vec_1;
    split(line, " ", line_vec);

    int cnt = line_vec.size();

    if (cnt == 6) {
      auto seg_size = strtol(line_vec[4].c_str(), 0, 10);
      if (seg_size > 0) {
        dbg(cnt, line);
        split(line_vec[0], "-", line_vec_1);
        type::addr_t start_addr = strtoull(line_vec_1[0].c_str(), 0, 16);
        type::addr_t end_addr = strtoull(line_vec_1[1].c_str(), 0, 16);
        this->shared_obj_map.push_back(std::make_tuple(start_addr, end_addr, std::string(line_vec[5])));
      }
    }
    FREE_CONTAINER(line_vec);
    FREE_CONTAINER(line_vec_1);
  }
  fin.close();
}

void SharedObjAnalysis::ReadSharedObjMap(std::string& file_name) {
  std::ifstream fin;
  fin.open(file_name, std::ios_base::in);
  if (!fin.is_open()) {
    std::cout << "Failed to open" << file_name << std::endl;
  }
  std::string line;
  type::addr_t start_addr;
  type::addr_t end_addr;
  std::string shared_obj;
  while (getline(fin, line)) {
    std::stringstream ss(line);
    ss >> start_addr >> end_addr >> shared_obj;
    this->shared_obj_map.push_back(std::make_tuple(start_addr, end_addr, std::string(shared_obj)));
  }
  fin.close();
}

void SharedObjAnalysis::DumpSharedObjMap(std::string& file_name) {
  std::ofstream fout;
  fout.open(file_name, std::ios_base::out);
  if (!fout.is_open()) {
    std::cout << "Failed to open" << file_name << std::endl;
  }
  for (auto t : this->shared_obj_map) {
    fout << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << std::endl;
  }
  fout.close();
}

void SharedObjAnalysis::GetDebugInfo(type::addr_t addr, type::addr_debug_info_t& debug_info) {}

void SharedObjAnalysis::GetDebugInfos(std::vector<type::addr_t>& addrs,
                                      std::vector<type::addr_debug_info_t>& debug_infos) {}

}  // namespace baguatool::collector