#ifndef DAIMON_MACH_USER_H
#define DAIMON_MACH_USER_H

#include "kcore.h"

void mach_enter_user(kword_t base, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);
void mach_return_to_kernel_request(void);

#endif
