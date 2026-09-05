#include "cmd.h"

static int
cmd_name_eq(const kword_t *s, const char *name)
{
        kword_t base[U_ARG_WORDS];
        unsigned int n;
        unsigned int i;
        unsigned int start;
        unsigned int out;
        unsigned int wi;
        unsigned int sh;
        unsigned int ch;

        if (s == 0)
                return 0;
        n = (unsigned int)(s[0] & 0777777UL);
        start = 0U;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((s[wi] >> sh) & 077UL) + 040U);
                if (ch == '/')
                        start = i + 1U;
        }
        for (i = 0U; i < U_ARG_WORDS; ++i)
                base[i] = 0;
        out = 0U;
        for (i = start; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((s[wi] >> sh) & 077UL) + 040U);
                wi = 1U + out / 6U;
                if (wi >= U_ARG_WORDS)
                        return 0;
                sh = 30U - (out % 6U) * 6U;
                base[wi] |= ((kword_t)((ch - 040U) & 077U)) << sh;
                ++out;
        }
        base[0] = out;
        return u_s6_eq(base, name);
}

static int
cmd_err(struct u_io *io, const char *name, kword_t *path)
{
        if (u_puts(io->err_fd, name) != 0 ||
            u_puts(io->err_fd, ": ") != 0)
                return 1;
        if (path != 0 && u_put_s6(io->err_fd, path) != 0)
                return 1;
        (void)u_crlf(io->err_fd);
        return 1;
}

static int
cmd_echo(int argc, kword_t **argv, struct u_io *io)
{
        int i;
        for (i = 1; i < argc; ++i) {
                if (i != 1 && u_putc(io->out_fd, ' ') != 0) return 1;
                if (u_put_s6(io->out_fd, argv[i]) != 0) return 1;
        }
        return u_crlf(io->out_fd);
}

static int
cmd_cat(int argc, kword_t **argv, struct u_io *io)
{
        int i;
        int fd;
        int ch;
        int rc;
        if (argc < 2) return cmd_err(io, "CAT", 0);
        rc = 0;
        for (i = 1; i < argc; ++i) {
                fd = dsys_open(argv[i], SYS_O_RDONLY);
                if (fd < 0) { rc = cmd_err(io, "CAT", argv[i]); continue; }
                for (;;) {
                        ch = dsys_readchar(fd);
                        if (ch == -2) break;
                        if (ch < 0 || u_putc(io->out_fd, ch) != 0) { rc = 1; break; }
                }
                if (dsys_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_ls_one(kword_t *path, struct u_io *io)
{
        struct vfs_stat st;
        struct vfs_dirent ent;
        kword_t name[U_ARG_WORDS];
        int fd;
        int r;
        if (dsys_stat(path, &st) != 0) return cmd_err(io, "LS", path);
        if (st.type != VFS_TYPE_DIR) {
                if (u_put_s6(io->out_fd, path) != 0 || u_crlf(io->out_fd) != 0) return 1;
                return 0;
        }
        fd = dsys_open(path, SYS_O_RDONLY);
        if (fd < 0) return cmd_err(io, "LS", path);
        for (;;) {
                r = dsys_dirread(fd, &ent);
                if (r == 0) break;
                if (r < 0) { (void)dsys_close(fd); return 1; }
                if (u_s6_from_dirent(name, U_ARG_WORDS, &ent) != 0) return 1;
                if (ent.type == VFS_TYPE_DIR) {
                        if (u_putc(io->out_fd, 'D') != 0) return 1;
                } else if (ent.type == VFS_TYPE_MOUNTSRC) {
                        if (u_putc(io->out_fd, 'M') != 0) return 1;
                } else if (ent.type == VFS_TYPE_BLOCK) {
                        if (u_putc(io->out_fd, 'B') != 0) return 1;
                } else if (ent.type == VFS_TYPE_CHAR) {
                        if (u_putc(io->out_fd, 'C') != 0) return 1;
                } else if (u_putc(io->out_fd, 'F') != 0) return 1;
                if (u_putc(io->out_fd, ' ') != 0 || u_put_s6(io->out_fd, name) != 0 || u_crlf(io->out_fd) != 0) return 1;
        }
        return dsys_close(fd) != 0;
}

static int
cmd_ls(int argc, kword_t **argv, struct u_io *io)
{
        static kword_t dot[2] = { 1UL, VFS_SIX6('.',' ',' ',' ',' ',' ') };
        int i;
        int rc;
        if (argc < 2) return cmd_ls_one(dot, io);
        rc = 0;
        for (i = 1; i < argc; ++i) if (cmd_ls_one(argv[i], io) != 0) rc = 1;
        return rc;
}

static int
cmd_mkdir(int argc, kword_t **argv, struct u_io *io)
{
        int i, r, rc = 0;
        if (argc < 2) return cmd_err(io, "MKDIR", 0);
        for (i = 1; i < argc; ++i) {
                r = dsys_mkdir(argv[i]);
                if (r == SYS_ERR_UNSUPPORTED) {
                        if (u_puts(io->err_fd, "MKDIR: ") != 0 ||
                            u_put_s6(io->err_fd, argv[i]) != 0 ||
                            u_puts(io->err_fd, ": UNSUPPORTED") != 0 ||
                            u_crlf(io->err_fd) != 0)
                                return 1;
                        rc = 1;
                } else if (r != 0) {
                        rc = cmd_err(io, "MKDIR", argv[i]);
                }
        }
        return rc;
}

static int
cmd_rm(int argc, kword_t **argv, struct u_io *io)
{
        int i, rc = 0;
        if (argc < 2) return cmd_err(io, "RM", 0);
        for (i = 1; i < argc; ++i) if (dsys_unlink(argv[i]) != 0) rc = cmd_err(io, "RM", argv[i]);
        return rc;
}

static int
cmd_pwd(int argc, kword_t **argv, struct u_io *io)
{
        kword_t path[U_PATH_WORDS];
        (void)argc; (void)argv;
        if (dsys_getcwd(path, U_PATH_WORDS) != 0) return cmd_err(io, "PWD", 0);
        return u_put_s6(io->out_fd, path) != 0 || u_crlf(io->out_fd) != 0;
}

static int
cmd_stat(int argc, kword_t **argv, struct u_io *io)
{
        struct vfs_stat st;
        int i, rc = 0;
        if (argc < 2) return cmd_err(io, "STAT", 0);
        for (i = 1; i < argc; ++i) {
                if (dsys_stat(argv[i], &st) != 0) { rc = cmd_err(io, "STAT", argv[i]); continue; }
                if (u_put_s6(io->out_fd, argv[i]) != 0 || u_puts(io->out_fd, " TYPE ") != 0 ||
                    u_put_uint(io->out_fd, st.type) != 0 || u_puts(io->out_fd, " CHARS ") != 0 ||
                    u_put_uint(io->out_fd, st.size_chars) != 0 || u_puts(io->out_fd, " WORDS ") != 0 ||
                    u_put_uint(io->out_fd, st.size_words) != 0 || u_puts(io->out_fd, " MODE ") != 0 ||
                    u_put_octal(io->out_fd, st.mode, 4U) != 0 || u_crlf(io->out_fd) != 0) return 1;
        }
        return rc;
}

static int
cmd_touch(int argc, kword_t **argv, struct u_io *io)
{
        int i, fd, rc = 0;
        if (argc < 2) return cmd_err(io, "TOUCH", 0);
        for (i = 1; i < argc; ++i) {
                fd = dsys_open(argv[i], SYS_O_WRONLY | SYS_O_CREAT | SYS_O_APPEND);
                if (fd < 0) rc = cmd_err(io, "TOUCH", argv[i]);
                else if (dsys_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_cp(int argc, kword_t **argv, struct u_io *io)
{
        struct vfs_stat st;
        kword_t buf[127];
        kword_t chars;
        int in, out, n, rc;

        if (argc != 3) return cmd_err(io, "CP", 0);
        if (dsys_stat(argv[1], &st) != 0 || st.type != VFS_TYPE_REG)
                return cmd_err(io, "CP", argv[1]);
        in = dsys_open(argv[1], SYS_O_RDONLY);
        if (in < 0) return cmd_err(io, "CP", argv[1]);
        out = dsys_open(argv[2], SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC);
        if (out < 0) { (void)dsys_close(in); return cmd_err(io, "CP", argv[2]); }
        rc = 0;
        chars = 0;
        for (;;) {
                n = dsys_read_words(in, buf, 127U);
                if (n == 0) break;
                if (n < 0) { rc = 1; break; }
                chars += (kword_t)(unsigned int)n * 4U;
                if (chars > st.size_chars) chars = st.size_chars;
                if (dsys_write_words(out, buf, (unsigned int)n, chars) != n) {
                        rc = 1;
                        break;
                }
        }
        if (dsys_close(in) != 0 || dsys_close(out) != 0) rc = 1;
        return rc;
}

static int
cmd_octal_mode(const kword_t *arg, unsigned int *modep)
{
        unsigned int i, n, wi, sh, ch, mode;

        if (arg == 0 || modep == 0) return -1;
        n = (unsigned int)(arg[0] & 0777777UL);
        if (n == 0U || n > 4U) return -1;
        mode = 0U;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((arg[wi] >> sh) & 077UL) + 040U);
                if (ch < '0' || ch > '7') return -1;
                mode = (mode << 3) | (ch - '0');
        }
        *modep = mode;
        return 0;
}

static int
cmd_chmod(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int mode;
        if (argc != 3 || cmd_octal_mode(argv[1], &mode) != 0)
                return cmd_err(io, "CHMOD", 0);
        return dsys_chmod(argv[2], mode) == 0 ? 0 :
            cmd_err(io, "CHMOD", argv[2]);
}

static unsigned int
cmd_arg_char(const kword_t *arg, unsigned int off)
{
        unsigned int wi;
        unsigned int sh;

        wi = 1U + off / 6U;
        sh = 30U - (off % 6U) * 6U;
        return (unsigned int)(((arg[wi] >> sh) & 077UL) + 040U);
}

static int
cmd_opt_name(const kword_t *arg, unsigned int off, unsigned int len,
    const char *name)
{
        unsigned int i;
        unsigned int ch;
        unsigned int want;

        for (i = 0U; i < len; ++i) {
                if (name[i] == 0)
                        return 0;
                ch = cmd_arg_char(arg, off + i);
                want = (unsigned int)name[i];
                if (ch >= 'a' && ch <= 'z') ch -= 'a' - 'A';
                if (want >= 'a' && want <= 'z') want -= 'a' - 'A';
                if (ch != want)
                        return 0;
        }
        return name[len] == 0;
}

static int
cmd_dtfs_options(const kword_t *arg, unsigned int *flagsp,
    unsigned int *typep)
{
        unsigned int n;
        unsigned int start;
        unsigned int i;
        unsigned int flags;
        int seen_access;
        unsigned int type;

        if (arg == 0 || flagsp == 0 || typep == 0)
                return -1;
        n = (unsigned int)(arg[0] & 0777777UL);
        if (n == 0U)
                return -1;
        flags = SYS_MOUNT_RDONLY;
        seen_access = 0;
        type = SYS_DTFS_TYPE_AUTO;
        start = 0U;
        for (i = 0U; i <= n; ++i) {
                if (i != n && cmd_arg_char(arg, i) != ',')
                        continue;
                if (i == start)
                        return -1;
                if (cmd_opt_name(arg, start, i - start, "RO")) {
                        if (seen_access) return -1;
                        flags = SYS_MOUNT_RDONLY;
                        seen_access = 1;
                } else if (cmd_opt_name(arg, start, i - start, "RW")) {
                        if (seen_access) return -1;
                        flags = SYS_MOUNT_RW;
                        seen_access = 1;
                } else if (cmd_opt_name(arg, start, i - start, "NATIVE")) {
                        if (type != SYS_DTFS_TYPE_AUTO) return -1;
                        type = SYS_DTFS_TYPE_NATIVE;
                } else if (cmd_opt_name(arg, start, i - start, "TENEX")) {
                        if (type != SYS_DTFS_TYPE_AUTO) return -1;
                        type = SYS_DTFS_TYPE_TENEX;
                } else if (cmd_opt_name(arg, start, i - start, "ITS")) {
                        if (type != SYS_DTFS_TYPE_AUTO) return -1;
                        type = SYS_DTFS_TYPE_ITS;
                } else {
                        return -1;
                }
                start = i + 1U;
        }
        *flagsp = flags;
        *typep = type;
        return 0;
}

static int
cmd_mkfs_dtfs(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int flags;
        unsigned int type;

        if (argc != 4 || !u_s6_eq(argv[1], "-O") ||
            cmd_dtfs_options(argv[2], &flags, &type) != 0 ||
            (type != SYS_DTFS_TYPE_NATIVE && type != SYS_DTFS_TYPE_TENEX &&
            type != SYS_DTFS_TYPE_ITS) || flags != SYS_MOUNT_RDONLY)
                return cmd_err(io, "MKFS.DTFS", 0);
        return dsys_dtfs_format(argv[3], type) == 0 ? 0 :
            cmd_err(io, "MKFS.DTFS", argv[3]);
}

static int
cmd_fsck_dtfs(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int flags;
        unsigned int type;
        kword_t *device;
        int found;

        flags = SYS_MOUNT_RDONLY;
        type = SYS_DTFS_TYPE_AUTO;
        if (argc == 2) {
                device = argv[1];
        } else if (argc == 4 && u_s6_eq(argv[1], "-O") &&
            cmd_dtfs_options(argv[2], &flags, &type) == 0) {
                device = argv[3];
        } else {
                return cmd_err(io, "FSCK.DTFS", 0);
        }
        if (flags != SYS_MOUNT_RDONLY)
                return cmd_err(io, "FSCK.DTFS", 0);
        found = dsys_dtfs_check(device, type);
        if (found < 0)
                return cmd_err(io, "FSCK.DTFS", device);
        if (u_puts(io->out_fd, "FSCK.DTFS ") != 0)
                return 1;
        if (found == (int)SYS_DTFS_TYPE_NATIVE) {
                if (u_puts(io->out_fd, "NATIVE OK") != 0) return 1;
        } else if (found == (int)SYS_DTFS_TYPE_TENEX) {
                if (u_puts(io->out_fd, "TENEX OK") != 0) return 1;
        } else if (found == (int)SYS_DTFS_TYPE_ITS) {
                if (u_puts(io->out_fd, "ITS OK") != 0) return 1;
        } else {
                return cmd_err(io, "FSCK.DTFS", device);
        }
        if (u_crlf(io->out_fd) != 0)
                return 1;
        return 0;
}

static int
cmd_mount_dtfs(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int flags;
        unsigned int type;
        kword_t *device;
        kword_t *target;

        flags = SYS_MOUNT_RDONLY;
        type = SYS_DTFS_TYPE_AUTO;
        if (argc == 3) {
                device = argv[1];
                target = argv[2];
        } else if (argc == 5 && u_s6_eq(argv[1], "-O") &&
            cmd_dtfs_options(argv[2], &flags, &type) == 0) {
                device = argv[3];
                target = argv[4];
        } else {
                return cmd_err(io, "MOUNT.DTFS", 0);
        }
        return dsys_dtfs_mount(device, target, flags | type) == 0 ? 0 :
            cmd_err(io, "MOUNT.DTFS", target);
}

static int
cmd_unmount(int argc, kword_t **argv, struct u_io *io)
{
        if (argc != 2) return cmd_err(io, "UNMOUNT", 0);
        return dsys_unmount(argv[1]) == 0 ? 0 :
            cmd_err(io, "UNMOUNT", argv[1]);
}

static int
cmd_mv(int argc, kword_t **argv, struct u_io *io)
{
        if (argc != 3) return cmd_err(io, "MV", 0);
        return dsys_rename(argv[1], argv[2]) == 0 ? 0 : cmd_err(io, "MV", argv[1]);
}

static int
cmd_hexdump(int argc, kword_t **argv, struct u_io *io)
{
        kword_t w[8];
        kword_t off;
        int fd, n, i, j, rc;
        if (argc < 2) return cmd_err(io, "HEXDUMP", 0);
        rc = 0;
        for (j = 1; j < argc; ++j) {
                fd = dsys_open(argv[j], SYS_O_RDONLY);
                if (fd < 0) { rc = cmd_err(io, "HEXDUMP", argv[j]); continue; }
                off = 0;
                for (;;) {
                        n = dsys_read_words(fd, w, 8U);
                        if (n < 0) { rc = 1; break; }
                        if (n == 0) break;
                        for (i = 0; i < n; ++i) {
                                if (u_put_octal(io->out_fd, off++, 6U) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                                    u_put_octal(io->out_fd, w[i], 12U) != 0 || u_crlf(io->out_fd) != 0) return 1;
                        }
                }
                if (dsys_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_ps(int argc, kword_t **argv, struct u_io *io)
{
        struct sys_procinfo p;
        unsigned int i;
        (void)argc; (void)argv;
        if (u_puts(io->out_fd, "PID PPID S WORDS COMM") != 0 || u_crlf(io->out_fd) != 0) return 1;
        for (i = 0U; i < SYS_PROC_SLOTS; ++i) {
                if (dsys_procinfo(i, &p) != 0) continue;
                if (u_put_uint(io->out_fd, p.pid) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.ppid) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.state) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.words) != 0 || u_putc(io->out_fd, ' ') != 0) return 1;
                { kword_t s6[2]; s6[0] = 6U; s6[1] = p.comm; if (u_put_s6(io->out_fd, s6) != 0) return 1; }
                if (u_crlf(io->out_fd) != 0) return 1;
        }
        return 0;
}

static int
cmd_devs(int argc, kword_t **argv, struct u_io *io)
{
        static kword_t dev[3] = { 7UL, VFS_SIX6('/','D','E','V','I','C'), VFS_SIX6('E',' ',' ',' ',' ',' ') };
        (void)argc; (void)argv;
        return cmd_ls_one(dev, io);
}

static int
cmd_free(int argc, kword_t **argv, struct u_io *io)
{
        struct sys_meminfo m;
        kword_t accounted;
        (void)argc; (void)argv;
        if (dsys_meminfo(&m) != 0) return cmd_err(io, "FREE", 0);
        accounted = m.resident_words + m.process_words + m.ramfs_used_words;
        if (u_puts(io->out_fd, "TOTAL ") != 0 || u_put_uint(io->out_fd, m.total_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "RESIDENT ") != 0 || u_put_uint(io->out_fd, m.resident_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "PROCESS ") != 0 || u_put_uint(io->out_fd, m.process_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "RAMFS ") != 0 || u_put_uint(io->out_fd, m.ramfs_used_words) != 0 || u_crlf(io->out_fd) != 0) return 1;
        if (m.total_words != 0 && m.total_words >= accounted) {
                if (u_puts(io->out_fd, "UNACCOUNTED ") != 0 || u_put_uint(io->out_fd, m.total_words - accounted) != 0 || u_crlf(io->out_fd) != 0) return 1;
        }
        return 0;
}

static int
cmd_df(int argc, kword_t **argv, struct u_io *io)
{
        struct sys_meminfo m;
        kword_t free_words;

        (void)argc;
        (void)argv;
        if (dsys_meminfo(&m) != 0)
                return cmd_err(io, "DF", 0);
        free_words = m.ramfs_capacity_words >= m.ramfs_used_words ?
            m.ramfs_capacity_words - m.ramfs_used_words : 0;
        if (u_puts(io->out_fd, "RAMFS0 USED ") != 0 ||
            u_put_uint(io->out_fd, m.ramfs_used_words) != 0 ||
            u_puts(io->out_fd, " CAPACITY ") != 0 ||
            u_put_uint(io->out_fd, m.ramfs_capacity_words) != 0 ||
            u_puts(io->out_fd, " FREE ") != 0 ||
            u_put_uint(io->out_fd, free_words) != 0 ||
            u_crlf(io->out_fd) != 0)
                return 1;
        return 0;
}

static int
cmd_halt(int argc, kword_t **argv, struct u_io *io)
{
        (void)argc;
        (void)argv;
        (void)io;
        return dsys_halt() == 0 ? 0 : 1;
}

static int
cmd_memstat(int argc, kword_t **argv, struct u_io *io)
{
        struct sys_meminfo m;
        (void)argc; (void)argv;
        if (dsys_meminfo(&m) != 0) return cmd_err(io, "MEMSTAT", 0);
#define FIELD(n,v) do { if (u_puts(io->out_fd,(n)) != 0 || u_put_uint(io->out_fd,(v)) != 0 || u_crlf(io->out_fd) != 0) return 1; } while (0)
        FIELD("TOTAL ", m.total_words);
        FIELD("RESIDENT ", m.resident_words);
        FIELD("PROCESS-WORDS ", m.process_words);
        FIELD("RAMFS-USED ", m.ramfs_used_words);
        FIELD("RAMFS-CAPACITY ", m.ramfs_capacity_words);
        FIELD("PROC-SLOTS ", m.process_slots_used);
        FIELD("PROC-SLOTS-MAX ", m.process_slots_total);
        FIELD("FILE-SLOTS ", m.file_slots_used);
        FIELD("FILE-SLOTS-MAX ", m.file_slots_total);
#undef FIELD
        return 0;
}

int
cmd_dispatch(int argc, kword_t **argv, struct u_io *io)
{
        if (argc <= 0 || argv == 0 || io == 0) return 1;
        if (cmd_name_eq(argv[0], "ECHO")) return cmd_echo(argc, argv, io);
        if (cmd_name_eq(argv[0], "CAT")) return cmd_cat(argc, argv, io);
        if (cmd_name_eq(argv[0], "LS")) return cmd_ls(argc, argv, io);
        if (cmd_name_eq(argv[0], "MKDIR")) return cmd_mkdir(argc, argv, io);
        if (cmd_name_eq(argv[0], "RM")) return cmd_rm(argc, argv, io);
        if (cmd_name_eq(argv[0], "PWD")) return cmd_pwd(argc, argv, io);
        if (cmd_name_eq(argv[0], "STAT")) return cmd_stat(argc, argv, io);
        if (cmd_name_eq(argv[0], "TOUCH")) return cmd_touch(argc, argv, io);
        if (cmd_name_eq(argv[0], "CP")) return cmd_cp(argc, argv, io);
        if (cmd_name_eq(argv[0], "CHMOD")) return cmd_chmod(argc, argv, io);
        if (cmd_name_eq(argv[0], "MKFS.DTFS")) return cmd_mkfs_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "FSCK.DTFS")) return cmd_fsck_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "MOUNT.DTFS")) return cmd_mount_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "UNMOUNT")) return cmd_unmount(argc, argv, io);
        if (cmd_name_eq(argv[0], "MV")) return cmd_mv(argc, argv, io);
        if (cmd_name_eq(argv[0], "HEXDUMP")) return cmd_hexdump(argc, argv, io);
        if (cmd_name_eq(argv[0], "PS")) return cmd_ps(argc, argv, io);
        if (cmd_name_eq(argv[0], "DEVS")) return cmd_devs(argc, argv, io);
        if (cmd_name_eq(argv[0], "FREE")) return cmd_free(argc, argv, io);
        if (cmd_name_eq(argv[0], "MEMSTAT")) return cmd_memstat(argc, argv, io);
        if (cmd_name_eq(argv[0], "DF")) return cmd_df(argc, argv, io);
        if (cmd_name_eq(argv[0], "HALT")) return cmd_halt(argc, argv, io);
        return cmd_err(io, "DSH: UNKNOWN", argv[0]);
}
