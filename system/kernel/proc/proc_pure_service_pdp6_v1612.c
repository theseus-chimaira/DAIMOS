/**
 * @file proc_pure_service_pdp6_v1612.c
 * @brief PDP-6 PURE service with a single strict MM owner scan.
 *
 * This belongs in a separately installed MRES, never fixed KCORE.
 * Only stopped/sleeping, noncurrent processes may lose their PURE prefix.
 * The exact retained suffix remains in its existing MM descriptor.
 * No new process flags, owner registry, or permanent RAM are required.
 *
 * A failed transaction is NOT a disk I/O error. Do not deliver TERM,
 * advertise READY_ONCE or clear a possibly pinned TRANSITION owner.
 * Only an exact, singly owned, unpinned extent permits retry unlock.
 * v1612 removes the redundant second MM scan of PACKED failure owners.
 */
#include "proc.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "proc_pure_ram_v1381.h"
#include "proc_pure_restore_v1384.h"
#include "proc_pure_ram_restore_v1391.h"
#include "vm_pdp6_pure_range_v1604.h"
#include "vm_pdp6.h"
#include "mm.h"
#include "bstore.h"

#define PPURE_READY_ONCE_V1612 01000UL
#define PPURE_COOLDOWN_V1612 0100UL

extern kword_t proc_swap_pressure_ticks;
extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern int exec_image_prefix_backing_v1386(kword_t, kword_t,
    kword_t, kword_t);
extern int exec_image_full_bounded_v1391(kword_t, kword_t,
    kword_t, kword_t);

/* Check the one authoritative process MM owner after a refused operation.
 * A free/duplicate/pinned descriptor cannot be unlocked for execution or
 * a later collector pass. This is the PDP-6 geometry backend boundary.
 */
static int
ppure_exact_owner_v1612(unsigned int slot, unsigned int type,
    kword_t base, kword_t words)
{
        unsigned int seen = 0U;
        int i;

        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int kind = MM_EXTENT_TYPE(e);

                if (MM_EXTENT_OWNER(e) != slot ||
                    (kind != MM_TYPE_PROCESS &&
                     kind != MM_TYPE_PACKED_PROCESS))
                        continue;
                /* An expected nonzero base verifies resident APR.
                 * For PACKED, base==0 means the single positive base
                 * is derived from MM itself, not from a VM mapping.
                 * Both modes require exactly one owner, exact size
                 * and an unpinned extent. A second owner fails closed.
                 */
                if (++seen != 1U || kind != type ||
                    !MM_EXTENT_BASE(e) ||
                    (base && MM_EXTENT_BASE(e) != base) ||
                    MM_EXTENT_WORDS(e) != words ||
                    MM_EXTENT_PINNED(e))
                        return 0;
        }
        return seen == 1U;
}

/* This is intentionally a strict RAM owner proof, not a source lookup.
 * VFS source identity is checked again inside each restore transaction. */
static int
ppure_retryable_v1612(unsigned int slot, struct proc *p,
    unsigned int mode)
{
        kword_t backing, words, prefix, base;
        kword_t *u;

        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p))
                return 0;
        backing = proc_swap_records[slot].state;
        words = VM_SPACE_WORDS(p);
        if (!backing || !words)
                return 0;
        u = (kword_t *)(unsigned long)PROC_UAREA_BASE(p);
        if (mode == PROC_STORAGE_RESIDENT) {
                base = VM_PDP6_BASE(p);
                return base && !u[PROC_SWAP_BACKING_OFFSET] &&
                    proc_storage_expect_v30(slot, mode) == 0 &&
                    ppure_exact_owner_v1612(slot, MM_TYPE_PROCESS,
                        base, words);
        }
        if (mode != PROC_STORAGE_PACKED ||
            VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            pdp6_pure_skip_v1604(backing, words, &prefix) != MM_OK ||
            proc_storage_expect_v30(slot, mode) != 0)
                return 0;
        /* The exact-one-owner scan accepts an MM-derived base when
         * PACKED has no runnable APR mapping. Do not scan twice and
         * do not invent any extra state recording that physical base.
         */
        return ppure_exact_owner_v1612(slot, MM_TYPE_PACKED_PROCESS,
            0UL, words - prefix);
}

/* The permanent reclaim trampoline's original ABI is unchanged:
 * required words, alignment, excluded owner. */
int
proc_pure_mres_reclaim_v1612(kword_t words, kword_t alignment,
    unsigned int excluded)
{
        unsigned int slot;

        if (backstore_enabled || backstore_blocks ||
            backstore_blocks_used || backstore_bitmap ||
            proc_swap_blocks_used || !proc_table || !proc_swap_records)
                return -1;
        /* Existing MM compact is the authoritative contiguous-space test.
         * Do not repeatedly target the same refused process. */
        if (mm_compact(words, alignment) == MM_OK)
                return 0;
        for (slot = 1U; slot < proc_high_slot; ++slot) {
                struct proc *p;
                kword_t base;
                unsigned int state;
                int rc;

                if (slot == excluded || slot == proc_current_slot)
                        continue;
                p = &proc_table[slot];
                state = PROC_STATE(p);
                if ((state != PROC_SLEEP && state != PROC_STOP) ||
                    PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
                    !PROC_UAREA_BASE(p) ||
                    proc_storage_expect_v30(slot,
                        PROC_STORAGE_RESIDENT) != 0)
                        continue;
                base = VM_PDP6_BASE(p);
                if (!base || mm_is_pinned(base) ||
                    !proc_swap_records[slot].state)
                        continue;
                PROC_SET_TRANSITION(p);
                rc = proc_pure_ram_drop_v1381(slot);
                if (rc != MM_OK) {
                        /* Rebase can commit even when its last unpin
                         * refuses. Unlock only an exact unpinned owner.
                         */
                        if (ppure_retryable_v1612(slot, p,
                            PROC_STORAGE_RESIDENT)) {
                                PROC_CLEAR_TRANSITION(p);
                        } else if (ppure_retryable_v1612(slot, p,
                            PROC_STORAGE_PACKED)) {
                                PROC_CLEAR_TRANSITION(p);
                                if (mm_compact(words, alignment) == MM_OK)
                                        return 0;
                        }
                        continue;
                }
                if (mm_compact(words, alignment) == MM_OK)
                        return 0;
        }
        return -1;
}

/* Service only an explicitly requested RUN/PACKED owner. The existing
 * scheduler owns request creation, READY_ONCE selection, and PI6 wakeup.
 */
int
proc_pure_mres_service_v1612(void)
{
        unsigned int slot;
        struct proc *p;
        kword_t request = proc_sched_cursor;
        int rc;

        if (!(request & PROC_SCHED_SWAP_REQUEST))
                return 0;
        slot = (unsigned int)(request & 0377UL);
        proc_sched_cursor = (kword_t)slot;
        if (!slot || slot >= proc_high_slot || !proc_table ||
            !proc_swap_records || proc_current_slot != 0UL)
                return 0;
        p = &proc_table[slot];
        if (PROC_STATE(p) != PROC_SRUN || PROC_TRANSITION(p) ||
            VM_PDP6_BASE(p) ||
            !proc_swap_records[slot].state ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0)
                return 0;
        PROC_SET_TRANSITION(p);
        rc = proc_pure_restore_v1384(slot,
            exec_image_prefix_backing_v1386);
        if (rc == PROC_PURE_PREFIX_FALLBACK_V1384)
                rc = proc_pure_ram_restore_v1391(slot,
                    exec_image_full_bounded_v1391);
        if (rc == MM_OK &&
            !PROC_TRANSITION(p) && VM_PDP6_BASE(p) &&
            proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) == 0) {
                proc_sched_cursor = (kword_t)slot |
                    PPURE_READY_ONCE_V1612;
                return (int)proc_sched_cursor;
        }
        /* On a recoverable source/fragmentation refusal the raw suffix
         * remains an exact unpinned PACKED owner. Other failures retain
         * TRANSITION and must be handled by the existing terminal reaper;
         * never unlock an unknown MM owner or attempt a disk exchange. */
        if (ppure_retryable_v1612(slot, p, PROC_STORAGE_PACKED))
                PROC_CLEAR_TRANSITION(p);
        proc_swap_pressure_ticks = PPURE_COOLDOWN_V1612;
        return 0;
}
