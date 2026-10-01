#ifndef DAIMON_BLOCKSET_MRES_H
#define DAIMON_BLOCKSET_MRES_H

#include "kcore.h"

/* KINIT-only relocated runtime-state destinations exported by the MRES. */
extern unsigned int blockset_state_addr;
extern unsigned int blockset_read_addr;
extern unsigned int blockset_write_addr;

#endif
