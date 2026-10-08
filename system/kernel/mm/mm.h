/**
 * @file mm.h
 * @brief DAIMOS contiguous physical-core allocator interface.
 *
 * PDP-6 has relocation/protection hardware but no paged MMU.  DAIMOS therefore
 * allocates contiguous physical extents and moves stopped process images when
 * fragmentation prevents an otherwise affordable allocation.  Free space is
 * represented by gaps between sorted allocated extents inside up to three
 * managed arenas.
 */
#ifndef DAIMON_MM_H
#define DAIMON_MM_H

#include "kcore.h"

/**
 * Maximum simultaneously allocated physical extents tracked by MM.
 *
 * The earlier 21-descriptor limit covered the measured eight-stage DSH
 * pipeline, but native compilation adds nested MAKE, KCC and KCPP processes.
 * Each needs both a user VM extent and a kernel u-area extent, so retain
 * bounded headroom for this ordinary build workload.
 */
#define MM_MAX_EXTENTS          32
/** Maximum disjoint managed physical arenas needed during/after KINIT. */
#define MM_MAX_ARENAS           3

#define MM_TYPE_FREE            0U
#define MM_TYPE_PROCESS         1U
#define MM_TYPE_MODULE          2U
#define MM_TYPE_KERNEL_DYNAMIC  3U

#define MM_ALLOC_LOW            0U
#define MM_ALLOC_HIGH           1U

#define MM_OK                    0
/* Retryable pressure results are deliberately contiguous: -1 through -3. */
#define MM_ERR_NOMEM           -1
#define MM_ERR_FRAGMENTED      -2
#define MM_ERR_DESCRIPTORS     -3
#define MM_ERR_INVAL           -4
#define MM_ERR_BUSY            -5

#define MM_HALF_MASK            0777777UL
#define MM_TYPE_SHIFT           18U
#define MM_TYPE_MASK            07UL
#define MM_PIN_SHIFT            21U
#define MM_PIN_MASK             0377UL
#define MM_PIN_FIELD_MASK       ((kword_t)MM_PIN_MASK << MM_PIN_SHIFT)
#define MM_OWNER_MASK           MM_HALF_MASK

struct mm_extent {
        kword_t span;           /**< LH size, RH physical base. */
        kword_t meta;           /**< LH pins/type, RH owner id. */
};


extern struct mm_extent mm_extents[MM_MAX_EXTENTS];
extern kword_t mm_arenas[MM_MAX_ARENAS];
extern kword_t mm_core_words;
extern int mm_extent_count;
extern int mm_arena_count;

/** Initialize MM bookkeeping before boot-time arenas are published. */
void mm_boot_init(kword_t core_words);
/** Add one free physical boot range, merging adjacent arenas. */
int mm_add_free(kword_t base, kword_t words);
/** Allocate an unaligned contiguous physical extent. */
int mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep);
/** Allocate with a power-of-two physical alignment requirement. */
int mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep);
/** Free one exact allocated extent after owner/type/pin validation. */
int mm_free(kword_t base, unsigned int type, unsigned int owner);
/** Increment an extent's physical pin count. */
int mm_pin(kword_t base);
/** Decrement an extent's physical pin count. */
int mm_unpin(kword_t base);
/** Return nonzero when the extent beginning at base is physically pinned. */
int mm_is_pinned(kword_t base);
/** Compact movable process extents until an aligned request can fit. */
int mm_compact(kword_t words, kword_t alignment);
/** Return the largest currently contiguous free run. */
kword_t mm_largest_free(void);

#define MM_ARENA_BASE(a) ((a) & MM_HALF_MASK)
#define MM_ARENA_WORDS(a) (((a) >> 18U) & MM_HALF_MASK)

#define MM_EXTENT_BASE(e) ((e)->span & MM_HALF_MASK)
#define MM_EXTENT_WORDS(e) (((e)->span >> 18U) & MM_HALF_MASK)
#define MM_EXTENT_OWNER(e) ((unsigned int)((e)->meta & MM_OWNER_MASK))
#define MM_EXTENT_TYPE(e) \
        ((unsigned int)(((e)->meta >> MM_TYPE_SHIFT) & MM_TYPE_MASK))
#define MM_EXTENT_PINS(e) \
        ((unsigned int)(((e)->meta >> MM_PIN_SHIFT) & MM_PIN_MASK))
#define MM_EXTENT_PINNED(e)     (((e)->meta & MM_PIN_FIELD_MASK) != 0UL)

#endif
