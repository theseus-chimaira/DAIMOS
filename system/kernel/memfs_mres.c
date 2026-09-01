#include "fs_mres.h"
#include "memfs_v1.h"
#include "file_v1.h"

static struct memfs_v1 memfs_mres_fs;

int
memfs_mres_dispatch(struct fs_mres_request *r)
{
        if (r == 0)
                return -1;
        switch ((unsigned int)r->op) {
        case FS_MRES_OP_MEMFS_INIT:
                memfs_mres_fs = *(const struct memfs_v1 *)(unsigned long)r->a;
                file_v1_root = &memfs_mres_fs;
                return 0;
        case FS_MRES_OP_LOOKUP:
                return memfs_v1_lookup(&memfs_mres_fs, r->a,
                    (const struct vfs_v1_name *)(unsigned long)r->b,
                    (vnode_v1_t *)(unsigned long)r->c);
        case FS_MRES_OP_READDIR:
                return memfs_v1_readdir(&memfs_mres_fs, r->a,
                    (unsigned int)r->b,
                    (struct vfs_v1_dirent *)(unsigned long)r->c);
        case FS_MRES_OP_STAT:
                return memfs_v1_stat(&memfs_mres_fs, r->a,
                    (struct vfs_v1_stat *)(unsigned long)r->b);
        case FS_MRES_OP_PARENT:
                return memfs_v1_parent(&memfs_mres_fs, r->a,
                    (vnode_v1_t *)(unsigned long)r->b, 0);
        case FS_MRES_OP_PARENT_NAME:
                return memfs_v1_parent(&memfs_mres_fs, r->a,
                    (vnode_v1_t *)(unsigned long)r->b,
                    (struct vfs_v1_name *)(unsigned long)r->c);
        case FS_MRES_OP_CREATE:
                return memfs_v1_create(&memfs_mres_fs, r->a,
                    (const struct vfs_v1_name *)(unsigned long)r->b,
                    (unsigned int)r->c,
                    (vnode_v1_t *)(unsigned long)r->d);
        case FS_MRES_OP_MKDIR:
                return memfs_v1_mkdir(&memfs_mres_fs, r->a,
                    (const struct vfs_v1_name *)(unsigned long)r->b,
                    (unsigned int)r->c,
                    (vnode_v1_t *)(unsigned long)r->d);
        case FS_MRES_OP_UNLINK:
                return memfs_v1_unlink(&memfs_mres_fs, r->a,
                    (const struct vfs_v1_name *)(unsigned long)r->b);
        case FS_MRES_OP_RENAME:
                return memfs_v1_rename(&memfs_mres_fs, r->a,
                    (const struct vfs_v1_name *)(unsigned long)r->b,
                    r->c, (const struct vfs_v1_name *)(unsigned long)r->d);
        case FS_MRES_OP_TRUNCATE:
                return memfs_v1_truncate_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b, r->c);
        case FS_MRES_OP_CHMOD:
                return memfs_v1_chmod(&memfs_mres_fs, r->a,
                    (unsigned int)r->b);
        case FS_MRES_OP_READ_WORDS:
                return memfs_v1_read_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b, (kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d);
        case FS_MRES_OP_WRITE_WORDS:
                return memfs_v1_write_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b,
                    (const kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d, r->e);
        case FS_MRES_OP_SYNC:
                return 0;
        default:
                return -1;
        }
}
