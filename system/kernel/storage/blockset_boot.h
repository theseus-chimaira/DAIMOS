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

/* Legacy boot-label LOGSTORE range.  This remains KINIT-only; BLOCKSET has
 * no resident LOGSTORE semantics. */
kword_t blockset_boot_log_blocks(void);
int blockset_boot_log_read(kword_t blockno,
    kword_t block[BLOCKSET_BLOCK_WORDS]);
int blockset_boot_log_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS]);

#endif
