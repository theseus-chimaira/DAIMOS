#include "proc_swap.h"
#include "vm_pdp6.h"
#include "diskset_mres.h"
#include "d6fs_provider.h"
#include "dtfs.h"
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

#define PROC_SWAP_HEADER_BLOCKS 1UL

/* Swap slot numbers and packed block spans are bounded positive quantities.
 * Use signed working values after unpacking so their comparisons stay compact
 * on the PDP-10; packed on-disk/in-memory fields remain unchanged. */

static inline kword_t
proc_swap_disk_blocks(void)
{
        int rc;

        rc = diskset_runtime_reg_call(DISKSET_MRES_OP_SWAP_BLOCKS,
            0UL, 0UL, 0UL);
        return rc > 0 ? (kword_t)rc : 0UL;
}

static int
proc_swap_disk_io(unsigned int op, kword_t block, kword_t count,
    kword_t *buffer)
{
        return diskset_runtime_reg_call(op, block, count,
            (kword_t)(unsigned long)buffer);
}

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
        total = (long)proc_swap_disk_blocks();
        if (blocks > total)
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

static int
proc_swap_transfer_words(unsigned int op, kword_t first,
    kword_t *buf, kword_t words)
{
        kword_t full;
        kword_t rem;
        kword_t block[DSK_WORDS_PER_SECTOR];

        full = words / DSK_WORDS_PER_SECTOR;
        rem = words % DSK_WORDS_PER_SECTOR;
        if (full != 0UL && proc_swap_disk_io(op, first, full, buf) != 0)
                return -1;
        if (rem == 0UL)
                return 0;
        if (op == DISKSET_MRES_OP_SWAP_WRITE) {
                fs_copy_words(buf + full * DSK_WORDS_PER_SECTOR, block,
                    (unsigned int)rem);
                fs_zero_words(block + rem,
                    (unsigned int)(DSK_WORDS_PER_SECTOR - rem));
                return proc_swap_disk_io(op, first + full, 1UL, block) == 0 ?
                    0 : -1;
        }
        if (proc_swap_disk_io(op, first + full, 1UL, block) != 0)
                return -1;
        fs_copy_words(block, buf + full * DSK_WORDS_PER_SECTOR,
            (unsigned int)rem);
        return 0;
}

int
proc_swap_attach(int slot, vnode_t backing, kword_t text_words,
    unsigned int pure)
{
        kword_t packed;
        unsigned int provider;
        unsigned int mount;
        unsigned int kind;

        if (proc_swap_records == 0 || slot < 0 || slot >= (int)proc_slots ||
            backing == VFS_NODE_NONE)
                return -1;
        provider = VFS_PROVIDER(backing);
        mount = VFS_MOUNT_ID(backing);
        kind = VFS_LOCAL_KIND(backing);
        if (provider < MEMFS_PROVIDER || provider > D6FS_PROVIDER ||
            mount == 0U || mount > VFS_NMOUNT ||
            (provider == MEMFS_PROVIDER && kind != MEMFS_KIND_NODE) ||
            (provider == DTFS_PROVIDER && kind != DTFS_KIND_FILE) ||
            (provider == D6FS_PROVIDER && kind != D6FS_KIND_NODE))
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

void
proc_swap_detach(int slot)
{
        if (proc_swap_records == 0 || slot < 0 || slot >= (int)proc_slots)
                return;
        if (proc_table != 0 && VM_PDP6_BASE(&proc_table[slot]) == 0UL &&
            proc_swap_records[slot].state != 0UL)
                proc_swap_blocks_used -=
                    proc_swap_records[slot].state & MM_HALF_MASK;
        proc_swap_records[slot].state = 0UL;
}

int
proc_swap_out(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t offset;
        kword_t swap_words;
        kword_t blocks;
        kword_t first;
        kword_t text_words;
        kword_t resident_state;
        kword_t *mem;
        unsigned int state;
        int pure;

        if (proc_swap_records == 0 || proc_table == 0 || slot <= 0 ||
            slot >= (int)proc_slots || slot == (int)proc_current_slot)
                return -1;
        p = &proc_table[slot];
        state = PROC_STATE(p);
        if ((state != PROC_SRUN && state != PROC_SLEEP && state != PROC_STOP) ||
            PROC_TRANSITION(p))
                return -1;
        r = &proc_swap_records[slot];
        resident_state = r->state;
        if (resident_state == 0UL)
                return -1;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        if (base == 0UL || words == 0UL || mm_is_pinned(base))
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_pin(base) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;

        text_words = (resident_state >> PROC_SWAP_TEXT_SHIFT) &
            PROC_SWAP_TEXT_MASK;
        pure = text_words != 0UL;
        if (pure) {
                offset = (kword_t)EXEC_USER_ORIGIN + text_words;
                if ((long)offset > (long)words)
                        goto fail_unpin;
        } else {
                offset = 0UL;
        }
        swap_words = words - offset;
        blocks = PROC_SWAP_HEADER_BLOCKS +
            (swap_words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR;
        if (blocks <= PROC_SWAP_HEADER_BLOCKS ||
            proc_swap_find(blocks, &first) != 0)
                goto fail_unpin;
        if (proc_swap_transfer_words(DISKSET_MRES_OP_SWAP_WRITE, first,
            &resident_state, 1UL) != 0 ||
            proc_swap_transfer_words(DISKSET_MRES_OP_SWAP_WRITE,
            first + PROC_SWAP_HEADER_BLOCKS, mem + offset, swap_words) != 0)
                goto fail_unpin;
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
        PROC_CLEAR_TRANSITION(p);
        return -1;
}

int
proc_swap_is_swapped(int slot)
{
        struct proc *p;

        if (proc_swap_records == 0 || proc_table == 0 || slot <= 0 ||
            slot >= (int)proc_slots)
                return 0;
        p = &proc_table[slot];
        return !PROC_IS_FREE(p) && VM_PDP6_BASE(p) == 0UL &&
            proc_swap_records[slot].state != 0UL && !PROC_TRANSITION(p);
}

/* Run one swap-in transaction from slot-0 executive context.  Selection is
 * deliberately derived from the existing scheduler fields, so no permanent
 * request queue or per-process swap scheduling state is needed. */
int
proc_swap_service_one(void)
{
        struct proc *p;
        int i;
        int best;
        int best_prio;
        int prio;

        if (proc_table == 0)
                return 0;

        /* proc_select_runnable() records the process it selected in the
         * scheduler cursor before returning slot 0 for swap service.  Honor
         * that choice first so equal-priority swapped tasks retain the same
         * round-robin ordering as resident tasks. */
        best = (int)proc_sched_cursor;
        if (best > 0 && best < (int)proc_high_slot &&
            PROC_STATE(&proc_table[best]) == PROC_SRUN &&
            proc_swap_is_swapped(best)) {
                if (proc_swap_in(best) == 0)
                        return best;
                (void)proc_event_apply((unsigned int)best, SYS_EVENT_TERM);
                return -1;
        }

        best = 0;
        best_prio = 0;
        for (i = 1; i < (int)proc_high_slot; ++i) {
                p = &proc_table[i];
                if (PROC_STATE(p) != PROC_SRUN || !proc_swap_is_swapped(i))
                        continue;
                prio = (int)PROC_NICE_ENCODED(p) +
                    (int)PROC_CPU_PENALTY(p);
                if (best == 0 || prio < best_prio) {
                        best = i;
                        best_prio = prio;
                }
        }
        if (best == 0)
                return 0;
        if (proc_swap_in(best) == 0)
                return best;

        /* proc_swap_in already performs normal MM reclaim.  A failure after
         * that point cannot be left as an SRUN process that wins scheduling
         * forever; terminate it with the normal fatal-event cleanup path. */
        (void)proc_event_apply((unsigned int)best, SYS_EVENT_TERM);
        return -1;
}

int
proc_swap_in(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t text_words;
        kword_t resident_state;
        vnode_t backing;
        kword_t offset;
        kword_t swap_words;
        kword_t first;
        kword_t blocks;
        kword_t *mem;
        int pure;

        if (proc_swap_records == 0 || proc_table == 0 || slot <= 0 ||
            slot >= (int)proc_slots)
                return -1;
        p = &proc_table[slot];
        r = &proc_swap_records[slot];
        if (VM_PDP6_BASE(p) != 0UL || PROC_IS_FREE(p) ||
            r->state == 0UL || PROC_TRANSITION(p))
                return -1;
        words = VM_SPACE_WORDS(p);
        if (words == 0UL)
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
        fs_zero_words(mem, (unsigned int)words);

        first = (r->state >> 18U) & MM_HALF_MASK;
        blocks = r->state & MM_HALF_MASK;
        if (blocks <= PROC_SWAP_HEADER_BLOCKS ||
            proc_swap_transfer_words(DISKSET_MRES_OP_SWAP_READ, first,
            &resident_state, 1UL) != 0 ||
            resident_state == 0UL)
                goto fail;
        text_words = (resident_state >> PROC_SWAP_TEXT_SHIFT) &
            PROC_SWAP_TEXT_MASK;
        {
                unsigned int provider;
                unsigned int mount;
                unsigned int kind;

                provider = (unsigned int)((resident_state >>
                    PROC_SWAP_PROVIDER_SHIFT) & PROC_SWAP_PROVIDER_MASK) +
                    MEMFS_PROVIDER;
                mount = (unsigned int)((resident_state >>
                    PROC_SWAP_MOUNT_SHIFT) & PROC_SWAP_MOUNT_MASK) + 1U;
                kind = provider == DTFS_PROVIDER ? DTFS_KIND_FILE :
                    MEMFS_KIND_NODE;
                backing = VFS_NODE_PACKED(provider,
                    (mount << VFS_MOUNT_SHIFT) | kind,
                    resident_state & PROC_SWAP_INDEX_MASK);
        }
        pure = text_words != 0UL;
        if (pure) {
                if (vfs_read_words(backing, EXEC_DXR_EXT_HDR_WORDS,
                    mem + EXEC_USER_ORIGIN, (unsigned int)text_words) !=
                    (int)text_words)
                        goto fail;
                offset = (kword_t)EXEC_USER_ORIGIN + text_words;
        } else {
                offset = 0UL;
        }
        if ((long)offset > (long)words)
                goto fail;
        swap_words = words - offset;
        if (PROC_SWAP_HEADER_BLOCKS +
            (swap_words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR != blocks ||
            proc_swap_transfer_words(DISKSET_MRES_OP_SWAP_READ,
            first + PROC_SWAP_HEADER_BLOCKS, mem + offset, swap_words) != 0)
                goto fail;

        if (mm_unpin(base) != MM_OK)
                goto fail_free;
        VM_PDP6_SET_BASE(p, base);
        proc_swap_blocks_used -= blocks;
        PROC_CLEAR_TRANSITION(p);
        r->state = resident_state;
        return 0;

fail:
        (void)mm_unpin(base);
fail_free:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
        PROC_CLEAR_TRANSITION(p);
        return -1;
}

int
proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner)
{
        int victim;
        int swapped;

        (void)alignment;
        if (proc_swap_records == 0 || proc_table == 0)
                return -1;
        swapped = 0;
        for (;;) {
                victim = proc_swap_victim(exclude_owner);
                if (victim < 0)
                        break;
                if (proc_swap_out(victim) != 0)
                        break;
                swapped = 1;
                if ((long)mm_total_free() >= (long)words)
                        return 0;
        }
        return swapped ? 0 : -1;
}
