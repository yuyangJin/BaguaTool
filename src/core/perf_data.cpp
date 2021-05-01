#include "perf_data.h"
#include "dbg.h"

namespace baguatool::core {

PerfData::PerfData() {
  // unsigned long int size = MAX_TRACE_MEM / 432;
  long int size = 10000;
  // printf("%lu\n", size);
  // dbg(size);
  this->sampler_data = new SaStruct[size];
  this->sampler_data_size = 0;
  strcpy(this->file_name, "SAMPLE.TXT");
}
PerfData::~PerfData() {
  delete[] this->sampler_data;
  if (this->has_open_output_file) {
    fclose(this->sampler_data_fp);
  }
}
// int SetAttribute(){}
int PerfData::Query() {
  for (unsigned long int i = 0; i < this->sampler_data_size; i++) {
    UNIMPLEMENTED();
  }
  return -1;
}
void PerfData::Record() { this->sampler_data_size++; }
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

  // OG_INFO("Rank %d : WRITE %d ADDR to %d TXT\n", mpiRank, call_path_addr_log_pointer[i], i);
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