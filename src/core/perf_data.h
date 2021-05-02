#ifndef PERF_DATA_
#define PERF_DATA_

#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stack>
#include <string>
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

typedef unsigned long long int addr_t;

// size : 8 * 50 + 4 + 4 + 4 + 4 = 432
typedef struct SAMPLER_STRUCT {
  unsigned long long int call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = 0;
  int count = 0;
  int procs_id = 0;
  int thread_id = 0;
} SaStruct;

class PerfData {
 private:
  SaStruct* sampler_data = nullptr;
  unsigned long int sampler_data_space_size = 0;
  unsigned long int sampler_data_size = 0;
  FILE* sampler_data_fp = nullptr;
  std::ifstream sampler_data_in;
  bool has_open_output_file = false;
  char file_name[MAX_LINE_LEN] = {0};
  // TODO: design a method to make metric_name portable
  std::string metric_name = std::string("TOT_CYC");

 public:
  PerfData();
  ~PerfData();
  int Query(addr_t* call_path, int call_path_len, int process_id, int thread_id);
  void Record(addr_t* call_path, int call_path_len, int process_id, int thread_id);
  void Read(const char*);
  void Dump();
  unsigned long int GetSize();
  void SetMetricName(std::string& metric_name);
  std::string& GetMetricName();
  void GetCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  int GetSamplingCount(unsigned long int data_index);
  int GetProcessId(unsigned long int data_index);
  int GetThreadId(unsigned long int data_index);
};

}  // namespace baguatool::core

#endif