/**
 * @file vm_pdp6_swap.c
 * @brief Resident PDP-6 process swap-out, swap-in, and reclaim policy.
 *
 * Swapping uses the D6FS blockset tail as raw full-sector backing. A resident
 * process's one-word record describes executable backing; on swap-out that
 * word is saved in the stable u-area and the record is reused for first-block
 * and block-count. Swap-in runs from slot-0 executive context so synchronous
 * storage I/O never suspends on a process stack whose user extent is absent.
 */
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "blockset_mres.h"
#include "d6fs_provider.h"
#include "dtfs.h"
#include "tsfs.h"
#include "memfs.h"
#include "exec.h"
#include "fs_mres.h"
#include "mm.h"
#include "storage.h"
#include "syscall.h"

#if EXEC_DXR_MAX_IMAGE_WORDS > PROC_SWAP_TEXT_MASK
#error "packed swap text field is too small for executable ABI"
#endif

struct proc_swap_record *proc_swap_records;
kword_t proc_swap_blocks_used;

/* Swap slot numbers and packed block spans are bounded positive quantities.
 * Use signed working values after unpacking so their comparisons stay compact
 * on the PDP-10; packed on-disk/in-memory fields remain unchanged. */

/** Find the first nonoverlapping free span in the configured swap tail. */
static int
proc_swap_find(kword_t block_words, kword_t *startp)
{
        long blocks;
        long total;
        long start;
        int i;

        if (block_words == 0UL || startp == 0)
                return -1;
        blocks = (long)block_words;
        total = (long)blockset_runtime_reg_call(BLOCKSET_MRES_OP_TAIL_BLOCKS,
            0UL, 0UL, 0UL);
        if (total <= 0L || blocks > total)
                return -1;
        start = 0L;
        for (;;) {
                long next;
                int conflict;

                if (start > total - blocks)
                        return -1;
                next = start;
                conflict = 0;
                for (i = 0; i < (int)proc_slots; ++i) {
                        kword_t span;
                        long first;
                        long count;
                        long end;

                        if (VM_PDP6_BASE(&proc_table[i]) != 0UL)
                                continue;
                        span = proc_swap_records[i].state;
                        count = (long)(span & MM_HALF_MASK);
                        if (count == 0L)
                                continue;
                        first = (long)((span >> 18U) & MM_HALF_MASK);
                        end = first + count;
                        if (start < end && first < start + blocks) {
                                if (end > next)
                                        next = end;
                                conflict = 1;
                        }
                }
                if (!conflict) {
                        *startp = (kword_t)start;
                        return 0;
                }
                if (next <= start)
                        return -1;
                start = next;
        }
}

/** Transfer a sector-aligned process extent to or from raw swap-tail blocks. */
static int
proc_swap_transfer_words(unsigned int op, kword_t first,
    kword_t *buf, kword_t words)
{
        kword_t blocks;

        if (words == 0UL ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;
        blocks = words / DSK_WORDS_PER_SECTOR;
        return blockset_runtime_reg_call(op, first, blocks,
            (kword_t)(unsigned long)buf) == 0 ? 0 : -1;
}

/** Pack resident executable backing and PURE-text metadata into one record word. */
int
proc_swap_attach(int slot, vnode_t backing, kword_t text_words,
    unsigned int pure)
{
        kword_t packed;
        unsigned int provider;
        unsigned int mount;
        unsigned int kind;

        provider = VFS_PROVIDER(backing);
        mount = VFS_MOUNT_ID(backing);
        kind = VFS_LOCAL_KIND(backing);
        /* Executable-backing provider IDs are deliberately contiguous.
         * Even providers (MEMFS/D6FS) use local node kind 1; odd providers
         * (DTFS/TSFS) use local file/node kind 2.  Keep the one-word swap
         * record validation compact while covering all four providers. */
        if (provider < MEMFS_PROVIDER || provider > TSFS_PROVIDER ||
            mount == 0U || mount > VFS_NMOUNT ||
            kind != 1U + (provider & 1U))
                return -1;
        if (pure == 0U)
                text_words = 0UL;
        if (text_words > PROC_SWAP_TEXT_MASK)
                return -1;
        packed = VFS_INDEX(backing);
        packed |= (kword_t)(mount - 1U) << PROC_SWAP_MOUNT_SHIFT;
        packed |= (kword_t)(provider - MEMFS_PROVIDER) <<
            PROC_SWAP_PROVIDER_SHIFT;
        packed |= text_words << PROC_SWAP_TEXT_SHIFT;
        proc_swap_records[slot].state = packed;
        return 0;
}

/** Drop resident/swapped backing state and account released swap blocks. */
void
proc_swap_detach(int slot)
{
        struct proc *p;

        p = &proc_table[slot];
        if (VM_PDP6_BASE(p) == 0UL &&
            proc_swap_records[slot].state != 0UL)
                proc_swap_blocks_used -=
                    proc_swap_records[slot].state & MM_HALF_MASK;
        proc_swap_records[slot].state = 0UL;
        if (PROC_HAS_UAREA(p))
                PROC_SWAP_BACKING_WORD(p) = 0UL;
}

/** Pin, write, free, and atomically publish one process as nonresident. */
int
proc_swap_out(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t blocks;
        kword_t first;
        kword_t resident_state;
        kword_t *mem;
        unsigned int state;

        p = &proc_table[slot];
        state = PROC_STATE(p);
        if ((state != PROC_SRUN && state != PROC_SLEEP && state != PROC_STOP) ||
            PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            PROC_USER_MAPPING_HELD(p))
                return -1;
        r = &proc_swap_records[slot];
        resident_state = r->state;
        if (resident_state == 0UL)
                return -1;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        if (base == 0UL || words == 0UL || mm_is_pinned(base) ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_pin(base) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;
        blocks = words / DSK_WORDS_PER_SECTOR;
        if (blocks == 0UL || proc_swap_find(blocks, &first) != 0 ||
            proc_swap_transfer_words(BLOCKSET_MRES_OP_TAIL_WRITE, first,
            mem, words) != 0)
                goto fail_unpin;

        /* Keep the resident executable-backing record in the already
         * resident u-area while the one-word swap record is reused for the
         * disk span.  No disk header or large executive-stack bounce buffer
         * is required. */
        PROC_SWAP_BACKING_WORD(p) = resident_state;
        if (mm_unpin(base) != MM_OK)
                goto fail_record;
        if (mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK)
                goto fail_record;
        VM_PDP6_SET_BASE(p, 0UL);
        r->state = ((first & MM_HALF_MASK) << 18U) |
            (blocks & MM_HALF_MASK);
        proc_swap_blocks_used += blocks;
        PROC_CLEAR_TRANSITION(p);
        return 0;

fail_unpin:
        (void)mm_unpin(base);
fail_record:
        PROC_SWAP_BACKING_WORD(p) = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return -1;
}

/* Run one swap-in transaction from slot-0 executive context.  Selection is
 * deliberately derived from the existing scheduler fields, so no permanent
 * request queue or per-process swap scheduling state is needed. */
/** Service the scheduler's single pending slot-0 swap-in request. */
int
proc_swap_service_one(void)
{
        struct proc *p;
        int slot;

        if ((proc_sched_cursor & PROC_SCHED_SWAP_REQUEST) == 0UL)
                return 0;
        slot = (int)(proc_sched_cursor & PROC_PGRP_MASK);
        if (slot <= 0 || slot >= (int)proc_high_slot) {
                proc_sched_cursor = (kword_t)slot;
                return 0;
        }
        p = &proc_table[slot];
        if (PROC_STATE(p) != PROC_SRUN || PROC_IS_FREE(p) ||
            VM_PDP6_BASE(p) != 0UL || proc_swap_records[slot].state == 0UL ||
            PROC_TRANSITION(p)) {
                proc_sched_cursor = (kword_t)slot;
                return 0;
        }

        /* Keep PROC_SCHED_SWAP_REQUEST set for the complete transaction.
         * PI6 treats that bit as slot-0 service busy and must not switch away
         * from the permanent idle stack while synchronous swap I/O is active. */
        if (proc_swap_in(slot) == 0) {
                proc_sched_cursor = (kword_t)slot;
                return slot;
        }
        proc_sched_cursor = (kword_t)slot;

        /* proc_swap_in already performs normal MM reclaim.  A failure after
         * that point cannot be left as an SRUN process that wins scheduling
         * forever; terminate it with the normal fatal-event cleanup path. */
        (void)proc_event_apply((unsigned int)slot, SYS_EVENT_TERM);
        return -1;
}

/** Allocate, restore, and republish one swapped process image. */
int
proc_swap_in(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t resident_state;
        kword_t first;
        kword_t blocks;
        kword_t *mem;

        p = &proc_table[slot];
        r = &proc_swap_records[slot];
        if (VM_PDP6_BASE(p) != 0UL || PROC_IS_FREE(p) ||
            r->state == 0UL || PROC_TRANSITION(p) || !PROC_HAS_UAREA(p))
                return -1;
        words = VM_SPACE_WORDS(p);
        resident_state = PROC_SWAP_BACKING_WORD(p);
        if (words == 0UL || resident_state == 0UL ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;

        first = (r->state >> 18U) & MM_HALF_MASK;
        blocks = r->state & MM_HALF_MASK;
        if (blocks == 0UL || blocks != words / DSK_WORDS_PER_SECTOR)
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_alloc_aligned(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &base) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        if (mm_pin(base) != MM_OK) {
                (void)mm_free(base, MM_TYPE_PROCESS, slot);
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;
        if (proc_swap_transfer_words(BLOCKSET_MRES_OP_TAIL_READ, first,
            mem, words) != 0)
                goto fail;

        if (mm_unpin(base) != MM_OK)
                goto fail_free;
        VM_PDP6_SET_BASE(p, base);
        proc_swap_blocks_used -= blocks;
        r->state = resident_state;
        PROC_SWAP_BACKING_WORD(p) = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return 0;

fail:
        (void)mm_unpin(base);
fail_free:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
        PROC_CLEAR_TRANSITION(p);
        return -1;
}

/** Swap victims until MM compaction can satisfy the requested free extent. */
int
proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner)
{
        int victim;

        for (;;) {
                victim = proc_swap_victim(exclude_owner);
                if (victim < 0)
                        break;
                if (proc_swap_out(victim) != 0)
                        break;
                if (mm_compact(words, alignment) == MM_OK)
                        return 0;
        }
        return -1;
}
