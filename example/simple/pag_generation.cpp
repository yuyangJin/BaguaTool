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

  auto hybrid_analysis = std::make_unique<baguatool::core::HybridAnalysis>();

  hybrid_analysis->ReadFunctionAbstractionGraphs(pag_dir_name);
  hybrid_analysis->ReadStaticProgramCallGraph(bin_name);
  hybrid_analysis->GenerateProgramAbstractionGraph();

  baguatool::core::ProgramAbstractionGraph* pag = hybrid_analysis->GetProgramAbstractionGraph();
  pag->DumpGraph("root_1.gml");

  // pag->PreOrderTraversal(0);

  pag->DumpGraphDot("root_1.dot");
}