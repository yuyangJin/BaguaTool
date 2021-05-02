#include "perf_data.h"
#include "dbg.h"

namespace baguatool::core {

PerfData::PerfData() {
  this->sampler_data_space_size = MAX_TRACE_MEM / sizeof(SaStruct);
  //long int size = 10000;
  // printf("%lu\n", size);
  // dbg(size);
  this->sampler_data = new SaStruct[this->sampler_data_space_size];
  this->sampler_data_size = 0;
  strcpy(this->file_name, "SAMPLE.TXT");
}
PerfData::~PerfData() {
  delete[] this->sampler_data;
  if (this->has_open_output_file) {
    fclose(this->sampler_data_fp);
  }
}

bool CallPathCmp(addr_t* cp_1, int cp_1_len, addr_t* cp_2, int cp_2_len) {
  if (cp_1_len != cp_2_len) {
    return false;
  } else {
    for (int i = 0; i < cp_1_len; i++) {
      if (cp_1[i] != cp_2[i]) {
        return false;
      }
    }
  }
  return true;
}

int PerfData::Query(addr_t* call_path, int call_path_len, int procs_id, int thread_id) {
  for (unsigned long int i = 0; i < this->sampler_data_size; i++) {
    //call_path
    if (CallPathCmp(call_path, call_path_len, this->sampler_data[i].call_path, this->sampler_data[i].call_path_len) == true && this->sampler_data[i].thread_id == thread_id && this->sampler_data[i].procs_id == procs_id){
      return i;
    }
  }

  return -1;
}
void PerfData::Record(addr_t* call_path, int call_path_len, int procs_id, int thread_id) {

  int index = this->Query(call_path, call_path_len, procs_id, thread_id);

  if (index >= 0) {
    this->sampler_data[index].count ++;
  } else {
    // Thread-safe, first fetch a index, then record data
    unsigned long long int x = __sync_fetch_and_add(&this->sampler_data_size, 1);
    
    this->sampler_data[this->sampler_data_size - 1].call_path_len = call_path_len;
    for (int i = 0; i < call_path_len; i++) {
      this->sampler_data[this->sampler_data_size - 1].call_path[i] = call_path[i];
    }
    this->sampler_data[this->sampler_data_size - 1].count = 1;
    this->sampler_data[this->sampler_data_size - 1].thread_id = thread_id;
    this->sampler_data[this->sampler_data_size - 1].procs_id = procs_id;
  }

  if (this->sampler_data_size >= this->sampler_data_space_size - 5) {
    // TODO: asynchronous dump
    this->Dump();
  }

}
void PerfData::Read(const char* infile_name) {
  // Open a file
  // this->sampler_data_in.open(this->file_name, std::ios::in);
  // dbg(infile_name);
  char infile_name_str[MAX_STR_LEN];
  strcpy(infile_name_str, std::string(infile_name).c_str());
  this->sampler_data_in.open(infile_name_str, std::ios::in);
  if (!(this->sampler_data_in.is_open())) {
    // LOG_INFO("Failed to open %s\n", this->file_name.c_str());
    LOG_INFO("Failed to open %s\n", infile_name_str);
    this->sampler_data_size = __sync_and_and_fetch(&this->sampler_data_size, 0);
    return;
  }

  // Read lines, each line is an SaStruct
  char line[MAX_LINE_LEN];
  while (!(this->sampler_data_in.eof())) {
    // Read a line
    this->sampler_data_in.getline(line, MAX_STR_LEN);

    // Parse the line
    // First '|'
    char delim[] = "|";
    // line_vec[4][MAX_STR_LEN];
    std::vector<std::string> line_vec = split(line, delim);
    int cnt = line_vec.size();

    // dbg(cnt);

    if (cnt == 3) {
      this->sampler_data[this->sampler_data_size].count = atoi(line_vec[1].c_str());
      this->sampler_data[this->sampler_data_size].thread_id = atoi(line_vec[2].c_str());

      // Then parse call path
      char delim_cp[] = " ";
      // char addr_vec[MAX_CALL_PATH_DEPTH][13];
      std::vector<std::string> addr_vec = split(line_vec[0], delim_cp);
      // int cnt = split(line_vec[0], delim_cp, addr_vec);
      int call_path_len = addr_vec.size();
      this->sampler_data[this->sampler_data_size].call_path_len = call_path_len;
      for (int i = 0; i < call_path_len; i++) {
        this->sampler_data[this->sampler_data_size].call_path[i] = strtoul(addr_vec[i].c_str(), NULL, 16);
        dbg(this->sampler_data[this->sampler_data_size].call_path[i]);
      }

      LOG_INFO("DATA[%lu]: %s | %d | %d\n", this->sampler_data_size, line_vec[0].c_str(),
               this->sampler_data[this->sampler_data_size].count,
               this->sampler_data[this->sampler_data_size].thread_id);

      // size ++
      __sync_fetch_and_add(&this->sampler_data_size, 1);
    }
  }

  this->sampler_data_in.close();
}

void PerfData::Dump() {
  if (!has_open_output_file) {
    this->sampler_data_fp = fopen(this->file_name, "w");
    if (!this->sampler_data_fp) {
      LOG_INFO("Failed to open %s\n", this->file_name);
      this->sampler_data_size = __sync_and_and_fetch(&this->sampler_data_size, 0);
      return;
    }
  }

  // LOG_INFO("Rank %d : WRITE %d ADDR to %d TXT\n", mpiRank, call_path_addr_log_pointer[i], i);
  for (unsigned long int i = 0; i < this->sampler_data_size; i++) {
    for (int j = 0; j < this->sampler_data[i].call_path_len; j++) {
      fprintf(this->sampler_data_fp, "%llx ", this->sampler_data[i].call_path[j]);
    }
    fprintf(this->sampler_data_fp, " | %d | %d\n", this->sampler_data[i].count, this->sampler_data[i].thread_id);
    fflush(this->sampler_data_fp);
  }
  this->sampler_data_size = __sync_and_and_fetch(&this->sampler_data_size, 0);
}

unsigned long int PerfData::GetSize() { return this->sampler_data_size; }

std::string& PerfData::GetMetricName() { return this->metric_name; }

void PerfData::SetMetricName(std::string& metric_name) {
  this->metric_name = std::string(metric_name);
}

void PerfData::GetCallPath(unsigned long int data_index, std::stack<unsigned long long>& call_path_stack) {
  SaStruct* data = &(this->sampler_data[data_index]);
  for (int i = 0; i < data->call_path_len; i++) {
    call_path_stack.push(data->call_path[i]);
  }

  return;
}

int PerfData::GetSamplingCount(unsigned long int data_index) {
  SaStruct* data = &(this->sampler_data[data_index]);
  return data->count;
}

int PerfData::GetProcessId(unsigned long int data_index) {
  SaStruct* data = &(this->sampler_data[data_index]);
  return data->procs_id;
}
int PerfData::GetThreadId(unsigned long int data_index) {
  SaStruct* data = &(this->sampler_data[data_index]);
  return data->thread_id;
}

}  // namespace baguatool::core