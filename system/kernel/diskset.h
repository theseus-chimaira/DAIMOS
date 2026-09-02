#ifndef DAIMON_DISKSET_H
#define DAIMON_DISKSET_H

#include "kcore.h"

#define DISKSET_MAX_MEMBERS 8U
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

struct diskset_phys {
        unsigned int member;
        kword_t block;
};

int diskset_valid(const struct diskset *set);
kword_t diskset_blocks(const struct diskset *set);
int diskset_map_block(const struct diskset *set, kword_t logical,
    struct diskset_phys *phys);
int diskset_read_block(void *opaque, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_write_block(void *opaque, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS]);

kword_t diskset_swap_blocks(const struct diskset *set);
int diskset_swap_read(struct diskset *set, kword_t logical,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_swap_write(struct diskset *set, kword_t logical,
    const kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_log_read(struct diskset *set, kword_t blockno,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_log_write(struct diskset *set, kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS]);

struct diskset *diskset_boot_get(void);

#endif
