#ifndef IPF_H
#define IPF_H

#ifdef WITH_IPF

#include "types.h"
#include "caps/capsimage.h"
#include "caps/fdc.h"
#include "caps/form.h"
#include "stdio.h"
#include "cap32.h"
#include "disk.h"

int ipf_load (FILE *, t_drive*);
int ipf_load (const std::string&, t_drive *);

#endif

#endif
