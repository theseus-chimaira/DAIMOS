#include "d6fs.h"


/* Dirent decode/validation lives in d6fs_pdp10.s; FCB validation stays in C until the compact assembly version is revalidated. */

/* d6fs_reader_init is implemented in d6fs_pdp10.s. */

/* d6fs_reader_fcb is implemented in d6fs_pdp10.s. */

/* d6fs_reader_read_words is implemented in d6fs_pdp10.s. */

/* d6fs_reader_commit_cache is implemented in d6fs_pdp10.s. */

/* d6fs_reader_write_block is implemented in d6fs_pdp10.s. */

/* d6fs_reader_zero_block is implemented in d6fs_pdp10.s. */

/* d6fs_reader_write_words is implemented in d6fs_pdp10.s. */

/* d6fs_reader_put_fcb is implemented in d6fs_pdp10.s. */

/* Compact PDP-10 bitmap helpers implemented in d6fs_pdp10.s. */
extern int d6fs_freemap_state(struct d6fs_reader *, kword_t);

/* d6fs_free_run is implemented in d6fs_pdp10.s. */


#ifndef __PDP10__
int
d6fs_fcb_decode_valid(const kword_t fcb[D6FS_FCB_WORDS],
    kword_t fs_blocks, unsigned int fcb_count, struct d6fs_fcb_info *info)
{
        unsigned int i;
        unsigned int high;
        kword_t run;
        kword_t highs;
        kword_t start;
        kword_t blocks;
        kword_t capacity;

        info->type = (unsigned int)((fcb[D6FS_FCB_META] >> 33) & 07UL);
        info->flags = (unsigned int)((fcb[D6FS_FCB_META] >> 24) & 0777UL);
        info->mode = (unsigned int)((fcb[D6FS_FCB_META] >> 12) & 07777UL);
        info->tail = (unsigned int)((fcb[D6FS_FCB_META] >> 8) & 017UL);
        info->extent_count =
            (unsigned int)((fcb[D6FS_FCB_META] >> 4) & 017UL);
        info->uid =
            (unsigned int)((fcb[D6FS_FCB_OWNER] >> 18) & D6FS_FCB_MASK);
        info->gid = (unsigned int)(fcb[D6FS_FCB_OWNER] & D6FS_FCB_MASK);
        info->size_words = fcb[D6FS_FCB_SIZE];
        info->mtime = fcb[D6FS_FCB_MTIME];
        info->parent_fcb =
            (unsigned int)((fcb[D6FS_FCB_PARENT] >> 18) & D6FS_FCB_MASK);
        if (info->type > D6FS_TYPE_FIFO ||
            info->extent_count > D6FS_EXTENTS ||
            (info->type == D6FS_TYPE_SYMLINK ? info->tail > 6U :
            info->type == D6FS_TYPE_FIFO ? info->tail != 0U :
            info->tail > 4U))
                return 0;
        if ((fcb[D6FS_FCB_META] & 017UL) |
            (fcb[D6FS_FCB_PARENT] & D6FS_FCB_MASK) |
            fcb[D6FS_FCB_RESERVED0] |
            fcb[D6FS_FCB_RESERVED0 + 1U] |
            fcb[D6FS_FCB_RESERVED0 + 2U])
                return 0;
        if (info->type == D6FS_TYPE_FREE)
                return info->extent_count == 0U && info->size_words == 0UL;
        if (info->parent_fcb >= fcb_count)
                return 0;
        if (info->type == D6FS_TYPE_FIFO &&
            (info->extent_count != 0U || info->size_words != 0UL))
                return 0;
        capacity = 0UL;
        highs = fcb[D6FS_FCB_LENHIGH];
        for (i = 0U; i < D6FS_EXTENTS; ++i) {
                high = (unsigned int)(highs & 037UL);
                highs >>= 5U;
                run = fcb[D6FS_FCB_EXTENT0 + i];
                if (i >= info->extent_count) {
                        if (run != 0UL || high != 0U)
                                return 0;
                        continue;
                }
                start = run >> 12;
                blocks = (((kword_t)high << 12) | (run & 07777UL)) + 1UL;
                if (start + blocks > fs_blocks)
                        return 0;
                capacity += blocks * (kword_t)D6FS_BLOCK_WORDS;
        }
        return info->size_words <= capacity;
}
#endif



/* d6fs_dirent_decode_valid is implemented in d6fs_pdp10.s. */

/* d6fs_file_block and d6fs_reader_read_words are implemented in d6fs_pdp10.s. */
