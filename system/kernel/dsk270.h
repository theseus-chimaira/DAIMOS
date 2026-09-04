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

/*
 * Current run I/O intentionally loops over sector I/O.  Keeping this tiny
 * policy wrapper inline costs no resident words until a caller needs it;
 * hardware streaming can replace it later without changing the API.
 */
static inline int
dsk270_read_run(unsigned int unit, kword_t sector, kword_t count,
    kword_t *buf)
{
        if (count == 0UL)
                return 0;
        if (buf == 0 || count > DSK270_SECTORS_PER_UNIT ||
            sector > DSK270_SECTORS_PER_UNIT - count)
                return -1;
        do {
                if (dsk270_read_sector(unit, sector, buf) != 0)
                        return -1;
                ++sector;
                buf += DSK270_WORDS_PER_SECTOR;
        } while (--count != 0UL);
        return 0;
}

static inline int
dsk270_write_run(unsigned int unit, kword_t sector, kword_t count,
    const kword_t *buf)
{
        if (count == 0UL)
                return 0;
        if (buf == 0 || count > DSK270_SECTORS_PER_UNIT ||
            sector > DSK270_SECTORS_PER_UNIT - count)
                return -1;
        do {
                if (dsk270_write_sector(unit, sector, buf) != 0)
                        return -1;
                ++sector;
                buf += DSK270_WORDS_PER_SECTOR;
        } while (--count != 0UL);
        return 0;
}

#endif
