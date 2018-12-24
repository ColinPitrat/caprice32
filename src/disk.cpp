#include "disk.h"

#include <sstream>

#include "log.h"

std::string chrn_to_string(unsigned char* chrn) {
  std::ostringstream oss;
  oss << static_cast<int>(chrn[0]) << "-"
    << static_cast<int>(chrn[1]) << "-"
    << static_cast<int>(chrn[2]) << "-"
    << static_cast<int>(chrn[3]);
  return oss.str();
}

void t_sector::setSizes(unsigned int size, unsigned int total_size) {
  size_ = size;
  total_size_ = total_size;
  weak_read_version_ = 0;
  weak_versions_ = 1;
  if (size_ > 0 && size_ <= total_size_) weak_versions_ = total_size_ / size_;
  LOG_DEBUG("weak_versions_ = " << weak_versions_ << " for " << chrn_to_string(CHRN));
}

