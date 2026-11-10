/**
 * @file mm_pure_rebase_v1381.h
 * @brief Staged zero-copy PURE prefix release (not production linked).
 */
#ifndef DAIMOS_MM_PURE_REBASE_V1381_H
#define DAIMOS_MM_PURE_REBASE_V1381_H

#include "mm.h"

/*
 * Convert exactly one pinned PROCESS extent into a PACKED_PROCESS extent
 * covering its original suffix [base+prefix, base+original_words).
 *
 * Caller disables PI, owns the process transition and immutable executable
 * backing, and knows that no user APR can run until publication completes.
 * The primitive does not copy data, allocate memory, unpin, or publish a
 * process mode. On failure the MM descriptor is unchanged.
 *
 * Success: 0, new pinned extent base is old base + prefix.
 * Errors: MM_ERR_INVAL or MM_ERR_BUSY (pinned other than exactly once).
 */
int mm_pure_rebase_pinned_v1381(kword_t base, unsigned int owner,
    kword_t prefix, kword_t original_words);

#endif
