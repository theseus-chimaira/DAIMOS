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
 * NOT production-linked: a real scheduler, mount lifetime and teardown
 * owner still have to qualify every transition before activation.
 */
#include "proc_pure_ram_restore_v1391.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm_pdp6.h"
#include "exec.h"
#include "mm_internal.h"
#include "mm_pure_rebase_v1381.h"
#include "fs_mres.h"

#if !defined(DAIMOS_SWAPLESS_PURE_RAM_RESTORE_V1391) || \
    DAIMOS_SWAPLESS_PURE_RAM_RESTORE_V1391 != 1
#error proc_pure_ram_restore_v1391_requires_swapless_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PRAM_PTR(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PRAM_PTR(x) ((kword_t *)(unsigned long)(x))
#endif

#define PRAM_SECTOR 0200UL

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

/*
 * Caller already holds the process transition. The live executable
 * backing guard is supplied by proc_swap_attach and the resident/
 * nonresident guard scan. No new kernel word is needed.
 */
static int
pram_prepare(unsigned int slot, unsigned int terminal,
    struct proc **pp, kword_t *backingp, kword_t *wordsp,
    kword_t *skipp)
{
        struct proc *p;
        kword_t backing, words, text, skip;
        kword_t *u;

        if (!slot || !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p) ||
            (terminal && PROC_STATE(p) == PROC_FREE) ||
            (!terminal && PROC_STATE(p) != PROC_SLEEP &&
             PROC_STATE(p) != PROC_STOP))
                return MM_ERR_INVAL;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (!terminal && (u[PROC_FDCTL_OFFSET] &
            (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)))
                return MM_ERR_BUSY;
        backing = proc_swap_records[slot].state;
        words = VM_SPACE_WORDS(p);
        text = (backing >> PROC_SWAP_TEXT_SHIFT) & PROC_SWAP_TEXT_MASK;
        if (!text || !words || text > MM_HALF_MASK - EXEC_USER_ORIGIN)
                return MM_ERR_INVAL;
        skip = (text + EXEC_USER_ORIGIN) & ~(PRAM_SECTOR - 1UL);
        if (!skip || skip >= words)
                return MM_ERR_INVAL;
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
        int rc;
        int source_pinned = 0;
        int target_pinned = 0;
        int source_uncertain = 0;

        if (!loader)
                return MM_ERR_INVAL;
        rc = pram_prepare(slot, 0U, &p, &backing, &words, &skip);
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
