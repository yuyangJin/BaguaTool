#ifndef UTILS_H_
#define UTILS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

void getFiles(std::string path, std::vector<std::string> &files);

#endif