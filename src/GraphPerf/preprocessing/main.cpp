#include "preprocess.h"
#include "../../pag.h"

int main(int argc, char** argv){
  // if (argc != 2) {
  //   printf("Usage: ./odd_even_sort <number_count> <input_file> <std_data_file>\n");
  //   return 1;
  // }
  // const int n = atoi(argv[1]);
  const char *dir_name = argv[1];

  Preprocess* prep = new Preprocess(); 
    
  std::vector<ProgramAbstractionGraph*> func_pag_vec;
    
  prep->ReadFunctionGraphs(dir_name, func_pag_vec);

  ProgramAbstractionGraph* root = prep->InterProceduralAnalysis(func_pag_vec);

  root->DumpGraph("root.gml");

  root->DumpGraphDot("root.dot");
  

  //PerfData* perf_data = pre->ReadPerformanceData();

  //prep->PerformanceDataEmbedding(root, func_pag_vec, perf_data);

  //prep->

}