/**
 * @file proc_pure_ram_v1381.c
 * @brief Zero-copy PURE prefix release and registry-free RAM restore.
 *
 * The MM extent table is authoritative. The existing two-bit storage
 * mode distinguishes resident from inactive RAM; a RAW mutable suffix
 * is recovered by exact owner/type/word-count matching. The PURE skip
 * boundary is derivable from the saved backing and original VM size.
 * No second registry, compression, disk allocator or temporary suffix
 * allocation is needed. MM rebases the existing pinned process extent to
 * its high-address writable suffix without copying a single data word.
 *
 * v1611: DROP is linked only in the explicit experimental swapless MRES.
 * Default KCORE and the accepted disk-swap profile remain unaffected.
 */
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

#if !defined(DAIMOS_SWAPLESS_PURE_RAM_V1381) || \
    DAIMOS_SWAPLESS_PURE_RAM_V1381 != 1
#error proc_pure_ram_v1381_requires_swapless_build
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

/* PI-excluded proof of a sole user extent. A zero expected base permits
 * discovery only before a transaction starts. Independent MODULE and
 * KERNEL_DYNAMIC owner namespaces are allowed, but never overlapping.
 * The second pass checks every other physical range, including a
 * foreign descriptor preceding the discovered source in the table.
 */
kword_t
proc_pure_owner_v1918(unsigned int slot, unsigned int type,
    kword_t base, kword_t words, unsigned int pins)
{
        int i, found = -1;

        if (!words || mm_extent_count <= 0 ||
            mm_extent_count > MM_MAX_EXTENTS ||
            (unsigned int)mm_extent_count > mm_extent_capacity)
                return 0UL;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int kind = MM_EXTENT_TYPE(e);
                kword_t start = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);

                if (!start || !size || size > MM_HALF_MASK + 1UL - start)
                        return 0UL;
                if (MM_EXTENT_OWNER(e) != slot ||
                    kind == MM_TYPE_MODULE || kind == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (found >= 0 || kind != type || size != words ||
                    MM_EXTENT_PINS(e) != pins || (base && base != start))
                        return 0UL;
                found = i;
                base = start;
        }
        if (found < 0)
                return 0UL;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                kword_t start = MM_EXTENT_BASE(e);

                if (i != found && start < base + words &&
                    base < start + MM_EXTENT_WORDS(e))
                        return 0UL;
        }
        return base;
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
proc_pure_ram_drop_v1381(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, skip, source, suffix;
        kword_t pi_state;
        kword_t *u;
        int rc;

        rc = pram_prepare(slot, PURE_LIFE_DROP_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        if (proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) != 0)
                return MM_ERR_INVAL;
        source = VM_PDP6_BASE(p);
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (!source || u[PROC_SWAP_BACKING_OFFSET] ||
            (u[PROC_FINALIZE_OFFSET] &
             (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507)) ||
            (source & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)))
                return MM_ERR_INVAL;
        pi_state = mach_pi_disable();
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 0U)) {
                rc = MM_ERR_INVAL;
                goto out;
        }
        suffix = source + skip;
        /* Save the original suffix identity before any mutation. A
         * failed pin/rebase must not authorize an unrelated RAW owner
         * discovered later by the service. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_FULL_ESCROW_V1917(suffix, 0UL);
        rc = mm_pin(source);
        if (rc != MM_OK)
                goto resident_done;
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 1U))
                goto uncertain;

        /* Both shapes keep the same high end. Reconcile a reported
         * failure using the original physical addresses, never a
         * newly discovered descriptor. PI remains off through
         * rebase, mode publication and checked pin retirement. */
        rc = mm_pure_rebase_pinned_v1381(source, slot, skip, words);
        if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, suffix,
            words - skip, 1U)) {
                u[PROC_SWAP_BACKING_OFFSET] = backing;
                VM_PDP6_SET_BASE(p, 0UL);
                pram_publish(slot, PROC_STORAGE_RESIDENT,
                    PROC_STORAGE_PACKED);
                rc = mm_unpin(suffix);
                if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                    suffix, words - skip, 0U))
                        goto uncertain;
                u[PROC_FINALIZE_OFFSET] = 0UL;
                if (rc == MM_OK)
                        PROC_CLEAR_TRANSITION(p);
                goto out;
        }
        if (rc == MM_OK ||
            !proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
                words, 1U))
                goto uncertain;
        if (mm_unpin(source) != MM_OK)
                rc = MM_ERR_BUSY;
resident_done:
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 0U))
                goto uncertain;
        u[PROC_FINALIZE_OFFSET] = 0UL;
        goto out;
uncertain:
        rc = MM_ERR_BUSY;
out:
        mach_pi_restore(pi_state);
        return rc;
}

/* v1611: the resident swapless MRES only needs the zero-copy DROP.
 * The historical unbounded restore and staged terminal cleanup are
 * retained for native regression, but neither is linked into that MRES.
 * Production FULL restoration uses the capacity-bounded v1391 path;
 * production terminal cleanup is already handled by v1609 KCORE. */
#ifndef DAIMOS_PURE_DROP_ONLY_V1611
int
proc_pure_ram_restore_v1381(unsigned int slot,
    proc_pure_ram_reload_v1381 reload)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source, target;
        kword_t *u;
        int rc;
        int source_pinned = 0;
        int target_pinned = 0;
        int source_uncertain = 0;

        if (!reload)
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
            !(source = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                0UL, raw, 0U)))
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
        if (reload(backing, target, PROC_UAREA_BASE(p))) {
                rc = MM_ERR_INVAL;
                goto fail_target;
        }
        /* Decode/reload may have initialized writable words. The raw
         * inactive suffix is always authoritative and overlays them.
         */
        fs_copy_words(PRAM_PTR(source), PRAM_PTR(target + skip),
            (unsigned int)raw);
        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_uncertain = 1;
                goto fail_target;
        }
        source_pinned = 0;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                goto fail_target;

        VM_PDP6_SET_BASE(p, target);
        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
        pram_publish(slot, PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT);
        rc = mm_unpin(target);
        if (rc != MM_OK)
                return MM_ERR_BUSY;
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

fail_target:
        if (target_pinned)
                (void)mm_unpin(target);
        (void)mm_free(target, MM_TYPE_PROCESS, slot);
fail_source:
        if (source_pinned && !source_uncertain)
                (void)mm_unpin(source);
        return rc;
}

int
proc_pure_ram_discard_v1381(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, skip, source;
        kword_t *u;
        int rc;

        rc = pram_prepare(slot, PURE_LIFE_TERMINAL_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            !(source = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                0UL, words - skip, 0U)))
                return MM_ERR_INVAL;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        pram_publish(slot, PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT);
        proc_swap_records[slot].state = 0UL;
        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
        u[PROC_CRED_OFFSET] &= ~PROC_SWAP_MOUNT_EXT_BIT;
        /* Teardown owns TRANSITION until the complete slot is retired. */
        return MM_OK;
}
#endif /* !DAIMOS_PURE_DROP_ONLY_V1611 */
