#ifndef DAIMON_KCORE_PI_H
#define DAIMON_KCORE_PI_H

#include "kcore.h"

#define PDP10_PI_LEVELS        7U
#define PDP10_PI_LEVEL_MIN     1U
#define PDP10_PI_LEVEL_MAX     7U
#define PDP10_PI_MASK(level)   (0200U >> (level))
#define PDP10_PI_HANDLED       0
#define PDP10_PI_NOT_HANDLED   1

typedef int (*pdp10_pi_handler)(unsigned int level, kword_t opaque);

void pdp10_pi_init(void);
int pdp10_pi_register(unsigned int level, pdp10_pi_handler handler,
    kword_t opaque);
void pdp10_pi_hw_enable(unsigned int mask);
void pdp10_pi_hw_clear(void);

void mach_words_zero(kword_t *dst, unsigned int words);
void mach_pi_stack_prepare(void);

#endif
