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

/*
 * Both the original APR extent and the shorter inactive raw suffix
 * are identified by the MM owner/type alone, with no persistent layout.
 */
static int
pram_extent(unsigned int slot, unsigned int type, kword_t words,
    kword_t *basep)
{
        int i;
        unsigned int found = 0U;
        kword_t base = 0UL;
        if (mm_extent_count <= 0 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return MM_ERR_INVAL;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int kind = MM_EXTENT_TYPE(e);
                if (MM_EXTENT_OWNER(e) != slot ||
                    (kind != MM_TYPE_PROCESS &&
                     kind != MM_TYPE_PACKED_PROCESS))
                        continue;
                if (++found != 1U || kind != type ||
                    MM_EXTENT_WORDS(e) != words ||
                    MM_EXTENT_PINNED(e) ||
                    !(base = MM_EXTENT_BASE(e)))
                        return MM_ERR_INVAL;
        }
        if (found != 1U)
                return MM_ERR_INVAL;
        *basep = base;
        return MM_OK;
}

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
            mm_extent_count <= 0 || mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return -1;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                unsigned int bit, pins;
                kword_t base = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);

                if (!base || !size || size > MM_HALF_MASK + 1UL - base)
                        return -1;
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
        if (VM_PDP6_BASE(p) ||
            (u[PROC_FINALIZE_OFFSET] &
             (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507)) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        pi_state = mach_pi_disable();
        if (pram_extent(slot, MM_TYPE_PACKED_PROCESS, raw, &source) !=
            MM_OK || source <= skip ||
            ((source - skip) & (VM_PDP6_ALIGN_WORDS - 1UL))) {
                mach_pi_restore(pi_state);
                return MM_ERR_INVAL;
        }
        if (proc_pure_full_owners_v1917(slot, source, raw, 0UL, words,
            0U, 0U) != 1) {
                mach_pi_restore(pi_state);
                return MM_ERR_BUSY;
        }
        /* Retain the exact RAW identity even if the first pin or the
         * allocation refuses. Zero target means no target was allocated;
         * a nonzero invalid address is deliberately unrecoverable. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(source, 0UL);
        rc = mm_pin(source);
        if (rc != MM_OK)
                goto fail_source_locked;
        source_pinned = 1;
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
        mach_pi_restore(pi_state);
        if (rc != MM_OK)
                goto fail_target;
        target_pinned = 1;
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
        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_uncertain = 1;
                goto fail_target_locked;
        }
        source_pinned = 0;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK) {
                int reply = pram_source_free_reply_v1839(slot, source,
                    raw, target, words);

                if (reply == 1) {
                        /* The source was freed despite its failed reply.
                         * Its exact sole successor contains the original
                         * writable suffix and the loaded immutable text.
                         */
                        rc = MM_OK;
                } else if (reply == 0) {
                        goto fail_target_locked;
                } else {
                        /* An uncertain physical owner must not be freed
                         * by rollback, even if it looks like scratch.
                         */
                        mach_pi_restore(pi_state);
                        return MM_ERR_BUSY;
                }
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
        if (source_pinned && !source_uncertain &&
            mm_unpin(source) != MM_OK)
                goto uncertain;
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
