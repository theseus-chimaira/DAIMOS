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
 * v1902: an uncommitted FULL restore can retain exact RAW+scratch MM
 * owners with TRANSITION held and a live private executive stack.
 * Slot-0 idle backoff retries rollback of only the scratch target,
 * then releases TRANSITION with original RAW authoritative.
 * v1906: a saved full-target base is mandatory for that rollback.
 * Ownership loss cannot turn a live foreign-claimed target into an
 * apparently sole RAW owner and prematurely clear TRANSITION.
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

#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
#include "proc_pure_bitmap_restore_v1664.h"
#include "proc_pure_bitmap_terminal_v1661.h"
#endif

#define PPURE_READY_ONCE_V1612 01000UL
#define PPURE_COOLDOWN_V1612 0100UL
#define PPURE_RECOVER_REQUEST_V1902 0400000UL

extern kword_t proc_swap_pressure_ticks;
extern int proc_pure_recover_arm_v1902(kword_t);
extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t);
extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern int exec_image_prefix_backing_v1386(kword_t, kword_t,
    kword_t, kword_t);
extern int exec_image_full_bounded_v1391(kword_t, kword_t,
    kword_t, kword_t);

#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
/* Fixed proc_storage_expect_v30 intentionally cannot read mode3.
 * Check the unchanged two-bit map only in this opt-in PURE module.
 */
static int
ppure_mode3_v1667(unsigned int slot)
{
        kword_t shift;
        if (!proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return 0;
        shift = (kword_t)(slot % PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        return ((proc_storage_runtime_v29.words[
            slot / PROC_STMAP_SLOTS_WORD_V28] >> shift) & 3UL) ==
            (kword_t)PROC_STORAGE_BITMAP_V1661;
}
#endif

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

/* Validate ONLY a stalled RAW FULL owner set. No new extent pointer is
 * recorded between attempts: the original MM table is authoritative.
 * Return 1 for exact sole RAW, 3 for exact RAW+temporary PROCESS, or
 * -1 for any ambiguity. Pins are allowed in the read-only admission
 * pass and rejected again, atomically, before physical release.
 */
static int
ppure_owners_v1902(unsigned int slot, kword_t raw, kword_t words,
    int require_unpinned, kword_t *targetp)
{
        unsigned int seen = 0U;
        kword_t source = 0UL, target = 0UL;
        int i;

        if (mm_extent_count <= 0 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return -1;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                kword_t base = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);
                unsigned int bit;

                if (MM_EXTENT_OWNER(e) != slot)
                        continue;
                /* MODULE and KERNEL_DYNAMIC use a separate owner
                 * namespace. Reserved/invalid MM types do not.
                 */
                if (type == MM_TYPE_MODULE ||
                    type == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (type == MM_TYPE_PACKED_PROCESS)
                        bit = 1U;
                else if (type == MM_TYPE_PROCESS)
                        bit = 2U;
                else
                        return -1;
                if ((seen & bit) || !base ||
                    (require_unpinned && MM_EXTENT_PINNED(e)) ||
                    (bit == 1U && size != raw) ||
                    (bit == 2U && (size != words ||
                        (base & (VM_PDP6_ALIGN_WORDS - 1UL)))) ||
                    size > MM_HALF_MASK - base)
                        return -1;
                seen |= bit;
                if (bit == 1U)
                        source = base;
                else
                        target = base;
        }
        if (!(seen & 1U) || (seen == 3U &&
            !(source + raw <= target || target + words <= source)))
                return -1;
        *targetp = target;
        return (int)seen;
}

/* v1906: after an MM free, absence of this *specific* provisional
 * target must be proved over the WHOLE physical extent table, not by
 * counting only descriptors still attributed to our process slot.
 * A foreign owner, duplicate, partial overlap or invalid MM geometry
 * at the original target means that free/ownership is uncertain.
 * Never delete the foreign descriptor to make the proof succeed.
 * Called from the existing PI-atomic slot-0 recovery window.
 */
static int
ppure_target_absent_v1906(kword_t target, kword_t words)
{
        int i;

        if (!target || !words ||
            (target & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            target > MM_HALF_MASK || words > MM_HALF_MASK - target ||
            mm_extent_count <= 0 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                kword_t base = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);

                if (!base || !size || base > MM_HALF_MASK ||
                    size > MM_HALF_MASK - base ||
                    (base < target + words && target < base + size))
                        return 0;
        }
        return 1;
}

/* Common fail-closed preflight for the original slot-0 FULL recovery.
 * A saved executive continuation is intentional: this operation does
 * NOT release a user stack, FILE, u-area or executable backing.
 */
static int
ppure_recover_preflight_v1902(unsigned int slot, struct proc **pp,
    kword_t **up, kword_t *rawp, kword_t *wordsp)
{
        struct proc *p;
        kword_t *u, backing, words, skip;
        unsigned int state;

        if (proc_current_slot != 0UL || !proc_table ||
            !proc_swap_records || !proc_storage_runtime_v29.words ||
            !slot || slot >= proc_high_slot ||
            slot >= proc_storage_runtime_v29.slots)
                return 0;
        p = &proc_table[slot];
        state = PROC_STATE(p);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p) ||
            (state != PROC_SRUN && state != PROC_SLEEP &&
             state != PROC_STOP) || VM_PDP6_BASE(p) ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0)
                return 0;
        u = (kword_t *)(unsigned long)PROC_UAREA_BASE(p);
        if (u[PROC_FDCTL_OFFSET] &
            (PROC_FATAL_CLOSE_BIT | PROC_USER_MAP_BIT) ||
            (u[PROC_FINALIZE_OFFSET] & PROC_FINALIZE_MARK))
                return 0;
        backing = proc_swap_records[slot].state;
        words = VM_SPACE_WORDS(p);
        if (!backing || !words ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK)
                return 0;
        *pp = p;
        *up = u;
        *rawp = words - skip;
        *wordsp = words;
        return 1;
}

/* Called from original MRES only when a previously failed FULL left
 * TRANSITION held. Mark a retry owner without claiming FINALIZE:
 * its private executive continuation MUST remain intact. This
 * shares the already-resident post-VM idle backoff timer.
 */
static void
ppure_recover_arm_v1902(unsigned int slot)
{
        struct proc *p;
        kword_t *u, raw, words, target;

        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words))
                return;
        if (ppure_owners_v1902(slot, raw, words, 0, &target) == 3 &&
            target &&
            proc_pure_recover_arm_v1902(PROC_UAREA_BASE(p)) == 0) {
                /* The u-area already exists and is saved across FULL.
                 * Its word 067 RH is an ordinary nonresident swap
                 * retry epoch, but TRANSITION prevents such scheduling
                 * while this PURE recovery tag is armed. Reuse those
                 * 18 bits to escrow the exact original scratch base.
                 * This does not allocate permanent or per-slot RAM.
                 */
                u[PROC_FINALIZE_OFFSET] =
                    (u[PROC_FINALIZE_OFFSET] & ~MM_HALF_MASK) | target;
        }
}

/* The original KCORE idle-stack reaper invokes this through the
 * existing (already patched) PURE service trampoline with a tagged
 * AC1 request. No KINIT extension or second MRES entry is needed.
 * The native MM free and final owner reconciliation are PI-atomic;
 * failure leaves original backing, VM0 and TRANSITION unchanged.
 */
static int
ppure_recover_one_v1902(unsigned int slot)
{
        struct proc *p;
        kword_t *u, raw, words, target, saved, original_target;
        int owners;

        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words) ||
            !(u[PROC_FINALIZE_OFFSET] & PROC_SWAP_RETRY_TAG_V1507))
                return -1;
        saved = mach_pi_disable();
        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words)) {
                mach_pi_restore(saved);
                return -1;
        }
        original_target = u[PROC_FINALIZE_OFFSET] & MM_HALF_MASK;
        if (!original_target ||
            (original_target & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            words > MM_HALF_MASK - original_target) {
                mach_pi_restore(saved);
                return -1;
        }
        owners = ppure_owners_v1902(slot, raw, words, 1, &target);
        if (owners == 3) {
                if (target != original_target) {
                        mach_pi_restore(saved);
                        return -1;
                }
                /* Never free the authoritative RAW suffix or publish
                 * the already-loaded temporary APR. Uncertainty in
                 * the free reply is resolved by the next MM scan.
                 */
                (void)mm_free(target, MM_TYPE_PROCESS, slot);
                owners = ppure_owners_v1902(slot, raw, words,
                    1, &target);
        }
        if (owners != 1 ||
            !ppure_target_absent_v1906(original_target, words) ||
            !ppure_recover_preflight_v1902(slot, &p, &u, &raw,
                &words)) {
                mach_pi_restore(saved);
                return -1;
        }
        /* All MM and process invariants now prove a sole unpinned
         * PACKED source. The original saved private CPU context,
         * FILE descriptors and backing stay untouched.
         */
        u[PROC_FINALIZE_OFFSET] = 0UL;
        PROC_CLEAR_TRANSITION(p);
        mach_pi_restore(saved);
        return 0;
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
proc_pure_mres_service_v1612(kword_t command_v1902)
{
        unsigned int slot;
        struct proc *p;
        kword_t request = proc_sched_cursor;
        int rc;
#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
        int bitmap;
#endif

        if ((command_v1902 & PPURE_RECOVER_REQUEST_V1902) != 0UL) {
                if (command_v1902 &
                    ~(PPURE_RECOVER_REQUEST_V1902 | 0377UL))
                        return -1;
                return ppure_recover_one_v1902(
                    (unsigned int)(command_v1902 & 0377UL));
        }
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
            !proc_swap_records[slot].state
#if !defined(DAIMOS_PBM_V1667) || DAIMOS_PBM_V1667 != 1
            || proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0
#endif
            )
                return 0;
#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
        bitmap = ppure_mode3_v1667(slot);
        if (!bitmap && proc_storage_expect_v30(slot,
            PROC_STORAGE_PACKED) != 0)
                return 0;
#endif
        PROC_SET_TRANSITION(p);
#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
        if (bitmap) {
                rc = proc_pure_bitmap_restore_v1664(slot,
                    exec_image_full_bounded_v1391);
        } else {
#endif
        rc = proc_pure_restore_v1384(slot,
            exec_image_prefix_backing_v1386);
        if (rc == PROC_PURE_PREFIX_FALLBACK_V1384)
                rc = proc_pure_ram_restore_v1391(slot,
                    exec_image_full_bounded_v1391);
#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
        }
#endif
        /* Both restore transactions publish the runnable APR/mode and
         * clear TRANSITION only after every fallible MM commit succeeds.
         * A lost successful return cannot revoke that committed state:
         * READY_ONCE must be derived from the publication, not the rc.
         * A failed transaction keeps TRANSITION or PACKED, so it cannot
         * satisfy this existing fail-closed readiness predicate.
         */
        if (!PROC_TRANSITION(p) && VM_PDP6_BASE(p) &&
            proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) == 0) {
                proc_sched_cursor = (kword_t)slot |
                    PPURE_READY_ONCE_V1612;
                return (int)proc_sched_cursor;
        }
        /* On a recoverable source/fragmentation refusal the raw suffix
         * remains an exact unpinned PACKED owner. Other failures retain
         * TRANSITION and must be handled by the existing terminal reaper;
         * never unlock an unknown MM owner or attempt a disk exchange. */
#if defined(DAIMOS_PBM_V1667) && DAIMOS_PBM_V1667 == 1
        if (bitmap ?
            proc_pure_bitmap_retry_v1667(slot) == MM_OK :
            ppure_retryable_v1612(slot, p, PROC_STORAGE_PACKED))
#else
        if (ppure_retryable_v1612(slot, p, PROC_STORAGE_PACKED))
#endif
                PROC_CLEAR_TRANSITION(p);
        if (PROC_TRANSITION(p))
                ppure_recover_arm_v1902(slot);
        proc_swap_pressure_ticks = PPURE_COOLDOWN_V1612;
        return 0;
}
