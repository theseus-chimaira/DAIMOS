#ifndef DAIMON_DTFS_V1_H
#define DAIMON_DTFS_V1_H

#include "vfs_v1.h"

#define DTFS_V1_PROVIDER        5U
#define DTFS_V1_KIND_ROOT       1U
#define DTFS_V1_KIND_FILE       2U
#define DTFS_V1_FILE_SLOTS      22U
#define DTFS_V1_NAME_MAX_CHARS  11U

/* Native-media management used by the mount/format syscalls. */
int dtfs_v1_format_unit(unsigned int unit);
int dtfs_v1_mount_unit(unsigned int unit, vnode_v1_t target,
    unsigned int flags, vnode_v1_t *rootp);

int dtfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep);
int dtfs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent);
int dtfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st);
int dtfs_v1_parent(vnode_v1_t node, vnode_v1_t *parentp);
int dtfs_v1_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep);
int dtfs_v1_unlink(vnode_v1_t dir, const struct vfs_v1_name *name);
int dtfs_v1_rename(vnode_v1_t olddir, const struct vfs_v1_name *oldname,
    vnode_v1_t newdir, const struct vfs_v1_name *newname);
int dtfs_v1_truncate(vnode_v1_t node, unsigned int words,
    kword_t size_chars);
int dtfs_v1_chmod(vnode_v1_t node, unsigned int mode);
int dtfs_v1_read_words(vnode_v1_t node, unsigned int off, kword_t *buf,
    unsigned int nwords);
int dtfs_v1_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars);
int dtfs_v1_sync(vnode_v1_t node);

/* Filled by KINIT after the STORAGE MRES has been installed. */
extern unsigned int dtfs_v1_dtc_read_addr;
extern unsigned int dtfs_v1_dtc_write_addr;
int dtfs_v1_dtc_call(unsigned int address, unsigned int unit,
    unsigned int block, kword_t *buf);

#endif
