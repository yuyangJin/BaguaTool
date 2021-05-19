#include <cstring>
#include <string>
#include "baguatool.h"

int main(int argc, char** argv) {
  const char* pag_file_name = argv[1];
  const char* perf_data_file_name = argv[2];
  // std::string perf_data_file_name_str(argv[2]);

  baguatool::core::ProgramAbstractionGraph* pag = new baguatool::core::ProgramAbstractionGraph();
  pag->ReadGraphGML(pag_file_name);

  baguatool::core::PerfData* perf_data = new baguatool::core::PerfData();
  perf_data->Read(perf_data_file_name);

  auto hybrid_analysis = std::make_unique<baguatool::core::HybridAnalysis>();

  hybrid_analysis->SetProgramAbstractionGraph(pag);
  hybrid_analysis->DataEmbedding(perf_data);
  std::string metric("TOT_CYC");
  std::string op("AVG");
  baguatool::core::perf_data_t total = hybrid_analysis->ReduceVertexPerfData(metric, op);
  std::string avg_metric("TOT_CYC_AVG");
  std::string new_metric("CYC_AVG_PERCENT");
  hybrid_analysis->ConvertVertexReducedDataToPercent(avg_metric, total, new_metric);

  auto graph_perf_data = hybrid_analysis->GetGraphPerfData();
  std::string output_file_name_str("output.json");
  graph_perf_data->Dump(output_file_name_str);

  hybrid_analysis->GetProgramAbstractionGraph()->DumpGraphGML("root_3.gml");
  hybrid_analysis->GenerateMultiProgramAbstractionGraph();

  auto mpag = hybrid_analysis->GetMultiProgramAbstractionGraph();
  mpag->DumpGraphGML("root_3.gml");
}