// Caprice 32
// File IO functions

#include <dirent.h>
#include <string>
#include <vector>

// Returns the file size of the file specified by the file descriptor 'fd'.
int file_size (int fd);

// Copy the content of in to out. Returns true if successful, false otherwise.
bool file_copy(FILE *in, FILE *out);

// Returns a vector containing the names of the files in the specified directory
std::vector<std::string> listDirectory(std::string &);

// Returns a vector containing the names of the files having extension "ext" in
// the specified directory
std::vector<std::string> listDirectoryExt(std::string &, const std::string &);
