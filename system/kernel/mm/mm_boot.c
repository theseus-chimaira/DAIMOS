#include "mm.h"
#include "mm_internal.h"

static kword_t
mm_arena_span(kword_t base, kword_t words)
{
        return ((words & MM_HALF_MASK) << 18U) | (base & MM_HALF_MASK);
}

static void
mm_arena_delete(int slot)
{
        while (++slot < mm_arena_count)
                mm_arenas[slot - 1] = mm_arenas[slot];
        --mm_arena_count;
}

void
mm_boot_init(kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0;
        mm_arena_count = 0;
}

/* Permanently remove an allocated boot range from managed core.  Packed boot
 * MRES is allocated at an arena edge, so committing it only trims that arena;
 * the range becomes unmanaged rather than free. */
int
mm_boot_reserve(kword_t base, unsigned int type, unsigned int owner)
{
        struct mm_extent *extent;
        kword_t words;
        kword_t arena_base;
        kword_t arena_end;
        int i;
        int arena;
        int found;

        for (i = 0; i < mm_extent_count; ++i)
                if (MM_EXTENT_BASE(&mm_extents[i]) == base)
                        break;
        if (i >= mm_extent_count)
                return MM_ERR_INVAL;
        extent = &mm_extents[i];
        if (MM_EXTENT_TYPE(extent) != type || MM_EXTENT_OWNER(extent) != owner ||
            MM_EXTENT_PINS(extent) != 0U)
                return MM_ERR_INVAL;
        words = MM_EXTENT_WORDS(extent);
        found = 0;
        for (arena = 0; arena < mm_arena_count; ++arena) {
                arena_base = MM_ARENA_BASE(mm_arenas[arena]);
                arena_end = arena_base + MM_ARENA_WORDS(mm_arenas[arena]);
                if (base == arena_base) {
                        if (words == MM_ARENA_WORDS(mm_arenas[arena]))
                                mm_arena_delete(arena);
                        else
                                mm_arenas[arena] = mm_arena_span(base + words,
                                    arena_end - (base + words));
                        found = 1;
                        break;
                }
                if (base + words == arena_end) {
                        mm_arenas[arena] = mm_arena_span(arena_base,
                            base - arena_base);
                        found = 1;
                        break;
                }
        }
        if (!found)
                return MM_ERR_INVAL;
        while (++i < mm_extent_count)
                mm_extents[i - 1] = mm_extents[i];
        --mm_extent_count;
        return MM_OK;
}

int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        return mm_alloc_aligned(words, 1UL, type, owner, preference, basep);
}

kword_t
mm_largest_free(void)
{
        kword_t largest;
        kword_t arena_base;
        kword_t arena_end;
        kword_t cursor;
        kword_t extent_base;
        kword_t extent_end;
        int arena;
        int i;

        largest = 0UL;
        i = 0;
        for (arena = 0; arena < mm_arena_count; ++arena) {
                arena_base = MM_ARENA_BASE(mm_arenas[arena]);
                arena_end = arena_base + MM_ARENA_WORDS(mm_arenas[arena]);
                cursor = arena_base;
                while (i < mm_extent_count &&
                    MM_EXTENT_BASE(&mm_extents[i]) < arena_base)
                        ++i;
                while (i < mm_extent_count) {
                        extent_base = MM_EXTENT_BASE(&mm_extents[i]);
                        if (extent_base >= arena_end)
                                break;
                        if (extent_base > cursor && extent_base - cursor > largest)
                                largest = extent_base - cursor;
                        extent_end = extent_base + MM_EXTENT_WORDS(&mm_extents[i]);
                        if (extent_end > cursor)
                                cursor = extent_end;
                        ++i;
                }
                if (arena_end > cursor && arena_end - cursor > largest)
                        largest = arena_end - cursor;
        }
        return largest;
}

/* Add a physically free boot-time range to the managed arena set.  Adjacent
 * arenas are merged immediately; allocations remain separate descriptors and
 * therefore need no free-space descriptors or coalescing. */
int
mm_add_free(kword_t base, kword_t words)
{
        kword_t end;
        kword_t left_base;
        kword_t left_end;
        kword_t right_base;
        kword_t right_end;
        int slot;
        int i;
        int left_adj;
        int right_adj;

        if (words == 0UL || base > MM_HALF_MASK || words > MM_HALF_MASK ||
            base >= mm_core_words || words > mm_core_words - base)
                return MM_ERR_INVAL;
        end = base + words;
        for (i = 0; i < mm_extent_count; ++i) {
                kword_t extent_base;
                kword_t extent_end;

                extent_base = MM_EXTENT_BASE(&mm_extents[i]);
                extent_end = extent_base + MM_EXTENT_WORDS(&mm_extents[i]);
                if (extent_base < end && base < extent_end)
                        return MM_ERR_INVAL;
        }
        slot = 0;
        while (slot < mm_arena_count && MM_ARENA_BASE(mm_arenas[slot]) < base)
                ++slot;
        left_adj = 0;
        right_adj = 0;
        left_base = 0UL;
        left_end = 0UL;
        right_base = 0UL;
        right_end = 0UL;
        if (slot != 0) {
                left_base = MM_ARENA_BASE(mm_arenas[slot - 1]);
                left_end = left_base + MM_ARENA_WORDS(mm_arenas[slot - 1]);
                if (left_end > base)
                        return MM_ERR_INVAL;
                left_adj = left_end == base;
        }
        if (slot < mm_arena_count) {
                right_base = MM_ARENA_BASE(mm_arenas[slot]);
                right_end = right_base + MM_ARENA_WORDS(mm_arenas[slot]);
                if (end > right_base)
                        return MM_ERR_INVAL;
                right_adj = end == right_base;
        }
        if (left_adj && right_adj) {
                mm_arenas[slot - 1] = mm_arena_span(left_base,
                    right_end - left_base);
                mm_arena_delete(slot);
                return MM_OK;
        }
        if (left_adj) {
                mm_arenas[slot - 1] = mm_arena_span(left_base,
                    end - left_base);
                return MM_OK;
        }
        if (right_adj) {
                mm_arenas[slot] = mm_arena_span(base, right_end - base);
                return MM_OK;
        }
        if (mm_arena_count >= MM_MAX_ARENAS)
                return MM_ERR_DESCRIPTORS;
        for (i = mm_arena_count; i > slot; --i)
                mm_arenas[i] = mm_arenas[i - 1];
        mm_arenas[slot] = mm_arena_span(base, words);
        ++mm_arena_count;
        return MM_OK;
}
