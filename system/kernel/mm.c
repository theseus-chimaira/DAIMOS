#include "mm.h"

struct mm_extent mm_extents[MM_MAX_EXTENTS];
kword_t mm_core_words;
unsigned int mm_extent_count;

static kword_t
mm_span(kword_t base, kword_t words)
{
        return ((words & MM_HALF_MASK) << 18U) | (base & MM_HALF_MASK);
}

static kword_t
mm_meta(unsigned int type, unsigned int owner, unsigned int pins)
{
        return ((kword_t)(pins & MM_PIN_MASK) << MM_PIN_SHIFT) |
            ((kword_t)(type & MM_TYPE_MASK) << MM_TYPE_SHIFT) |
            ((kword_t)owner & MM_OWNER_MASK);
}

static void
mm_delete(unsigned int slot)
{
        while (++slot < mm_extent_count)
                mm_extents[slot - 1U] = mm_extents[slot];
        --mm_extent_count;
}

static int
mm_insert(unsigned int slot, const struct mm_extent *extent)
{
        unsigned int i;

        if (mm_extent_count >= MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;
        i = mm_extent_count++;
        while (i > slot) {
                mm_extents[i] = mm_extents[i - 1U];
                --i;
        }
        mm_extents[slot] = *extent;
        return MM_OK;
}

void
mm_boot_init(kword_t managed_base, kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0U;
        if (managed_base >= core_words || managed_base > MM_HALF_MASK)
                return;
        mm_extents[0].span = mm_span(managed_base, core_words - managed_base);
        mm_extents[0].meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
        mm_extent_count = 1U;
}

kword_t
mm_total_free(void)
{
        unsigned int i;
        kword_t total;

        total = 0UL;
        for (i = 0U; i < mm_extent_count; ++i)
                if (MM_EXTENT_TYPE(&mm_extents[i]) == MM_TYPE_FREE)
                        total += MM_EXTENT_WORDS(&mm_extents[i]);
        return total;
}

kword_t
mm_largest_free(void)
{
        unsigned int i;
        kword_t largest;
        kword_t words;

        largest = 0UL;
        for (i = 0U; i < mm_extent_count; ++i) {
                if (MM_EXTENT_TYPE(&mm_extents[i]) != MM_TYPE_FREE)
                        continue;
                words = MM_EXTENT_WORDS(&mm_extents[i]);
                if (words > largest)
                        largest = words;
        }
        return largest;
}

int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        struct mm_extent used;
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_words;
        kword_t base;
        unsigned int i;
        int step;

        if (basep == 0 || words == 0UL || words > MM_HALF_MASK ||
            type == MM_TYPE_FREE || type > MM_TYPE_KERNEL_DYNAMIC ||
            preference > MM_ALLOC_HIGH)
                return MM_ERR_INVAL;

        if (preference == MM_ALLOC_LOW) {
                i = 0U;
                step = 1;
        } else {
                i = mm_extent_count;
                step = -1;
        }
        while ((step > 0 && i < mm_extent_count) || (step < 0 && i != 0U)) {
                if (step < 0)
                        --i;
                freep = &mm_extents[i];
                if (MM_EXTENT_TYPE(freep) == MM_TYPE_FREE) {
                        free_base = MM_EXTENT_BASE(freep);
                        free_words = MM_EXTENT_WORDS(freep);
                        if (free_words >= words) {
                                if (free_words == words) {
                                        freep->meta = mm_meta(type, owner, 0U);
                                        *basep = free_base;
                                        return MM_OK;
                                }
                                if (mm_extent_count >= MM_MAX_EXTENTS)
                                        return MM_ERR_DESCRIPTORS;
                                if (preference == MM_ALLOC_LOW) {
                                        base = free_base;
                                        freep->span = mm_span(free_base + words,
                                            free_words - words);
                                        used.span = mm_span(base, words);
                                        used.meta = mm_meta(type, owner, 0U);
                                        if (mm_insert(i, &used) != MM_OK)
                                                return MM_ERR_DESCRIPTORS;
                                } else {
                                        base = free_base + free_words - words;
                                        freep->span = mm_span(free_base,
                                            free_words - words);
                                        used.span = mm_span(base, words);
                                        used.meta = mm_meta(type, owner, 0U);
                                        if (mm_insert(i + 1U, &used) != MM_OK)
                                                return MM_ERR_DESCRIPTORS;
                                }
                                *basep = base;
                                return MM_OK;
                        }
                }
                if (step > 0)
                        ++i;
        }
        return mm_total_free() >= words ? MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
}

static void
mm_coalesce(unsigned int slot)
{
        struct mm_extent *left;
        struct mm_extent *right;
        kword_t base;
        kword_t words;

        if (slot != 0U && MM_EXTENT_TYPE(&mm_extents[slot - 1U]) == MM_TYPE_FREE) {
                left = &mm_extents[slot - 1U];
                right = &mm_extents[slot];
                words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                left->span = mm_span(MM_EXTENT_BASE(left), words);
                mm_delete(slot);
                --slot;
        }
        if (slot + 1U < mm_extent_count &&
            MM_EXTENT_TYPE(&mm_extents[slot + 1U]) == MM_TYPE_FREE) {
                left = &mm_extents[slot];
                right = &mm_extents[slot + 1U];
                base = MM_EXTENT_BASE(left);
                words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                left->span = mm_span(base, words);
                mm_delete(slot + 1U);
        }
}

int
mm_free(kword_t base, unsigned int type, unsigned int owner)
{
        unsigned int i;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_TYPE(extent) != type ||
                    MM_EXTENT_OWNER(extent) != owner)
                        return MM_ERR_INVAL;
                if (MM_EXTENT_PINS(extent) != 0U)
                        return MM_ERR_BUSY;
                extent->meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                mm_coalesce(i);
                return MM_OK;
        }
        return MM_ERR_INVAL;
}

int
mm_pin(kword_t base)
{
        unsigned int i;
        unsigned int pins;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_TYPE(extent) == MM_TYPE_FREE)
                        return MM_ERR_INVAL;
                pins = MM_EXTENT_PINS(extent);
                if (pins == MM_PIN_MASK)
                        return MM_ERR_BUSY;
                extent->meta += (kword_t)1UL << MM_PIN_SHIFT;
                return MM_OK;
        }
        return MM_ERR_INVAL;
}

int
mm_unpin(kword_t base)
{
        unsigned int i;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_PINS(extent) == 0U)
                        return MM_ERR_INVAL;
                extent->meta -= (kword_t)1UL << MM_PIN_SHIFT;
                return MM_OK;
        }
        return MM_ERR_INVAL;
}
