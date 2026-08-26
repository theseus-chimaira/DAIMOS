#include "syscall_v1.h"
#include "proc_v1.h"

static unsigned int
sys_v1_file_flags(unsigned int flags)
{
        unsigned int f;

        f = 0U;
        if ((flags & SYS_V1_O_RDWR) == SYS_V1_O_RDWR)
                f |= FILE_V1_O_READ | FILE_V1_O_WRITE;
        else if ((flags & SYS_V1_O_WRONLY) != 0U)
                f |= FILE_V1_O_WRITE;
        else
                f |= FILE_V1_O_READ;
        if ((flags & SYS_V1_O_APPEND) != 0U)
                f |= FILE_V1_O_APPEND;
        if ((flags & SYS_V1_O_CREAT) != 0U)
                f |= FILE_V1_O_CREAT;
        if ((flags & SYS_V1_O_TRUNC) != 0U)
                f |= FILE_V1_O_TRUNC;
        return f;
}

int
sys_v1_open(unsigned int owner, const kword_t *path, unsigned int flags)
{
        return file_v1_open(owner, path, sys_v1_file_flags(flags));
}

int
sys_v1_close(unsigned int owner, int fd)
{
        return file_v1_close(owner, fd);
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
sys_v1_stat_path(const kword_t *path, struct sys_v1_stat *st)
{
        struct vfs_v1_stat vst;

        if (st == 0 || file_v1_stat_path(path, &vst) != 0)
                return -1;
        st->type = (kword_t)vst.type;
        st->size_chars = vst.size_chars;
        st->size_words = vst.size_words;
        st->mode = (kword_t)vst.mode;
        return 0;
}

int
sys_v1_dirread(unsigned int owner, int fd, struct sys_v1_dirent *ent)
{
        struct vfs_v1_dirent vent;
        unsigned int i;
        int rc;

        if (ent == 0)
                return -1;
        rc = file_v1_readdir(owner, fd, &vent);
        if (rc <= 0)
                return rc;
        ent->chars = vent.name.chars;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                ent->words[i] = vent.name.words[i];
        ent->type = vent.type;
        return 1;
}

int
sys_v1_mkdir(const kword_t *path, unsigned int mode)
{
        return file_v1_mkdir(path, mode);
}

int
sys_v1_unlink(const kword_t *path)
{
        return file_v1_unlink(path);
}

int
sys_v1_truncate(const kword_t *path, kword_t chars)
{
        return file_v1_truncate(path, chars);
}

static kword_t *
sys_v1_user_words(kword_t addr)
{
        kword_t base;

        if (proc_v1_current == 0)
                return 0;
        base = PROC_V1_MEM_BASE(proc_v1_current);
        return (kword_t *)(unsigned long)(base + (addr & PROC_V1_HALF_MASK));
}

int
exec_native_syscall_v1(kword_t *ac)
{
        unsigned int owner;
        unsigned int sysno;
        kword_t *p;
        int rc;

        if (ac == 0 || proc_v1_current == 0)
                return -1;
        owner = proc_v1_slot(proc_v1_current);
        if (owner >= PROC_V1_NPROC)
                return -1;
        sysno = (unsigned int)(ac[1] & PROC_V1_HALF_MASK);
        switch (sysno) {
        case SYS_V1_OPEN:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_open(owner, p,
                    (unsigned int)(ac[3] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_CLOSE:
                rc = sys_v1_close(owner, (int)(ac[2] & PROC_V1_HALF_MASK));
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
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_stat_path(sys_v1_user_words(ac[2]),
                    (struct sys_v1_stat *)p);
                break;
        case SYS_V1_DIRREAD:
                p = sys_v1_user_words(ac[3]);
                rc = p == 0 ? -1 : sys_v1_dirread(owner,
                    (int)(ac[2] & PROC_V1_HALF_MASK),
                    (struct sys_v1_dirent *)p);
                break;
        case SYS_V1_MKDIR:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_mkdir(p,
                    (unsigned int)(ac[3] & PROC_V1_HALF_MASK));
                break;
        case SYS_V1_UNLINK:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_unlink(p);
                break;
        case SYS_V1_TRUNCATE:
                p = sys_v1_user_words(ac[2]);
                rc = p == 0 ? -1 : sys_v1_truncate(p, ac[3]);
                break;
        default:
                rc = -1;
                break;
        }
        ac[1] = (kword_t)rc;
        return rc;
}
