#include "fs_mres.h"
#include "memfs.h"

static struct memfs memfs_mres_fs;

#define MEMFS_MOUNT_BITS ((kword_t)077UL << 24U)

static void
memfs_mount_result(vnode_t from, vnode_t *nodep)
{
        *nodep |= from & MEMFS_MOUNT_BITS;
}

int
memfs_mres_dispatch(struct fs_mres_request *r)
{
        if (r == 0)
                return -1;
        switch ((unsigned int)r->op) {
        case FS_MRES_OP_MEMFS_INIT:
                memfs_mres_fs = *(const struct memfs *)(unsigned long)r->a;
                return 0;
        case FS_MRES_OP_LOOKUP:
        {
                vnode_t *nodep;
                int rc;

                nodep = (vnode_t *)(unsigned long)r->c;
                rc = memfs_lookup(&memfs_mres_fs, r->a,
                    (const struct vfs_name *)(unsigned long)r->b, nodep);
                if (rc == 0)
                        memfs_mount_result(r->a, nodep);
                return rc;
        }
        case FS_MRES_OP_READDIR:
                return memfs_readdir(&memfs_mres_fs, r->a,
                    (unsigned int)r->b,
                    (struct vfs_dirent *)(unsigned long)r->c);
        case FS_MRES_OP_STAT:
                return memfs_stat(&memfs_mres_fs, r->a,
                    (struct vfs_stat *)(unsigned long)r->b);
        case FS_MRES_OP_PARENT:
        case FS_MRES_OP_PARENT_NAME:
        {
                vnode_t *parentp;
                int rc;

                parentp = (vnode_t *)(unsigned long)r->b;
                rc = memfs_parent(&memfs_mres_fs, r->a, parentp,
                    (unsigned int)r->op == FS_MRES_OP_PARENT ? 0 :
                    (struct vfs_name *)(unsigned long)r->c);
                if (rc == 0)
                        memfs_mount_result(r->a, parentp);
                return rc;
        }
        case FS_MRES_OP_CREATE:
        {
                vnode_t *nodep;
                int rc;

                nodep = (vnode_t *)(unsigned long)r->d;
                rc = memfs_create(&memfs_mres_fs, r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (unsigned int)r->c, nodep);
                if (rc == 0)
                        memfs_mount_result(r->a, nodep);
                return rc;
        }
        case FS_MRES_OP_MKDIR:
        {
                vnode_t *nodep;
                int rc;

                nodep = (vnode_t *)(unsigned long)r->d;
                rc = memfs_mkdir(&memfs_mres_fs, r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    (unsigned int)r->c, nodep);
                if (rc == 0)
                        memfs_mount_result(r->a, nodep);
                return rc;
        }
        case FS_MRES_OP_UNLINK:
                return memfs_unlink(&memfs_mres_fs, r->a,
                    (const struct vfs_name *)(unsigned long)r->b);
        case FS_MRES_OP_RENAME:
                return memfs_rename(&memfs_mres_fs, r->a,
                    (const struct vfs_name *)(unsigned long)r->b,
                    r->c, (const struct vfs_name *)(unsigned long)r->d);
        case FS_MRES_OP_TRUNCATE:
                return memfs_truncate_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b, r->c);
        case FS_MRES_OP_CHMOD:
                return memfs_chmod(&memfs_mres_fs, r->a,
                    (unsigned int)r->b);
        case FS_MRES_OP_READ_WORDS:
                return memfs_read_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b, (kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d);
        case FS_MRES_OP_WRITE_WORDS:
                return memfs_write_words(&memfs_mres_fs, r->a,
                    (unsigned int)r->b,
                    (const kword_t *)(unsigned long)r->c,
                    (unsigned int)r->d, r->e);
        case FS_MRES_OP_SYNC:
                return 0;
        case FS_MRES_OP_MEMFS_USAGE:
                r->a = (kword_t)memfs_mres_fs.used_words;
                r->b = (kword_t)memfs_mres_fs.pool_words;
                return 0;
        default:
                return -1;
        }
}
