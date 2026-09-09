#include "mm.h"
#include "mm_internal.h"

void
mm_boot_init(kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0U;
        mm_compaction_count = 0UL;
        mm_words_moved = 0UL;
        mm_allocation_failures = 0UL;
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
        extent.span = ((words & MM_HALF_MASK) << 18U) |
            (base & MM_HALF_MASK);
        extent.meta = (kword_t)MM_TYPE_FREE << MM_TYPE_SHIFT;
        if (mm_extent_insert(slot, &extent) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        mm_extent_coalesce(slot);
        return MM_OK;
}
