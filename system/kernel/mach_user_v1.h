#ifndef DAIMON_MACH_USER_V1_H
#define DAIMON_MACH_USER_V1_H

#include "kcore.h"

#define MACH_USER_V1_SYSCALL_CONTEXT_WORDS 16U

void mach_enter_user_v1(kword_t base, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);
void mach_syscall_context_save_v1(kword_t *buf);
void mach_syscall_context_restore_v1(kword_t *buf);
void mach_return_to_kernel_request_v1(void);

#endif
