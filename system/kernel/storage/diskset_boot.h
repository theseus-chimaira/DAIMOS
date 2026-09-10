#ifndef DAIMON_DISKSET_BOOT_H
#define DAIMON_DISKSET_BOOT_H

#include "diskset.h"

int diskset_boot_discover(kword_t *super_ap, kword_t *super_bp);
kword_t diskset_boot_blocks(void);
int diskset_boot_read(kword_t blockno, kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_boot_writable(void);
int diskset_boot_write(kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS]);
kword_t diskset_boot_log_blocks(void);
int diskset_boot_log_read(kword_t blockno,
    kword_t block[DISKSET_BLOCK_WORDS]);
int diskset_boot_log_write(kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS]);

#endif
