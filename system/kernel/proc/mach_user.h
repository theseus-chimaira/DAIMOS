/**
 * @file mach_user.h
 * @brief Machine boundary used to suppress a pending return to user mode.
 *
 * The concrete PDP-6 monitor-UUO entry/return implementation lives in
 * mach_user_pdp6.s. Generic kernel code uses only the request interface below.
 */
#ifndef DAIMON_MACH_USER_H
#define DAIMON_MACH_USER_H

#include "kcore.h"

/** Force the current syscall return path to remain in executive context. */
void mach_return_to_kernel_request(void);

#endif
