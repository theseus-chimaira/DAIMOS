#include "mm.h"
#include "vm.h"
#include "proc_swap.h"
#include "fs_mres.h"

/**
 * @file mm.c
 * @brief Permanent contiguous-core allocator and PDP-6 process compactor.
 *
 * Free memory is implicit: mm_arenas[] describes the small set of managed
 * physical ranges and mm_extents[] contains allocated extents only, sorted by
 * physical base.  This avoids consuming a descriptor for every fragmented free
 * hole.  Each extent uses two words: size,,base and pins/type,,owner.
 *
 * Ordinary allocation follows the cheapest pressure sequence: direct fit,
 * reclaim clean D6FS cache, compact movable process images if fragmented,
 * reclaim/swap an eligible process, then retry.  Runtime module relocation is
 * not part of this allocator path today; mm_compact() moves process extents
 * only through vm_extent_move().
 *
 * The allocator is permanent KCORE.  Keep helper state bounded and avoid
 * policy-specific free-list structures: descriptor RAM and generated code both
 * count directly against the permanent-memory limit.
 */

struct mm_extent mm_extents[MM_MAX_EXTENTS];
kword_t mm_arenas[MM_MAX_ARENAS];
kword_t mm_core_words;
int mm_extent_count;
int mm_arena_count;

/* Extent indexes are bounded by MM_MAX_EXTENTS, and validated physical
 * bases/lengths fit the positive PDP-10 core-address domain.  Keep bounded
 * arithmetic signed so GCC need not synthesize unsigned sign-bit compares. */

/* KCC does not inline the former packing helpers.  Keep these as expressions
 * so constructing/rebasing descriptors does not emit permanent helper bodies. */
#define MM_SPAN(base, words) \
        ((((words) & MM_HALF_MASK) << 18U) | ((base) & MM_HALF_MASK))
#define MM_META(type, owner, pins) \
        (((kword_t)((pins) & MM_PIN_MASK) << MM_PIN_SHIFT) | \
        ((kword_t)((type) & MM_TYPE_MASK) << MM_TYPE_SHIFT) | \
        ((kword_t)(owner) & MM_OWNER_MASK))

/** Remove one allocated descriptor and close the two-word table hole. */
static void
mm_delete(int slot)
{
        if (++slot < mm_extent_count)
                fs_move_words((const kword_t *)&mm_extents[slot],
                    (kword_t *)&mm_extents[slot - 1],
                    (unsigned int)(mm_extent_count - slot) * 2U);
        --mm_extent_count;
}

/**
 * @brief Insert an extent into the physical-base-sorted descriptor table.
 * @param slot Insertion index in the range 0..mm_extent_count.
 * @param extent Descriptor to copy into the table.
 * @return MM_OK or MM_ERR_DESCRIPTORS.
 *
 * fs_move_words() is overlap-safe, so one bulk move replaces the former
 * two-word structure-copy loop when opening the descriptor-table hole.
 */
int
mm_extent_insert(int slot, const struct mm_extent *extent)
{
        if (mm_extent_count >= MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;
        if (slot < mm_extent_count)
                fs_move_words((const kword_t *)&mm_extents[slot],
                    (kword_t *)&mm_extents[slot + 1],
                    (unsigned int)(mm_extent_count - slot) * 2U);
        ++mm_extent_count;
        mm_extents[slot] = *extent;
        return MM_OK;
}

/**
 * @brief Find an aligned free gap according to low/high placement policy.
 * @param words Requested contiguous words.
 * @param alignment Power-of-two word alignment.
 * @param preference MM_ALLOC_LOW or MM_ALLOC_HIGH.
 * @param basep Receives the selected physical base.
 * @return 1 when a fit exists, otherwise 0.
 *
 * Low placement returns the first suitable gap. High placement scans all
 * arenas and retains the highest suitable candidate. The caller validates all
 * values as positive 18-bit quantities before entry.
 */
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
        int arena;
        int i;

        request = (int)words;
        align = (int)alignment;
        /* Physical bases are nonnegative 18-bit values, so -1 is a compact
         * unambiguous sentinel for "no high-placement candidate yet". */
        high_candidate = -1;
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
                                        if (request <= extent_base - candidate) {
                                                *basep = (kword_t)candidate;
                                                return 1;
                                        }
                                } else if (extent_base - cursor >= request) {
                                        candidate = (extent_base - request) &
                                            ~(align - 1);
                                        if (candidate >= cursor) {
                                                high_candidate = candidate;
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
                                if (request <= arena_end - candidate) {
                                        *basep = (kword_t)candidate;
                                        return 1;
                                }
                        } else if (arena_end - cursor >= request) {
                                candidate = (arena_end - request) &
                                    ~(align - 1);
                                if (candidate >= cursor) {
                                        high_candidate = candidate;
                                }
                        }
                }
        }
        if (high_candidate >= 0) {
                *basep = (kword_t)high_candidate;
                return 1;
        }
        return 0;
}

/**
 * @brief Return total currently free managed-core words.
 */
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

/**
 * @brief Allocate without invoking reclaim, compaction, or swapping.
 * @param words Requested contiguous words, 1..0777777.
 * @param alignment Power-of-two physical alignment, 1..0777777.
 * @param type Nonzero MM_TYPE_* allocation class.
 * @param owner 18-bit owner identifier stored verbatim in the descriptor.
 * @param preference MM_ALLOC_LOW or MM_ALLOC_HIGH.
 * @param basep Receives the physical base on success.
 * @return MM_OK or one MM_ERR_* status.
 *
 * This entry is intentionally policy-free and is used by reclaimable caches
 * and boot paths that must not recurse into the ordinary pressure machinery.
 */
int
mm_alloc_aligned_noreclaim(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        struct mm_extent used;
        kword_t base;
        int slot;

        if (basep == 0 || words == 0UL || (words & ~MM_HALF_MASK) != 0UL ||
            alignment == 0UL || (alignment & ~MM_HALF_MASK) != 0UL ||
            (alignment & (alignment - 1UL)) != 0UL ||
            type == MM_TYPE_FREE || (type & ~03U) != 0U ||
            (preference & ~01U) != 0U)
                return MM_ERR_INVAL;
        if (!mm_find_fit(words, alignment, preference, &base))
                return (long)mm_total_free() >= (long)words ?
                    MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
        slot = 0;
        while (slot < mm_extent_count &&
            MM_EXTENT_BASE(&mm_extents[slot]) < base)
                ++slot;
        used.span = MM_SPAN(base, words);
        used.meta = MM_META(type, owner, 0U);
        if (mm_extent_insert(slot, &used) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        *basep = base;
        return MM_OK;
}

/** Return the descriptor index whose physical base matches, or count if absent. */
static int
mm_find_base(kword_t base)
{
        int i;

        for (i = 0; i < mm_extent_count; ++i)
                if (MM_EXTENT_BASE(&mm_extents[i]) == base)
                        break;
        return i;
}

/** @brief Return nonzero when the extent at base has any physical pin. */
int
mm_is_pinned(kword_t base)
{
        int i;

        i = mm_find_base(base);
        return i < mm_extent_count && MM_EXTENT_PINNED(&mm_extents[i]);
}

extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t state);

/**
 * @brief Slide one movable process extent upward without a second descriptor.
 * @param slot Descriptor index of the process extent to move.
 * @param alignment Required destination alignment.
 * @return MM_OK, MM_ERR_FRAGMENTED, or the VM backend error.
 *
 * While PI is disabled, remove only the source descriptor from the fit search:
 * its old physical range then participates in the candidate free span. This
 * permits an overlap-safe slide into a gap smaller than the object itself.
 * Restore the descriptor before invoking vm_extent_move() so backend pin/state
 * validation still observes a normal MM allocation. Descriptor publication is
 * completed before PI is restored.
 */
static int
mm_move_extent(int slot, kword_t alignment)
{
        struct mm_extent moved;
        kword_t old_base;
        kword_t new_base;
        kword_t words;
        kword_t pi_state;
        unsigned int owner;
        int insert;
        int rc;

        moved = mm_extents[slot];
        old_base = MM_EXTENT_BASE(&moved);
        words = MM_EXTENT_WORDS(&moved);

        pi_state = mach_pi_disable();
        mm_delete(slot);
        rc = mm_find_fit(words, alignment, MM_ALLOC_HIGH, &new_base) ?
            MM_OK : MM_ERR_FRAGMENTED;
        (void)mm_extent_insert(slot, &moved);
        if (rc != MM_OK || new_base <= old_base) {
                mach_pi_restore(pi_state);
                return MM_ERR_FRAGMENTED;
        }

        owner = MM_EXTENT_OWNER(&moved);
        rc = vm_extent_move(owner, old_base, words, new_base);
        if (rc == MM_OK) {
                moved.span = MM_SPAN(new_base, words);
                mm_delete(slot);
                insert = 0;
                while (insert < mm_extent_count &&
                    MM_EXTENT_BASE(&mm_extents[insert]) < new_base)
                        ++insert;
                (void)mm_extent_insert(insert, &moved);
        }
        mach_pi_restore(pi_state);
        return rc;
}

/**
 * @brief Compact movable process images until an aligned request can fit.
 * @param words Requested contiguous free run.
 * @param alignment Required power-of-two alignment.
 * @return MM_OK, MM_ERR_NOMEM, MM_ERR_FRAGMENTED, or MM_ERR_INVAL.
 *
 * V0.9 compaction moves process extents only. Runtime module relocation exists
 * separately and is not wired into this allocator path.
 */
int
mm_compact(kword_t words, kword_t alignment)
{
        struct mm_extent *extent;
        kword_t fit_base;
        int i;

        if (words == 0UL || (words & ~MM_HALF_MASK) != 0UL ||
            alignment == 0UL || (alignment & ~MM_HALF_MASK) != 0UL ||
            (alignment & (alignment - 1UL)) != 0UL)
                return MM_ERR_INVAL;
        if ((long)mm_total_free() < (long)words)
                return MM_ERR_NOMEM;
        if (mm_find_fit(words, alignment, MM_ALLOC_LOW, &fit_base))
                return MM_OK;

        i = 0;
        while (i < mm_extent_count) {
                extent = &mm_extents[i];
                if (MM_EXTENT_TYPE(extent) != MM_TYPE_PROCESS ||
                    MM_EXTENT_PINNED(extent) ||
                    mm_move_extent(i, VM_EXTENT_ALIGN_WORDS) != MM_OK) {
                        ++i;
                        continue;
                }
                if (mm_find_fit(words, alignment, MM_ALLOC_LOW, &fit_base))
                        return MM_OK;
                i = 0;
        }
        return MM_ERR_FRAGMENTED;
}

/** @brief Allocate a contiguous extent with one-word alignment. */
int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        return mm_alloc_aligned(words, 1UL, type, owner, preference, basep);
}

/**
 * @brief Allocate with reclaim/compaction/swap pressure handling.
 * @return MM_OK or the final MM_ERR_* status.
 *
 * The three stages are: direct attempt; cache reclaim followed by retry and
 * compaction if fragmented; MEMFS eviction; then process-swap reclaim.
 * Only MM_ERR_NOMEM, MM_ERR_FRAGMENTED, and MM_ERR_DESCRIPTORS advance to the
 * next pressure stage. All validation/busy errors return immediately.
 */
int
mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        int rc;
        int stage;

        stage = 0;
        for (;;) {
                rc = mm_alloc_aligned_noreclaim(words, alignment, type, owner,
                    preference, basep);
                if (stage != 0 && rc == MM_ERR_FRAGMENTED) {
                        rc = mm_compact(words, alignment);
                        if (rc == MM_OK)
                            rc = mm_alloc_aligned_noreclaim(words,
                                    alignment, type, owner, preference, basep);
                }
                if (rc == MM_OK || rc < MM_ERR_DESCRIPTORS || stage == 3)
                        return rc;
                if (stage == 0)
                        (void)fs_d6fs_cache_reclaim(words);
                else if (stage == 1)
                        (void)fs_memfs_reclaim(words);
                else
                        (void)proc_swap_reclaim(words, alignment,
                            type == MM_TYPE_PROCESS ? owner : PROC_NO_SLOT);
                ++stage;
        }
}

/** @brief Free one unpinned extent after exact type/owner validation. */
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
        if (MM_EXTENT_PINNED(extent))
                return MM_ERR_BUSY;
        mm_delete(i);
        return MM_OK;
}

/** Adjust one extent's packed physical pin count by exactly +1 or -1. */
static int
mm_pin_adjust(kword_t base, int delta)
{
        int i;
        struct mm_extent *extent;

        i = mm_find_base(base);
        if (i >= mm_extent_count)
                return MM_ERR_INVAL;
        extent = &mm_extents[i];
        if (delta > 0) {
                if ((extent->meta & MM_PIN_FIELD_MASK) == MM_PIN_FIELD_MASK)
                        return MM_ERR_BUSY;
                extent->meta += (kword_t)1UL << MM_PIN_SHIFT;
        } else {
                if (!MM_EXTENT_PINNED(extent))
                        return MM_ERR_INVAL;
                extent->meta -= (kword_t)1UL << MM_PIN_SHIFT;
        }
        return MM_OK;
}

/** @brief Add one physical pin to the extent beginning at base. */
int
mm_pin(kword_t base)
{
        return mm_pin_adjust(base, 1);
}

/** @brief Remove one physical pin from the extent beginning at base. */
int
mm_unpin(kword_t base)
{
        return mm_pin_adjust(base, -1);
}
