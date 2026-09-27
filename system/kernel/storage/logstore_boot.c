#include "logstore.h"
#include "blockset_boot.h"

static kword_t logstore_boot_start;
static kword_t logstore_boot_count;

void
logstore_boot_configure(kword_t start, kword_t blocks)
{
        logstore_boot_start = start;
        logstore_boot_count = blocks;
}

kword_t
logstore_boot_start_block(void)
{
        return logstore_boot_start;
}

kword_t
logstore_boot_blocks(void)
{
        return logstore_boot_count;
}

int
logstore_boot_read(kword_t blockno, kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (blockno >= logstore_boot_count)
                return -1;
        return blockset_boot_read(logstore_boot_start + blockno, block);
}

int
logstore_boot_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (blockno >= logstore_boot_count)
                return -1;
        return blockset_boot_write(logstore_boot_start + blockno, block);
}
