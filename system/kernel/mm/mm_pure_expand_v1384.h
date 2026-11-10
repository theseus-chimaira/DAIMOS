/**
 * @file mm_pure_expand_v1384.h
 * @brief Staged in-place PURE prefix claim, rollback and commit.
 *
 * These MM primitives do not change user RAM and do not allocate or copy
 * physical words. Callers must hold process TRANSITION and disable PI for
 * each descriptor mutation. No operation is linked into production KCORE.
 */
#ifndef DAIMOS_MM_PURE_EXPAND_V1384_H
#define DAIMOS_MM_PURE_EXPAND_V1384_H

#include "mm.h"

/*
 * Inputs: physical suffix base, owner, omitted prefix, full APR word count.
 * Require exactly one pin on the PACKED_PROCESS suffix extent. On success
 * its descriptor covers the original full APR span while STILL typed
 * PACKED_PROCESS, so the process remains un-runnable throughout VFS I/O.
 *
 * Return MM_ERR_FRAGMENTED if the prefix gap is occupied or unmanaged;
 * this is the explicit safe-full-restore fallback condition.
 */
int mm_pure_claim_prefix_v1384(kword_t suffix, unsigned int owner,
    kword_t prefix, kword_t full_words);

/*
 * Inputs: original APR base, owner, omitted prefix, full APR word count.
 * Roll back a successful claim to its raw suffix. Revalidation happens
 * before all mutations. The suffix remains the sole authoritative data.
 */
int mm_pure_release_prefix_v1384(kword_t base, unsigned int owner,
    kword_t prefix, kword_t full_words);

/*
 * Inputs as for release. After a completely successful prefix reload,
 * convert the pinned full-size PACKED_PROCESS descriptor into a PROCESS
 * descriptor. The caller publishes the resident process in the SAME
 * PI-disabled region before re-enabling interrupts.
 */
int mm_pure_commit_prefix_v1384(kword_t base, unsigned int owner,
    kword_t prefix, kword_t full_words);

#endif
