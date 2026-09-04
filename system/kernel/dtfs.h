#ifndef DAIMON_DTFS_H
#define DAIMON_DTFS_H

#include "vfs.h"

#define DTFS_PROVIDER        5U
#define DTFS_KIND_ROOT       1U
#define DTFS_KIND_FILE       2U
#define DTFS_FILE_SLOTS      22U
#define DTFS_NAME_MAX_CHARS  11U

/* Native-media management used by the mount/format syscalls. */
int dtfs_format_unit(unsigned int unit);
int dtfs_mount_unit(unsigned int unit, vnode_t target,
    unsigned int flags, vnode_t *rootp);

int dtfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int dtfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int dtfs_stat(vnode_t node, struct vfs_stat *st);
int dtfs_parent(vnode_t node, vnode_t *parentp);
int dtfs_create(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep);
int dtfs_unlink(vnode_t dir, const struct vfs_name *name);
int dtfs_rename(vnode_t olddir, const struct vfs_name *oldname,
    vnode_t newdir, const struct vfs_name *newname);
int dtfs_truncate(vnode_t node, unsigned int words,
    kword_t size_chars);
int dtfs_chmod(vnode_t node, unsigned int mode);
int dtfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords);
int dtfs_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars);
int dtfs_sync(vnode_t node);

/* Direct DTC veneers are patched by MINIT after STORAGE is installed. */
int dtfs_dtc_read(unsigned int unit, unsigned int block, kword_t *buf);
int dtfs_dtc_write(unsigned int unit, unsigned int block, kword_t *buf);

/*
 * Counted Type-551 transfer ABI.  The resident service keeps the historical
 * one-block entry points; callers encode (count-1)*0200 in the LH of block.
 * This costs no extra MRES export or installed service pointer.
 */
static inline int
dtfs_dtc_read_run(unsigned int unit, unsigned int block,
    unsigned int count, kword_t *buf)
{
        kword_t request;

        if (count == 0U || block > 01101U ||
            count > 01102U - block || buf == 0)
                return -1;
        request = (kword_t)block |
            ((kword_t)(count - 1U) * 0200UL << 18);
        return dtfs_dtc_read(unit, (unsigned int)request, buf);
}

static inline int
dtfs_dtc_write_run(unsigned int unit, unsigned int block,
    unsigned int count, kword_t *buf)
{
        kword_t request;

        if (count == 0U || block > 01101U ||
            count > 01102U - block || buf == 0)
                return -1;
        request = (kword_t)block |
            ((kword_t)(count - 1U) * 0200UL << 18);
        return dtfs_dtc_write(unit, (unsigned int)request, buf);
}

#endif
