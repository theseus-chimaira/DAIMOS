#ifndef DAIMON_DISKSET_H
#define DAIMON_DISKSET_H

#include "kcore.h"

#define DISKSET_MAX_MEMBERS 4U
#define DISKSET_BLOCK_WORDS 0200U

struct diskset {
        unsigned int members;
        unsigned int unit[DISKSET_MAX_MEMBERS];
        kword_t base[DISKSET_MAX_MEMBERS];
        kword_t blocks[DISKSET_MAX_MEMBERS];
        kword_t swap_tail_blocks;
        kword_t logstore_start;
        kword_t logstore_blocks;
};

int diskset_boot_init(const struct diskset *config);
kword_t diskset_blocks(void);
int diskset_read_block(kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_write_block(kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_writable(void);

kword_t diskset_swap_blocks(void);
int diskset_swap_read(kword_t logical, kword_t count, kword_t *block);
int diskset_swap_write(kword_t logical, kword_t count, const kword_t *block);

kword_t diskset_log_blocks(void);
int diskset_log_read(kword_t blockno,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_log_write(kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS]);

#endif
