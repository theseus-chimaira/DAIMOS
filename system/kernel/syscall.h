#ifndef DAIMON_SYSCALL_H
#define DAIMON_SYSCALL_H

#include "file.h"

/* PDP-6 monitor-UUO ABI.  043 is the bulk character-stream write call;
 * 074..077 stay reserved for process/self-hosting extension. */
#define SYS_WRITE            1U      /* unsupported legacy generic call */
#define SYS_READ             4U      /* unsupported legacy generic call */
#define SYS_EXIT             040U
#define SYS_OPEN             041U
#define SYS_CLOSE            042U
#define SYS_WRITE_CHARS      043U
#define SYS_GETCHAR          044U
#define SYS_CHDIR            045U
#define SYS_GETCWD           046U
#define SYS_STAT             047U
#define SYS_DIRREAD          050U
#define SYS_MKDIR            051U
#define SYS_UNLINK           052U
#define SYS_RENAME           053U
#define SYS_TRUNCATE         054U
#define SYS_READ_WORDS       055U
#define SYS_WRITE_WORDS      056U
#define SYS_PROCINFO         057U
#define SYS_MEMINFO          060U
#define SYS_READCHAR         061U
#define SYS_WRITECHAR        062U
#define SYS_HALT             063U
#define SYS_CHMOD            064U
#define SYS_DTFS_FORMAT      065U
#define SYS_DTFS_MOUNT       066U
#define SYS_UNMOUNT          067U
#define SYS_FLOCK            070U
#define SYS_DUP              071U
#define SYS_SYMLINK          072U
#define SYS_NICE             073U
#define SYS_UUO_EXT_FIRST    074U
#define SYS_UUO_EXT_LAST     077U

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

#define SYS_PROC_SLOTS       256U

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
