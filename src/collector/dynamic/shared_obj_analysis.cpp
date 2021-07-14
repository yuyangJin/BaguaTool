#include "shared_obj_analysis.h"

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
        // dbg(cnt, line);
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
  for (auto& t : this->shared_obj_map) {
    fout << std::get<0>(t) << " " << std::get<1>(t) << " " << std::get<2>(t) << std::endl;
  }
  fout.close();
}

type::addr_t search_exe_and_section_from_map(
    std::vector<std::tuple<type::addr_t, type::addr_t, std::string>>& shared_obj_map, type::addr_t addr, string& exe) {
  for (auto& t : shared_obj_map) {
    type::addr_t start_addr = std::get<0>(t);
    type::addr_t end_addr = std::get<1>(t);
    std::string& shared_obj = std::get<2>(t);
    if (addr >= start_addr && addr <= end_addr) {
      exe = std::string(shared_obj);
      // dbg (start_addr);
      if (addr > 0x400000000) {
        return addr - start_addr;
      } else {
        return addr;
      }
    }
  }
  return 0;
}

void execute_cmd(const char* cmd, std::string& result) {
  char buffer[128];
  FILE* pipe = popen(cmd, "r");
  if (!pipe) throw std::runtime_error("popen() failed!");
  try {
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
      result += buffer;
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return;
}

void SharedObjAnalysis::GetDebugInfo(type::addr_t addr, type::addr_debug_info_t& debug_info) {
  std::string exe;
  type::addr_t offset;

  /** Get offset and shared object of input address */
  offset = search_exe_and_section_from_map(this->shared_obj_map, addr, exe);
  dbg(offset);
  debug_info.SetAddress(offset);

  if (exe.find(".so") == std::string::npos) {
    return;
  }

  struct link_map* lm = (struct link_map*)dlopen(exe.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!lm) {
    fputs(dlerror(), stderr);
    exit(1);
  }
  type::addr_t base_addr = lm->l_addr;
  type::addr_t new_load_addr = offset + base_addr;

  Dl_info DlInfo;
  int ret = dladdr((void*)new_load_addr, &DlInfo);

  if (ret && DlInfo.dli_sname) { /** If dladdr obtains function name successfully */
    int status = 0;
    char* cpp_name = abi::__cxa_demangle(DlInfo.dli_sname, 0, 0, &status);
    if (status >= 0) {
      std::string func_name = std::string(cpp_name);
      debug_info.SetFuncName(func_name);
      dbg(DlInfo.dli_fname, func_name);
    } else {
      std::string func_name = std::string(DlInfo.dli_sname);
      debug_info.SetFuncName(func_name);
      dbg(DlInfo.dli_fname, func_name);
    }
  } else { /** If dladdr fails to obtain function name */
    std::stringstream offset_ss;
    offset_ss << std::hex << offset;
    std::string result;
    std::string cmd_line =
        std::string("addr2line -fC -e ") + std::string(DlInfo.dli_fname) + std::string(" ") + offset_ss.str();
    execute_cmd(cmd_line.c_str(), result);
    // dbg(cmd_line, result);
    std::stringstream ss(result);
    std::string func_name;
    ss >> func_name;
    dbg(DlInfo.dli_fname, func_name);
    debug_info.SetFuncName(func_name);
  }

  dlclose(lm);
}

void SharedObjAnalysis::GetDebugInfos(std::vector<type::addr_t>& addrs,
                                      std::map<type::addr_t, type::addr_debug_info_t*>& debug_info_map) {
  /** Classify addrs by exe */
  std::map<std::string, std::vector<std::pair<type::addr_t, type::addr_t>>>
      exe_to_addrs;  // map < vector < offest, address > >
  for (auto addr : addrs) {
    std::string exe;
    type::addr_t offset;

    /** Get offset and shared object of each address */
    offset = search_exe_and_section_from_map(this->shared_obj_map, addr, exe);
    exe_to_addrs[exe].push_back(std::make_pair(offset, addr));
  }

  for (auto& kv : exe_to_addrs) {
    dbg(kv.first);
    if (kv.first.find(".so") == std::string::npos) {
      continue;
    }
    struct link_map* lm = (struct link_map*)dlopen(kv.first.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!lm) {
      fputs(dlerror(), stderr);
      exit(1);
    }
    type::addr_t base_addr = lm->l_addr;
    for (auto& p : kv.second) {
      type::addr_t offset = p.first;
      type::addr_t raw_addr = p.second;
      type::addr_t new_load_addr = offset + base_addr;
      std::string func_name;

      Dl_info DlInfo;
      int ret = dladdr((void*)new_load_addr, &DlInfo);
      // dbg(raw_addr, offset);

      if (ret && DlInfo.dli_sname) { /** If dladdr obtains function name successfully */
        int status = 0;
        char* cpp_name = abi::__cxa_demangle(DlInfo.dli_sname, 0, 0, &status);
        if (status >= 0) {
          func_name = std::string(cpp_name);
          // debug_info.SetFuncName(func_name);
          // dbg(DlInfo.dli_fname, func_name);
        } else {
          func_name = std::string(DlInfo.dli_sname);
          // debug_info.SetFuncName(func_name);
          // dbg(DlInfo.dli_fname, func_name);
        }
      } else { /** If dladdr fails to obtain function name */
        std::stringstream offset_ss;
        offset_ss << std::hex << offset;
        std::string result;
        std::string cmd_line =
            std::string("addr2line -fC -e ") + std::string(DlInfo.dli_fname) + std::string(" ") + offset_ss.str();
        execute_cmd(cmd_line.c_str(), result);
        // dbg(cmd_line, result);
        std::stringstream ss(result);

        ss >> func_name;
        // dbg(DlInfo.dli_fname, func_name);
      }

      dbg(raw_addr, offset, DlInfo.dli_fname, func_name);
      type::addr_debug_info_t* debug_info = new type::addr_debug_info_t();
      debug_info->SetAddress(offset);
      debug_info->SetFuncName(func_name);
      debug_info_map[raw_addr] = debug_info;
    }
    dlclose(lm);
  }

  for (auto& kv : exe_to_addrs) {
    FREE_CONTAINER(kv.second);
  }
  FREE_CONTAINER(exe_to_addrs);
}

}  // namespace baguatool::collector