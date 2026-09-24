#ifndef DAIMON_BLOCKSET_BOOT_H
#define DAIMON_BLOCKSET_BOOT_H

#include "blockset.h"

/* Stage-1 currently describes at most the four DSK270 boot members. */
unsigned int blockset_boot_member_count_hint(void);
int blockset_boot_discover(kword_t *super_ap, kword_t *super_bp);
kword_t blockset_boot_blocks(void);
int blockset_boot_read(kword_t blockno, kword_t block[BLOCKSET_BLOCK_WORDS]);
int blockset_boot_writable(void);
int blockset_boot_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS]);

/* Return 1 for a validated directly addressable singleton. */
int blockset_boot_direct(unsigned int *unitp, kword_t *basep,
    kword_t *blocksp, kword_t *tailp);
int blockset_boot_member(unsigned int index, unsigned int *unitp,
    kword_t *basep, kword_t *blocksp, kword_t *tailp);
unsigned int blockset_boot_badmap_count(void);
int blockset_boot_badmap_load(kword_t *entries, unsigned int count);
kword_t *blockset_boot_badmap_staged(void);


#endif
