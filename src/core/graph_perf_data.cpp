#include "graph_perf_data.h"
#include <string>
#include <vector>
#include "dbg.h"

namespace baguatool::core {

GraphPerfData::GraphPerfData() {
  std::cout << this->j_perf_data.size() << std::endl;
  this->j_perf_data.clear();
  std::cout << this->j_perf_data.size() << std::endl;
}
GraphPerfData::~GraphPerfData() {}

void GraphPerfData::Read(std::string& input_file) {
  std::ifstream input(input_file);
  input >> this->j_perf_data;
  input.close();
}

void GraphPerfData::Dump(std::string& output_file) {
  std::ofstream output(output_file);
  output << this->j_perf_data << std::endl;
  output.close();
}

void GraphPerfData::SetPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id, int thread_id,
                                type::perf_data_t data) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(procs_id);
  std::string thread_id_str = std::to_string(thread_id);
  // dbg(vertex_id_str, metric_str, process_id_str, thread_id_str);

  // std::cout << this->j_perf_data.size() << std::endl;
  // std::cout << this->j_perf_data.dump() << std::endl;

  this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str] = data;
}

type::perf_data_t GraphPerfData::GetPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id,
                                             int thread_id) {
  type::perf_data_t data = 0;

  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(procs_id);
  std::string thread_id_str = std::to_string(thread_id);

  if (this->j_perf_data.contains(vertex_id_str)) {
    if (this->j_perf_data[vertex_id_str].contains(metric_str)) {
      if (this->j_perf_data[vertex_id_str][metric_str].contains(process_id_str)) {
        if (this->j_perf_data[vertex_id_str][metric_str][process_id_str].contains(thread_id_str)) {
          auto ret = this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str];
          if (ret != nullptr) {
            data +=
                this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str].get<type::perf_data_t>();
          }
        }
      }
    }
  }

  return data;
}

bool GraphPerfData::HasMetric(type::vertex_t vertex_id, std::string& metric) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);

  if (this->j_perf_data[vertex_id_str].contains(metric_str)) {
    return true;
  } else {
    return false;
  }
}
void GraphPerfData::GetVertexPerfDataMetrics(type::vertex_t vertex_id, std::vector<std::string>& metrics) {
  std::string vertex_id_str = std::to_string(vertex_id);

  for (auto& el : this->j_perf_data[vertex_id_str].items()) {
    metrics.push_back(std::string(el.key()));
  }
  return;
}

int GraphPerfData::GetMetricsPerfDataProcsNum(type::vertex_t vertex_id, std::string& metric) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);

  int size = this->j_perf_data[vertex_id_str][metric_str].size();

  return size;
}

int GraphPerfData::GetProcsPerfDataThreadNum(type::vertex_t vertex_id, std::string& metric, int procs_id) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(procs_id);

  int size = this->j_perf_data[vertex_id_str][metric_str][process_id_str].size();

  return size;
}

// type::perf_data_t** GraphPerfData::GetMetricPerfData(type::vertex_t vertex_id, std::string& metric) {}
void GraphPerfData::GetProcsPerfData(type::vertex_t vertex_id, std::string& metric, int procs_id,
                                     std::vector<type::perf_data_t>& proc_perf_data) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(procs_id);
  std::string thread_id_str;

  for (auto& thread_perf_data : j_perf_data[vertex_id_str][metric_str][process_id_str].items()) {
    thread_id_str = std::string(thread_perf_data.key());

    if (this->j_perf_data[vertex_id_str][metric_str][process_id_str].contains(thread_id_str)) {
      proc_perf_data.push_back(
          this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str].get<type::perf_data_t>());
      // dbg(proc_perf_data[i]);
    } else {
      proc_perf_data.push_back(0.0);
    }
    std::string(thread_perf_data.key());
  }

  // int size = this->j_perf_data[vertex_id_str][metric_str][process_id_str].size();
  // auto this->j_perf_data[vertex_id_str][metric_str][process_id_str].keys()

  // if (size > 0) {
  //   // proc_perf_data = new type::perf_data_t[size]();
  //   for (int i = 0; i < size; i++) {
  //     // memset();
  //     thread_id_str = std::to_string(i);
  //     if (this->j_perf_data[vertex_id_str][metric_str][process_id_str].contains(thread_id_str)) {
  //       proc_perf_data.push_back(
  //           this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str].get<type::perf_data_t>());
  //       dbg(proc_perf_data[i]);
  //     } else {
  //       proc_perf_data.push_back(0.0);
  //     }
  //   }
  // }
  return;
}

}  // namespace baguatool::core