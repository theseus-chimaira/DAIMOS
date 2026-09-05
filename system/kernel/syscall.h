#ifndef DAIMON_SYSCALL_H
#define DAIMON_SYSCALL_H

#include "file.h"

#define SYS_WRITE            1U
#define SYS_EXIT             2U
#define SYS_OPEN             3U
#define SYS_READ             4U
#define SYS_CLOSE            5U
#define SYS_PUTCHAR          6U
#define SYS_GETCHAR          7U
#define SYS_CHDIR            8U
#define SYS_GETCWD           9U
#define SYS_STAT             10U
#define SYS_DIRREAD          14U
#define SYS_MKDIR            18U
#define SYS_UNLINK           20U
#define SYS_RENAME           21U
#define SYS_TRUNCATE         22U
#define SYS_READ_WORDS       31U
#define SYS_WRITE_WORDS      32U
#define SYS_PROCINFO         33U
#define SYS_MEMINFO          34U
#define SYS_READCHAR         35U
#define SYS_WRITECHAR        36U
#define SYS_HALT             37U
#define SYS_CHMOD            38U
#define SYS_DTFS_FORMAT      39U
#define SYS_DTFS_MOUNT       40U
#define SYS_UNMOUNT          41U
#define SYS_FLOCK            42U
#define SYS_DUP              43U
#define SYS_SYMLINK          44U

#define SYS_MOUNT_RW         0U
#define SYS_MOUNT_RDONLY     1U

#define SYS_DTFS_CTL_FORMAT  0U
#define SYS_DTFS_CTL_CHECK   1U
#define SYS_DTFS_TYPE_AUTO   0U
#define SYS_DTFS_TYPE_NATIVE 010U
#define SYS_DTFS_TYPE_TENEX  020U
#define SYS_DTFS_TYPE_ITS    030U
#define SYS_DTFS_TYPE_MASK   030U

#define SYS_O_RDONLY         000000U
#define SYS_O_WRONLY         000001U
#define SYS_O_RDWR           000002U
#define SYS_O_APPEND         000004U
#define SYS_O_CREAT          000010U
#define SYS_O_TRUNC          000020U
#define SYS_LOCK_SHARED      VFS_LOCK_SHARED
#define SYS_LOCK_EXCLUSIVE   VFS_LOCK_EXCLUSIVE
#define SYS_LOCK_UNLOCK      VFS_LOCK_UNLOCK
#define SYS_ERR_UNSUPPORTED  VFS_ERR_UNSUPPORTED

#define SYS_PROC_SLOTS       2U

struct sys_procinfo {
        kword_t pid;
        kword_t ppid;
        kword_t state;
        kword_t words;
        kword_t comm;
};

struct sys_meminfo {
        kword_t total_words;
        kword_t resident_words;
        kword_t process_words;
        kword_t ramfs_used_words;
        kword_t ramfs_capacity_words;
        kword_t process_slots_used;
        kword_t process_slots_total;
        kword_t file_slots_used;
        kword_t file_slots_total;
};

int sys_procinfo(unsigned int slot, struct sys_procinfo *info);
int sys_meminfo(struct sys_meminfo *info);

/* Called by mach_user.s; consumes the fixed native syscall AC snapshot. */
int exec_native_syscall(void);

#endif
