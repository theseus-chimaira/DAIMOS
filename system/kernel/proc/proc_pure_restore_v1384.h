/**
 * @file proc_pure_restore_v1384.h
 * @brief Staged prefix-only RAW PURE process restoration.
 *
 * The ordinary v1381 full-extent RESTORE remains the safe fallback.
 * This optional path does not allocate physical RAM or duplicate mutable
 * words. It cannot be used for compressed DXR until prefix-safe decoding
 * is independently accepted. The callback is a temporary boundary:
 * production must share EXEC's image loader for FULL and PURE_PREFIX.
 */
#ifndef DAIMOS_PREFIX_RESTORE_V1384_H
#define DAIMOS_PREFIX_RESTORE_V1384_H

#include "proc.h"

/* Return 0 only after writing the complete missing prefix.
 * -2: unsupported executable format (use full-extent restore).
 * -1: corrupt backing/read/decode failure (no runnable publication).
 * This callback MUST NOT write at or beyond base+prefix.
 */
typedef int (*proc_pure_prefix_loader_v1384)(kword_t backing,
    kword_t base, kword_t uarea, kword_t prefix);

/* +1 means an unavailable gap or unsupported format: the caller can use
 * the existing v1381 full-extent restore with adequate free memory.
 * All other results are MM_OK or negative MM error values.
 */
#define PROC_PURE_PREFIX_FALLBACK_V1384 1

int proc_pure_restore_v1384(unsigned int slot,
    proc_pure_prefix_loader_v1384 loader);

#endif
