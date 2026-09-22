#ifndef DAIMON_FS_BACKING_H
#define DAIMON_FS_BACKING_H

#include "kcore.h"

/*
 * Filesystem-independent logical backing descriptor.
 *
 * ops packs the read callback in the left half and the write callback in
 * the right half.  Both callbacks use the compact ABI
 *
 *     callback(opaque, logical_block, block_buffer)
 *
 * and return zero on success.  opaque is interpreted only by the selected
 * lower storage adapter.  blocks is the exported logical capacity.
 *
 * The descriptor is deliberately three words so it can live inside a
 * dynamically allocated mount/provider context without a permanent table.
 */
struct fs_backing {
        kword_t ops;
        kword_t opaque;
        kword_t blocks;
};

int fs_backing_read(struct fs_backing *backing, kword_t logical,
    kword_t *block);
int fs_backing_write(struct fs_backing *backing, kword_t logical,
    const kword_t *block);

#endif
