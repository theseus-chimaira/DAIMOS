#include "proc_swap.h"
#include "diskset_mres.h"
#include "exec.h"
#include "fs_mres.h"
#include "mm.h"
#include "storage.h"

#define PROC_SWAP_TEXT_MASK  0377777UL
#define PROC_SWAP_PURE_BIT   (0400000UL << 18U)

#if EXEC_DXR_MAX_IMAGE_WORDS > PROC_SWAP_TEXT_MASK
#error "packed swap text field is too small for executable ABI"
#endif

struct proc_swap_record *proc_swap_records;
kword_t proc_swap_words_read;
kword_t proc_swap_words_written;

static kword_t
proc_swap_disk_blocks(void)
{
        struct diskset_mres_request req;
        int rc;

        req.op = DISKSET_MRES_OP_SWAP_BLOCKS;
        req.a = 0UL;
        req.b = 0UL;
        req.c = 0UL;
        rc = diskset_runtime_call(&req);
        return rc > 0 ? (kword_t)rc : 0UL;
}

static int
proc_swap_disk_io(unsigned int op, kword_t block, kword_t count,
    kword_t *buffer)
{
        struct diskset_mres_request req;

        req.op = (kword_t)op;
        req.a = block;
        req.b = count;
        req.c = (kword_t)(unsigned long)buffer;
        return diskset_runtime_call(&req);
}

static kword_t
proc_swap_blocks_used(void)
{
        kword_t used;
        unsigned int i;

        used = 0UL;
        for (i = 0U; i < proc_slots; ++i)
                used += proc_swap_records[i].disk_span & MM_HALF_MASK;
        return used;
}

static int
proc_swap_find(kword_t blocks, kword_t *startp)
{
        kword_t total;
        kword_t start;
        unsigned int i;

        if (blocks == 0UL || startp == 0)
                return -1;
        total = proc_swap_disk_blocks();
        if (blocks > total)
                return -1;
        start = 0UL;
        for (;;) {
                kword_t next;
                int conflict;

                if (start > total - blocks)
                        return -1;
                next = start;
                conflict = 0;
                for (i = 0U; i < proc_slots; ++i) {
                        kword_t span;
                        kword_t first;
                        kword_t count;
                        kword_t end;

                        span = proc_swap_records[i].disk_span;
                        count = span & MM_HALF_MASK;
                        if (count == 0UL)
                                continue;
                        first = (span >> 18U) & MM_HALF_MASK;
                        end = first + count;
                        if (start < end && first < start + blocks) {
                                if (end > next)
                                        next = end;
                                conflict = 1;
                        }
                }
                if (!conflict) {
                        *startp = start;
                        return 0;
                }
                if (next <= start)
                        return -1;
                start = next;
        }
}

static int
proc_swap_write_words(kword_t first, const kword_t *src, kword_t words)
{
        kword_t full;
        kword_t rem;
        kword_t block[DSK_WORDS_PER_SECTOR];

        full = words / DSK_WORDS_PER_SECTOR;
        rem = words % DSK_WORDS_PER_SECTOR;
        if (full != 0UL && proc_swap_disk_io(DISKSET_MRES_OP_SWAP_WRITE,
            first, full, (kword_t *)(unsigned long)src) != 0)
                return -1;
        if (rem == 0UL)
                return 0;
        fs_copy_words(src + full * DSK_WORDS_PER_SECTOR, block,
            (unsigned int)rem);
        fs_zero_words(block + rem,
            (unsigned int)(DSK_WORDS_PER_SECTOR - rem));
        return proc_swap_disk_io(DISKSET_MRES_OP_SWAP_WRITE, first + full,
            1UL, block) == 0 ? 0 : -1;
}

static int
proc_swap_read_words(kword_t first, kword_t *dst, kword_t words)
{
        kword_t full;
        kword_t rem;
        kword_t block[DSK_WORDS_PER_SECTOR];

        full = words / DSK_WORDS_PER_SECTOR;
        rem = words % DSK_WORDS_PER_SECTOR;
        if (full != 0UL && proc_swap_disk_io(DISKSET_MRES_OP_SWAP_READ,
            first, full, dst) != 0)
                return -1;
        if (rem == 0UL)
                return 0;
        if (proc_swap_disk_io(DISKSET_MRES_OP_SWAP_READ, first + full,
            1UL, block) != 0)
                return -1;
        fs_copy_words(block, dst + full * DSK_WORDS_PER_SECTOR,
            (unsigned int)rem);
        return 0;
}

void
proc_swap_attach(unsigned int slot, vnode_t backing, kword_t image_words,
    kword_t text_words, unsigned int pure)
{
        struct proc_swap_record *r;

        if (proc_swap_records == 0 || slot >= proc_slots)
                return;
        r = &proc_swap_records[slot];
        r->backing = backing;
        r->image_span = ((text_words & PROC_SWAP_TEXT_MASK) << 18U) |
            (image_words & MM_HALF_MASK);
        if (pure != 0U)
                r->image_span |= PROC_SWAP_PURE_BIT;
        r->disk_span = 0UL;
}

void
proc_swap_detach(unsigned int slot)
{
        if (proc_swap_records == 0 || slot >= proc_slots)
                return;
        proc_swap_records[slot].backing = VFS_NODE_NONE;
        proc_swap_records[slot].image_span = 0UL;
        proc_swap_records[slot].disk_span = 0UL;
}

int
proc_swap_out(unsigned int slot)
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
        kword_t *mem;
        unsigned int state;
        int pure;

        if (proc_swap_records == 0 || proc_table == 0 || slot == 0U ||
            slot >= proc_slots || slot == (unsigned int)proc_current_slot)
                return -1;
        p = &proc_table[slot];
        state = PROC_STATE(p);
        if ((state != PROC_SRUN && state != PROC_SLEEP && state != PROC_STOP) ||
            PROC_TRANSITION(p))
                return -1;
        r = &proc_swap_records[slot];
        if (r->backing == VFS_NODE_NONE || r->disk_span != 0UL)
                return -1;
        base = PROC_MEM_BASE(p);
        words = PROC_MEM_WORDS(p);
        if (base == 0UL || words == 0UL || mm_is_pinned(base))
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_pin(base) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;

        pure = (r->image_span & PROC_SWAP_PURE_BIT) != 0UL;
        text_words = (r->image_span >> 18U) & PROC_SWAP_TEXT_MASK;
        if (pure) {
                offset = (kword_t)EXEC_USER_ORIGIN + text_words;
                if (offset > words)
                        goto fail_unpin;
        } else {
                offset = 0UL;
        }
        swap_words = words - offset;
        blocks = (swap_words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR;
        if (blocks == 0UL || proc_swap_find(blocks, &first) != 0)
                goto fail_unpin;
        r->disk_span = ((first & MM_HALF_MASK) << 18U) |
            (blocks & MM_HALF_MASK);
        if (proc_swap_write_words(first, mem + offset, swap_words) != 0) {
                r->disk_span = 0UL;
                goto fail_unpin;
        }
        proc_swap_words_written += swap_words;
        if (mm_unpin(base) != MM_OK)
                goto fail_record;
        if (mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK)
                goto fail_record;
        PROC_SET_MEM_BASE(p, 0UL);
        PROC_CLEAR_TRANSITION(p);
        return 0;

fail_unpin:
        (void)mm_unpin(base);
fail_record:
        r->disk_span = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return -1;
}

int
proc_swap_in(unsigned int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t text_words;
        kword_t image_words;
        kword_t offset;
        kword_t swap_words;
        kword_t first;
        kword_t blocks;
        kword_t *mem;
        int pure;

        if (proc_swap_records == 0 || proc_table == 0 || slot == 0U ||
            slot >= proc_slots)
                return -1;
        p = &proc_table[slot];
        r = &proc_swap_records[slot];
        if (PROC_MEM_BASE(p) != 0UL || PROC_STATE(p) == PROC_FREE ||
            r->disk_span == 0UL || PROC_TRANSITION(p))
                return -1;
        words = PROC_MEM_WORDS(p);
        if (words == 0UL)
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_alloc_aligned(words, EXEC_PDP6_ALIGN_WORDS,
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

        pure = (r->image_span & PROC_SWAP_PURE_BIT) != 0UL;
        text_words = (r->image_span >> 18U) & PROC_SWAP_TEXT_MASK;
        image_words = r->image_span & MM_HALF_MASK;
        first = (r->disk_span >> 18U) & MM_HALF_MASK;
        blocks = r->disk_span & MM_HALF_MASK;
        if (pure) {
                if (text_words > image_words ||
                    vfs_read_words(r->backing, EXEC_DXR_EXT_HDR_WORDS,
                    mem + EXEC_USER_ORIGIN, (unsigned int)text_words) !=
                    (int)text_words)
                        goto fail;
                offset = (kword_t)EXEC_USER_ORIGIN + text_words;
        } else {
                offset = 0UL;
        }
        if (offset > words)
                goto fail;
        swap_words = words - offset;
        if ((swap_words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR != blocks ||
            proc_swap_read_words(first, mem + offset, swap_words) != 0)
                goto fail;
        proc_swap_words_read += swap_words;

        if (mm_unpin(base) != MM_OK)
                goto fail_free;
        PROC_SET_MEM_BASE(p, base);
        PROC_CLEAR_TRANSITION(p);
        r->disk_span = 0UL;
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
                if (proc_swap_out((unsigned int)victim) != 0)
                        break;
                swapped = 1;
                if (mm_total_free() >= words)
                        return 0;
        }
        return swapped ? 0 : -1;
}

kword_t
proc_swap_used_blocks(void)
{
        return proc_swap_records == 0 ? 0UL : proc_swap_blocks_used();
}

kword_t
proc_swap_free_blocks(void)
{
        kword_t total;
        kword_t used;

        total = proc_swap_disk_blocks();
        used = proc_swap_records == 0 ? 0UL : proc_swap_blocks_used();
        return total > used ? total - used : 0UL;
}
