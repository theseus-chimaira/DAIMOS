/**
 * @file proc_pure_restore_v1384.c
 * @brief In-place PURE prefix restoration without a duplicate suffix.
 *
 * Inactive MM owner/type + backing + process VM size determine all geometry.
 * Prefix reserve/release use a PACKED_PROCESS full-span descriptor while
 * VFS I/O is performed. Thus the original writable suffix remains owned
 * and un-runnable, and every reader/allocator sees an occupied full span.
 * The final PROCESS descriptor plus VM/mode publication is PI-atomic.
 *
 * Not production linked. Caller must hold PROCESS TRANSITION and executable
 * immutability/unmount exclusion for the entire operation.
 */
#include "proc_pure_restore_v1384.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm_pdp6.h"
#include "exec.h"
#include "mm_internal.h"
#include "mm_pure_expand_v1384.h"
#include "fs_mres.h"

#if !defined(DAIMOS_SWAPLESS_PREFIX_V1384) || \
    DAIMOS_SWAPLESS_PREFIX_V1384 != 1
#error proc_pure_restore_v1384_requires_swapless_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PRP_PTR(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PRP_PTR(x) ((kword_t *)(unsigned long)(x))
#endif

extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern int proc_storage_publish_v30(unsigned int, unsigned int,
    unsigned int);
extern void proc_swap_storage_invariant_v30(void);
extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);

static int
prp_owned_suffix(unsigned int slot, kword_t words, kword_t *base)
{
        unsigned int count = 0U;
        int i;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                if (MM_EXTENT_OWNER(e) != slot ||
                    (type != MM_TYPE_PROCESS &&
                     type != MM_TYPE_PACKED_PROCESS))
                        continue;
                ++count;
                if (count != 1U || type != MM_TYPE_PACKED_PROCESS ||
                    MM_EXTENT_WORDS(e) != words ||
                    MM_EXTENT_PINNED(e) || !MM_EXTENT_BASE(e))
                        return MM_ERR_INVAL;
                *base = MM_EXTENT_BASE(e);
        }
        return count == 1U ? MM_OK : MM_ERR_INVAL;
}

int
proc_pure_restore_v1384(unsigned int slot,
    proc_pure_prefix_loader_v1384 loader)
{
        struct proc *p;
        kword_t *u, backing, words, pure, prefix, raw, suffix, base;
        kword_t pi_state;
        int rc;

        if (!loader || !slot || !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p) ||
            (PROC_STATE(p) != PROC_SLEEP && PROC_STATE(p) != PROC_STOP))
                return MM_ERR_INVAL;
        u = PRP_PTR(PROC_UAREA_BASE(p));
        if (u[PROC_FDCTL_OFFSET] &
            (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT))
                return MM_ERR_BUSY;
        backing = proc_swap_records[slot].state;
        words = VM_SPACE_WORDS(p);
        pure = (backing >> PROC_SWAP_TEXT_SHIFT) & PROC_SWAP_TEXT_MASK;
        if (!pure || pure > MM_HALF_MASK - EXEC_USER_ORIGIN ||
            !words || (words & (VM_PDP6_ALIGN_WORDS - 1UL)))
                return MM_ERR_INVAL;
        prefix = (pure + EXEC_USER_ORIGIN) & ~0177UL;
        if (!prefix || prefix >= words ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        raw = words - prefix;
        rc = prp_owned_suffix(slot, raw, &suffix);
        if (rc != MM_OK)
                return rc;
        if (suffix < prefix)
                return MM_ERR_INVAL;
        base = suffix - prefix;
        if (!base || (base & (VM_PDP6_ALIGN_WORDS - 1UL)))
                return MM_ERR_INVAL;
        rc = mm_pin(suffix);
        if (rc != MM_OK)
                return rc;

        /* Claim the prefix while PI is off, but keep the MM owner type
         * PACKED_PROCESS and VM base zero while potentially slow VFS
         * calls run with ordinary PI behavior.
         */
        pi_state = mach_pi_disable();
        rc = mm_pure_claim_prefix_v1384(suffix, slot, prefix, words);
        mach_pi_restore(pi_state);
        if (rc != MM_OK) {
                if (mm_unpin(suffix) != MM_OK)
                        return MM_ERR_BUSY; /* leave TRANSITION held */
                return rc == MM_ERR_FRAGMENTED ?
                    PROC_PURE_PREFIX_FALLBACK_V1384 : rc;
        }
        fs_zero_words(PRP_PTR(base), (unsigned int)prefix);
        rc = loader(backing, base, PROC_UAREA_BASE(p), prefix);
        if (rc != 0) {
                int unsupported = (rc == -2);
                /* No mutable suffix word was touched. Revoke the prefix
                 * reservation, restore its free-gap ownership, and only
                 * then release the pin on the original suffix base.
                 */
                pi_state = mach_pi_disable();
                rc = mm_pure_release_prefix_v1384(base, slot,
                    prefix, words);
                mach_pi_restore(pi_state);
                if (rc != MM_OK)
                        return MM_ERR_BUSY; /* claimed, pinned, excluded */
                if (mm_unpin(suffix) != MM_OK)
                        return MM_ERR_BUSY;
                return unsupported ? PROC_PURE_PREFIX_FALLBACK_V1384 :
                    MM_ERR_INVAL;
        }

        /* Prefix is completely initialized and immutable, writable RAM
         * stayed bit-identical. The MM retype cannot allocate or fail
         * after validation; publish the complete APR and mode atomically.
         */
        pi_state = mach_pi_disable();
        rc = mm_pure_commit_prefix_v1384(base, slot, prefix, words);
        if (rc == MM_OK) {
                VM_PDP6_SET_BASE(p, base);
                u[PROC_SWAP_BACKING_OFFSET] = 0UL;
                if (proc_storage_publish_v30(slot, PROC_STORAGE_PACKED,
                    PROC_STORAGE_RESIDENT) != 0)
                        proc_swap_storage_invariant_v30();
        }
        mach_pi_restore(pi_state);
        if (rc != MM_OK) {
                /* The descriptor remains full-size PACKED on failure. */
                pi_state = mach_pi_disable();
                if (mm_pure_release_prefix_v1384(base, slot,
                    prefix, words) != MM_OK) {
                        mach_pi_restore(pi_state);
                        return MM_ERR_BUSY;
                }
                mach_pi_restore(pi_state);
                if (mm_unpin(suffix) != MM_OK)
                        return MM_ERR_BUSY;
                return rc;
        }
        if (mm_unpin(base) != MM_OK)
                return MM_ERR_BUSY; /* committed, leave TRANSITION held */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;
}
