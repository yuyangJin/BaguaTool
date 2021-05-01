#include <iostream>
#include <nlohmann/json.hpp>

using namespace nlohmann;

int main() {
  // create an array using push_back
  json jj;
  // jj["1"]["TOT_INS"][0][0] = 2.31;
  // jj["1"]["TOT_INS"][0][3] = 3.66;
  // jj["1"]["TOT_INS"][1][0] = 8.12;
  jj["1"]["TOT_INS"]["1"]["0"] = 47823.2134;

  // std::cout << jj["1"]["TOT_INS"].size() << " " << jj["1"]["TOT_INS"][0].size() << (jj["1"]["TOT_INS"][0][1] !=
  // nullptr )  << " " <<jj["1"].contains("TOT_0NS")<< std::endl;

  std::cout << jj.dump() << std::endl;
  return 0;
}