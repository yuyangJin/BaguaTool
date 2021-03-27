#ifndef UTILS_H_
#define UTILS_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

void getFiles(std::string path, std::vector<std::string> &files);

#endif