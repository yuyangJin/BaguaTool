#include "baguatool.h"

int main() {
  auto perf_data = std::make_unique<baguatool::core::PerfData>();
  std::string file_name("./SAMPLE-1-0.TXT");
  perf_data->Read(file_name.c_str());
  perf_data->Dump("SAMPLE.TXT");
}