#ifndef DAIMON_DSK270_H
#define DAIMON_DSK270_H

#include "kcore.h"

#define DSK270_UNITS             4U
#define DSK270_WORDS_PER_SECTOR  0200U
#define DSK270_SECTORS_PER_CYL   054U
#define DSK270_CYLINDERS         02000U
#define DSK270_SECTORS_PER_UNIT  0130000UL

#define DSK270_HW_UNIT_SHIFT     16U
#define DSK270_CYL_SHIFT          6U

int dsk270_read_sector(unsigned int unit, kword_t sector, kword_t *buf);
int dsk270_write_sector(unsigned int unit, kword_t sector,
    const kword_t *buf);


#endif
