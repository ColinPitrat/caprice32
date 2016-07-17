#ifndef LOG_H
#define LOG_H

#include <iostream>

extern bool log_verbose;

#ifdef DEBUG
#define LOG(x) if(log_verbose) { std::cout << __FILE__ << ":" << __LINE__ << " - " << x << std::endl; }
#else
#define LOG(x)
#endif

#endif
