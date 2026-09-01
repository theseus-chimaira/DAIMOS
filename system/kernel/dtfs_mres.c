#include "fs_mres.h"
#include "dtfs.h"

int
dtfs_mres_dispatch(struct fs_mres_request *r)
{
        if (r == 0)
                return -1;
        switch ((unsigned int)r->op) {
        case FS_MRES_OP_LOOKUP:
                return dtfs_lookup(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (vnode_t *)(unsigned long)r->c);
        case FS_MRES_OP_READDIR:
                return dtfs_readdir(r->a, (unsigned int)r->b,
                    (struct vfs_dirent *)(unsigned long)r->c);
        case FS_MRES_OP_STAT:
                return dtfs_stat(r->a,
                    (struct vfs_stat *)(unsigned long)r->b);
        case FS_MRES_OP_PARENT:
                return dtfs_parent(r->a,
                    (vnode_t *)(unsigned long)r->b);
        case FS_MRES_OP_CREATE:
                return dtfs_create(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (unsigned int)r->c,
                    (vnode_t *)(unsigned long)r->d);
        case FS_MRES_OP_UNLINK:
                return dtfs_unlink(r->a,
                    (const struct vfs_name *)(unsigned long)r->b);
        case FS_MRES_OP_RENAME:
                return dtfs_rename(r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    r->c, (const struct vfs_name *)(unsigned long)r->d);
        case FS_MRES_OP_TRUNCATE:
                return dtfs_truncate(r->a, (unsigned int)r->b, r->c);
        case FS_MRES_OP_CHMOD:
                return dtfs_chmod(r->a, (unsigned int)r->b);
        case FS_MRES_OP_READ_WORDS:
                return dtfs_read_words(r->a, (unsigned int)r->b,
                    (kword_t *)(unsigned long)r->c, (unsigned int)r->d);
        case FS_MRES_OP_WRITE_WORDS:
                return dtfs_write_words(r->a, (unsigned int)r->b,
                    (const kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d, r->e);
        case FS_MRES_OP_SYNC:
                return dtfs_sync(r->a);
        case FS_MRES_OP_DTFS_BIND:
                dtfs_dtc_read_addr = (unsigned int)r->a;
                dtfs_dtc_write_addr = (unsigned int)r->b;
                return 0;
        case FS_MRES_OP_FORMAT_UNIT:
                return dtfs_format_unit((unsigned int)r->a);
        case FS_MRES_OP_MOUNT_UNIT:
                return dtfs_mount_unit((unsigned int)r->a, r->b,
                    (unsigned int)r->c,
                    (vnode_t *)(unsigned long)r->d);
        default:
                return -1;
        }
}
