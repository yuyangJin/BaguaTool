#ifndef UTILS_H_
#define UTILS_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "common.h"

#ifndef MAX_STR_LEN
#define MAX_STR_LEN 256
#endif

using namespace std;

void getFiles(std::string path, std::vector<std::string> &files);
vector<string> split(const string &str, const string &delim);
int split(char *str, const char *delim, char dst[][MAX_STR_LEN]);

template <class keyType, class valueType>
void DumpMap(std::map<keyType, valueType>& m, std::string& file_name){
  char file_name_str[MAX_STR_LEN];
  strcpy(file_name_str, file_name.c_str());
  std::ofstream fout;
  fout.open(file_name_str);

  for (auto &kv: m) {
    fout << kv.first << " " << kv.second << std::endl;
  }

  fout.close();
}

#endif
