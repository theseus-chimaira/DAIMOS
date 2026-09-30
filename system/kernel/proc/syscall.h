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

/* UUO 077 is an operation-multiplexed extension/control call. */
#define SYS_EXT_PIPE               020U
#define SYS_EXT_MKFIFO             021U
#define SYS_EXT_EXEC               022U
#define SYS_EXT_GETTIME            023U
#define SYS_EXT_DUP2               024U
#define SYS_EXT_SEEK               032U
#define SYS_EXT_CHOWN              033U
#define SYS_EXT_RMDIR              034U
#define SYS_EXT_UTIME              035U
#define SYS_EXT_DTC_READ_BLOCK     040U
#define SYS_EXT_TSFS_MOUNT         041U
#define SYS_EXT_D6FS_MOUNT         042U
#define SYS_EXT_RTCTL              043U
#define SYS_EXT_LOGCTL             044U
#define SYS_EXT_DTC_WRITE_BLOCK    045U
#define SYS_EXT_MEMFS_MOUNT        046U

#define SYS_LOGCTL_STATUS             0U
#define SYS_LOGCTL_READ_BLOCK         1U
#define SYS_LOGCTL_WRITE_BLOCK        2U
#define SYS_LOGCTL_MTC_STATUS         3U
#define SYS_LOGCTL_MTC_WRITE          4U
#define SYS_LOGCTL_MTC_FILEMARK       5U
#define SYS_LOGCTL_MTC_REWIND         6U
#define SYS_LOGCTL_WAIT               7U
#define SYS_LOGCTL_APPEND            010U

#define SYS_RTCTL_DISABLE            0U
#define SYS_RTCTL_ENABLE             1U
#define SYS_RTCTL_YIELD              2U

#define SYS_SEEK_SET                0U
#define SYS_SEEK_CUR                1U
#define SYS_SEEK_END                2U

/* Userspace-validated TSFS mount handoff.  The transient scanner validates
 * media metadata before the kernel sees this compact runtime description. */
#define SYS_TSFS_MOUNT_WORDS         20U
#define SYS_TSFS_FILE_LOC            16U /* packed FILE runtime state */
#define SYS_TSFS_FILE_SHAPE          17U /* packed EXTENT/member-map state */
#define SYS_TSFS_EXTENT_LOC          18U /* packed EXTENT runtime state */
#define SYS_TSFS_EXTENT_SHAPE        19U /* reserved */

/* Compact process hierarchy/control operations for UUO 077. */
#define SYS_PROCCTL_GETPGRP       0U
#define SYS_PROCCTL_GETSESSION    1U
#define SYS_PROCCTL_GETDOMAIN     2U
#define SYS_PROCCTL_NEWSESSION    3U
#define SYS_PROCCTL_NEWDOMAIN     4U
#define SYS_PROCCTL_GETEVENTS     5U
#define SYS_PROCCTL_EVENT_PID     6U
#define SYS_PROCCTL_EVENT_PGRP    7U
#define SYS_PROCCTL_GETTTY       010U
#define SYS_PROCCTL_TTY_ATTACH   011U
#define SYS_PROCCTL_TTY_DETACH   012U
#define SYS_PROCCTL_TTY_GETFG    013U
#define SYS_PROCCTL_TTY_SETFG    014U
#define SYS_PROCCTL_GETUID       015U
#define SYS_PROCCTL_GETGID       016U
#define SYS_PROCCTL_SETUID       017U
#define SYS_PROCCTL_SETGID       025U
#define SYS_PROCCTL_TTY_GETMODE  026U
#define SYS_PROCCTL_TTY_SETMODE  027U
#define SYS_PROCCTL_ISATTY       030U
#define SYS_PROCCTL_UMASK        031U

#define SYS_TTY_MODE_CANONICAL   01U
#define SYS_TTY_MODE_ECHO        02U
#define SYS_TTY_MODE_SIGNALS     04U
#define SYS_TTY_MODE_COOKED \
        (SYS_TTY_MODE_CANONICAL | SYS_TTY_MODE_ECHO | SYS_TTY_MODE_SIGNALS)
#define SYS_TTY_MODE_RAW         0U

/* Compact controlling-terminal state returned by GETTTY. */
#define SYS_TTY_NO_TTY            0U
#define SYS_TTY_DETACHED          1U
#define SYS_TTY_ATTACHED_BASE     2U
#define SYS_TTY_ID_MAX           20U
#define SYS_TTY_ATTACHED(id) \
        (SYS_TTY_ATTACHED_BASE + (unsigned int)(id))
#define SYS_TTY_IS_ATTACHED(v) \
        ((unsigned int)(v) >= SYS_TTY_ATTACHED_BASE && \
        (unsigned int)(v) <= SYS_TTY_ATTACHED(SYS_TTY_ID_MAX))
#define SYS_TTY_ID(v) \
        ((unsigned int)(v) - SYS_TTY_ATTACHED_BASE)

#define SYS_EVENT_INT             0U
#define SYS_EVENT_TERM            1U
#define SYS_EVENT_HUP             2U
#define SYS_EVENT_TSTP            3U
#define SYS_EVENT_CONT            4U
#define SYS_EVENT_ALRM            5U
#define SYS_EVENT_CHLD            6U
#define SYS_EVENT_PIPE            7U
#define SYS_EVENT_COUNT           8U
#define SYS_EVENT_TARGET_MASK     0377U
#define SYS_EVENT_CODE_MASK       07U
#define SYS_EVENT_CODE_SHIFT         8U
#define SYS_EVENT_ARG(target, event) \
        ((((unsigned int)(event) & SYS_EVENT_CODE_MASK) << \
        SYS_EVENT_CODE_SHIFT) | \
        ((unsigned int)(target) & SYS_EVENT_TARGET_MASK))
#define SYS_EVENT_ARG_MASK \
        (SYS_EVENT_TARGET_MASK | (SYS_EVENT_CODE_MASK << SYS_EVENT_CODE_SHIFT))
#define SYS_EVENT_BIT(event)      (1U << (event))

#define SYS_RUN_VERSION_2         2U
#define SYS_RUN_V2_FIXED_WORDS    6U
#define SYS_RUN_V2_MIN_WORDS      8U
#define SYS_RUN_FD_MAX           16U
#define SYS_RUN_ARG_MAX          16U
#define SYS_RUN_ENV_MAX          16U
#define SYS_RUN_PATH_MAX_CHARS  102U
#define SYS_RUN_ARG_MAX_CHARS   102U
#define SYS_RUN_PGRP_INHERIT    0U
#define SYS_RUN_PGRP_NEW        1U
#define SYS_RUN_PGRP_JOIN       2U
#define SYS_RUN_FD_MAP(child_fd, parent_fd) \
        ((((kword_t)(child_fd) & 017UL) << 18U) | \
        ((kword_t)(parent_fd) & 017UL))
#define SYS_RUN_HEADER(version, words) \
        ((((kword_t)(version) & 0777777UL) << 18U) | \
        ((kword_t)(words) & 0777777UL))

#define SYS_EXEC_VERSION_1        1U
#define SYS_EXEC_V1_FIXED_WORDS   3U
#define SYS_EXEC_V1_MIN_WORDS     5U

#define SYS_WAIT_NOHANG         0001U
#define SYS_WAIT_PGRP_FLAG      0400U
#define SYS_WAIT_ID_MASK        0377U
#define SYS_WAIT_KIND_SHIFT       18U
#define SYS_WAIT_KIND_MASK         03U
#define SYS_WAIT_EXITED             1U
#define SYS_WAIT_STOPPED            2U
#define SYS_WAIT_CONTINUED          3U
#define SYS_WAIT_EVENT_FLAG     0400000U
#define SYS_WAIT_EVENT_MASK         0177U
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

/*
 * RUN V2 is self-contained: path, argc counted SIXBIT argument records,
 * envc counted SIXBIT NAME=VALUE records, then fd mappings follow this
 * fixed header inline.  No user pointers are embedded in the launch block.
 */
struct sys_run_v2 {
        kword_t version_words;
        kword_t flags;
        kword_t pgrp;
        kword_t fdmap_count;
        kword_t argc;
        kword_t envc;
        kword_t path[1];
};

struct sys_exec_v1 {
        kword_t version_words;
        kword_t argc;
        kword_t envc;
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
        kword_t memfs_used_words;
        kword_t memfs_capacity_words;
        kword_t process_slots_used;
        kword_t process_slots_total;
        kword_t file_slots_used;
        kword_t file_slots_total;
};

int proc_run_block(const struct sys_run_v2 *args, unsigned int available_words);
int proc_wait_status(unsigned int selector, kword_t *statusp, unsigned int flags);
int proc_control(unsigned int op, unsigned int arg);
int sys_procinfo(unsigned int slot, struct sys_procinfo *info);
int sys_meminfo(struct sys_meminfo *info);

/* Called by mach_user.s; consumes the fixed native syscall AC snapshot. */
int exec_native_syscall(void);

#endif
