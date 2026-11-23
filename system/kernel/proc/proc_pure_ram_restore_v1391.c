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

/* A failed mm_free reply does not prove that the PACKED owner survived.
 * Called under PI exclusion after both original transaction pins have
 * been released. Only these two exact, unpinned owner sets are known:
 *   1: source is gone; the complete destination is the sole VM owner.
 *   0: source and destination both survive; destination may roll back.
 *  -1: anything else is uncertain and must retain both reservations.
 * Do not infer a free from a missing source alone: the full destination
 * must also be verified, and duplicate/foreign process extents fail closed.
 */
static int
pram_source_free_reply_v1839(unsigned int slot, kword_t source,
    kword_t raw, kword_t target, kword_t words)
{
        unsigned int seen = 0U;
        int i;

        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int type = MM_EXTENT_TYPE(e);
                unsigned int bit;

                /* A source or target base with an unexpected owner/type
                 * is not evidence that the original source was freed.
                 */
                if ((MM_EXTENT_BASE(e) == source ||
                     MM_EXTENT_BASE(e) == target) &&
                    (MM_EXTENT_OWNER(e) != slot ||
                     (type != MM_TYPE_PROCESS &&
                      type != MM_TYPE_PACKED_PROCESS)))
                        return -1;
                if (MM_EXTENT_OWNER(e) != slot ||
                    (type != MM_TYPE_PROCESS &&
                     type != MM_TYPE_PACKED_PROCESS))
                        continue;
                if (MM_EXTENT_PINNED(e))
                        return -1;
                if (type == MM_TYPE_PROCESS &&
                    MM_EXTENT_BASE(e) == target &&
                    MM_EXTENT_WORDS(e) == words)
                        bit = 2U;
                else if (type == MM_TYPE_PACKED_PROCESS &&
                    MM_EXTENT_BASE(e) == source &&
                    MM_EXTENT_WORDS(e) == raw)
                        bit = 1U;
                else
                        return -1;
                if (seen & bit)
                        return -1;
                seen |= bit;
        }
        if (seen == 2U)
                return 1;
        return seen == 3U ? 0 : -1;
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
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            pram_extent(slot, MM_TYPE_PACKED_PROCESS, raw, &source) !=
                MM_OK)
                return MM_ERR_INVAL;
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        source_pinned = 1;
        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                goto fail_source;
        rc = mm_pin(target);
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
        rc = mm_unpin(target);
        if (rc != MM_OK) {
                mach_pi_restore(pi_state);
                goto fail_target;
        }
        target_pinned = 0;
        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_uncertain = 1;
                mach_pi_restore(pi_state);
                goto fail_target;
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
                        mach_pi_restore(pi_state);
                        goto fail_target;
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
        PROC_CLEAR_TRANSITION(p);
        mach_pi_restore(pi_state);
        return MM_OK;

fail_target:
        /* A failed rollback is not an ordinary loader failure. Keep the
         * process TRANSITION-owned so its still-live MM extent cannot be
         * mistaken for an available VM or reclaimed by another process.
         * Never discard an MM error or release the source on uncertain
         * target ownership. No new persistent state is necessary.
         */
        if (target_pinned && mm_unpin(target) != MM_OK)
                return MM_ERR_BUSY;
        if (mm_free(target, MM_TYPE_PROCESS, slot) != MM_OK)
                return MM_ERR_BUSY;
fail_source:
        if (source_pinned && !source_uncertain &&
            mm_unpin(source) != MM_OK)
                return MM_ERR_BUSY;
        return rc;
}
