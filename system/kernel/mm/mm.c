#include "mm.h"
#include "vm.h"
#include "module_runtime.h"
#include "proc_swap.h"
#include "fs_mres.h"

struct mm_extent mm_extents[MM_MAX_EXTENTS];
kword_t mm_core_words;
int mm_extent_count;

/* Extent indexes are bounded by MM_MAX_EXTENTS, and validated physical
 * bases/lengths fit the positive PDP-10 core-address domain.  Keep bounded
 * arithmetic signed so GCC need not synthesize unsigned sign-bit compares. */

static inline kword_t
mm_span(kword_t base, kword_t words)
{
        return ((words & MM_HALF_MASK) << 18U) | (base & MM_HALF_MASK);
}

static inline kword_t
mm_meta(unsigned int type, unsigned int owner, unsigned int pins)
{
        return ((kword_t)(pins & MM_PIN_MASK) << MM_PIN_SHIFT) |
            ((kword_t)(type & MM_TYPE_MASK) << MM_TYPE_SHIFT) |
            ((kword_t)owner & MM_OWNER_MASK);
}

static void
mm_delete(int slot)
{
        while (++slot < mm_extent_count)
                mm_extents[slot - 1U] = mm_extents[slot];
        --mm_extent_count;
}

void mm_extent_coalesce(int slot);
int mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment,
    unsigned int type, unsigned int owner, unsigned int preference,
    kword_t *basep);

int
mm_extent_insert(int slot, const struct mm_extent *extent)
{
        int i;

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

kword_t
mm_total_free(void)
{
        int i;
        kword_t total;

        total = 0UL;
        for (i = 0U; i < mm_extent_count; ++i)
                if (MM_EXTENT_TYPE(&mm_extents[i]) == MM_TYPE_FREE)
                        total += MM_EXTENT_WORDS(&mm_extents[i]);
        return total;
}

static int
mm_use_free(int slot, long base, long words,
    unsigned int type, unsigned int owner, kword_t *basep)
{
        struct mm_extent used;
        struct mm_extent tail;
        struct mm_extent *freep;
        long free_base;
        long free_end;
        long before;
        long after;
        int needed;

        freep = &mm_extents[slot];
        free_base = MM_EXTENT_BASE(freep);
        free_end = free_base + MM_EXTENT_WORDS(freep);
        if (base < free_base || words > free_end - base)
                return MM_ERR_INVAL;
        before = base - free_base;
        after = free_end - (base + words);
        needed = (before != 0L ? 1 : 0) + (after != 0L ? 1 : 0);
        if (mm_extent_count + needed > MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;

        used.span = mm_span(base, words);
        used.meta = mm_meta(type, owner, 0U);
        if (before == 0UL) {
                *freep = used;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_extent_insert(slot + 1U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        } else {
                freep->span = mm_span(free_base, before);
                if (mm_extent_insert(slot + 1U, &used) != MM_OK)
                        return MM_ERR_DESCRIPTORS;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_extent_insert(slot + 2U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        }
        *basep = base;
        return MM_OK;
}

static int
mm_has_aligned_fit(kword_t words, kword_t alignment)
{
        struct mm_extent *e;
        kword_t base;
        kword_t end;
        int i;

        for (i = 0U; i < mm_extent_count; ++i) {
                e = &mm_extents[i];
                if (MM_EXTENT_TYPE(e) != MM_TYPE_FREE)
                        continue;
                base = (MM_EXTENT_BASE(e) + alignment - 1UL) &
                    ~(alignment - 1UL);
                end = MM_EXTENT_BASE(e) + MM_EXTENT_WORDS(e);
                if ((long)base <= (long)end &&
                    (long)words <= (long)(end - base))
                        return 1;
        }
        return 0;
}

int
mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_words;
        kword_t free_end;
        kword_t base;
        int i;
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
                        if ((long)free_words >= (long)words) {
                                if (preference == MM_ALLOC_LOW) {
                                        base = (free_base + alignment - 1UL) &
                                            ~(alignment - 1UL);
                                        if ((long)base >= (long)free_base &&
                                            (long)base <= (long)free_end &&
                                            (long)words <= (long)(free_end - base))
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                } else {
                                        base = (free_end - words) &
                                            ~(alignment - 1UL);
                                        if ((long)base >= (long)free_base)
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                }
                        }
                }
                if (step > 0)
                        ++i;
        }
        return (long)mm_total_free() >= (long)words ?
            MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
}

static int
mm_find_base(kword_t base)
{
        int i;

        for (i = 0U; i < mm_extent_count; ++i)
                if (MM_EXTENT_BASE(&mm_extents[i]) == base)
                        break;
        return i;
}

int
mm_is_pinned(kword_t base)
{
        int i;

        i = mm_find_base(base);
        return i < mm_extent_count && MM_EXTENT_PINS(&mm_extents[i]) != 0U;
}

int
mm_move_module(unsigned int owner)
{
        struct mm_extent *extent;
        kword_t old_base;
        kword_t new_base;
        kword_t words;
        int i;
        int rc;

        if (module_moves_enabled == 0U || owner == 0U ||
            owner > MODULE_RUNTIME_MAX ||
            MODULE_RUNTIME_INIT_WORDS(module_runtime_descs[owner]) == 0UL)
                return MM_ERR_BUSY;
        old_base = MODULE_RUNTIME_BASE(module_runtime_descs[owner]);
        if (old_base == 0UL)
                return MM_ERR_INVAL;
        i = mm_find_base(old_base);
        extent = i < mm_extent_count ? &mm_extents[i] : 0;
        if (extent == 0 || MM_EXTENT_TYPE(extent) != MM_TYPE_MODULE ||
            MM_EXTENT_OWNER(extent) != owner)
                return MM_ERR_INVAL;
        if (MM_EXTENT_PINS(extent) != 0U)
                return MM_ERR_BUSY;
        words = MM_EXTENT_WORDS(extent);

        rc = mm_alloc_aligned_noreclaim(words, 1UL, MM_TYPE_MODULE, owner,
            MM_ALLOC_LOW, &new_base);
        if (rc != MM_OK)
                return rc;
        if ((long)new_base >= (long)old_base) {
                (void)mm_free(new_base, MM_TYPE_MODULE, owner);
                return MM_ERR_FRAGMENTED;
        }
        if (module_runtime_move(owner, new_base, (unsigned int)words) != 0) {
                (void)mm_free(new_base, MM_TYPE_MODULE, owner);
                return MM_ERR_INVAL;
        }
        rc = mm_free(old_base, MM_TYPE_MODULE, owner);
        if (rc != MM_OK)
                return rc;
        return MM_OK;
}

int
mm_compact(kword_t words, kword_t alignment)
{
        struct mm_extent *extent;
        kword_t base;
        unsigned int owner;
        int i;

        if (words == 0UL || alignment == 0UL ||
            (alignment & (alignment - 1UL)) != 0UL)
                return MM_ERR_INVAL;
        /* Free memory is positive; a negative cast is a larger unsigned size. */
        if ((long)words < 0L ||
            (long)mm_total_free() < (long)words)
                return MM_ERR_NOMEM;
        if (mm_has_aligned_fit(words, alignment))
                return MM_OK;

        i = 0U;
        while (i < mm_extent_count) {
                extent = &mm_extents[i];
                if (MM_EXTENT_TYPE(extent) != MM_TYPE_MODULE ||
                    MM_EXTENT_PINS(extent) != 0U) {
                        ++i;
                        continue;
                }
                owner = MM_EXTENT_OWNER(extent);
                if (mm_move_module(owner) != MM_OK) {
                        ++i;
                        continue;
                }
                if (mm_has_aligned_fit(words, alignment))
                        return MM_OK;
                i = 0U;
        }

        i = 0U;
        while (i < mm_extent_count) {
                extent = &mm_extents[i];
                if (MM_EXTENT_TYPE(extent) != MM_TYPE_PROCESS ||
                    MM_EXTENT_PINS(extent) != 0U) {
                        ++i;
                        continue;
                }
                base = MM_EXTENT_BASE(extent);
                owner = MM_EXTENT_OWNER(extent);
                if (vm_extent_move(owner, base, MM_EXTENT_WORDS(extent)) != MM_OK) {
                        ++i;
                        continue;
                }
                if (mm_has_aligned_fit(words, alignment))
                        return MM_OK;
                i = 0U;
        }
        return MM_ERR_FRAGMENTED;
}

int
mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        int attempt;
        int rc;

        for (attempt = 0; attempt != 2; ++attempt) {
                rc = mm_alloc_aligned_noreclaim(words, alignment, type, owner,
                    preference, basep);
                if (rc == MM_ERR_FRAGMENTED) {
                        rc = mm_compact(words, alignment);
                        if (rc == MM_OK)
                                rc = mm_alloc_aligned_noreclaim(words, alignment,
                                    type, owner, preference, basep);
                }
                if (rc == MM_OK ||
                    (rc != MM_ERR_NOMEM && rc != MM_ERR_FRAGMENTED))
                        return rc;
                if (attempt == 0)
                        (void)proc_swap_reclaim(words, alignment,
                            type == MM_TYPE_PROCESS ? owner : PROC_NO_SLOT);
        }
        return rc;
}

void
mm_extent_coalesce(int slot)
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
        if (slot + 1 < mm_extent_count &&
            MM_EXTENT_TYPE(&mm_extents[slot + 1]) == MM_TYPE_FREE) {
                left = &mm_extents[slot];
                right = &mm_extents[slot + 1];
                base = MM_EXTENT_BASE(left);
                if (base + MM_EXTENT_WORDS(left) == MM_EXTENT_BASE(right)) {
                        words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                        left->span = mm_span(base, words);
                        mm_delete(slot + 1);
                }
        }
}

int
mm_free(kword_t base, unsigned int type, unsigned int owner)
{
        int i;
        struct mm_extent *extent;

        i = mm_find_base(base);
        if (i >= mm_extent_count)
                return MM_ERR_INVAL;
        extent = &mm_extents[i];
        if (MM_EXTENT_TYPE(extent) != type || MM_EXTENT_OWNER(extent) != owner)
                return MM_ERR_INVAL;
        if (MM_EXTENT_PINS(extent) != 0U)
                return MM_ERR_BUSY;
        extent->meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
        mm_extent_coalesce(i);
        return MM_OK;
}

int
mm_pin(kword_t base)
{
        int i;
        unsigned int pins;
        struct mm_extent *extent;

        i = mm_find_base(base);
        if (i >= mm_extent_count)
                return MM_ERR_INVAL;
        extent = &mm_extents[i];
        if (MM_EXTENT_TYPE(extent) == MM_TYPE_FREE)
                return MM_ERR_INVAL;
        pins = MM_EXTENT_PINS(extent);
        if (pins == MM_PIN_MASK)
                return MM_ERR_BUSY;
        extent->meta += (kword_t)1UL << MM_PIN_SHIFT;
        return MM_OK;
}

int
mm_unpin(kword_t base)
{
        int i;
        struct mm_extent *extent;

        i = mm_find_base(base);
        if (i >= mm_extent_count)
                return MM_ERR_INVAL;
        extent = &mm_extents[i];
        if (MM_EXTENT_PINS(extent) == 0U)
                return MM_ERR_INVAL;
        extent->meta -= (kword_t)1UL << MM_PIN_SHIFT;
        return MM_OK;
}
