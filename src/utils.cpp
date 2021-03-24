#include "utils.h"

void getFiles(std::string path, std::vector<std::string> &files)
{
  DIR *dir;
  struct dirent *ptr;

  if ((dir = opendir(path.c_str())) == NULL)
  {
    perror("Open dir error...");
    exit(1);
  }

  while ((ptr = readdir(dir)) != NULL)
  {
    if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
      continue;
    else if (ptr->d_type == 8)
      files.push_back(path + ptr->d_name);
    else if (ptr->d_type == 10)
      continue;
    else if (ptr->d_type == 4)
    {
      //files.push_back(ptr->d_name);
      getFiles(path + ptr->d_name + "/", files);
    }
  }
  closedir(dir);
}