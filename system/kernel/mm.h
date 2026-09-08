#ifndef DAIMON_MM_H
#define DAIMON_MM_H

#include "kcore.h"

/*
 * PDP-6 V0.9 managed-core allocator.
 *
 * Descriptors are kept in physical-base order.  Each descriptor uses only two
 * PDP-10 words so the allocator has a small fixed resident footprint.  The
 * initial limit is deliberately modest; descriptor exhaustion is reported
 * separately from core exhaustion and can be raised after measurement.
 */
#define MM_MAX_EXTENTS          32U

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
extern kword_t mm_core_words;
extern unsigned int mm_extent_count;

void mm_boot_init(kword_t managed_base, kword_t core_words);
int mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep);
int mm_free(kword_t base, unsigned int type, unsigned int owner);
int mm_pin(kword_t base);
int mm_unpin(kword_t base);
kword_t mm_total_free(void);
kword_t mm_largest_free(void);

#define MM_EXTENT_BASE(e) ((e)->span & MM_HALF_MASK)
#define MM_EXTENT_WORDS(e) (((e)->span >> 18U) & MM_HALF_MASK)
#define MM_EXTENT_OWNER(e) ((unsigned int)((e)->meta & MM_OWNER_MASK))
#define MM_EXTENT_TYPE(e) \
        ((unsigned int)(((e)->meta >> MM_TYPE_SHIFT) & MM_TYPE_MASK))
#define MM_EXTENT_PINS(e) \
        ((unsigned int)(((e)->meta >> MM_PIN_SHIFT) & MM_PIN_MASK))

#endif
