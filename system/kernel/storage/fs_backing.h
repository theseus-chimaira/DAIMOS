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

/* Direct-device opaque encoding.  The RH remains the physical base block.
 * The LH is normally the raw unit number; one otherwise-unused unit bit tags
 * DRM236 so existing DSK270 handoffs remain binary compatible. */
#define FS_BACKING_DIRECT_DRM_TAG 0400000UL
#define FS_BACKING_DIRECT_UNIT_MASK 07UL

int fs_backing_read(struct fs_backing *backing, kword_t logical,
    kword_t *block);
int fs_backing_write(struct fs_backing *backing, kword_t logical,
    const kword_t *block);

#endif
