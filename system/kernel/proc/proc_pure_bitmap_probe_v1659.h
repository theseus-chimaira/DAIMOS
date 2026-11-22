/*
 * PDP-6 v1659 experimental bounded PURE RAW bitmap probe.
 *
 * A completed probe reports an exact 36-bit encoded size, but deliberately
 * leaves the original singly owned RAW suffix untouched and unpinned.
 * This is NOT a compressed-process publication or a mode-3 owner.
 * No new KCORE object or persistent process state is installed.
 */
#ifndef PURE_BITMAP_PROBE_V1659_H
#define PURE_BITMAP_PROBE_V1659_H

#include "mm.h"

#define PURE_BITMAP_NO_GAIN_V1659 (-6)

/* Caller already holds PROC_TRANSITION on an inactive PURE PACKED owner.
 * On success, *packed_words is the measured and encoded output length.
 * Temporary destination has been unpinned AND freed, RAW remains exact.
 * On MM cleanup uncertainty, return BUSY and KEEP TRANSITION held.
 */
int proc_pure_bitmap_probe_v1659(unsigned int slot,
    kword_t *packed_words);

#endif
