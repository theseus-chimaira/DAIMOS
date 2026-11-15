/**
 * @file vm_pdp6_pure_range_v1604.h
 * @brief PDP-6-specific contiguous APR PURE omission geometry.
 *
 * P1-E staging only. This routine must never be used as the portable
 * description of retained writable memory: a KA10 can instead retain
 * its private segment without releasing a shared read-only high segment.
 */
#ifndef DAIMOS_PDP6_PURE_RANGE_V1604_H
#define DAIMOS_PDP6_PURE_RANGE_V1604_H

#include "exec.h"
#include "proc_swap.h"
#include "mm.h"

#define PDP6_PURE_SECTOR_V1604 0200UL

/* Return the first retained logical word in a single contiguous APR image.
 * Retain every word from SKIP through WORDS-1, including all mutable data.
 * The layout is intentionally restricted to the accepted PDP-6 DXR/TX2
 * sector-rounded PURE prefix and not an architectural shared-text API.
 */
static int
pdp6_pure_skip_v1604(kword_t backing, kword_t words, kword_t *skipp)
{
        kword_t text;
        kword_t skip;

        text = (backing >> PROC_SWAP_TEXT_SHIFT) & PROC_SWAP_TEXT_MASK;
        if (!text || !words || text > MM_HALF_MASK - EXEC_USER_ORIGIN)
                return MM_ERR_INVAL;
        skip = (text + EXEC_USER_ORIGIN) &
            ~(PDP6_PURE_SECTOR_V1604 - 1UL);
        if (!skip || skip >= words)
                return MM_ERR_INVAL;
        *skipp = skip;
        return MM_OK;
}

#endif
