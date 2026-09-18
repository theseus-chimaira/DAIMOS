#include "mm.h"
#include "vm.h"
#include "module_runtime.h"
#include "proc_swap.h"
#include "fs_mres.h"

struct mm_extent mm_extents[MM_MAX_EXTENTS];
kword_t mm_arenas[MM_MAX_ARENAS];
kword_t mm_core_words;
int mm_extent_count;
int mm_arena_count;

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
                mm_extents[slot - 1] = mm_extents[slot];
        --mm_extent_count;
}

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
                mm_extents[i] = mm_extents[i - 1];
                --i;
        }
        mm_extents[slot] = *extent;
        return MM_OK;
}

static int
mm_find_fit(kword_t words, kword_t alignment, unsigned int preference,
    kword_t *basep)
{
        int arena_base;
        int arena_end;
        int cursor;
        int extent_base;
        int extent_end;
        int candidate;
        int high_candidate;
        int request;
        int align;
        int have_high;
        int arena;
        int i;

        request = (int)words;
        align = (int)alignment;
        have_high = 0;
        high_candidate = 0;
        i = 0;
        for (arena = 0; arena < mm_arena_count; ++arena) {
                arena_base = (int)MM_ARENA_BASE(mm_arenas[arena]);
                arena_end = arena_base +
                    (int)MM_ARENA_WORDS(mm_arenas[arena]);
                cursor = arena_base;
                while (i < mm_extent_count &&
                    (int)MM_EXTENT_BASE(&mm_extents[i]) < arena_base)
                        ++i;
                while (i < mm_extent_count) {
                        extent_base = (int)MM_EXTENT_BASE(&mm_extents[i]);
                        if (extent_base >= arena_end)
                                break;
                        if (extent_base > cursor) {
                                if (preference == MM_ALLOC_LOW) {
                                        candidate = (cursor + align - 1) &
                                            ~(align - 1);
                                        if (candidate >= cursor &&
                                            request <= extent_base - candidate) {
                                                *basep = (kword_t)candidate;
                                                return 1;
                                        }
                                } else if (extent_base - cursor >= request) {
                                        candidate = (extent_base - request) &
                                            ~(align - 1);
                                        if (candidate >= cursor) {
                                                high_candidate = candidate;
                                                have_high = 1;
                                        }
                                }
                        }
                        extent_end = extent_base +
                            (int)MM_EXTENT_WORDS(&mm_extents[i]);
                        if (extent_end > cursor)
                                cursor = extent_end;
                        ++i;
                }
                if (arena_end > cursor) {
                        if (preference == MM_ALLOC_LOW) {
                                candidate = (cursor + align - 1) &
                                    ~(align - 1);
                                if (candidate >= cursor &&
                                    request <= arena_end - candidate) {
                                        *basep = (kword_t)candidate;
                                        return 1;
                                }
                        } else if (arena_end - cursor >= request) {
                                candidate = (arena_end - request) &
                                    ~(align - 1);
                                if (candidate >= cursor) {
                                        high_candidate = candidate;
                                        have_high = 1;
                                }
                        }
                }
        }
        if (have_high) {
                *basep = (kword_t)high_candidate;
                return 1;
        }
        return 0;
}

kword_t
mm_total_free(void)
{
        kword_t total;
        int i;

        total = 0UL;
        for (i = 0; i < mm_arena_count; ++i)
                total += MM_ARENA_WORDS(mm_arenas[i]);
        for (i = 0; i < mm_extent_count; ++i)
                total -= MM_EXTENT_WORDS(&mm_extents[i]);
        return total;
}

static int
mm_has_aligned_fit(kword_t words, kword_t alignment)
{
        kword_t base;

        return mm_find_fit(words, alignment, MM_ALLOC_LOW, &base);
}

int
mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        struct mm_extent used;
        kword_t base;
        int slot;

        if (basep == 0 || words == 0UL || words > MM_HALF_MASK ||
            alignment == 0UL || alignment > MM_HALF_MASK ||
            (alignment & (alignment - 1UL)) != 0UL ||
            type == MM_TYPE_FREE || type > MM_TYPE_KERNEL_DYNAMIC ||
            preference > MM_ALLOC_HIGH)
                return MM_ERR_INVAL;
        if (!mm_find_fit(words, alignment, preference, &base))
                return (long)mm_total_free() >= (long)words ?
                    MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
        slot = 0;
        while (slot < mm_extent_count &&
            MM_EXTENT_BASE(&mm_extents[slot]) < base)
                ++slot;
        used.span = mm_span(base, words);
        used.meta = mm_meta(type, owner, 0U);
        if (mm_extent_insert(slot, &used) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        *basep = base;
        return MM_OK;
}

static int
mm_find_base(kword_t base)
{
        int i;

        for (i = 0; i < mm_extent_count; ++i)
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
        if ((long)words < 0L ||
            (long)mm_total_free() < (long)words)
                return MM_ERR_NOMEM;
        if (mm_has_aligned_fit(words, alignment))
                return MM_OK;

        i = 0;
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
                i = 0;
        }

        i = 0;
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
                i = 0;
        }
        return MM_ERR_FRAGMENTED;
}

int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        return mm_alloc_aligned(words, 1UL, type, owner, preference, basep);
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
        mm_delete(i);
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
