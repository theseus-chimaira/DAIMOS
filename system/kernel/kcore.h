#ifndef DAIMON_KCORE_H
#define DAIMON_KCORE_H

typedef unsigned long kword_t;

#define KCORE_BASE              000060UL
#define KCORE_ENTRY_ADDR        (KCORE_BASE + 0UL)
#define KCORE_EARLY_INIT_ADDR   (KCORE_BASE + 1UL)
#define KCORE_PUTCHAR_ADDR      (KCORE_BASE + 2UL)

#define CTY_NATIVE_PI_LEVEL     4U
#define CTY_E_OK                0

int kcore_early_init_impl(void);
int kcore_putchar_impl(int c);
void kcore_entry_impl(void);

void kcore_pi_low_init(void);
void pdp10_halt(void);

#endif
