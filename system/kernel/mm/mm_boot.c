#include "mm.h"
#include "mm_internal.h"

void
mm_boot_init(kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0U;
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
        struct mm_extent *left;
        struct mm_extent *right;
        unsigned int slot;
        kword_t end;
        kword_t merged_words;

        if (words == 0UL || base > MM_HALF_MASK || words > MM_HALF_MASK ||
            base >= mm_core_words || words > mm_core_words - base)
                return MM_ERR_INVAL;
        end = base + words;
        slot = 0U;
        while (slot < mm_extent_count &&
            MM_EXTENT_BASE(&mm_extents[slot]) < base)
                ++slot;
        left = slot != 0U ? &mm_extents[slot - 1U] : 0;
        right = slot < mm_extent_count ? &mm_extents[slot] : 0;
        if (left != 0 && MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) > base)
                return MM_ERR_INVAL;
        if (right != 0 && end > MM_EXTENT_BASE(right))
                return MM_ERR_INVAL;

        /* Do not require a temporary descriptor when this range can be
         * returned directly into an adjacent free extent.  This matters
         * during late KINIT, where all descriptor slots can legitimately
         * be occupied just before reclaiming KINIT itself. */
        if (left != 0 && MM_EXTENT_TYPE(left) == MM_TYPE_FREE &&
            MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) == base) {
                merged_words = MM_EXTENT_WORDS(left) + words;
                left->span = ((merged_words & MM_HALF_MASK) << 18U) |
                    (MM_EXTENT_BASE(left) & MM_HALF_MASK);
                mm_extent_coalesce(slot - 1U);
                return MM_OK;
        }
        if (right != 0 && MM_EXTENT_TYPE(right) == MM_TYPE_FREE &&
            end == MM_EXTENT_BASE(right)) {
                merged_words = words + MM_EXTENT_WORDS(right);
                right->span = ((merged_words & MM_HALF_MASK) << 18U) |
                    (base & MM_HALF_MASK);
                return MM_OK;
        }

        extent.span = ((words & MM_HALF_MASK) << 18U) |
            (base & MM_HALF_MASK);
        extent.meta = (kword_t)MM_TYPE_FREE << MM_TYPE_SHIFT;
        if (mm_extent_insert(slot, &extent) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        mm_extent_coalesce(slot);
        return MM_OK;
}
