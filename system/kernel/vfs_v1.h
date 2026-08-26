#ifndef DAIMON_VFS_V1_H
#define DAIMON_VFS_V1_H

#include "kcore.h"

/* Compact synthetic vnode handle: provider:6, kind:12, index:18. */
typedef kword_t vnode_v1_t;

#define VFS_V1_NODE_NONE        0UL
#define VFS_V1_PROVIDER_SHIFT   30U
#define VFS_V1_KIND_SHIFT       18U
#define VFS_V1_PROVIDER_MASK    077UL
#define VFS_V1_KIND_MASK        07777UL
#define VFS_V1_INDEX_MASK       0777777UL

#define VFS_V1_NODE(provider, kind, index) \
        ((((kword_t)(provider) & VFS_V1_PROVIDER_MASK) << \
        VFS_V1_PROVIDER_SHIFT) | \
        (((kword_t)(kind) & VFS_V1_KIND_MASK) << VFS_V1_KIND_SHIFT) | \
        ((kword_t)(index) & VFS_V1_INDEX_MASK))
#define VFS_V1_PROVIDER(node) \
        ((unsigned int)(((node) >> VFS_V1_PROVIDER_SHIFT) & \
        VFS_V1_PROVIDER_MASK))
#define VFS_V1_KIND(node) \
        ((unsigned int)(((node) >> VFS_V1_KIND_SHIFT) & VFS_V1_KIND_MASK))
#define VFS_V1_INDEX(node) \
        ((unsigned int)((node) & VFS_V1_INDEX_MASK))

#define VFS_V1_TYPE_DIR         1U
#define VFS_V1_TYPE_REG         2U
#define VFS_V1_TYPE_CHAR        3U
#define VFS_V1_TYPE_BLOCK       4U
#define VFS_V1_TYPE_MOUNTSRC    5U

#define VFS_V1_NAME_WORDS       4U
#define VFS_V1_NAME_MAX_CHARS   (VFS_V1_NAME_WORDS * 6U)

/* Compile-time PDP-10 SIXBIT packing, also usable by host tests. */
#define VFS_V1_SIXCHAR(ch)      ((kword_t)(((unsigned int)(ch) - 040U) & 077U))
#define VFS_V1_SIX6(a,b,c,d,e,f) \
        ((VFS_V1_SIXCHAR(a) << 30) | (VFS_V1_SIXCHAR(b) << 24) | \
        (VFS_V1_SIXCHAR(c) << 18) | (VFS_V1_SIXCHAR(d) << 12) | \
        (VFS_V1_SIXCHAR(e) << 6) | VFS_V1_SIXCHAR(f))

struct vfs_v1_name {
        unsigned int chars;
        kword_t words[VFS_V1_NAME_WORDS];
};

struct vfs_v1_dirent {
        struct vfs_v1_name name;
        unsigned int type;
};

struct vfs_v1_stat {
        unsigned int type;
        unsigned int mode;
        kword_t size_chars;
        kword_t size_words;
};

int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
int vfs_v1_name_set_uint(struct vfs_v1_name *name, unsigned int value);
int vfs_v1_name_get_uint(const struct vfs_v1_name *name, unsigned int *valuep);
int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);

#endif
