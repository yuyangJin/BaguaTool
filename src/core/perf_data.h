#ifndef PERF_DATA_
#define PERF_DATA_

#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <stack>
#include "common/tprintf.h"
#include "common/utils.h"

#ifndef MAX_CALL_PATH_DEPTH
#define MAX_CALL_PATH_DEPTH 50
#endif

#ifndef MAX_TRACE_MEM
#define MAX_TRACE_MEM 536870912
#endif

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 256
#endif

namespace baguatool::core {

typedef struct SAMPLER_STRUCT {
  unsigned long long int call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = 0;
  int count = 0;
  int thread_id = -1;
} SaStruct;

class PerfData {
 private:
  SaStruct* sampler_data = nullptr;
  unsigned long int sampler_data_size = 0;
  FILE* sampler_data_fp = nullptr;
  std::ifstream sampler_data_in;
  bool has_open_output_file = false;
  char file_name[MAX_LINE_LEN];

 public:
  PerfData();
  ~PerfData();
  // int SetAttribute();
  int Query();
  void Record();
  void Read(std::string&);
  void Dump();
  unsigned long int GetSize();
  void GetCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  int GetSamplingCount(unsigned long int data_index);
};

}  // namespace baguatool::core

#endif