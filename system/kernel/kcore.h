#ifndef DAIMON_KCORE_H
#define DAIMON_KCORE_H

typedef unsigned long kword_t;

#define KCORE_BASE              000060UL
#define KCORE_ENTRY_ADDR        (KCORE_BASE + 0UL)

void kcore_entry_impl(void);
void pdp10_halt(void);

#endif
