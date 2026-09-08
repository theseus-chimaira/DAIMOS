#ifndef DAIMON_MRES_H
#define DAIMON_MRES_H

#include "kinit.h"
#include "mres_reloc.h"

#define MRES_MAGIC              SIXBIT("MRES1 ")
#define MRES_HEADER_WORDS       3U

extern kword_t __kcore_load_begin;
extern kword_t __kcore_low_init_end;
extern kword_t __kcore_low_end;
extern unsigned int mres_last_owner;

void kcore_load(void);
void mres_init(void);
int mres_install(const kword_t *package, unsigned int *basep);
unsigned int mres_export(const kword_t *package, unsigned int base,
    unsigned int index);
int mres_call(unsigned int address, void *request);

#endif
