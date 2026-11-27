/**
 * @file proc_pure_ram_restore_v1391.c
 * @brief Registry-free FULL restore with an explicit destination APR limit.
 *
 * The MM extent table is authoritative. The existing two-bit storage
 * mode distinguishes resident from inactive RAM; a RAW mutable suffix
 * is recovered by exact owner/type/word-count matching. The PURE skip
 * boundary is derivable from the saved backing and original VM size.
 * No second registry, compression, disk allocator or temporary suffix
 * allocation is needed. The v1381 DROP has already rebased the original
 * extent; this FULL restore allocates a separate pinned APR, checks the
 * exact capacity through the loader ABI, and overlays retained mutable
 * words only after the file read succeeds.
 *
 * v1654: Unpin the destination BEFORE releasing the only authoritative
 * PACKED source and publishing the runnable APR. A failed final unpin
 * cannot leave a RESIDENT/TRANSITION process with its source already
 * destroyed. Keep the short fallible commit window PI-atomic.
 * v1839: a failed source-free return can follow successful physical
 * deletion. Verify the sole surviving PROCESS owner before deciding
 * whether to publish or roll back the destination; quarantine ambiguous
 * MM owner sets without releasing potentially unique process storage.
 * v1855: both live MM extents must still have their ORIGINAL exact one
 * pin at commit. A second pin on the temporary PROCESS APR otherwise
 * survives a successful mm_unpin() return, and releasing the last RAW
 * source would publish a still-pinned incomplete transaction.
 *
 * Linked only by the opt-in PURE MRES. The default KCORE is unchanged.
 */
#include "proc_pure_ram_restore_v1391.h"
#include "proc_pure_ram_v1381.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm_pdp6.h"
#include "exec.h"
#include "mm_internal.h"
#include "mm_pure_rebase_v1381.h"
#include "fs_mres.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"

#if !defined(DAIMOS_PURE_FULL_V1391) || DAIMOS_PURE_FULL_V1391 != 1
#error proc_pure_ram_restore_v1391_requires_swapless_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PRAM_PTR(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PRAM_PTR(x) ((kword_t *)(unsigned long)(x))
#endif

/* Reuse the already linked assembler's exact two-bit mode operations. */
extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern int proc_storage_publish_v30(unsigned int, unsigned int,
    unsigned int);
extern void proc_swap_storage_invariant_v30(void);
extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);

/* Both v1839's failed-free reply and v1855's precommit pin check
 * inspect the same authoritative MM descriptors under PI exclusion.
 * Return a bitset: 1 for the exact PACKED source, 2 for the exact
 * PROCESS destination; -1 for an unexpected owner, type, geometry,
 * duplicate, or pin count. The expected pin count is 1 before the
 * commit and 0 after both original transaction unpins have completed.
 * v1917: rollback uses the same scan with independent pin counts.
 * Count 2 means an uncertain unpin reply (zero or one pin); that
 * extent must not be unpinned speculatively. A foreign overlapping
 * extent is never evidence that the original allocation disappeared.
 */
int
proc_pure_full_owners_v1917(unsigned int slot, kword_t source,
    kword_t raw, kword_t target, kword_t words,
    unsigned int source_pins, unsigned int target_pins)
{
        unsigned int seen = 0U;
        int i;

        if ((!source && !target) ||
            (source && (source > MM_HALF_MASK || !raw ||
             raw > MM_HALF_MASK + 1UL - source)) ||
            (target && (target > MM_HALF_MASK || !words ||
             words > MM_HALF_MASK + 1UL - target ||
             (target & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
             (source && source < target + words &&
              target < source + raw))) ||
            mm_extent_count <= 0 || mm_pure_table_v1922() != MM_OK ||
            (source && mm_pure_range_v1922(source, raw) != MM_OK) ||
            (target && mm_pure_range_v1922(target, words) != MM_OK))
                return -1;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                unsigned int bit, pins;
                kword_t base = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);

                if (MM_EXTENT_OWNER(e) != slot ||
                    type == MM_TYPE_MODULE ||
                    type == MM_TYPE_KERNEL_DYNAMIC) {
                        if ((source && base < source + raw &&
                             source < base + size) ||
                            (target && base < target + words &&
                             target < base + size))
                                return -1;
                        continue;
                }
                if (type == MM_TYPE_PROCESS && base == target &&
                    size == words) {
                        bit = 2U;
                        pins = target_pins;
                } else if (type == MM_TYPE_PACKED_PROCESS &&
                    base == source && size == raw) {
                        bit = 1U;
                        pins = source_pins;
                } else {
                        return -1;
                }
                if (pins == 2U) {
                        if (MM_EXTENT_PINS(e) > 1U)
                                return -1;
                } else if (MM_EXTENT_PINS(e) != pins)
                        return -1;
                if (seen & bit)
                        return -1;
                seen |= bit;
        }
        return (int)seen;
}

static int
pram_exact_owner_set_v1855(unsigned int slot, kword_t source,
    kword_t raw, kword_t target, kword_t words, unsigned int pins)
{
        return proc_pure_full_owners_v1917(slot, source, raw, target, words,
            pins, pins);
}

/* A failed mm_free reply does not prove that the PACKED owner survived.
 * The exact unpinned destination alone means the free committed;
 * both exact unpinned owners permit a checked target rollback.
 * Everything else must remain quarantined, including foreign owners.
 */
static int
pram_source_free_reply_v1839(unsigned int slot, kword_t source,
    kword_t raw, kword_t target, kword_t words)
{
        int owners = pram_exact_owner_set_v1855(slot, source, raw,
            target, words, 0U);

        if (owners == 2)
                return 1;
        return owners == 3 ? 0 : -1;
}

/*
 * Caller already holds the process transition. The live executable
 * backing guard is supplied by proc_swap_attach and the resident/
 * nonresident guard scan. No new kernel word is needed.
 */
static int
pram_prepare(unsigned int slot, unsigned int action,
    struct proc **pp, kword_t *backingp, kword_t *wordsp,
    kword_t *skipp)
{
        struct proc *p;
        kword_t backing, words, skip;
        int rc;

        rc = pure_life_prepare_v1604(slot, action, &p, &backing,
            &words);
        if (rc != MM_OK)
                return rc;
        if (action != PURE_LIFE_TERMINAL_V1610) {
                rc = pure_admit_source_v1608(p, backing);
                if (rc != MM_OK)
                        return rc;
        }
        rc = pdp6_pure_skip_v1604(backing, words, &skip);
        if (rc != MM_OK)
                return rc;
        *pp = p;
        *backingp = backing;
        *wordsp = words;
        *skipp = skip;
        return MM_OK;
}

static void
pram_publish(unsigned int slot, unsigned int before, unsigned int after)
{
        if (proc_storage_publish_v30(slot, before, after) != 0)
                proc_swap_storage_invariant_v30();
}

int
proc_pure_ram_restore_v1391(unsigned int slot,
    proc_pure_ram_loader_v1391 loader)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source, target;
        kword_t *u;
        kword_t pi_state;
        int rc;
        int source_pinned = 0;
        int target_pinned = 0;
        int source_uncertain = 0;
        int target_uncertain = 0;

        if (!loader)
                return MM_ERR_INVAL;
        rc = pram_prepare(slot, PURE_LIFE_RESTORE_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        if (proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0)
                return MM_ERR_INVAL;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        raw = words - skip;
        if (VM_PDP6_BASE(p) || (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (u[PROC_FINALIZE_OFFSET] &
             (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507)) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        pi_state = mach_pi_disable();
        source = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            0UL, raw, 0U);
        if (source <= skip ||
            ((source - skip) & (VM_PDP6_ALIGN_WORDS - 1UL))) {
                mach_pi_restore(pi_state);
                return MM_ERR_INVAL;
        }
        /* Retain the exact RAW identity even if the first pin or the
         * allocation refuses. Zero target means no target was allocated;
         * a nonzero invalid address is deliberately unrecoverable. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(source, 0UL);
        rc = mm_pin(source);
        if (rc != MM_OK)
                goto fail_source_locked;
        source_pinned = 1;
        if (proc_pure_full_owners_v1917(slot, source, raw, 0UL, words,
            1U, 0U) != 1)
                goto uncertain;
        target = 0UL;
        /* Keep disk PI enabled during the allocator's gap scan. Its
         * own PI6 exclusion, our slot-0 TRANSITION and the source pin
         * protect this allocation. Escrow the returned physical base
         * before any loader or additional fallible MM operation. */
        mach_pi_restore(pi_state);
        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        pi_state = mach_pi_disable();
        if (rc != MM_OK && !target)
                goto fail_source;
        /* Escrow the allocator's original target, not a later scan of
         * current owner numbers. The service arms idle retry only if
         * this transaction cannot prove that the target is gone.
         * Word 067 already belongs to this TRANSITION-held owner.
         */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(source,
            target && target <= MM_HALF_MASK ? target : 1UL);
        if (rc != MM_OK)
                goto uncertain;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            1U, 0U) != 3)
                goto uncertain;
        rc = mm_pin(target);
        if (rc != MM_OK)
                goto fail_target_locked;
        target_pinned = 1;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            1U, 1U) != 3)
                goto uncertain;
        mach_pi_restore(pi_state);
        fs_zero_words(PRAM_PTR(target), (unsigned int)skip);
        /* Only the allocator-supplied APR length is trusted. This call
         * happens after the complete destination extent is allocated
         * and pinned. A malicious header cannot expand the reservation.
         */
        if (loader(backing, target, PROC_UAREA_BASE(p), words)) {
                rc = MM_ERR_INVAL;
                goto fail_target;
        }
        /* Check before the physical copy as well as before commit.
         * The loader may have returned with an unexpected owner or
         * extra pin; do not overwrite somebody else's destination.
         * The copy itself runs with ordinary PI behavior, protected
         * by the exact transaction pins.
         */
        pi_state = mach_pi_disable();
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            1U, 1U) != 3)
                goto uncertain;
        mach_pi_restore(pi_state);
        /* Decode/reload may have initialized writable words. The raw
         * inactive suffix is always authoritative and overlays them.
         */
        fs_copy_words(PRAM_PTR(source), PRAM_PTR(target + skip),
            (unsigned int)raw);
        /*
         * Finish ALL fallible target cleanup while the authoritative
         * source is still pinned and intact. Neither a target-unpin error
         * nor a source-release refusal may publish a RESIDENT owner.
         * No allocation or filesystem operation occurs with PI disabled.
         */
        pi_state = mach_pi_disable();
        if (pram_exact_owner_set_v1855(slot, source, raw,
            target, words, 1U) != 3) {
                /* Unexpected pins cannot be released by guessing.
                 * Preserve the sole authoritative RAW suffix, full
                 * temporary APR and TRANSITION for operator recovery.
                 */
                mach_pi_restore(pi_state);
                return MM_ERR_BUSY;
        }
        rc = mm_unpin(target);
        if (rc != MM_OK) {
                target_uncertain = 1;
                goto fail_target_locked;
        }
        target_pinned = 0;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            1U, 0U) != 3)
                goto uncertain;
        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_uncertain = 1;
                goto fail_target_locked;
        }
        source_pinned = 0;
        /* Do not release the authoritative source until BOTH original
         * owners are independently proven unpinned. Check deletion even
         * on reported success, before publishing the sole successor. */
        if (pram_exact_owner_set_v1855(slot, source, raw,
            target, words, 0U) != 3)
                goto uncertain;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        switch (pram_source_free_reply_v1839(slot, source, raw, target, words)) {
        case 1:
                break;
        case 0:
                /* Proven refusal: keep the typed MM error. Only a
                 * false success needs conversion to a hard refusal. */
                if (rc == MM_OK)
                        rc = MM_ERR_BUSY;
                goto fail_target_locked;
        default:
                goto uncertain;
        }

        VM_PDP6_SET_BASE(p, target);
        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
        pram_publish(slot, PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT);
        u[PROC_FINALIZE_OFFSET] = 0UL;
        PROC_CLEAR_TRANSITION(p);
        mach_pi_restore(pi_state);
        return MM_OK;

fail_target:
        pi_state = mach_pi_disable();
fail_target_locked:
        /* A base-only unpin is unsafe after an ownership change.
         * Verify both original extents and the exact transaction pins
         * before touching either. Reconcile a failed target-unpin
         * reply without dropping a second pin. Keep PI excluded until
         * the rollback is finished or explicitly quarantined.
         */
        if (target_uncertain &&
            proc_pure_full_owners_v1917(slot, source, raw, target, words,
                (unsigned int)source_pinned, 0U) == 3)
                target_pinned = 0;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            source_uncertain ? 2U : (unsigned int)source_pinned,
            (unsigned int)target_pinned) != 3)
                goto uncertain;
        if (target_pinned && mm_unpin(target) != MM_OK)
                goto uncertain;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            source_uncertain ? 2U : (unsigned int)source_pinned, 0U) != 3)
                goto uncertain;
        if (mm_free(target, MM_TYPE_PROCESS, slot) != MM_OK)
                rc = MM_ERR_BUSY;
        if (proc_pure_full_owners_v1917(slot, source, raw, target, words,
            source_uncertain ? 2U : (unsigned int)source_pinned,
            0U) != 1)
                goto uncertain;
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(source, 0UL);
        goto fail_source_locked;
fail_source:
        if (proc_pure_full_owners_v1917(slot, source, raw, 0UL, words,
            1U, 0U) != 1)
                goto uncertain;
fail_source_locked:
        if (source_pinned && !source_uncertain) {
                if (proc_pure_full_owners_v1917(slot, source, raw, 0UL, words,
                    1U, 0U) != 1 || mm_unpin(source) != MM_OK)
                        goto uncertain;
        }
        if (proc_pure_full_owners_v1917(slot, source, raw, 0UL, words,
            0U, 0U) == 1)
                u[PROC_FINALIZE_OFFSET] = 0UL;
        else
                rc = MM_ERR_BUSY;
        mach_pi_restore(pi_state);
        return rc;
uncertain:
        mach_pi_restore(pi_state);
        return MM_ERR_BUSY;
}

#ifdef DAIMOS_PBM_RESTORE_V1980
#include "proc_pure_bitmap_directory_v1977.h"
#include "proc_pure_bitmap_terminal_v1661.h"
#include "proc_pure_bitmap_restore_v1980.h"
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
#include "proc_pure_full_escrow_v1989.h"
#endif

extern int mm_pure_bitmap_finish_v1664(unsigned int, kword_t, kword_t,
    kword_t, kword_t);

/* Private continuation identity, not a new resident registry. The exact
 * source/target geometry is kept in ordinary call locals. */
struct pbr_identity_v1980 {
        kword_t backing, words, uarea, cred, escrow;
        kword_t *modep;
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        kword_t brk;                   /* exact current escrow/heap witness */
#endif
};

/* PI excluded. Validate u-area ownership before reading any of its
 * words. The original bitmap owner is immovable, but a stale u-area
 * pointer must not authorize a physical write or release. */
static struct proc *
pbr_process_v1980(unsigned int slot, unsigned int terminal)
{
        struct proc *p;
        kword_t area;
        int i;

        if (!slot || (terminal ? proc_current_slot == slot : proc_current_slot != 0UL) ||
            !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words || slot >= proc_slots ||
            slot >= proc_high_slot || slot >= proc_storage_runtime_v29.slots)
                return 0;
        p = &proc_table[slot];
        area = PROC_UAREA_BASE(p);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) || !area ||
            VM_PDP6_BASE(p) || (PROC_STATE(p) != PROC_SRUN &&
            PROC_STATE(p) != PROC_SLEEP && PROC_STATE(p) != PROC_STOP) ||
            mm_pure_table_v1922() != MM_OK)
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_BASE(e) == area)
                        return MM_EXTENT_TYPE(e) == MM_TYPE_KERNEL_DYNAMIC &&
                            MM_EXTENT_OWNER(e) == 01000U + slot &&
                            MM_EXTENT_WORDS(e) == PROC_UAREA_WORDS &&
                            !MM_EXTENT_PINS(e) ? p : 0;
        }
        return 0;
}

/* Recheck after returning from interruptible backing I/O, and before
 * rollback. Backing availability is NOT required to discard a failed
 * provisional target. Identity and original ownership still are. */
static int
pbr_identity_ok_v1980(unsigned int slot, const struct pbr_identity_v1980 *id)
{
        struct proc *p = pbr_process_v1980(slot, 0U);
        kword_t *u;
        if (!p || PROC_UAREA_BASE(p) != id->uarea ||
            VM_SPACE_WORDS(p) != id->words ||
            proc_swap_records[slot].state != id->backing ||
            proc_storage_runtime_v29.words + slot / PROC_STMAP_SLOTS_WORD_V28 != id->modep ||
            ((*id->modep >> ((slot % PROC_STMAP_SLOTS_WORD_V28) * 2U)) & 3UL) !=
                PROC_STORAGE_BITMAP_V1661)
                return 0;
        u = PRAM_PTR(id->uarea);
        return u[PROC_SWAP_BACKING_OFFSET] == id->backing &&
            (u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976) == id->cred &&
            u[PROC_FINALIZE_OFFSET] == id->escrow &&
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
            u[PROC_BRK_OFFSET] == id->brk &&
#endif
            !(u[PROC_FDCTL_OFFSET] & (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT));
}

#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
/* RUN/EXEC/boot initialize floor to allocated VM words. BRK retains
 * that floor and rounds only the allocation, not the current break.
 * Check the complete invariant before borrowing any alignment bits. */
static int
pbr_heap_ok_v1989(kword_t brk, kword_t words)
{
        kword_t floor = brk >> 18U, current = brk & MM_HALF_MASK;
        return floor && !(floor & 01777UL) && floor <= current &&
            current <= words && words <= MM_HALF_MASK &&
            !(words & 01777UL) && current > words - 02000UL;
}

/* PI excluded; caller has checked original identity and source length.
 * An invalid allocation address cannot be encoded as an absent target:
 * mark its field permanently invalid, with no deferred authority. */
static void
pbr_save_v1989(struct pbr_identity_v1980 *id, kword_t source,
    kword_t encoded, kword_t target, unsigned int phase)
{
        kword_t *u = PRAM_PTR(id->uarea);
        kword_t field = PBR_BRK_TAG_V1989;
        if (target > MM_HALF_MASK || (target & 01777UL))
                field |= PBR_BRK_BAD_V1989;
        else
                field |= target >> 10U;
        id->brk = PBR_HEAP_V1989(id->brk) | (field << 18U);
        id->escrow = PBR_TAG_V1989(source, encoded, phase);
        u[PROC_BRK_OFFSET] = id->brk;
        u[PROC_FINALIZE_OFFSET] = id->escrow;
}

static void
pbr_clear_v1989(const struct pbr_identity_v1980 *id)
{
        kword_t *u = PRAM_PTR(id->uarea);
        u[PROC_BRK_OFFSET] = PBR_HEAP_V1989(id->brk);
        u[PROC_FINALIZE_OFFSET] = 0UL;
}

/* Update only the phase while retaining both exact physical spans. */
static void
pbr_phase_v1989(struct pbr_identity_v1980 *id, unsigned int phase)
{
        id->escrow = (id->escrow & ~0700000UL) |
            ((kword_t)phase << PBR_PHASE_SHIFT_V1989);
        PRAM_PTR(id->uarea)[PROC_FINALIZE_OFFSET] = id->escrow;
}

/* Shared synchronous/deferred rollback. A phase authorizes only the
 * next operation on the exact previously witnessed owner and pin.
 * Revoke authority BEFORE attempting any mutation; rearm only upon
 * an exact unchanged refusal. Unknown replies or identity changes
 * must not leave a retry phase that could adopt a later replacement.
 * No I/O, allocation, decode or deferred FULL publication. */
static int
pbr_rollback_v1989(unsigned int slot, struct pbr_identity_v1980 *id,
    kword_t source, kword_t encoded, kword_t target, unsigned int phase)
{
        int owners;
        if (!pbr_identity_ok_v1980(slot, id))
                return MM_ERR_BUSY;
        pbr_phase_v1989(id, PBR_HOLD_V1989);
        if (phase == PBR_PAIR_V1989) {
                if (proc_pure_full_owners_v1917(slot, source, encoded,
                    target, id->words, 1U, 1U) != 3)
                        return MM_ERR_BUSY;
                (void)mm_unpin(target);
                if (!pbr_identity_ok_v1980(slot, id))
                        return MM_ERR_BUSY;
                if (proc_pure_full_owners_v1917(slot, source, encoded,
                    target, id->words, 1U, 0U) != 3) {
                        if (proc_pure_full_owners_v1917(slot, source, encoded,
                            target, id->words, 1U, 1U) == 3)
                                goto retry;
                        return MM_ERR_BUSY;
                }
                phase = PBR_FREE_V1989;
        }
        if (phase == PBR_FREE_V1989) {
                if (proc_pure_full_owners_v1917(slot, source, encoded,
                    target, id->words, 1U, 0U) != 3)
                        return MM_ERR_BUSY;
                (void)mm_free(target, MM_TYPE_PROCESS, slot);
                if (!pbr_identity_ok_v1980(slot, id))
                        return MM_ERR_BUSY;
                owners = proc_pure_full_owners_v1917(slot, source, encoded,
                    target, id->words, 1U, 0U);
                if (owners == 3)
                        goto retry;
                if (owners != 1)
                        return MM_ERR_BUSY;
                /* Exact old target interval absent; forget it only now. */
                target = 0UL;
                phase = PBR_SOURCE_V1989;
                pbr_save_v1989(id, source, encoded, target, PBR_HOLD_V1989);
        }
        if (phase != PBR_SOURCE_V1989 || target ||
            proc_pure_full_owners_v1917(slot, source, encoded,
                0UL, id->words, 1U, 0U) != 1)
                return MM_ERR_BUSY;
        (void)mm_unpin(source);
        if (!pbr_identity_ok_v1980(slot, id))
                return MM_ERR_BUSY;
        if (proc_pure_full_owners_v1917(slot, source, encoded,
            0UL, id->words, 0U, 0U) != 1) {
                if (proc_pure_full_owners_v1917(slot, source, encoded,
                    0UL, id->words, 1U, 0U) == 1)
                        goto retry;
                return MM_ERR_BUSY;
        }
        pbr_clear_v1989(id);
        return MM_OK;
retry:
        pbr_phase_v1989(id, phase);
        return MM_ERR_BUSY;
}

/* Reconstruct only the saved transaction, not geometry from a current
 * descriptor. The stable process/u-area and agreeing backing records
 * remain the metadata trust boundary; not an allocation generation or
 * authentication mechanism. No backing mount is required for rollback. */
int
proc_pure_bitmap_recover_v1989(unsigned int slot)
{
        kword_t saved = mach_pi_disable(), *u, source, encoded, target, skip;
        struct pbr_identity_v1980 id;
        struct proc *p = pbr_process_v1980(slot, 0U);
        unsigned int phase;
        int rc = MM_ERR_BUSY;
        if (!p)
                goto out;
        id.uarea = PROC_UAREA_BASE(p);
        u = PRAM_PTR(id.uarea);
        id.backing = proc_swap_records[slot].state;
        id.words = VM_SPACE_WORDS(p);
        id.cred = u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976;
        id.escrow = u[PROC_FINALIZE_OFFSET];
        id.brk = u[PROC_BRK_OFFSET];
        id.modep = &proc_storage_runtime_v29.words[slot / PROC_STMAP_SLOTS_WORD_V28];
        phase = (unsigned int)((id.escrow & MM_HALF_MASK) >> PBR_PHASE_SHIFT_V1989);
        source = PROC_PURE_FULL_SOURCE_V1917(id.escrow);
        encoded = id.escrow & PBR_LEN_V1989;
        target = PBR_TARGET_BASE_V1989(id.brk);
        if (!id.backing || !(id.cred & PROC_SWAP_MUTABLE_V1976) ||
            !pbr_identity_ok_v1980(slot, &id) ||
            phase < PBR_PAIR_V1989 || phase > PBR_SOURCE_V1989 ||
            (phase == PBR_SOURCE_V1989 ? target != 0UL : !target) ||
            (PBR_HEAP_FIELD_V1989(id.brk) & (PBR_BRK_TAG_V1989 | PBR_BRK_BAD_V1989))
                != PBR_BRK_TAG_V1989 ||
            !pbr_heap_ok_v1989(PBR_HEAP_V1989(id.brk), id.words) ||
            !source || (source & (
#ifdef DAIMOS_PBM_PLACE_V1985
                PDP6_PURE_SECTOR_V1604 - 1UL
#else
                VM_PDP6_ALIGN_WORDS - 1UL
#endif
                )) ||
            id.escrow != PBR_TAG_V1989(source, encoded, phase) ||
            pdp6_pure_skip_v1604(id.backing, id.words, &skip) != MM_OK ||
            encoded > skip || encoded >= id.words - skip ||
            encoded < (id.words - skip + 35UL) / 36UL)
                goto out;
        rc = pbr_rollback_v1989(slot, &id, source, encoded, target, phase);
        if (rc == MM_OK)
                PROC_CLEAR_TRANSITION(p);
out:
        mach_pi_restore(saved);
        return rc;
}
#endif

/* PI excluded. Discover only an inactive, exact unpinned owner.
 * Restore and clean retry share the K13 geometry contract. */
static int
pbr_source_v1981(unsigned int slot, kword_t words, kword_t skip,
    kword_t *sourcep, kword_t *encodedp)
{
        kword_t source, encoded, raw = words - skip;
        int i;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_OWNER(e) != slot || MM_EXTENT_TYPE(e) != MM_TYPE_PACKED_PROCESS)
                        continue;
                source = MM_EXTENT_BASE(e);
                encoded = MM_EXTENT_WORDS(e);
                if (!source || (source & (
#ifdef DAIMOS_PBM_PLACE_V1985
                    PDP6_PURE_SECTOR_V1604 - 1UL
#else
                    VM_PDP6_ALIGN_WORDS - 1UL
#endif
                    )) ||
                    encoded > skip || encoded >= raw || encoded < (raw + 35UL) / 36UL ||
                    proc_pure_full_owners_v1917(slot, source, encoded, 0UL, words, 0U, 0U) != 1)
                        return 0;
                *sourcep = source;
                *encodedp = encoded;
                return 1;
        }
        return 0;
}

/* A clean retry is still inactive, never a dispatch. Backing may be
 * unavailable. Do not clear any escrow (including old RAW/terminal
 * markers), adopt a provisional target or consume a deferred pin. */
int
proc_pure_bitmap_unlock_v1981(unsigned int slot)
{
        kword_t saved = mach_pi_disable(), source, encoded, skip, backing, words;
        struct proc *p = pbr_process_v1980(slot, 0U);
        kword_t *u, mode;
        int rc = MM_ERR_BUSY;
        if (!p)
                goto out;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        backing = proc_swap_records[slot].state;
        words = VM_SPACE_WORDS(p);
        mode = proc_storage_runtime_v29.words[slot / PROC_STMAP_SLOTS_WORD_V28];
        mode = (mode >> ((slot % PROC_STMAP_SLOTS_WORD_V28) * 2U)) & 3UL;
        if (mode != PROC_STORAGE_BITMAP_V1661 || !backing ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing || u[PROC_FINALIZE_OFFSET] ||
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
            !pbr_heap_ok_v1989(u[PROC_BRK_OFFSET], words) ||
#endif
            !(u[PROC_CRED_OFFSET] & PROC_SWAP_MUTABLE_V1976) ||
            (u[PROC_FDCTL_OFFSET] & (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)) ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK ||
            !pbr_source_v1981(slot, words, skip, &source, &encoded))
                goto out;
        PROC_CLEAR_TRANSITION(p);
        rc = MM_OK;
out:
        mach_pi_restore(saved);
        return rc;
}

int
proc_pure_bitmap_restore_v1980(unsigned int slot,
    proc_pure_ram_loader_v1391 loader)
{
        struct pbr_identity_v1980 id;
        struct proc *p;
        kword_t skip, raw, source = 0UL, encoded = 0UL, target = 0UL;
        kword_t saved, *u, mask;
        unsigned int target_pins = 0U;
        int rc, owners;

        saved = mach_pi_disable();
        rc = MM_ERR_INVAL;
        if (!loader || !(p = pbr_process_v1980(slot, 0U)))
                goto out;
        if (pram_prepare(slot, PURE_LIFE_RESTORE_V1610, &p,
            &id.backing, &id.words, &skip) != MM_OK ||
            (id.words & (VM_PDP6_ALIGN_WORDS - 1UL)))
                goto out;
        id.uarea = PROC_UAREA_BASE(p);
        u = PRAM_PTR(id.uarea);
        id.cred = u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976;
        id.escrow = 0UL;
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        id.brk = u[PROC_BRK_OFFSET];
        if (!pbr_heap_ok_v1989(id.brk, id.words))
                goto out;
#endif
        id.modep = &proc_storage_runtime_v29.words[slot / PROC_STMAP_SLOTS_WORD_V28];
        mask = 3UL << ((slot % PROC_STMAP_SLOTS_WORD_V28) * 2U);
        if (!(id.cred & PROC_SWAP_MUTABLE_V1976) || !pbr_identity_ok_v1980(slot, &id))
                goto out;
        raw = id.words - skip;
        if (!pbr_source_v1981(slot, id.words, skip, &source, &encoded))
                goto out;
        /* Combined old kind bits deliberately refuse old RAW recovery.
         * Mode3 distinguishes FULL from v1978 dirty preparation.
         * v1989 preserves the exact count and aligned target separately;
         * the old staged format records only the target in word067 RH. */
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        pbr_save_v1989(&id, source, encoded, 0UL, PBR_HOLD_V1989);
#else
        id.escrow = PROC_PURE_BITMAP_FULL_V1980(source, 0UL);
        u[PROC_FINALIZE_OFFSET] = id.escrow;
#endif
        rc = mm_pin(source);
        if (proc_pure_full_owners_v1917(slot, source, encoded, 0UL, id.words, 1U, 0U) != 1) {
                if (rc != MM_OK && pbr_identity_ok_v1980(slot, &id) &&
                    proc_pure_full_owners_v1917(slot, source, encoded, 0UL, id.words, 0U, 0U) == 1) {
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
                        pbr_clear_v1989(&id);
#else
                        u[PROC_FINALIZE_OFFSET] = 0UL;
#endif
                        goto out;
                }
                goto uncertain;
        }
        /* A zero->one witness in this uninterrupted window establishes
         * the pin even if its success reply was lost. No reclaim/I/O. */
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        if (!pbr_identity_ok_v1980(slot, &id))
                goto uncertain;
#endif
        rc = mm_alloc_aligned_noreclaim(id.words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK && !target)
                goto rollback;
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        if (!pbr_identity_ok_v1980(slot, &id))
                goto uncertain;
        pbr_save_v1989(&id, source, encoded, target ? target : 1UL, PBR_HOLD_V1989);
#else
        id.escrow = PROC_PURE_BITMAP_FULL_V1980(source,
            target && target <= MM_HALF_MASK ? target : 1UL);
        u[PROC_FINALIZE_OFFSET] = id.escrow;
#endif
        if (!target || proc_pure_full_owners_v1917(slot, source, encoded,
            target, id.words, 1U, 0U) != 3)
                goto uncertain;
        if (rc != MM_OK)
                goto rollback;
        rc = mm_pin(target);
        if (proc_pure_full_owners_v1917(slot, source, encoded,
            target, id.words, 1U, 1U) != 3) {
                if (rc != MM_OK && proc_pure_full_owners_v1917(slot, source, encoded,
                    target, id.words, 1U, 0U) == 3)
                        goto rollback;
                goto uncertain;
        }
        target_pins = 1U;
        if (!pbr_identity_ok_v1980(slot, &id))
                goto uncertain;
        mach_pi_restore(saved);
        fs_zero_words(PRAM_PTR(target), (unsigned int)skip);
        rc = loader(id.backing, target, id.uarea, id.words) ? MM_ERR_INVAL : MM_OK;
        saved = mach_pi_disable();
        if (!pbr_identity_ok_v1980(slot, &id) ||
            proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 1U, 1U) != 3)
                goto uncertain;
        if (rc != MM_OK)
                goto rollback;
        if (pure_admit_source_v1608(&proc_table[slot], id.backing) != MM_OK) {
                rc = MM_ERR_INVAL;
                goto rollback;
        }
        /* Both exact pins and TRANSITION protect disjoint physical
         * spans. Restore normal PI behavior for the long codec pass. */
        mach_pi_restore(saved);
        rc = pbmd_decode_v1977(source, encoded, target + skip, raw, PBMD_FORWARD_V1977);
        /* Same stable disjoint spans, no external callback. Compare
         * before PI exclusion so the long verification pass is also
         * interruptible. The final owner/identity proof follows both. */
        if (rc == MM_OK)
                rc = pbmd_verify_v1979(target + skip, raw, source, encoded);
        saved = mach_pi_disable();
        if (!pbr_identity_ok_v1980(slot, &id) ||
            proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 1U, 1U) != 3)
                goto uncertain;
        if (rc == PBMD_WRITE_FAULT_V1977)
                goto uncertain;
        if (rc != MM_OK)
                goto rollback;
        rc = mm_pure_bitmap_finish_v1664(slot, source, encoded, target, id.words);
        owners = proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 0U, 0U);
        if (owners == 2 && pbr_identity_ok_v1980(slot, &id)) {
                /* Original bitmap absent, exact sole APR unpinned.
                 * Lost success reply cannot undo a verified commit. */
                VM_PDP6_SET_BASE(&proc_table[slot], target);
                u[PROC_SWAP_BACKING_OFFSET] = 0UL;
                *id.modep &= ~mask;
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
                pbr_clear_v1989(&id);
#else
                u[PROC_FINALIZE_OFFSET] = 0UL;
#endif
                PROC_CLEAR_TRANSITION(&proc_table[slot]);
                rc = MM_OK;
                goto out;
        }
        if (rc == MM_OK)
                goto uncertain;
rollback:
        /* No I/O or new pin acquisitions from here. A failed operation
         * is reconciled by exact witnesses, never by its reply alone. */
        if (!pbr_identity_ok_v1980(slot, &id) ||
            proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 1U, target_pins) != (target ? 3 : 1))
                goto uncertain;
#ifdef DAIMOS_PBM_FULL_ESCROW_V1989
        {
                unsigned int phase = target ? (target_pins ? PBR_PAIR_V1989 :
                    PBR_FREE_V1989) : PBR_SOURCE_V1989;
                if (pbr_rollback_v1989(slot, &id, source, encoded, target, phase) != MM_OK)
                        goto uncertain;
        }
#else
        if (target) {
                if (target_pins)
                        (void)mm_unpin(target);
                if (proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 1U, 0U) != 3)
                        goto uncertain;
                (void)mm_free(target, MM_TYPE_PROCESS, slot);
                if (proc_pure_full_owners_v1917(slot, source, encoded, target, id.words, 1U, 0U) != 1)
                        goto uncertain;
                id.escrow = PROC_PURE_BITMAP_FULL_V1980(source, 0UL);
                u[PROC_FINALIZE_OFFSET] = id.escrow;
        }
        (void)mm_unpin(source);
        if (proc_pure_full_owners_v1917(slot, source, encoded, 0UL, id.words, 0U, 0U) != 1)
                goto uncertain;
        u[PROC_FINALIZE_OFFSET] = 0UL;
#endif
        goto out;
uncertain:
        rc = MM_ERR_BUSY;
out:
        mach_pi_restore(saved);
        return rc;
}
#ifdef DAIMOS_PBM_TERM_V1982
/* Terminal only: word 0407 is no longer a user heap bound once a
 * nonreturning finalizer owns TRANSITION. Retain the exact MM span
 * there across PREVM retry; word 067 RH remains the exit status.
 * Do not change either immutable backing identity before release. */
#define PBT_SPAN_TAG_V1982 (PROC_PURE_TERM_TAG_V1920 | PROC_PURE_KIND_MASK_V1919)
#define PBT_LIFE_V1982 (PROC_FINALIZE_MARK | PROC_FINALIZE_PREVM | PROC_FINISH_RETRY_ARMED_V1763)

extern int pdp6_pure_detach_v1609(int, struct proc *, kword_t);

static int
pbt_geometry_v1982(kword_t base, kword_t encoded, kword_t words, kword_t skip)
{
        kword_t raw = words - skip;
        return base && !(base & (
#ifdef DAIMOS_PBM_PLACE_V1985
            PDP6_PURE_SECTOR_V1604 - 1UL
#else
            VM_PDP6_ALIGN_WORDS - 1UL
#endif
            )) &&
            encoded <= skip && encoded < raw && encoded >= (raw + 35UL) / 36UL;
}

/* Called exclusively by the terminal MRES gateway after the caller
 * has excluded a returning executive stack and owns TRANSITION.
 * No backing I/O, decoding, allocation or speculative pin retirement. */
static int
pbt_detach_v1982(unsigned int slot, struct proc *p, kword_t backing)
{
        kword_t saved = mach_pi_disable(), words, skip, base, encoded, tag;
        kword_t *u, *modep, mask, shift;
        int i, owners, rc = MM_ERR_BUSY;

        if (!p || pbr_process_v1980(slot, 1U) != p || !backing ||
            proc_swap_records[slot].state != backing)
                goto out;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        words = VM_SPACE_WORDS(p);
        if ((words & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            !(u[PROC_CRED_OFFSET] & PROC_SWAP_MUTABLE_V1976) ||
            (u[PROC_FDCTL_OFFSET] & PROC_USER_MAP_BIT) ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK)
                goto out;
        shift = (slot % PROC_STMAP_SLOTS_WORD_V28) * 2U;
        mask = 3UL << shift;
        modep = &proc_storage_runtime_v29.words[slot / PROC_STMAP_SLOTS_WORD_V28];
        if (((*modep & mask) >> shift) != PROC_STORAGE_BITMAP_V1661)
                goto out;
        /* The caller normally already checked all FILE entries. Refuse
         * direct terminal use while a descriptor still owns a FILE. */
        for (i = 0; i < 020; ++i)
                if (u[PROC_FILE_TABLE_OFFSET + 2U * i])
                        goto out;
        tag = u[PROC_FINALIZE_OFFSET];
        if ((tag & PROC_FINALIZE_MARK) ? !(tag & PROC_FINALIZE_PREVM) :
            (tag & (PBT_LIFE_V1982 | MM_HALF_MASK)) != 0UL)
                goto out;
        if (tag & PBT_SPAN_TAG_V1982) {
                /* Combined kind + terminal tag distinguishes this
                 * exact-span escrow from dirty DROP/FULL and RAW. */
                if ((tag & PBT_SPAN_TAG_V1982) != PBT_SPAN_TAG_V1982)
                        goto out;
                base = u[PROC_BRK_OFFSET] & MM_HALF_MASK;
                encoded = u[PROC_BRK_OFFSET] >> 18U;
                if (!pbt_geometry_v1982(base, encoded, words, skip) ||
                    (tag & ~(PBT_LIFE_V1982 | MM_HALF_MASK)) !=
                        (PBT_SPAN_TAG_V1982 | PROC_PURE_TERM_ESCROW_V1920(base)))
                        goto out;
        } else {
                if (tag & ~(PBT_LIFE_V1982 | MM_HALF_MASK))
                        goto out;
                /* Discover once, including pinned owners: a refused
                 * free must not adopt a replacement on a later retry. */
                for (i = 0; i < mm_extent_count; ++i) {
                        const struct mm_extent *e = &mm_extents[i];
                        if (MM_EXTENT_OWNER(e) == slot && MM_EXTENT_TYPE(e) == MM_TYPE_PACKED_PROCESS)
                                break;
                }
                if (i == mm_extent_count)
                        goto out;
                base = MM_EXTENT_BASE(&mm_extents[i]);
                encoded = MM_EXTENT_WORDS(&mm_extents[i]);
                if (!pbt_geometry_v1982(base, encoded, words, skip) ||
                    !proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, base,
                        encoded, MM_EXTENT_PINS(&mm_extents[i])))
                        goto out;
                u[PROC_BRK_OFFSET] = (encoded << 18U) | base;
                tag |= PBT_SPAN_TAG_V1982 | PROC_PURE_TERM_ESCROW_V1920(base);
                u[PROC_FINALIZE_OFFSET] = tag;
        }
        owners = proc_pure_full_owners_v1917(slot, base, encoded, 0UL, words, 0U, 0U);
        if (owners == 1) {
                (void)mm_free(base, MM_TYPE_PACKED_PROCESS, slot);
                owners = proc_pure_full_owners_v1917(slot, base, encoded, 0UL, words, 0U, 0U);
        }
        if (owners != 0)
                goto out;
        /* Exact original interval absent, no other user owner. PI
         * excluded from discovery through free, witness and publish. */
        *modep &= ~mask;
        u[PROC_FINALIZE_OFFSET] = tag & (PBT_LIFE_V1982 | MM_HALF_MASK);
        u[PROC_BRK_OFFSET] = 0UL;
        rc = MM_OK;
out:
        mach_pi_restore(saved);
        return rc;
}

/* New directory terminal export; mode2 uses the unchanged RAW leaf.
 * No installed build selects this export until K15/K16 are complete. */
int
proc_pure_term_v1982(int slot, struct proc *p, kword_t backing)
{
        if (slot <= 0 || !proc_table || !proc_swap_records ||
            (unsigned int)slot >= proc_slots || (unsigned int)slot >= proc_high_slot ||
            p != &proc_table[slot] || proc_swap_records[slot].state != backing)
                return MM_ERR_INVAL;
        if (proc_storage_expect_v30((unsigned int)slot, PROC_STORAGE_PACKED) == 0)
                return pdp6_pure_detach_v1609(slot, p, backing);
        return pbt_detach_v1982((unsigned int)slot, p, backing);
}
#endif

#endif
