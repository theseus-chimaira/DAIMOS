#ifndef DAIMON_LPT_H
#define DAIMON_LPT_H

#include "kcore.h"

/* PDP-6 line printer, I/O device 0124. */
#define LPT_ST_DONE        0000100UL
#define LPT_ST_BUSY        0000200UL
#define LPT_ST_ERROR       0000400UL
#define LPT_CO_CLEAR       0002000UL

int lpt_putchar(int c);

#endif
