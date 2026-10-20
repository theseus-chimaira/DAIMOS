/**
 * @file proc_startup_snapshot_v72.c
 * @brief Preserve only counted SIXBIT startup records while a process swaps.
 *
 * A process's 02000-word startup/stack reservation belongs to the pageable
 * user image, not its stable u-area.  MonitorFS needs argv/envp even when the
 * image is absent.  Keep one exact-size, nonmoving kernel-dynamic allocation
 * per *swapped* process, not a second process image or a fixed per-slot cache.
 *
 * Layout: [allocated words][argc,,envc][relative argv/env pointers][records].
 * Relocating pointers relative to the counted-record origin lets the existing
 * assembly S6REC renderer serve both live images and compact snapshots.
 * Only the u-area's trailing word stores the snapshot address; the three-word
 * process descriptor and on-disk swap format remain unchanged.
 */
#include "proc.h"
#include "proc_swap.h"
#include "exec.h"
#include "syscall.h"
#include "vm_pdp6.h"
#include "mm.h"
#include "mm_internal.h"
#include "fs_mres.h"

#define PROC_SNAP_OWNER_BASE_V72 02000U

/* Count dynamically resident snapshot words in generic MEMINFO accounting. */
kword_t proc_snap_words_v72;

extern void proc_swap_storage_invariant_v30(void);

/**
 * @brief Capture a complete, validated startup prefix before swap frees VM.
 * @param slot Stable process slot, with user extent pinned by swap-out.
 * @return Zero on success; -1 with no side effects when no snapshot fits.
 *
 * The no-reclaim allocator is essential: regular mm_alloc() could recurse
 * into process swapping when this routine runs inside swap reclaim itself.
 * A failed allocation aborts the swap transaction without losing metadata.
 */
int
proc_snap_take_v72(unsigned int slot)
{
        struct proc *p;
        const kword_t *src;
        kword_t *dst;
        kword_t logical;
        kword_t base;
        kword_t counts;
        kword_t cursor;
        kword_t record;
        kword_t chars;
        kword_t span;
        kword_t total;
        kword_t snapshot;
        unsigned int argc;
        unsigned int envc;
        unsigned int entries;
        unsigned int i;

        if (slot == 0U || slot >= proc_slots)
                return -1;
        p = &proc_table[slot];
        base = VM_PDP6_BASE(p);
        if (!PROC_HAS_UAREA(p) || PROC_STARTUP_SNAPSHOT_WORD(p) != 0UL ||
            base == 0UL || VM_SPACE_WORDS(p) < EXEC_DXR_STACK_WORDS ||
            !mm_is_pinned(base))
                return -1;

        logical = VM_SPACE_WORDS(p) - EXEC_DXR_STACK_WORDS;
        src = (const kword_t *)(unsigned long)(base + logical);
        counts = src[0];
        argc = (unsigned int)((counts >> 18U) & PROC_HALF_MASK);
        envc = (unsigned int)(counts & PROC_HALF_MASK);
        if (argc > SYS_RUN_ARG_MAX || envc > SYS_RUN_ENV_MAX)
                return -1;
        entries = argc + envc;
        cursor = 1UL + (kword_t)entries + (envc != 0U ? 1UL : 0UL);
        if (cursor >= EXEC_DXR_STACK_WORDS ||
            (envc != 0U && src[1U + entries] != 0UL))
                return -1;

        /* vm_space_startup writes records consecutively.  Verify the whole
         * pointer table and every count before allocating or copying it. */
        for (i = 0U; i < entries; ++i) {
                record = src[1U + i];
                if (record != logical + cursor)
                        return -1;
                chars = src[cursor];
                if (chars > 6UL * (EXEC_DXR_STACK_WORDS - cursor - 1UL))
                        return -1;
                span = 1UL + (chars + 5UL) / 6UL;
                cursor += span;
        }

        /* The first word is the allocated extent length for exact teardown
         * and MEMINFO accounting; the remaining words mirror startup data. */
        total = cursor + 1UL;
        if (mm_alloc_aligned_noreclaim(total, 1UL,
            MM_TYPE_KERNEL_DYNAMIC, PROC_SNAP_OWNER_BASE_V72 + slot,
            MM_ALLOC_LOW, &snapshot) != MM_OK)
                return -1;
        dst = (kword_t *)(unsigned long)snapshot;
        dst[0] = total;
        fs_copy_words(src, &dst[1], (unsigned int)cursor);
        for (i = 0U; i < entries; ++i)
                dst[2U + i] -= logical;
        PROC_STARTUP_SNAPSHOT_WORD(p) = snapshot;
        proc_snap_words_v72 += total;
        return 0;
}

/**
 * @brief Free the snapshot on swap-in, EXIT or aborted VM destruction.
 * Inconsistent MM ownership is fatal; publishing a leaked or dangling
 * snapshot would violate process lifecycle and swap-state invariants.
 */
void
proc_snap_drop_v72(unsigned int slot)
{
        struct proc *p;
        kword_t snapshot;
        kword_t total;

        if (slot == 0U || slot >= proc_slots)
                return;
        p = &proc_table[slot];
        if (!PROC_HAS_UAREA(p))
                return;
        snapshot = PROC_STARTUP_SNAPSHOT_WORD(p);
        if (snapshot == 0UL)
                return;
        total = *(kword_t *)(unsigned long)snapshot;
        if (mm_free(snapshot, MM_TYPE_KERNEL_DYNAMIC,
            PROC_SNAP_OWNER_BASE_V72 + slot) != MM_OK)
                proc_swap_storage_invariant_v30();
        PROC_STARTUP_SNAPSHOT_WORD(p) = 0UL;
        proc_snap_words_v72 -= total;
}
