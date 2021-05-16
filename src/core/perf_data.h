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

typedef double perf_data_t;
typedef unsigned long long int addr_t;

// size : 8 * 50 + 4 + 4 + 4 + 4 = 432
typedef struct VERTEX_DATA_STRUCT {
  addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};  //
  int call_path_len = 0;                        //
  perf_data_t value = 0;                        //
  int procs_id = 0;                             // process id
  int thread_id = 0;                            // user-defined thread id
} VDS;

typedef struct EDGE_DATA_STRUCT {
  addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};  //
  int call_path_len = 0;                        //
  perf_data_t value = 0;                        //
  int procs_id = 0;                             // process id
  int out_procs_id = 0;                         // process id of communication process
  int thread_id = 0;                            // user-defined thread id
  unsigned long out_thread_id = 0;              // user-defined thread id of a created thread
} EDS;

class PerfData {
 private:
  VDS* vertex_perf_data = nullptr;
  unsigned long int vertex_perf_data_space_size = 0;
  unsigned long int vertex_perf_data_count = 0;
  EDS* edge_perf_data = nullptr;
  unsigned long int edge_perf_data_space_size = 0;
  unsigned long int edge_perf_data_count = 0;
  FILE* perf_data_fp = nullptr;
  std::ifstream perf_data_in_file;
  bool has_open_output_file = false;
  char file_name[MAX_LINE_LEN] = {0};
  // TODO: design a method to make metric_name portable
  std::string metric_name = std::string("TOT_CYC");

 public:
  PerfData();
  ~PerfData();
  int QueryVertexData(addr_t* call_path, int call_path_len, int process_id, int thread_id);
  void RecordVertexData(addr_t* call_path, int call_path_len, int process_id, int thread_id, perf_data_t value);
  void Read(const char*);
  void Dump(const char*);
  unsigned long int GetVertexDataSize();
  void SetMetricName(std::string& metric_name);
  std::string& GetMetricName();
  void GetVertexDataCallPath(unsigned long int data_index, std::stack<unsigned long long>&);
  perf_data_t GetVertexDataValue(unsigned long int data_index);
  int GetVertexDataProcsId(unsigned long int data_index);
  int GetVertexDataThreadId(unsigned long int data_index);
};

}  // namespace baguatool::core

#endif