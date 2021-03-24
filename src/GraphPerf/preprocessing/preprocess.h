#ifndef PREPROCESS_H_
#define PREPROCESS_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "../../pag.h"


class Preprocess{
 private:
  
  
 public:

  Preprocess() {

  }
  ~Preprocess(){

  }

  void ReadFunctionGraphs(const char* dir_name, std::vector<ProgramAbstractionGraph*> &func_pag_vec);

  ProgramAbstractionGraph* InterProceduralAnalysis(std::vector<ProgramAbstractionGraph*> &func_pag_vec);

  //void ConnectCallerCallee(ProgramAbstractionGraph* pag, int vertex_id, void* extra);

};

#endif