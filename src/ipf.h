#ifndef IPF_H
#define IPF_H

#include "disk.h"
#include <string>

int ipf_load (FILE *, t_drive*);
int ipf_load (const std::string&, t_drive *);

#endif
