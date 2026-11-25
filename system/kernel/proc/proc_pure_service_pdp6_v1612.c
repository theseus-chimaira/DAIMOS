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

/* PI-excluded retry proof, using the same sole-owner scanner as DROP
 * and PREFIX. Escrowed transactions must use their saved identities,
 * not discover a replacement owner here.
 */
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
        if (!backing || !words || (words & (VM_PDP6_ALIGN_WORDS - 1UL)))
                return 0;
        u = (kword_t *)(unsigned long)PROC_UAREA_BASE(p);
        if (u[PROC_FINALIZE_OFFSET] &
            (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507))
                return 0;
        if (mode == PROC_STORAGE_RESIDENT) {
                base = VM_PDP6_BASE(p);
                return base && !(base & (VM_PDP6_ALIGN_WORDS - 1UL)) &&
                    !u[PROC_SWAP_BACKING_OFFSET] &&
                    proc_storage_expect_v30(slot, mode) == 0 &&
                    proc_pure_owner_v1918(slot, MM_TYPE_PROCESS,
                        base, words, 0U) != 0UL;
        }
        if (mode != PROC_STORAGE_PACKED ||
            VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            pdp6_pure_skip_v1604(backing, words, &prefix) != MM_OK ||
            proc_storage_expect_v30(slot, mode) != 0)
                return 0;
        base = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            0UL, words - prefix, 0U);
        return base > prefix &&
            !((base - prefix) & (VM_PDP6_ALIGN_WORDS - 1UL));
}

/* Keep the owner proof and TRANSITION release in the same PI window. */
static int
ppure_unlock_v1917(unsigned int slot, struct proc *p, unsigned int mode)
{
        kword_t saved = mach_pi_disable();
        int ok = ppure_retryable_v1612(slot, p, mode);

        if (ok)
                PROC_CLEAR_TRANSITION(p);
        mach_pi_restore(saved);
        return ok;
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
        kword_t *u, raw, words, escrow, saved;

        saved = mach_pi_disable();
        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words)) {
                mach_pi_restore(saved);
                return;
        }
        /* FULL escrows the original target before the first target
         * pin or loader call. A later owner scan cannot reconstruct
         * that identity after a foreign claim or partial rollback.
         */
        escrow = u[PROC_FINALIZE_OFFSET];
        if ((escrow & PROC_SWAP_RETRY_TAG_V1507) &&
            PROC_PURE_FULL_SOURCE_V1917(escrow) &&
            proc_pure_recover_arm_v1902(PROC_UAREA_BASE(p)) == 0) {
                /* The permanent armer overwrites word 067. Restore
                 * both original transaction identities, not an MM guess. */
                u[PROC_FINALIZE_OFFSET] = escrow;
        }
        mach_pi_restore(saved);
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
        kword_t *u, raw, words, source, target, saved;
        int owners;

        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words) ||
            !(u[PROC_FINALIZE_OFFSET] & PROC_SWAP_RETRY_TAG_V1507))
                return -1;
        saved = mach_pi_disable();
        if (!ppure_recover_preflight_v1902(slot, &p, &u, &raw,
            &words) ||
            !(u[PROC_FINALIZE_OFFSET] & PROC_SWAP_RETRY_TAG_V1507)) {
                mach_pi_restore(saved);
                return -1;
        }
        source = PROC_PURE_FULL_SOURCE_V1917(u[PROC_FINALIZE_OFFSET]);
        target = u[PROC_FINALIZE_OFFSET] & MM_HALF_MASK;
        if (source <= words - raw ||
            ((source - (words - raw)) & (VM_PDP6_ALIGN_WORDS - 1UL))) {
                mach_pi_restore(saved);
                return -1;
        }
        owners = proc_pure_full_owners_v1917(slot, source, raw,
            target, words, 0U, 0U);
        if (owners == 3 && target) {
                /* Never free the authoritative RAW suffix or publish
                 * the already-loaded temporary APR. Uncertainty in
                 * the free reply is resolved by the next MM scan.
                 */
                (void)mm_free(target, MM_TYPE_PROCESS, slot);
                owners = proc_pure_full_owners_v1917(slot, source, raw,
                    target, words, 0U, 0U);
        }
        if (owners != 1 ||
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
        int rc;

        if (backstore_enabled || backstore_blocks ||
            backstore_blocks_used || backstore_bitmap ||
            proc_swap_blocks_used || !proc_table || !proc_swap_records)
                return -1;
        /* Existing MM compact is the authoritative contiguous-space test.
         * Preserve hard MM refusals just as the permanent allocator does:
         * an invalid or busy move must not justify further victim DROP.
         * Ordinary resource pressure may still reclaim a PURE prefix. */
        rc = mm_compact(words, alignment);
        if (rc == MM_OK || rc < MM_ERR_DESCRIPTORS)
                return rc;
        for (slot = 1U; slot < proc_high_slot; ++slot) {
                struct proc *p;
                kword_t base;
                unsigned int state;

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
                        if (ppure_unlock_v1917(slot, p,
                            PROC_STORAGE_RESIDENT)) {
                                continue;
                        } else if (!ppure_unlock_v1917(slot, p,
                            PROC_STORAGE_PACKED)) {
                                /* Neither original nor committed owner
                                 * can be proved safe. Quarantine this
                                 * victim and stop allocator pressure. */
                                return rc < MM_ERR_DESCRIPTORS ?
                                    rc : MM_ERR_BUSY;
                        }
                }
                /* A completed DROP stays committed, even if a later
                 * compaction refuses. Do not reclaim another victim. */
                rc = mm_compact(words, alignment);
                if (rc == MM_OK || rc < MM_ERR_DESCRIPTORS)
                        return rc;
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
        if (bitmap) {
                if (proc_pure_bitmap_retry_v1667(slot) == MM_OK)
                        PROC_CLEAR_TRANSITION(p);
        } else
#endif
                (void)ppure_unlock_v1917(slot, p, PROC_STORAGE_PACKED);
        if (PROC_TRANSITION(p))
                ppure_recover_arm_v1902(slot);
        proc_swap_pressure_ticks = PPURE_COOLDOWN_V1612;
        return 0;
}
