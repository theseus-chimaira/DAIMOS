/**
 * @file monitorfs_swap_v63.c
 * @brief Cold MonitorFS startup-metadata reads for swapped processes.
 *
 * A sleeping parent is normally swapped when a child starts.  Its three-word
 * proc descriptor and stable u-area remain resident, but argv/envp live at
 * the top of the swapped user image.  Treating the missing image base as EOF
 * made /MONITOR/PROCESSES/PID/{NAME,CMDLINE,ENVIRONMENT} silently empty.
 *
 * Read the original S6REC records from BACKSTORE instead.  Only one shared
 * 0200-word block is used; no per-process metadata, extra cache, or permanent
 * data is allocated.  The first block is loaded lazily and subsequent words
 * reuse it until an actual block boundary is crossed.  This rare path is
 * deliberately separate from the compact resident-image assembler fast path.
 */
#include "monitorfs.h"
#include "proc.h"
#include "proc_swap.h"
#include "exec.h"
#include "syscall.h"
#include "bstore.h"

/* Direct writes to the shared block workspace invalidate BCACHE slot zero. */
extern void bcache_workspace_invalidate(void);

#define MFS_SWAP_BLOCK_MASK   (BSTORE_BLOCK_WORDS - 1U)
#define MFS_SWAP_S6REC_TEXT   010000000000UL
#define MFS_SWAP_INVALID     0777777777777UL
#define MFS_SWAP_COUNT_LIMIT (SYS_RUN_ARG_MAX + SYS_RUN_ENV_MAX)

struct mfs_swap_source_v63 {
        kword_t first;
        kword_t blocks;
        kword_t skip;
        kword_t image_words;
        kword_t first_record;
        kword_t cached_block;
};

/** Read one logical user word from the physical swap tail. */
static int
mfs_swap_word_v63(struct mfs_swap_source_v63 *src, kword_t logical,
    kword_t *value)
{
        kword_t offset;
        kword_t block;

        if (logical < src->skip || logical >= src->image_words)
                return -1;
        offset = logical - src->skip;
        block = offset >> 7U;
        if (block >= src->blocks)
                return -1;
        if (src->cached_block != block) {
                bcache_workspace_invalidate();
                if (backstore_read(src->first + block, 1UL,
                    fs_block_workspace) != 0)
                        return -1;
                src->cached_block = block;
        }
        *value = fs_block_workspace[offset & MFS_SWAP_BLOCK_MASK];
        return 0;
}

/** Return character INDEX of a validated, counted logical SIXBIT record. */
static int
mfs_swap_char_v63(struct mfs_swap_source_v63 *src, kword_t record,
    kword_t index, unsigned int *result)
{
        kword_t payload;

        if (mfs_swap_word_v63(src, record + 1UL + index / 6UL,
            &payload) != 0)
                return -1;
        *result = (unsigned int)((payload >>
            (30U - 6U * (unsigned int)(index % 6UL))) & 077UL);
        return 0;
}

/** Emit one serialized S6REC after checking that the complete record fits. */
static int
mfs_swap_copy_v63(struct mfs_swap_source_v63 *src, kword_t record,
    kword_t off, kword_t *buf, unsigned int capacity)
{
        kword_t count;
        kword_t words;
        kword_t i;

        if (record < src->first_record || record >= src->image_words ||
            mfs_swap_word_v63(src, record, &count) != 0 ||
            count > 6UL * (src->image_words - record - 1UL))
                return -1;
        words = 1UL + (count + 5UL) / 6UL;
        if (off != 0UL)
                return off >= words ? 0 : -1;
        if (words > (kword_t)capacity)
                return -1;
        buf[0] = MFS_SWAP_S6REC_TEXT | count;
        for (i = 1UL; i < words; ++i)
                if (mfs_swap_word_v63(src, record + i, &buf[i]) != 0)
                        return -1;
        return (int)words;
}

/** For NAME, emit basename(argv[0]), retaining the original counted SIXBIT. */
static int
mfs_swap_name_v63(struct mfs_swap_source_v63 *src, kword_t record,
    kword_t off, kword_t *buf, unsigned int capacity)
{
        kword_t count;
        kword_t start;
        kword_t length;
        kword_t words;
        kword_t i;
        unsigned int ch;

        if (off != 0UL)
                return 0;
        if (record < src->first_record || record >= src->image_words ||
            mfs_swap_word_v63(src, record, &count) != 0 ||
            count > 6UL * (src->image_words - record - 1UL))
                return -1;
        start = 0UL;
        for (i = 0UL; i < count; ++i) {
                if (mfs_swap_char_v63(src, record, i, &ch) != 0)
                        return -1;
                if (ch == 017U)             /* SIXBIT slash */
                        start = i + 1UL;
        }
        length = count - start;
        words = 1UL + (length + 5UL) / 6UL;
        if (words > (kword_t)capacity)
                return -1;
        buf[0] = MFS_SWAP_S6REC_TEXT | length;
        for (i = 1UL; i < words; ++i)
                buf[i] = 0UL;
        for (i = 0UL; i < length; ++i) {
                if (mfs_swap_char_v63(src, record, start + i,
                    &ch) != 0)
                        return -1;
                buf[1UL + i / 6UL] |= (kword_t)ch <<
                    (30U - 6U * (unsigned int)(i % 6UL));
        }
        return (int)words;
}

/**
 * @brief Read a swapped process startup leaf from its allocated BACKSTORE.
 * @param node Validated provider-3 process-file vnode.
 * @param off Serialized S6REC word offset (not character offset).
 * @param buf Caller-supplied guest destination.
 * @param capacity Destination word count.
 * @return Transferred word count, zero for EOF, -1 on any invalid state/I/O.
 *
 * The swap descriptor is stable throughout the executive syscall.  Swapping
 * and scheduling only replace executive continuations after the current
 * kernel syscall finishes; the local block-cache identity is not persistent.
 */
int
mfsproc_swapped_read_words_v63(vnode_t node, kword_t off, kword_t *buf,
    unsigned int capacity)
{
        struct mfs_swap_source_v63 src;
        struct proc *p;
        kword_t state;
        kword_t backing;
        kword_t pure;
        kword_t base;
        kword_t counts;
        kword_t argc;
        kword_t envc;
        kword_t index;
        kword_t limit;
        kword_t pointer;
        kword_t count;
        kword_t record_words;
        unsigned int slot;
        unsigned int leaf;

        if (buf == 0 || capacity == 0U || proc_table == 0 ||
            proc_swap_records == 0)
                return -1;
        slot = MONITORFS_PROCESS_ID(node);
        leaf = MONITORFS_PROCESS_LEAF(node);
        if (slot == 0U || slot >= proc_slots || leaf < 3U || leaf > 5U)
                return -1;
        p = &proc_table[slot];
        if (!PROC_HAS_UAREA(p) || (p->vm_state & PROC_HALF_MASK) != 0UL)
                return -1;
        src.image_words = (p->vm_state >> 18U) & PROC_HALF_MASK;
        if (src.image_words < EXEC_DXR_STACK_WORDS ||
            (src.image_words & MFS_SWAP_BLOCK_MASK) != 0UL)
                return -1;
        state = proc_swap_records[slot].state;
        src.first = (state >> 18U) & PROC_HALF_MASK;
        src.blocks = state & PROC_HALF_MASK;
        if (src.blocks == 0UL)
                return -1;
        backing = PROC_SWAP_BACKING_WORD(p);
        pure = (backing >> PROC_SWAP_TEXT_SHIFT) & PROC_SWAP_TEXT_MASK;
        src.skip = pure == 0UL ? 0UL :
            (pure + EXEC_USER_ORIGIN) & ~(kword_t)MFS_SWAP_BLOCK_MASK;
        if (src.skip > src.image_words ||
            ((src.image_words - src.skip) >> 7U) != src.blocks)
                return -1;
        src.cached_block = MFS_SWAP_INVALID;
        base = src.image_words - EXEC_DXR_STACK_WORDS;
        if (mfs_swap_word_v63(&src, base, &counts) != 0)
                return -1;
        argc = (counts >> 18U) & PROC_HALF_MASK;
        envc = counts & PROC_HALF_MASK;
        if (argc > SYS_RUN_ARG_MAX || envc > SYS_RUN_ENV_MAX ||
            argc + envc > MFS_SWAP_COUNT_LIMIT)
                return -1;
        src.first_record = base + 1UL + argc + envc +
            (envc != 0UL ? 1UL : 0UL);
        if (src.first_record >= src.image_words)
                return -1;
        if (leaf == MONITORFS_PROCESS_LEAF_NAME) {
                if (argc == 0UL)
                        return 0;
                if (mfs_swap_word_v63(&src, base + 1UL, &pointer) != 0)
                        return -1;
                return mfs_swap_name_v63(&src, pointer, off, buf, capacity);
        }

        /* CMDLINE and ENVIRONMENT are concatenated counted records.
         * Only complete records can be transferred by the native reader. */
        index = leaf == MONITORFS_PROCESS_LEAF_CMDLINE ? 0UL : argc;
        limit = leaf == MONITORFS_PROCESS_LEAF_CMDLINE ? argc : argc + envc;
        while (index < limit) {
                if (mfs_swap_word_v63(&src, base + 1UL + index,
                    &pointer) != 0 ||
                    pointer < src.first_record ||
                    pointer >= src.image_words ||
                    mfs_swap_word_v63(&src, pointer, &count) != 0 ||
                    count > 6UL * (src.image_words - pointer - 1UL))
                        return -1;
                record_words = 1UL + (count + 5UL) / 6UL;
                if (off < record_words)
                        return mfs_swap_copy_v63(&src, pointer, off,
                            buf, capacity);
                off -= record_words;
                ++index;
        }
        return 0;
}
