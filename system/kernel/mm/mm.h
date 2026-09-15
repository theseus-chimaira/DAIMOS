#ifndef DAIMON_MM_H
#define DAIMON_MM_H

#include "kcore.h"

/*
 * PDP-6 V0.9 managed-core allocator.
 *
 * The extent table contains allocated ranges only, sorted by physical base.
 * Each allocation descriptor is two PDP-10 words.  Free memory is represented
 * implicitly by gaps between allocations inside a tiny set of packed managed
 * arenas, so fragmentation never consumes allocation descriptors.
 *
 * Twenty allocation descriptors preserve the measured v0.9 runtime capacity.
 * KINIT needs at most three simultaneously disjoint managed arenas: the low
 * reclaimable region, the high tail above its reserve stack, and the current
 * run of dead MRES source packages.  Those arenas merge as KINIT is reclaimed.
 */
#define MM_MAX_EXTENTS          20
#define MM_MAX_ARENAS           3

#define MM_TYPE_FREE            0U
#define MM_TYPE_PROCESS         1U
#define MM_TYPE_MODULE          2U
#define MM_TYPE_KERNEL_DYNAMIC  3U

#define MM_ALLOC_LOW            0U
#define MM_ALLOC_HIGH           1U

#define MM_OK                    0
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
#define MM_OWNER_MASK           MM_HALF_MASK

struct mm_extent {
        kword_t span;           /* LH size, RH physical base. */
        kword_t meta;           /* pin count, type, owner id. */
};


extern struct mm_extent mm_extents[MM_MAX_EXTENTS];
extern kword_t mm_arenas[MM_MAX_ARENAS];
extern kword_t mm_core_words;
extern int mm_extent_count;
extern int mm_arena_count;

void mm_boot_init(kword_t core_words);
int mm_add_free(kword_t base, kword_t words);
int mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep);
int mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep);
int mm_free(kword_t base, unsigned int type, unsigned int owner);
int mm_pin(kword_t base);
int mm_unpin(kword_t base);
int mm_is_pinned(kword_t base);
int mm_move_module(unsigned int owner);
int mm_compact(kword_t words, kword_t alignment);
kword_t mm_total_free(void);
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

#endif
