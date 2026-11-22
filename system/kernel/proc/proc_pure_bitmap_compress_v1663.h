/* v1663 opt-in full bounded PURE bitmap preparation and atomic commit. */
#ifndef DAIMOS_PBC1663_H
#define DAIMOS_PBC1663_H
#include "mm.h"

#define PURE_BITMAP_NO_GAIN_V1663 (-6)

/* Caller holds TRANSITION for a noncurrent sleeping/stopped RAW PURE
 * process and passes an initially zero output. On success, mode3 is
 * published, the single compressed MM owner is unpinned and the
 * original RAW MM owner is gone; TRANSITION remains held for the
 * lifecycle caller. Ordinary refusal restores one exact unpinned
 * RAW owner. Uncertain cleanup returns BUSY and must NOT unlock.
 */
int proc_pure_bitmap_compress_v1663(unsigned int slot,
    kword_t *encoded_wordsp);

#endif
