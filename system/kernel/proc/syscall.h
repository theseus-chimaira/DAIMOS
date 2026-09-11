#ifndef DAIMON_SYSCALL_H
#define DAIMON_SYSCALL_H

#include "file.h"

/* PDP-6 monitor-UUO ABI.  043 is the bulk character-stream write call.
 * 074..077 are the compact process/self-hosting extension bank. */
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
#define SYS_RUN             074U
#define SYS_WAIT            075U
#define SYS_GETPID          076U
#define SYS_PROCCTL         077U
#define SYS_UUO_EXT_FIRST   SYS_RUN
#define SYS_UUO_EXT_LAST    SYS_PROCCTL

#define SYS_RUN_VERSION_1       1U
#define SYS_RUN_V1_FIXED_WORDS  7U
#define SYS_RUN_V1_MIN_WORDS    9U
#define SYS_RUN_FD_MAX         16U
#define SYS_RUN_PATH_MAX_CHARS 102U
#define SYS_RUN_PGRP_INHERIT    0U
#define SYS_RUN_PGRP_NEW        1U
#define SYS_RUN_PGRP_JOIN       2U
#define SYS_RUN_FD_MAP(child_fd, parent_fd) \
        ((((kword_t)(child_fd) & 017UL) << 18U) | \
        ((kword_t)(parent_fd) & 017UL))
#define SYS_RUN_HEADER(version, words) \
        ((((kword_t)(version) & 0777777UL) << 18U) | \
        ((kword_t)(words) & 0777777UL))

#define SYS_WAIT_NOHANG         0001U
#define SYS_WAIT_PGRP_FLAG      0400U
#define SYS_WAIT_ID_MASK        0377U
#define SYS_WAIT_KIND_SHIFT       18U
#define SYS_WAIT_KIND_MASK         03U
#define SYS_WAIT_EXITED             1U
#define SYS_WAIT_STOPPED            2U
#define SYS_WAIT_CONTINUED          3U
#define SYS_WAIT_STATUS(kind, value) \
        ((((kword_t)(kind) & SYS_WAIT_KIND_MASK) << SYS_WAIT_KIND_SHIFT) | \
        ((kword_t)(value) & 0777777UL))
#define SYS_WAIT_STATUS_KIND(status) \
        ((unsigned int)(((status) >> SYS_WAIT_KIND_SHIFT) & SYS_WAIT_KIND_MASK))
#define SYS_WAIT_STATUS_VALUE(status) \
        ((unsigned int)((status) & 0777777UL))

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

struct sys_run_v1 {
        kword_t version_words;
        kword_t flags;
        kword_t pgrp;
        kword_t fdmap_count;
        kword_t ac1;
        kword_t ac2;
        kword_t ac3;
        kword_t path[1];
};

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

int proc_run_block(const struct sys_run_v1 *args, unsigned int available_words);
int proc_wait_status(unsigned int selector, kword_t *statusp, unsigned int flags);
int sys_procinfo(unsigned int slot, struct sys_procinfo *info);
int sys_meminfo(struct sys_meminfo *info);

/* Called by mach_user.s; consumes the fixed native syscall AC snapshot. */
int exec_native_syscall(void);

#endif
