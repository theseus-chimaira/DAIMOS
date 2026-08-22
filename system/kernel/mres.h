#ifndef DAIMON_MRES_H
#define DAIMON_MRES_H

#include "kinit.h"

/*
 * Resident code/data is linked once at its final low address.  The initialized
 * words are stored once in the disposable high boot image; BSS is represented
 * only by the final aggregate boundary.
 */
extern kword_t __resident_load_begin;
extern kword_t __resident_low_init_end;
extern kword_t __resident_low_end;

void mres_load(void);

#endif
