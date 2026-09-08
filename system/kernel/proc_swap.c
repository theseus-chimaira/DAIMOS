#include "proc_swap.h"
#include "diskset_mres.h"
#include "exec.h"
#include "fs_mres.h"
#include "mm.h"
#include "storage.h"

#define PROC_SWAP_META_FLAGS_MASK   077U
#define PROC_SWAP_META_HEADER_SHIFT 6U
#define PROC_SWAP_META_HEADER_MASK  07U
#define PROC_SWAP_META_STATE_SHIFT  9U
#define PROC_SWAP_META_STATE_MASK   07U

struct proc_swap_record {
        vnode_t backing;
        kword_t image_span;          /* LH text words, RH initialized image. */
        kword_t disk_span;           /* LH first SWAP block, RH block count. */
        kword_t meta;                /* flags, DXR header words, saved state. */
};

static struct proc_swap_record proc_swap_records[PROC_NPROC];
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
        for (i = 0U; i < PROC_NPROC; ++i)
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
                for (i = 0U; i < PROC_NPROC; ++i) {
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
        unsigned int i;

        full = words / DSK_WORDS_PER_SECTOR;
        rem = words % DSK_WORDS_PER_SECTOR;
        if (full != 0UL && proc_swap_disk_io(DISKSET_MRES_OP_SWAP_WRITE,
            first, full, (kword_t *)(unsigned long)src) != 0)
                return -1;
        if (rem == 0UL)
                return 0;
        for (i = 0U; (kword_t)i < rem; ++i)
                block[i] = src[full * DSK_WORDS_PER_SECTOR + i];
        while (i < DSK_WORDS_PER_SECTOR)
                block[i++] = 0UL;
        return proc_swap_disk_io(DISKSET_MRES_OP_SWAP_WRITE, first + full,
            1UL, block) == 0 ? 0 : -1;
}

static int
proc_swap_read_words(kword_t first, kword_t *dst, kword_t words)
{
        kword_t full;
        kword_t rem;
        kword_t block[DSK_WORDS_PER_SECTOR];
        unsigned int i;

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
        for (i = 0U; (kword_t)i < rem; ++i)
                dst[full * DSK_WORDS_PER_SECTOR + i] = block[i];
        return 0;
}

void
proc_swap_attach(unsigned int slot, vnode_t backing, kword_t image_words,
    kword_t text_words, unsigned int header_words, unsigned int flags)
{
        struct proc_swap_record *r;

        if (slot >= PROC_NPROC)
                return;
        r = &proc_swap_records[slot];
        r->backing = backing;
        r->image_span = ((text_words & MM_HALF_MASK) << 18U) |
            (image_words & MM_HALF_MASK);
        r->disk_span = 0UL;
        r->meta = ((kword_t)(header_words & PROC_SWAP_META_HEADER_MASK) <<
            PROC_SWAP_META_HEADER_SHIFT) |
            (kword_t)(flags & PROC_SWAP_META_FLAGS_MASK);
}

void
proc_swap_detach(unsigned int slot)
{
        if (slot >= PROC_NPROC)
                return;
        proc_swap_records[slot].backing = VFS_NODE_NONE;
        proc_swap_records[slot].image_span = 0UL;
        proc_swap_records[slot].disk_span = 0UL;
        proc_swap_records[slot].meta = 0UL;
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

        if (slot == 0U || slot >= PROC_NPROC)
                return -1;
        p = &proc_table[slot];
        state = PROC_STATE(p);
        if (state != PROC_SIDL && state != PROC_SLEEP)
                return -1;
        r = &proc_swap_records[slot];
        if (r->backing == VFS_NODE_NONE || r->disk_span != 0UL)
                return -1;
        base = PROC_MEM_BASE(p);
        words = PROC_MEM_WORDS(p);
        if (base == 0UL || words == 0UL || mm_pin(base) != MM_OK)
                return -1;
        mem = (kword_t *)(unsigned long)base;

        pure = (r->meta & PROC_SWAP_EXEC_PURE) != 0UL;
        text_words = (r->image_span >> 18U) & MM_HALF_MASK;
        if (pure) {
                offset = (kword_t)EXEC_USER_ORIGIN + text_words;
                if (offset > words) {
                        (void)mm_unpin(base);
                        return -1;
                }
        } else {
                offset = 0UL;
        }
        swap_words = words - offset;
        blocks = (swap_words + DSK_WORDS_PER_SECTOR - 1UL) /
            DSK_WORDS_PER_SECTOR;
        if (blocks == 0UL || proc_swap_find(blocks, &first) != 0) {
                (void)mm_unpin(base);
                return -1;
        }
        r->disk_span = ((first & MM_HALF_MASK) << 18U) |
            (blocks & MM_HALF_MASK);
        r->meta = (r->meta & ~((kword_t)PROC_SWAP_META_STATE_MASK <<
            PROC_SWAP_META_STATE_SHIFT)) |
            ((kword_t)state << PROC_SWAP_META_STATE_SHIFT);
        if (proc_swap_write_words(first, mem + offset, swap_words) != 0) {
                r->disk_span = 0UL;
                (void)mm_unpin(base);
                return -1;
        }
        proc_swap_words_written += swap_words;
        if (mm_unpin(base) != MM_OK ||
            mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK) {
                r->disk_span = 0UL;
                return -1;
        }
        PROC_SET_MEM_BASE(p, 0UL);
        PROC_SET_STATE(p, PROC_SSWAP);
        return 0;
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
        unsigned int header_words;
        unsigned int old_state;
        unsigned int i;
        int pure;

        if (slot == 0U || slot >= PROC_NPROC)
                return -1;
        p = &proc_table[slot];
        r = &proc_swap_records[slot];
        if (PROC_STATE(p) != PROC_SSWAP || r->disk_span == 0UL)
                return -1;
        words = PROC_MEM_WORDS(p);
        if (words == 0UL || mm_alloc_aligned(words, EXEC_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &base) != MM_OK)
                return -1;
        if (mm_pin(base) != MM_OK) {
                (void)mm_free(base, MM_TYPE_PROCESS, slot);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;
        for (i = 0U; (kword_t)i < words; ++i)
                mem[i] = 0UL;

        pure = (r->meta & PROC_SWAP_EXEC_PURE) != 0UL;
        text_words = (r->image_span >> 18U) & MM_HALF_MASK;
        image_words = r->image_span & MM_HALF_MASK;
        header_words = (unsigned int)((r->meta >>
            PROC_SWAP_META_HEADER_SHIFT) & PROC_SWAP_META_HEADER_MASK);
        first = (r->disk_span >> 18U) & MM_HALF_MASK;
        blocks = r->disk_span & MM_HALF_MASK;
        if (pure) {
                if (text_words > image_words || header_words == 0U ||
                    vfs_read_words(r->backing, header_words,
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
        old_state = (unsigned int)((r->meta >> PROC_SWAP_META_STATE_SHIFT) &
            PROC_SWAP_META_STATE_MASK);
        if (old_state != PROC_SIDL && old_state != PROC_SLEEP)
                old_state = PROC_SIDL;
        PROC_SET_MEM_BASE(p, base);
        PROC_SET_STATE(p, old_state);
        r->disk_span = 0UL;
        return 0;

fail:
        (void)mm_unpin(base);
fail_free:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
        return -1;
}

int
proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner)
{
        unsigned int i;
        int swapped;

        (void)alignment;
        swapped = 0;
        for (i = 1U; i < PROC_NPROC; ++i) {
                if (i == exclude_owner)
                        continue;
                if (PROC_STATE(&proc_table[i]) != PROC_SIDL &&
                    PROC_STATE(&proc_table[i]) != PROC_SLEEP)
                        continue;
                if (proc_swap_out(i) != 0)
                        continue;
                swapped = 1;
                if (mm_total_free() >= words)
                        return 0;
        }
        return swapped ? 0 : -1;
}

kword_t
proc_swap_used_blocks(void)
{
        return proc_swap_blocks_used();
}

kword_t
proc_swap_free_blocks(void)
{
        kword_t total;
        kword_t used;

        total = proc_swap_disk_blocks();
        used = proc_swap_blocks_used();
        return total > used ? total - used : 0UL;
}
