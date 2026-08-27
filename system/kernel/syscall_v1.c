#include "syscall_v1.h"
#include "proc_v1.h"
#include "procfs_v1.h"
#include "cty.h"
#include "mach_user_v1.h"
#ifdef __PDP10__
#include "kboot_v1.h"
#endif

#ifdef __PDP10__
kword_t sys_v1_total_words;
kword_t sys_v1_resident_words;
#else
static kword_t sys_v1_total_words;
static kword_t sys_v1_resident_words;
#endif

static unsigned int
sys_v1_file_flags(unsigned int flags)
{
        static const unsigned int access[4] = {
                FILE_V1_O_READ,
                FILE_V1_O_WRITE,
                FILE_V1_O_READ | FILE_V1_O_WRITE,
                FILE_V1_O_READ | FILE_V1_O_WRITE
        };

        return access[flags & 3U] |
            (flags & (SYS_V1_O_APPEND | SYS_V1_O_CREAT | SYS_V1_O_TRUNC));
}

int
sys_v1_open(unsigned int owner, const kword_t *path, unsigned int flags)
{
        return file_v1_open(owner, path, sys_v1_file_flags(flags));
}

#ifndef __PDP10__
int
sys_v1_close(unsigned int owner, int fd)
{
        return file_v1_close(owner, fd);
}
#endif

int
sys_v1_putchar(int ch)
{
#ifdef __PDP10__
        typedef int (*putchar_fn)(int);
        if (kcore_cty_putchar_v1 == 0)
                return -1;
        return (*(putchar_fn)(unsigned long)kcore_cty_putchar_v1)(ch);
#else
        return cty_putchar(ch);
#endif
}

int
sys_v1_getchar(void)
{
#ifdef __PDP10__
        typedef int (*getchar_fn)(void);
        if (kcore_cty_getchar_v1 == 0)
                return -1;
        return (*(getchar_fn)(unsigned long)kcore_cty_getchar_v1)();
#else
        return cty_getchar();
#endif
}

int
sys_v1_readchar(unsigned int owner, int fd)
{
        int rc;

        if (fd == 0)
                return sys_v1_getchar();
        rc = file_v1_readchar(owner, fd);
        return rc == FILE_V1_DEVICE_IO ? sys_v1_getchar() : rc;
}


int
sys_v1_writechar(unsigned int owner, int fd, int ch)
{
        int rc;

        if (fd == 1 || fd == 2)
                return sys_v1_putchar(ch);
        rc = file_v1_writechar(owner, fd, (unsigned int)ch & 0777U);
        if (rc == FILE_V1_DEVICE_IO)
                return sys_v1_putchar(ch);
        return rc;
}


#ifndef __PDP10__
int
sys_v1_chdir(unsigned int owner, const kword_t *path)
{
        return file_v1_chdir(owner, path);
}

int
sys_v1_getcwd(unsigned int owner, kword_t *buf, unsigned int nwords)
{
        return file_v1_getcwd(owner, buf, nwords);
}

int
sys_v1_read_words(unsigned int owner, int fd, kword_t *buf,
    unsigned int nwords)
{
        return file_v1_read_words(owner, fd, buf, nwords);
}

int
sys_v1_write_words(unsigned int owner, int fd, const kword_t *buf,
    unsigned int nwords, kword_t size_chars)
{
        return file_v1_write_words(owner, fd, buf, nwords, size_chars);
}

int
sys_v1_stat_path(unsigned int owner, const kword_t *path,
    struct vfs_v1_stat *st)
{
        return file_v1_stat_path_owner(owner, path, st);
}

int
sys_v1_dirread(unsigned int owner, int fd, struct vfs_v1_dirent *ent)
{
        return file_v1_readdir(owner, fd, ent);
}

int
sys_v1_mkdir(unsigned int owner, const kword_t *path, unsigned int mode)
{
        return file_v1_mkdir_owner(owner, path, mode);
}

int
sys_v1_unlink(unsigned int owner, const kword_t *path)
{
        return file_v1_unlink_owner(owner, path);
}

int
sys_v1_rename(unsigned int owner, const kword_t *oldpath,
    const kword_t *newpath)
{
        return file_v1_rename(owner, oldpath, newpath);
}

int
sys_v1_truncate(unsigned int owner, const kword_t *path, kword_t chars)
{
        return file_v1_truncate_owner(owner, path, chars);
}
#endif

int
sys_v1_procinfo(unsigned int slot, struct sys_v1_procinfo *info)
{
        struct proc_v1 *p;

        if (info == 0 || (p = proc_v1_get(slot)) == 0)
                return -1;
        info->pid = (kword_t)PROC_V1_PID(p);
        info->ppid = (kword_t)proc_v1_ppid(p);
        info->state = (kword_t)PROC_V1_STATE(p);
        info->words = PROC_V1_MEM_WORDS(p);
        info->comm = proc_v1_comm(p);
        return 0;
}

extern void pdp10_halt(void);

#ifndef __PDP10__
int
sys_v1_halt(void)
{
        pdp10_halt();
        return -1;
}
#endif

#ifndef __PDP10__
void
sys_v1_set_memory_bounds(kword_t total_words, kword_t resident_words)
{
        sys_v1_total_words = total_words;
        sys_v1_resident_words = resident_words;
}
#endif

int
sys_v1_meminfo(struct sys_v1_meminfo *info)
{
        struct memfs_v1 *fs;
        unsigned int i;
        kword_t proc_words;
        kword_t proc_slots;

        if (info == 0)
                return -1;
        proc_words = 0;
        proc_slots = 0;
        for (i = 0U; i < PROC_V1_NPROC; ++i) {
                if (PROC_V1_STATE(&proc_v1_table[i]) == PROC_V1_FREE)
                        continue;
                ++proc_slots;
                proc_words += PROC_V1_MEM_WORDS(&proc_v1_table[i]);
        }
        fs = file_v1_rootfs();
        info->total_words = sys_v1_total_words;
        info->resident_words = sys_v1_resident_words;
        info->process_words = proc_words;
        info->ramfs_used_words = fs == 0 ? 0 : (kword_t)fs->used_words;
        info->ramfs_capacity_words = fs == 0 ? 0 : (kword_t)fs->pool_words;
        info->process_slots_used = proc_slots;
        info->process_slots_total = PROC_V1_NPROC;
        info->file_slots_used = file_v1_used_slots();
        info->file_slots_total = FILE_V1_NFILE;
        return 0;
}

#ifndef __PDP10__
static kword_t *
sys_v1_user_words(kword_t addr)
{
        kword_t base;
        kword_t end;
        kword_t word;

        if (proc_v1_current == 0)
                return 0;
        base = PROC_V1_MEM_BASE(proc_v1_current);
        end = base + PROC_V1_MEM_WORDS(proc_v1_current);
        word = addr & PROC_V1_HALF_MASK;
        if (word < base || word >= end)
                return 0;
        return (kword_t *)(unsigned long)word;
}

int
exec_native_syscall_v1(kword_t *ac)
{
        unsigned int owner;
        unsigned int sysno;
        kword_t *p;
        kword_t *q;
        int rc;

        if (ac == 0 || proc_v1_current == 0)
                return -1;
        owner = proc_v1_slot(proc_v1_current);
        sysno = (unsigned int)(ac[1] & PROC_V1_HALF_MASK);
        switch (sysno) {
        case SYS_V1_EXIT:
                proc_v1_set_state(proc_v1_current, PROC_V1_ZOMB);
                mach_return_to_kernel_request_v1();
                rc = (int)(ac[2] & PROC_V1_HALF_MASK);
                break;
        case SYS_V1_OPEN:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_open(owner, p,
                    (unsigned int)(ac[3] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_CLOSE:
                rc = sys_v1_close(owner, (int)(ac[2] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_PUTCHAR:
                rc = sys_v1_putchar((int)(ac[2] & 0177UL));
                break;
        case SYS_V1_GETCHAR:
                rc = sys_v1_getchar();
                break;
        case SYS_V1_CHDIR:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_chdir(owner, p);
                break;
        case SYS_V1_GETCWD:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_getcwd(owner, p,
                    (unsigned int)(ac[3] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_READ_WORDS:
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_read_words(owner,
                    (int)(ac[2] & PROC_V1_HALF_MASK), p,
                    (unsigned int)(ac[4] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_WRITE_WORDS:
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_write_words(owner,
                    (int)(ac[2] & PROC_V1_HALF_MASK), p,
                    (unsigned int)(ac[4] & PROC_V1_HALF_MASK), ac[5]);
                break;
        case SYS_V1_STAT:
                p = sys_v1_user_words(ac[2]);
                q = sys_v1_user_words(ac[3]);
                rc = p == 0 || q == 0 ? -1 : sys_v1_stat_path(owner, p,
                    (struct vfs_v1_stat *)q);
                break;
        case SYS_V1_DIRREAD:
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_dirread(owner,
                    (int)(ac[2] & PROC_V1_HALF_MASK),
                    (struct vfs_v1_dirent *)p);
                break;
        case SYS_V1_MKDIR:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_mkdir(owner, p,
                    (unsigned int)(ac[3] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_UNLINK:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_unlink(owner, p);
                break;
        case SYS_V1_RENAME:
                p = sys_v1_user_words(ac[2]);
                q = sys_v1_user_words(ac[3]);
                rc = p == 0 || q == 0 ? -1 : sys_v1_rename(owner, p, q);
                break;
        case SYS_V1_TRUNCATE:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_truncate(owner, p, ac[3]);
                break;
        case SYS_V1_PROCINFO:
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_procinfo(
                    (unsigned int)(ac[2] & PROC_V1_HALF_MASK),
                    (struct sys_v1_procinfo *)p);
                break;
        case SYS_V1_MEMINFO:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_meminfo(
                    (struct sys_v1_meminfo *)p);
                break;
        case SYS_V1_READCHAR:
                rc = sys_v1_readchar(owner, (int)(ac[2] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_WRITECHAR:
                rc = sys_v1_writechar(owner,
                    (int)(ac[2] & PROC_V1_HALF_MASK),
                    (int)(ac[3] & 0777UL));
                break;
        case SYS_V1_HALT:
                rc = sys_v1_halt();
                break;
        default:
                rc = -1;
                break;
        }
        ac[1] = (kword_t)rc;
        return rc;
}
#endif
