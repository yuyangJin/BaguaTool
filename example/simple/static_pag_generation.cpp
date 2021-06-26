#include <cstring>
#include "baguatool.h"

int main(int argc, char** argv) {
  const char* bin_name = argv[1];
  char pag_dir_name[20] = {0};
  char pcg_name[20] = {0};
  strcpy(pag_dir_name, bin_name);
  strcat(pag_dir_name, ".pag/");

  strcpy(pcg_name, bin_name);
  strcat(pcg_name, ".pcg");

  // const char* perf_data_file_name = argv[2];
  // baguatool::core::PerfData* perf_data = new baguatool::core::PerfData();
  // perf_data->Read(perf_data_file_name);

  auto hybrid_analysis = std::make_unique<baguatool::graph_perf::GPerf>();

  hybrid_analysis->ReadFunctionAbstractionGraphs(pag_dir_name);
  hybrid_analysis->ReadStaticProgramCallGraph(bin_name);
  // hybrid_analysis->GenerateProgramCallGraph(bin_name, perf_data_file_name);
  hybrid_analysis->GetProgramCallGraph()->DumpGraphGML("hy_pcg.gml");

  hybrid_analysis->GenerateStaticProgramAbstractionGraph();

  baguatool::core::ProgramAbstractionGraph* pag = hybrid_analysis->GetProgramAbstractionGraph();
  pag->DumpGraphGML("root_1.gml");

  // pag->PreOrderTraversal(0);

  pag->DumpGraphDot("root_1.dot");

  // hybrid_analysis->DataEmbedding(perf_data);
  // std::string metric("TOT_CYC");
  // std::string op("SUM");
  // baguatool::type::perf_data_t total = hybrid_analysis->ReduceVertexPerfData(metric, op);
  // std::string avg_metric("TOT_CYC_SUM");
  // std::string new_metric("CYCAVGPERCENT");
  // hybrid_analysis->ConvertVertexReducedDataToPercent(avg_metric, total, new_metric);

  // auto graph_perf_data = hybrid_analysis->GetGraphPerfData();
  // std::string output_file_name_str("output.json");
  // graph_perf_data->Dump(output_file_name_str);

  // hybrid_analysis->GetProgramAbstractionGraph()->PreserveHotVertices("CYCAVGPERCENT");

  // hybrid_analysis->GetProgramAbstractionGraph()->DumpGraphGML("root_3.gml");
}