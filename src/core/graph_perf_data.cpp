#include "graph_perf_data.h"
#include <string>
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

void GraphPerfData::SetPerfData(vertex_t vertex_id, std::string& metric, int process_id, int thread_id,
                                perf_data_t data) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(process_id);
  std::string thread_id_str = std::to_string(thread_id);
  dbg(vertex_id_str, metric_str, process_id_str, thread_id_str);

  std::cout << this->j_perf_data.size() << std::endl;
  std::cout << this->j_perf_data.dump() << std::endl;

  this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str] = data;
}

perf_data_t GraphPerfData::GetPerfData(vertex_t vertex_id, std::string& metric, int process_id, int thread_id) {
  perf_data_t data = 0;

  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(process_id);
  std::string thread_id_str = std::to_string(thread_id);

  dbg(this->j_perf_data.contains(vertex_id_str));

  if (this->j_perf_data.contains(vertex_id_str)) {
    if (this->j_perf_data[vertex_id_str].contains(metric_str)) {
      if (this->j_perf_data[vertex_id_str][metric_str].size() > (unsigned int)process_id) {
        if (this->j_perf_data[vertex_id_str][metric_str][process_id_str].size() > (unsigned int)thread_id) {
          auto ret = this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str];
          if (ret != nullptr) {
            data += this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str].get<perf_data_t>();
          }
        }
      }
    }
  }

  return data;
}

bool GraphPerfData::HasMetric(vertex_t vertex_id, std::string& metric) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);

  if (this->j_perf_data[vertex_id_str].contains(metric_str)) {
    return true;
  } else {
    return false;
  }
}
void GraphPerfData::GetVertexPerfDataMetrics(vertex_t vertex_id, std::vector<std::string>& metrics) {
  std::string vertex_id_str = std::to_string(vertex_id);

  for (auto& el : this->j_perf_data[vertex_id_str].items()) {
    metrics.push_back(std::string(el.key()));
  }
  return;
}
// perf_data_t** GraphPerfData::GetMetricPerfData(vertex_t vertex_id, std::string& metric) {}
void GraphPerfData::GetProcessPerfData(vertex_t vertex_id, std::string& metric, int process_id,
                                       perf_data_t* proc_perf_data) {
  std::string vertex_id_str = std::to_string(vertex_id);
  std::string metric_str = std::string(metric);
  std::string process_id_str = std::to_string(process_id);
  std::string thread_id_str;

  int size = this->j_perf_data[vertex_id_str][metric_str][process_id_str].size();
  proc_perf_data = new perf_data_t[size]();
  for (int i = 0; i < size; i++) {
    // memset();
    thread_id_str = std::to_string(i);
    if (this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str] != nullptr) {
      proc_perf_data[i] =
          this->j_perf_data[vertex_id_str][metric_str][process_id_str][thread_id_str].get<perf_data_t>();
    } else {
      proc_perf_data[i] = 0.0;
    }
  }
  return;
}

}  // namespace baguatool::core