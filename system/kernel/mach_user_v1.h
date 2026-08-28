#ifndef DAIMON_MACH_USER_V1_H
#define DAIMON_MACH_USER_V1_H

#include "kcore.h"

void mach_syscall_trampoline_v1(void);
void mach_enter_user_v1(kword_t base, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);
void mach_return_to_kernel_request_v1(void);

#endif
