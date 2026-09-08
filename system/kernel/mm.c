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

static void mm_coalesce(unsigned int slot);

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
mm_boot_init(kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0U;
}

/* Add a physically free boot-time range.  KINIT deliberately adds only
 * ranges that do not contain its own live image or packaged MRES sources. */
int
mm_add_free(kword_t base, kword_t words)
{
        struct mm_extent extent;
        unsigned int slot;
        kword_t end;

        if (words == 0UL || base > MM_HALF_MASK || words > MM_HALF_MASK ||
            base >= mm_core_words || words > mm_core_words - base)
                return MM_ERR_INVAL;
        end = base + words;
        slot = 0U;
        while (slot < mm_extent_count &&
            MM_EXTENT_BASE(&mm_extents[slot]) < base)
                ++slot;
        if (slot != 0U) {
                struct mm_extent *left = &mm_extents[slot - 1U];
                if (MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) > base)
                        return MM_ERR_INVAL;
        }
        if (slot < mm_extent_count &&
            end > MM_EXTENT_BASE(&mm_extents[slot]))
                return MM_ERR_INVAL;
        extent.span = mm_span(base, words);
        extent.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
        if (mm_insert(slot, &extent) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        mm_coalesce(slot);
        return MM_OK;
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

static int
mm_use_free(unsigned int slot, kword_t base, kword_t words,
    unsigned int type, unsigned int owner, kword_t *basep)
{
        struct mm_extent used;
        struct mm_extent tail;
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_end;
        kword_t before;
        kword_t after;
        unsigned int needed;

        freep = &mm_extents[slot];
        free_base = MM_EXTENT_BASE(freep);
        free_end = free_base + MM_EXTENT_WORDS(freep);
        if (base < free_base || words > free_end - base)
                return MM_ERR_INVAL;
        before = base - free_base;
        after = free_end - (base + words);
        needed = (before != 0UL ? 1U : 0U) + (after != 0UL ? 1U : 0U);
        if (mm_extent_count + needed > MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;

        used.span = mm_span(base, words);
        used.meta = mm_meta(type, owner, 0U);
        if (before == 0UL) {
                *freep = used;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_insert(slot + 1U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        } else {
                freep->span = mm_span(free_base, before);
                if (mm_insert(slot + 1U, &used) != MM_OK)
                        return MM_ERR_DESCRIPTORS;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_insert(slot + 2U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        }
        *basep = base;
        return MM_OK;
}

int
mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_words;
        kword_t free_end;
        kword_t base;
        unsigned int i;
        int step;

        if (basep == 0 || words == 0UL || words > MM_HALF_MASK ||
            alignment == 0UL || alignment > MM_HALF_MASK ||
            (alignment & (alignment - 1UL)) != 0UL ||
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
                        free_end = free_base + free_words;
                        if (free_words >= words) {
                                if (preference == MM_ALLOC_LOW) {
                                        base = (free_base + alignment - 1UL) &
                                            ~(alignment - 1UL);
                                        if (base >= free_base && base <= free_end &&
                                            words <= free_end - base)
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                } else {
                                        base = (free_end - words) &
                                            ~(alignment - 1UL);
                                        if (base >= free_base)
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                }
                        }
                }
                if (step > 0)
                        ++i;
        }
        return mm_total_free() >= words ? MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
}

int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        return mm_alloc_aligned(words, 1UL, type, owner, preference, basep);
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
                if (MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) ==
                    MM_EXTENT_BASE(right)) {
                        words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                        left->span = mm_span(MM_EXTENT_BASE(left), words);
                        mm_delete(slot);
                        --slot;
                }
        }
        if (slot + 1U < mm_extent_count &&
            MM_EXTENT_TYPE(&mm_extents[slot + 1U]) == MM_TYPE_FREE) {
                left = &mm_extents[slot];
                right = &mm_extents[slot + 1U];
                base = MM_EXTENT_BASE(left);
                if (base + MM_EXTENT_WORDS(left) == MM_EXTENT_BASE(right)) {
                        words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                        left->span = mm_span(base, words);
                        mm_delete(slot + 1U);
                }
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
