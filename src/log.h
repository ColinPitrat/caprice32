#ifndef LOG_H
#define LOG_H

#include <iostream>

#ifdef DEBUG
#define LOG(x) std::cout << __FILE__ << ":" << __LINE__ << " - " << x << std::endl;
#else
#define LOG(x)
#endif

#endif
