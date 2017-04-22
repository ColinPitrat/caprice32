#ifndef LOG_H
#define LOG_H

#include <iostream>

extern bool log_verbose;

#define LOG_TO(stream,level,message) stream << (level) << " " << __FILE__ << ":" << __LINE__ << " - " << message << std::endl; // NOLINT(misc-macro-parentheses): Not having parentheses around message is a feature, it allows using streams in LOG macros

#define LOG_ERROR(message) LOG_TO(std::cerr, "ERROR  ", message)
#define LOG_WARNING(message) LOG_TO(std::cerr, "WARNING", message)
#define LOG_INFO(message) LOG_TO(std::cerr, "INFO   ", message)
#define LOG_VERBOSE(message) if(log_verbose) { LOG_TO(std::cout, "VERBOSE", message) }

#ifdef DEBUG
#define LOG_DEBUG(message) if(log_verbose) { LOG_TO(std::cout, "DEBUG  ", message) }
#else
#define LOG_DEBUG(message)
#endif

#endif
