/**
 * @file proc_pure_restore_v1384.c
 * @brief In-place PURE prefix restoration without a duplicate suffix.
 *
 * Inactive MM owner/type + backing + process VM size determine all geometry.
 * Prefix reserve/release use a PACKED_PROCESS full-span descriptor while
 * VFS I/O is performed. Thus the original writable suffix remains owned
 * and un-runnable, and every reader/allocator sees an occupied full span.
 * The final unpinned PROCESS descriptor plus VM/mode publication is
 * PI-atomic. No fallible MM operation follows resident publication.
 *
 * v1611: linked only in experimental opt-in PURE MRES. The caller must
 * hold PROCESS TRANSITION and executable unmount exclusion throughout.
 */
#include "proc_pure_restore_v1384.h"
#include "proc_pure_ram_v1381.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm_pdp6.h"
#include "exec.h"
#include "mm_internal.h"
#include "mm_pure_expand_v1384.h"
#include "fs_mres.h"
#include "proc_pure_admit_v1608.h"
#include "proc_pure_lifecycle_v1604.h"
#include "vm_pdp6_pure_range_v1604.h"

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

int
proc_pure_restore_v1384(unsigned int slot,
    proc_pure_prefix_loader_v1384 loader)
{
        struct proc *p;
        kword_t *u, backing, words, prefix, raw, suffix, base;
        kword_t pi_state;
        int rc;

        if (!loader)
                return MM_ERR_INVAL;
        rc = pure_life_prepare_v1604(slot, PURE_LIFE_RESTORE_V1610,
            &p, &backing, &words);
        if (rc != MM_OK)
                return rc;
        u = PRP_PTR(PROC_UAREA_BASE(p));
        /* v1609: fast in-place prefix restore must observe the SAME
         * executable source admission as the bounded full restore.
         * Check before pinning or claiming a physical MM gap. A missing
         * mount cannot be replaced with stale or unrelated executable
         * text; the inactive writable suffix stays intact for retry.
         */
        if (pure_admit_source_v1608(p, backing) != MM_OK)
                return MM_ERR_INVAL;
        if ((words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            pdp6_pure_skip_v1604(backing, words, &prefix) != MM_OK ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            VM_PDP6_BASE(p) ||
            (u[PROC_FINALIZE_OFFSET] &
             (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507)) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        raw = words - prefix;
        pi_state = mach_pi_disable();
        suffix = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            0UL, raw, 0U);
        if (suffix <= prefix ||
            ((suffix - prefix) & (VM_PDP6_ALIGN_WORDS - 1UL))) {
                rc = MM_ERR_INVAL;
                goto out;
        }
        base = suffix - prefix;
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(suffix, 0UL);
        rc = mm_pin(suffix);
        if (rc != MM_OK)
                goto raw_done;
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            suffix, raw, 1U))
                goto uncertain;

        /* The full-span PACKED claim and its pin remain authoritative
         * while the loader runs with ordinary disk PI behavior. */
        rc = mm_pure_claim_prefix_v1384(suffix, slot, prefix, words);
        if (rc != MM_OK) {
                if (rc == MM_ERR_FRAGMENTED)
                        rc = PROC_PURE_PREFIX_FALLBACK_V1384;
                if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                    suffix, raw, 1U))
                        goto raw_unpin;
                goto release;
        }
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            base, words, 1U))
                goto uncertain;
        mach_pi_restore(pi_state);
        fs_zero_words(PRP_PTR(base), (unsigned int)prefix);
        rc = loader(backing, base, PROC_UAREA_BASE(p), prefix);
        pi_state = mach_pi_disable();
        if (rc != 0) {
                rc = rc == -2 ? PROC_PURE_PREFIX_FALLBACK_V1384 :
                    MM_ERR_INVAL;
                goto release;
        }

        /* Check the original full-span owner again before committing.
         * Retype and pin retirement are one MM store; verify the
         * resulting PROCESS descriptor before publishing a runnable APR.
         * A lost commit reply is harmless only with that exact proof. */
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            base, words, 1U))
                goto uncertain;
        rc = mm_pure_commit_prefix_v1384(base, slot, prefix, words);
        if (proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base,
            words, 0U)) {
                VM_PDP6_SET_BASE(p, base);
                u[PROC_SWAP_BACKING_OFFSET] = 0UL;
                if (proc_storage_publish_v30(slot, PROC_STORAGE_PACKED,
                    PROC_STORAGE_RESIDENT) != 0)
                        proc_swap_storage_invariant_v30();
                u[PROC_FINALIZE_OFFSET] = 0UL;
                PROC_CLEAR_TRANSITION(p);
                rc = MM_OK;
                goto out;
        }
        if (rc == MM_OK)
                goto uncertain;
release:
        /* Never shrink or unpin a replacement or multiply pinned
         * extent. A refused release may have committed; require the
         * original RAW suffix before retiring its sole transaction pin. */
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            base, words, 1U))
                goto uncertain;
        if (mm_pure_release_prefix_v1384(base, slot, prefix, words) != MM_OK)
                rc = MM_ERR_BUSY;
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            suffix, raw, 1U))
                goto uncertain;
raw_unpin:
        if (mm_unpin(suffix) != MM_OK)
                rc = MM_ERR_BUSY;
raw_done:
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            suffix, raw, 0U))
                goto uncertain;
        u[PROC_FINALIZE_OFFSET] = 0UL;
        goto out;
uncertain:
        rc = MM_ERR_BUSY;
out:
        mach_pi_restore(pi_state);
        return rc;
}
