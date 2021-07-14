#include <string>
#include "baguatool.h"

int main(int argc, char** argv) {
  std::unique_ptr<baguatool::collector::SharedObjAnalysis> shared_obj_analysis =
      std::make_unique<baguatool::collector::SharedObjAnalysis>();
  std::string file_name = std::string(argv[1]);
  shared_obj_analysis->ReadSharedObjMap(file_name);

  std::vector<baguatool::type::addr_t> addrs;
  for (int i = 2; i < argc; i++) {
    baguatool::type::addr_t addr = strtoull(argv[i], 0, 16);
    addrs.push_back(addr);
  }
  baguatool::type::addr_debug_info_t debug_info;
  shared_obj_analysis->GetDebugInfo(addrs[0], debug_info);

  std::map<baguatool::type::addr_t, baguatool::type::addr_debug_info_t*> debug_info_map;
  shared_obj_analysis->GetDebugInfos(addrs, debug_info_map);

  std::unique_ptr<baguatool::collector::SharedObjAnalysis> shared_obj_analysis_self =
      std::make_unique<baguatool::collector::SharedObjAnalysis>();

  shared_obj_analysis_self->CollectSharedObjMap();
  file_name = std::string("test_self.map");
  shared_obj_analysis_self->DumpSharedObjMap(file_name);
}