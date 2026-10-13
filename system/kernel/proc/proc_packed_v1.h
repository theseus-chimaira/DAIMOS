/*
 * @file proc_packed_v1.h
 * @brief Contract for future in-core packed process allocations.
 *
 * This header is intentionally declarative. No runtime packed-process state
 * exists until eviction, restoration, and rollback can be atomic.
 */
#ifndef DAIMOS_PROC_PACKED_V1_H
#define DAIMOS_PROC_PACKED_V1_H

#include "mm.h"

/* Packed extents belong to their process slot and need no APR alignment. */
#define PROC_PACKED_MM_TYPE MM_TYPE_PACKED_PROCESS

/*
 * A future nonresident packed process must retain:
 * - stable u-area containing saved registers and PURE executable backing;
 * - an MM_TYPE_PACKED_PROCESS extent owned by its process slot;
 * - a packed span (RH physical base, LH compressed word count).
 *
 * The existing proc_swap_records[slot].state is ambiguous across resident
 * and legacy disk-swapped processes. It MUST NOT hold a packed span until
 * a separately validated ownership discriminator has been introduced.
 * A packed process may not be scheduled or swapped by legacy disk routines.
 */
#endif
