/**
 * @file proc_xip_policy_v1394.h
 * @brief Restricted, no-user-write raw MEMFS XIP execution policy.
 *
 * Staged for privileged process admission; deliberately not called by the
 * general MEMFS XIP pin/format probe or linked into production KCORE.
 */
#ifndef DAIMOS_PROC_XIP_POLICY_V1394_H
#define DAIMOS_PROC_XIP_POLICY_V1394_H

#include "kcore.h"

/* The trusted-source argument is set only after an immutable MEMFS vnode
 * is pinned and verified by the caller. Never take it from user memory.
 * Return zero only for the exact two-instruction, stackless EXIT profile.
 */
int proc_xip_policy_v1394(const kword_t *image, kword_t words,
    kword_t entry, unsigned int uid, unsigned int trusted_source,
    unsigned int argc, unsigned int envc);

#endif
