#ifndef DAIMON_BLOCKSET_LAYOUT_H
#define DAIMON_BLOCKSET_LAYOUT_H

#include "kcore.h"

/* Historical on-disk DBOOT extension.  The D6FSR2 magic is retained so
 * existing disk images remain compatible; ownership now belongs to BLOCKSET. */
#define BLOCKSET_BOOT_MEMBERS              4U
#define BLOCKSET_LAYOUT_MAGIC_D6FSR2       0442654636222UL
#define BLOCKSET_LAYOUT_MAGIC_WORD         006U
#define BLOCKSET_LAYOUT_RANGE_WORD         007U
#define BLOCKSET_LAYOUT_SUPER_A            010U
#define BLOCKSET_LAYOUT_SUPER_B            011U
#define BLOCKSET_LAYOUT_SWAP_TAIL          012U
#define BLOCKSET_LAYOUT_BOOTSTREAM         013U
#define BLOCKSET_LAYOUT_LOGSTORE_START     014U
#define BLOCKSET_LAYOUT_LOGSTORE_BLOCKS    015U
#define BLOCKSET_LAYOUT_BADMAP_START       016U
#define BLOCKSET_LAYOUT_BADMAP_BLOCKS      017U

struct blockset_layout {
        kword_t base;
        kword_t usable_blocks;
        kword_t super_a;
        kword_t super_b;
        kword_t swap_tail_blocks;
        kword_t bootstream_blocks;
        kword_t logstore_start;
        kword_t logstore_blocks;
        kword_t badmap_start;
        kword_t badmap_blocks;
};

#endif
