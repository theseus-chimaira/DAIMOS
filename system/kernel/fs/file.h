#ifndef DAIMON_FILE_H
#define DAIMON_FILE_H

#include "vfs.h"

#define FILE_O_READ          0001U
#define FILE_O_WRITE         0002U
#define FILE_O_APPEND        0004U
#define FILE_O_CREAT         0010U
#define FILE_O_TRUNC         0020U
#define FILE_DEVICE_IO       VFS_DEVICE_IO

#ifndef FILE_NFILE
#define FILE_NFILE           16U
#endif
#define FILE_FD_FIRST        0U
#define FILE_FD_MAX          15U

/*
 * FILE descriptors use two words.  The complete 36-bit word offset is
 * kept intact.  Descriptor metadata occupies the nine bits which are known
 * zero in every canonical DAIMOS vnode: providers, mount ids and local kinds
 * are all 0..7 even though VFS reserves six bits for each field.
 *
 * node_meta therefore contains a canonical vnode plus:
 *   LH 400000  readable
 *   LH 200000  writable
 *   LH 100000  directory
 *   LH 007040  four-bit lock-family token (the originating fd)
 *   LH 000030  regular/lock state: 0 other, 1 regular, 2 shared, 3 exclusive
 */
struct file {
        kword_t node_meta;
        kword_t offset;
};

#define FILE_NODE_VNODE_MASK       070707777777UL
#define FILE_META_READ             0400000000000UL
#define FILE_META_WRITE            0200000000000UL
#define FILE_META_DIR              0100000000000UL
#define FILE_META_OWNER_MASK       0007040000000UL
#define FILE_META_STATE_MASK       0000030000000UL
#define FILE_META_REGULAR          0000010000000UL
#define FILE_META_LOCK_SHARED      0000020000000UL
#define FILE_META_LOCK_EXCL        0000030000000UL
#define FILE_NODE(nm)              ((vnode_t)((nm) & FILE_NODE_VNODE_MASK))
/* FILE descriptors and cwd are process-private and live in the stable
 * process u-area.  KCORE retains only a pointer to the current table. */
int file_lookup_path(const kword_t *path, vnode_t *nodep);
int file_check_access(vnode_t node, unsigned int need);
int file_check_owner(vnode_t node);
int file_check_root(void);
int file_open(const kword_t *path, unsigned int flags);
int file_close(int fd);
int file_dup(int fd);
int file_dup2(int oldfd, int newfd);
kword_t file_seek(int fd, kword_t offset, unsigned int whence);
int file_lock(int fd, unsigned int op);
void file_unlock_mount(unsigned int mount_id);
void file_close_all(void);
int file_readchar(int fd);
int file_writechar(int fd, unsigned int ch);
int file_read_words(int fd, kword_t *buf, unsigned int nwords);
int file_write_words(int fd, const kword_t *buf, unsigned int nwords);
int file_readdir(int fd, struct vfs_dirent *ent);
int file_stat_path(const kword_t *path, struct vfs_stat *st);
int file_mkdir(const kword_t *path, unsigned int mode);
int file_mkfifo(const kword_t *path, unsigned int mode);
int file_symlink(const kword_t *target, const kword_t *linkpath);
int file_unlink(const kword_t *path);
int file_rmdir(const kword_t *path);
int file_truncate(const kword_t *path, kword_t words);
int file_rename(const kword_t *oldpath, const kword_t *newpath);
int file_chdir(const kword_t *path);
int file_getcwd(kword_t *buf, unsigned int nwords);
unsigned int file_used_slots(void);

#endif
