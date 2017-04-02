// Caprice 32
// File IO functions

#include <dirent.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int file_size (int fd) {
   struct stat s;

   if (!fstat(fd, &s)) {
      return s.st_size;
   } else {
      return 0;
   }
}

bool file_copy(FILE *in, FILE *out) {
  size_t read;
  char buffer[1024];
  while((read = fread(buffer, 1, 1024, in)) > 0) {
      if (fwrite(buffer, 1, read, out) != read) {
        break;
      }
  }
  if (ferror(in) || ferror(out)) {
    return false;
  }
  return true;
}

// Returns a vector containing the names of the files in the specified directory
std::vector<std::string> listDirectory(std::string sDirectory) {
   std::vector<std::string> s;

   if (sDirectory[sDirectory.size() - 1] != '/') {
      sDirectory += "/";
   }
   DIR* pDir;
   struct dirent *pent;
   pDir = opendir(sDirectory.c_str());
   if (!pDir){
       printf ("opendir(%s) failed; terminating\n", sDirectory.c_str());
       return s;
   }
   while ((pent = readdir(pDir))){
       std::string fileName = std::string(pent->d_name);
       if (fileName != ".." and fileName != ".") {
           s.push_back(fileName);
       }
   }
   closedir(pDir);
   sort(s.begin(), s.end()); // sort elements
   return s;
}
