#ifndef DAIMON_KCORE_H
#define DAIMON_KCORE_H

#ifndef DAIMON_KWORD_T_DEFINED
#define DAIMON_KWORD_T_DEFINED
typedef unsigned long kword_t;
#endif

#define KCORE_BASE              000060UL
#define KCORE_ENTRY_ADDR        KCORE_BASE

extern kword_t kcore_boot_handoff[2];

void kcore_entry_impl(void);
void pdp10_halt(void);

#endif
