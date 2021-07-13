#include <string>
#include "baguatool.h"

int main(int argc, char** argv) {
  std::unique_ptr<baguatool::collector::SharedObjAnalysis> shared_obj_analysis =
      std::make_unique<baguatool::collector::SharedObjAnalysis>();
  std::string file_name = std::string(argv[1]);
  shared_obj_analysis->ReadSharedObjMap(file_name);

  baguatool::type::addr_t addr = strtoull(argv[2], 0, 16);
  baguatool::type::addr_debug_info_t debug_info;
  shared_obj_analysis->GetDebugInfo(addr, debug_info);

  std::unique_ptr<baguatool::collector::SharedObjAnalysis> shared_obj_analysis_self =
      std::make_unique<baguatool::collector::SharedObjAnalysis>();

  shared_obj_analysis_self->CollectSharedObjMap();
  file_name = std::string("test_self.map");
  shared_obj_analysis_self->DumpSharedObjMap(file_name);
}