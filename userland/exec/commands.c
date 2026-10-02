#include "cmd.h"
#include "text.h"
#include "dtfs_media.h"

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
        struct u_text_reader r;
        char line[259];
        int i;
        int n;
        int own_sink;
        int rc;

        own_sink = u_text_sink_attach(io->out_fd) == 0;
        rc = 0;
        if (argc < 2) {
                r.fd = -1;
                if (u_text_open_fd(&r, io->in_fd) != 0) {
                        rc = cmd_err(io, "CAT", 0);
                } else {
                        for (;;) {
                                n = u_text_getline(&r, line, sizeof(line));
                                if (n == U_TEXT_EOF)
                                        break;
                                if (n < 0 || u_puts(io->out_fd, line) != 0 ||
                                    u_crlf(io->out_fd) != 0) {
                                        rc = 1;
                                        break;
                                }
                        }
                        u_text_close(&r);
                }
                if (own_sink && u_text_sink_detach() != 0)
                        rc = 1;
                return rc;
        }
        for (i = 1; i < argc; ++i) {
                r.fd = -1;
                {
                        int fd = dsys_open(argv[i], SYS_O_RDONLY);
                        if (fd < 0 || u_text_open_fd(&r, fd) != 0) {
                                if (fd >= 0) (void)dsys_close(fd);
                                rc = cmd_err(io, "CAT", argv[i]);
                                continue;
                        }
                }
                for (;;) {
                        n = u_text_getline(&r, line, sizeof(line));
                        if (n == U_TEXT_EOF) break;
                        if (n < 0 || u_puts(io->out_fd, line) != 0 ||
                            u_crlf(io->out_fd) != 0) {
                                rc = 1;
                                break;
                        }
                }
                u_text_close(&r);
        }
        if (own_sink && u_text_sink_detach() != 0)
                rc = 1;
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
        static kword_t dot[2] = { 1UL, PDP10_SIX6('.',' ',' ',' ',' ',' ') };
        int i;
        int own_sink;
        int rc;

        /* Directory listings are naturally line-oriented S6REC text.  On a
         * terminal, batching each completed line through the existing text
         * sink avoids one WRITECHAR trap per output character.  If DSH has
         * already attached the sink for redirection, simply use that sink and
         * leave its lifetime to the caller. */
        own_sink = u_text_sink_attach(io->out_fd) == 0;
        rc = 0;
        if (argc < 2) {
                rc = cmd_ls_one(dot, io);
        } else {
                for (i = 1; i < argc; ++i)
                        if (cmd_ls_one(argv[i], io) != 0)
                                rc = 1;
        }
        if (own_sink && u_text_sink_detach() != 0)
                rc = 1;
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
                    u_put_uint(io->out_fd, st.type) != 0 || u_puts(io->out_fd, " WORDS ") != 0 ||
                    u_put_uint(io->out_fd, st.size_words) != 0 || u_puts(io->out_fd, " MODE ") != 0 ||
                    u_put_octal(io->out_fd, st.mode, 4U) != 0 || u_puts(io->out_fd, " UID ") != 0 ||
                    u_put_uint(io->out_fd, st.uid) != 0 || u_puts(io->out_fd, " GID ") != 0 ||
                    u_put_uint(io->out_fd, st.gid) != 0 || u_puts(io->out_fd, " MTIME ") != 0 ||
                    u_put_octal(io->out_fd, st.mtime, 12U) != 0 || u_crlf(io->out_fd) != 0) return 1;
        }
        return rc;
}

static int
cmd_touch(int argc, kword_t **argv, struct u_io *io)
{
        kword_t now;
        int i, fd, rc;

        if (argc < 2) return cmd_err(io, "TOUCH", 0);
        now = dsys_gettime();
        if (now == 0UL) return cmd_err(io, "TOUCH", 0);
        rc = 0;
        for (i = 1; i < argc; ++i) {
                fd = dsys_open(argv[i], SYS_O_WRONLY | SYS_O_CREAT | SYS_O_APPEND);
                if (fd < 0) { rc = cmd_err(io, "TOUCH", argv[i]); continue; }
                if (dsys_close(fd) != 0 || dsys_utime(argv[i], now) != 0)
                        rc = cmd_err(io, "TOUCH", argv[i]);
        }
        return rc;
}

static int
cmd_cp(int argc, kword_t **argv, struct u_io *io)
{
        struct vfs_stat st;
        kword_t buf[127];
        int in, out, n, rc;

        if (argc != 3) return cmd_err(io, "CP", 0);
        if (dsys_stat(argv[1], &st) != 0 || st.type != VFS_TYPE_REG)
                return cmd_err(io, "CP", argv[1]);
        in = dsys_open(argv[1], SYS_O_RDONLY);
        if (in < 0) return cmd_err(io, "CP", argv[1]);
        out = dsys_open(argv[2], SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC);
        if (out < 0) { (void)dsys_close(in); return cmd_err(io, "CP", argv[2]); }
        rc = 0;
        for (;;) {
                n = dsys_read_words(in, buf, 127U);
                if (n == 0) break;
                if (n < 0) { rc = 1; break; }
                if (u_write_words_all(out, buf, (unsigned int)n) != 0) {
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

static int
cmd_uint_arg(const kword_t *arg, unsigned int *vp)
{
        unsigned int i, n, wi, sh, ch, v;

        if (arg == 0 || vp == 0) return -1;
        n = (unsigned int)(arg[0] & 0777777UL);
        if (n == 0U || n > 6U) return -1;
        v = 0U;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((arg[wi] >> sh) & 077UL) + 040U);
                if (ch < '0' || ch > '9') return -1;
                v = v * 10U + (ch - '0');
                if (v > 0777U) return -1;
        }
        *vp = v;
        return 0;
}

static int
cmd_chown(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int uid, gid;
        int i, rc;

        if (argc < 4 || cmd_uint_arg(argv[1], &uid) != 0 ||
            cmd_uint_arg(argv[2], &gid) != 0)
                return cmd_err(io, "CHOWN", 0);
        rc = 0;
        for (i = 3; i < argc; ++i)
                if (dsys_chown(argv[i], uid, gid) != 0)
                        rc = cmd_err(io, "CHOWN", argv[i]);
        return rc;
}

static int
cmd_rmdir(int argc, kword_t **argv, struct u_io *io)
{
        int i, rc;

        if (argc < 2) return cmd_err(io, "RMDIR", 0);
        rc = 0;
        for (i = 1; i < argc; ++i)
                if (dsys_rmdir(argv[i]) != 0)
                        rc = cmd_err(io, "RMDIR", argv[i]);
        return rc;
}

static int
cmd_put2(int fd, unsigned int v)
{
        return u_putc(fd, '0' + (int)((v / 10U) % 10U)) != 0 ||
            u_putc(fd, '0' + (int)(v % 10U)) != 0;
}

static int
cmd_date(int argc, kword_t **argv, struct u_io *io)
{
        kword_t t;
        unsigned int bcd, year, month, day, hour, minute, second;

        (void)argv;
        if (argc != 1) return cmd_err(io, "DATE", 0);
        t = dsys_gettime();
        if (t == 0UL) return cmd_err(io, "DATE", 0);
        bcd = (unsigned int)((t >> 26) & 0377UL);
        year = ((bcd >> 4) & 017U) * 10U + (bcd & 017U);
        year += (t & 0200000000000UL) != 0UL ? 2100U : 2000U;
        month = (unsigned int)((t >> 22) & 017UL);
        day = (unsigned int)((t >> 17) & 037UL);
        hour = (unsigned int)((t >> 12) & 037UL);
        minute = (unsigned int)((t >> 6) & 077UL);
        second = (unsigned int)(t & 077UL);
        if (u_put_uint(io->out_fd, year) != 0 || u_putc(io->out_fd, '-') != 0 ||
            cmd_put2(io->out_fd, month) != 0 || u_putc(io->out_fd, '-') != 0 ||
            cmd_put2(io->out_fd, day) != 0 || u_putc(io->out_fd, ' ') != 0 ||
            cmd_put2(io->out_fd, hour) != 0 || u_putc(io->out_fd, ':') != 0 ||
            cmd_put2(io->out_fd, minute) != 0 || u_putc(io->out_fd, ':') != 0 ||
            cmd_put2(io->out_fd, second) != 0 || u_puts(io->out_fd, " UTC") != 0)
                return 1;
        return u_crlf(io->out_fd);
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

static unsigned int
cmd_record_words(const kword_t *record)
{
        return 1U + ((unsigned int)record[0] + 5U) / 6U;
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
        kword_t dir[DTFS_BLOCK_WORDS];
        unsigned int flags;
        unsigned int i;
        unsigned int block;
        unsigned int type;

        if (argc != 4 || !u_s6_eq(argv[1], "-O") ||
            cmd_dtfs_options(argv[2], &flags, &type) != 0 ||
            (type != SYS_DTFS_TYPE_NATIVE && type != SYS_DTFS_TYPE_TENEX &&
            type != SYS_DTFS_TYPE_ITS) || flags != SYS_MOUNT_RDONLY ||
            !u_s6_eq(argv[3], "/DEV/DTC0"))
                return cmd_err(io, "MKFS.DTFS", 0);
        for (i = 0U; i != DTFS_BLOCK_WORDS; ++i)
                dir[i] = 0UL;
        block = DTFS_DIR_BLOCK;
        if (type == SYS_DTFS_TYPE_NATIVE) {
                dir[0] = (kword_t)DTFS_OWNER_RESERVED << 31U;
                dir[14] = (kword_t)DTFS_OWNER_RESERVED << 21U;
                dir[82] = ((kword_t)DTFS_OWNER_NATIVE_TAG << 11U) |
                    ((kword_t)DTFS_OWNER_NATIVE_TAG << 6U) |
                    ((kword_t)DTFS_OWNER_NATIVE_TAG << 1U);
                dir[DTFS_MAGIC_WORD] = DTFS_NATIVE_MAGIC;
        } else if (type == SYS_DTFS_TYPE_TENEX) {
                dir[0] = ((kword_t)DTFS_TENEX_RESERVED << 31U) |
                    ((kword_t)DTFS_TENEX_RESERVED << 26U);
                dir[14] = (kword_t)DTFS_TENEX_RESERVED << 26U;
                dir[82] = ((kword_t)DTFS_TENEX_INVALID << 16U) |
                    ((kword_t)DTFS_TENEX_INVALID << 11U) |
                    ((kword_t)DTFS_TENEX_INVALID << 6U) |
                    ((kword_t)DTFS_TENEX_INVALID << 1U);
        } else {
                dir[DTFS_ITS_MAP_FIRST] = DTFS_ITS_MAP_RESERVED;
                dir[DTFS_ITS_MAP_DIR] = DTFS_ITS_MAP_DIRWORD;
                dir[DTFS_ITS_MAP_LAST] = DTFS_ITS_MAP_END;
                block = DTFS_ITS_DIR_BLOCK;
        }
        return dsys_dtc_write_block(0U, block, dir) == 0 ? 0 :
            cmd_err(io, "MKFS.DTFS", argv[3]);
}

static int
cmd_dtfs_probe(kword_t *device, int deep, struct u_io *io)
{
        kword_t block[SYS_RUN_V2_FIXED_WORDS + 4U * U_PATH_WORDS + 3U];
        kword_t path[U_PATH_WORDS];
        kword_t deep_arg[U_PATH_WORDS];
        struct sys_run_v2 *run;
        kword_t status;
        unsigned int words;
        unsigned int total;
        unsigned int i;
        unsigned int value;
        int pid;

        if (u_s6_pack(path, U_PATH_WORDS, "/SYSTEM/EXEC/DTFSPROBE") != 0 ||
            (deep && u_s6_pack(deep_arg, U_PATH_WORDS, "DEEP") != 0))
                return -1;
        total = SYS_RUN_V2_FIXED_WORDS;
        words = cmd_record_words(path);
        for (i = 0U; i < words; ++i)
                block[total++] = path[i];
        for (i = 0U; i < words; ++i)
                block[total++] = path[i];
        words = cmd_record_words(device);
        for (i = 0U; i < words; ++i)
                block[total++] = device[i];
        if (deep) {
                words = cmd_record_words(deep_arg);
                for (i = 0U; i < words; ++i)
                        block[total++] = deep_arg[i];
        }
        block[total++] = SYS_RUN_FD_MAP(0U, (unsigned int)io->in_fd);
        block[total++] = SYS_RUN_FD_MAP(1U, (unsigned int)io->out_fd);
        block[total++] = SYS_RUN_FD_MAP(2U, (unsigned int)io->err_fd);

        run = (struct sys_run_v2 *)block;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, total);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = deep ? 3UL : 2UL;
        run->envc = 0UL;
        pid = dsys_run(run);
        if (pid < 0 || dsys_wait((unsigned int)pid, &status, 0U) != pid ||
            SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                return -1;
        value = SYS_WAIT_STATUS_VALUE(status);
        if (value != SYS_DTFS_TYPE_NATIVE && value != SYS_DTFS_TYPE_TENEX &&
            value != SYS_DTFS_TYPE_ITS)
                return -1;
        return (int)value;
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
        found = cmd_dtfs_probe(device, 1, io);
        if (found < 0 || (type != SYS_DTFS_TYPE_AUTO &&
            found != (int)type))
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
        int found;

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
        found = cmd_dtfs_probe(device, 0, io);
        if (found < 0 || (type != SYS_DTFS_TYPE_AUTO &&
            found != (int)type))
                return cmd_err(io, "MOUNT.DTFS", device);
        return dsys_dtfs_mount(device, target, flags | (unsigned int)found) == 0 ? 0 :
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
        struct sys_meminfo m;
        unsigned int i;
        unsigned int slots;
        unsigned int used;
        int own_sink;
        int rc;
        (void)argc; (void)argv;
        slots = SYS_PROC_SLOTS;
        used = SYS_PROC_SLOTS;
        if (dsys_meminfo(&m) == 0 && m.process_slots_total <= SYS_PROC_SLOTS) {
                slots = (unsigned int)m.process_slots_total;
                if (m.process_slots_used <= slots)
                        used = (unsigned int)m.process_slots_used;
        }
        own_sink = u_text_sink_attach(io->out_fd) == 0;
        rc = 0;
        if (u_puts(io->out_fd, "PID PPID S WORDS COMM") != 0 ||
            u_crlf(io->out_fd) != 0) {
                rc = 1;
                goto ps_done;
        }
        for (i = 0U; i < slots && used != 0U; ++i) {
                if (dsys_procinfo(i, &p) != 0) continue;
                --used;
                if (u_put_uint(io->out_fd, p.pid) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.ppid) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.state) != 0 || u_putc(io->out_fd, ' ') != 0 ||
                    u_put_uint(io->out_fd, p.words) != 0 || u_putc(io->out_fd, ' ') != 0) {
                        rc = 1;
                        goto ps_done;
                }
                {
                        kword_t s6[2];
                        s6[0] = 6U;
                        s6[1] = p.comm;
                        if (u_put_s6(io->out_fd, s6) != 0) {
                                rc = 1;
                                goto ps_done;
                        }
                }
                if (u_crlf(io->out_fd) != 0) {
                        rc = 1;
                        goto ps_done;
                }
        }
ps_done:
        if (own_sink && u_text_sink_detach() != 0)
                rc = 1;
        return rc;
}

static int
cmd_devs(int argc, kword_t **argv, struct u_io *io)
{
        static kword_t dev[2] = { 4UL, PDP10_SIX6('/','D','E','V',' ',' ') };
        (void)argc; (void)argv;
        return cmd_ls_one(dev, io);
}

static int
cmd_mods(int argc, kword_t **argv, struct u_io *io)
{
        kword_t path[U_PATH_WORDS];

        (void)argc;
        (void)argv;
        if (u_s6_pack(path, U_PATH_WORDS, "/MONITOR/DEVICES") != 0)
                return 1;
        return cmd_ls_one(path, io);
}

static int
cmd_mounts(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[192];
        int rc;

        (void)argc;
        (void)argv;
        if (u_puts(io->out_fd, "ROOT /") != 0 || u_crlf(io->out_fd) != 0)
                return 1;
        if (u_text_open(&r, "/CONFIG/FSTAB") != 0)
                return 0;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0)
                if (line[0] != 0 && line[0] != '#' &&
                    (u_puts(io->out_fd, line) != 0 ||
                    u_crlf(io->out_fd) != 0)) {
                        u_text_close(&r);
                        return 1;
                }
        u_text_close(&r);
        return rc == U_TEXT_EOF ? 0 : 1;
}

static int
cmd_free(int argc, kword_t **argv, struct u_io *io)
{
        struct sys_meminfo m;
        kword_t accounted;
        (void)argc; (void)argv;
        if (dsys_meminfo(&m) != 0) return cmd_err(io, "FREE", 0);
        accounted = m.resident_words + m.process_words + m.memfs_used_words;
        if (u_puts(io->out_fd, "TOTAL ") != 0 || u_put_uint(io->out_fd, m.total_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "RESIDENT ") != 0 || u_put_uint(io->out_fd, m.resident_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "PROCESS ") != 0 || u_put_uint(io->out_fd, m.process_words) != 0 || u_crlf(io->out_fd) != 0 ||
            u_puts(io->out_fd, "MEMFS ") != 0 || u_put_uint(io->out_fd, m.memfs_used_words) != 0 || u_crlf(io->out_fd) != 0) return 1;
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
        free_words = m.memfs_capacity_words >= m.memfs_used_words ?
            m.memfs_capacity_words - m.memfs_used_words : 0;
        if (u_puts(io->out_fd, "MEMFS USED ") != 0 ||
            u_put_uint(io->out_fd, m.memfs_used_words) != 0 ||
            u_puts(io->out_fd, " CAPACITY ") != 0 ||
            u_put_uint(io->out_fd, m.memfs_capacity_words) != 0 ||
            u_puts(io->out_fd, " FREE ") != 0 ||
            u_put_uint(io->out_fd, free_words) != 0 ||
            u_crlf(io->out_fd) != 0)
                return 1;
        return 0;
}

static int
cmd_ttyout(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int tty;
        unsigned int sink;
        int rc;

        if ((argc != 2 && argc != 3) ||
            cmd_uint_arg(argv[1], &tty) != 0 || tty > SYS_TTY_ID_MAX)
                return cmd_err(io, "TTYOUT", 0);
        if (argc == 2) {
                rc = dsys_ttyctl(SYS_TTYCTL_GETOUT, tty, 0U);
                if (rc < 0)
                        return cmd_err(io, "TTYOUT", 0);
                return u_put_uint(io->out_fd, (unsigned int)rc) != 0 ||
                    u_crlf(io->out_fd) != 0;
        }
        if (u_s6_eq(argv[2], "NATIVE")) {
                sink = SYS_TTY_SINK_NATIVE;
        } else if (u_s6_eq(argv[2], "CTY")) {
                sink = SYS_TTY_SINK_CTY;
        } else if (u_s6_eq(argv[2], "DPY")) {
                sink = SYS_TTY_SINK_DPY;
        } else if (cmd_uint_arg(argv[2], &sink) != 0 ||
            sink > SYS_TTY_SINK_DPY) {
                return cmd_err(io, "TTYOUT", 0);
        }
        return dsys_ttyctl(SYS_TTYCTL_SETOUT, tty, sink) < 0 ?
            cmd_err(io, "TTYOUT", 0) : 0;
}

/*
 * Keep TSFS discovery in a short-lived helper rather than linking the
 * scanner into DSH.  RUN returns a child PID; wait for that exact process so
 * the helper's text and scan buffers disappear again before the prompt.
 */
static int
cmd_mount_tsfs(int argc, kword_t **argv, struct u_io *io)
{
        kword_t block[SYS_RUN_V2_FIXED_WORDS + 4U * U_PATH_WORDS + 3U];
        kword_t path[U_PATH_WORDS];
        struct sys_run_v2 *run;
        kword_t status;
        unsigned int path_words;
        unsigned int arg_words;
        unsigned int total;
        unsigned int i;
        int pid;

        if (argc != 3)
                return cmd_err(io, "MOUNT.TSFS", 0);
        if (u_s6_pack(path, U_PATH_WORDS, "/SYSTEM/EXEC/MOUNT.TSFS") != 0)
                return 1;
        path_words = cmd_record_words(path);
        total = SYS_RUN_V2_FIXED_WORDS;
        for (i = 0U; i < path_words; ++i)
                block[total++] = path[i];
        for (i = 0U; i < path_words; ++i)
                block[total++] = path[i];
        arg_words = cmd_record_words(argv[1]);
        for (i = 0U; i < arg_words; ++i)
                block[total++] = argv[1][i];
        arg_words = cmd_record_words(argv[2]);
        for (i = 0U; i < arg_words; ++i)
                block[total++] = argv[2][i];
        block[total++] = SYS_RUN_FD_MAP(0U, (unsigned int)io->in_fd);
        block[total++] = SYS_RUN_FD_MAP(1U, (unsigned int)io->out_fd);
        block[total++] = SYS_RUN_FD_MAP(2U, (unsigned int)io->err_fd);

        run = (struct sys_run_v2 *)block;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, total);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = 3UL;
        run->envc = 0UL;
        pid = dsys_run(run);
        if (pid < 0)
                return 1;
        if (dsys_wait((unsigned int)pid, &status, 0U) != pid ||
            SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                return 1;
        return SYS_WAIT_STATUS_VALUE(status) == 0U ? 0 : 1;
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
        int own_sink;
        int rc;
        (void)argc; (void)argv;
        if (dsys_meminfo(&m) != 0) return cmd_err(io, "MEMSTAT", 0);

        /* Normal terminal output otherwise traps once per character.  Use the
         * existing native S6REC sink when DSH has not already attached it for
         * redirection, reducing each MEMSTAT line to one WRITE_WORDS trap. */
        own_sink = u_text_sink_attach(io->out_fd) == 0;
        rc = 0;
#define FIELD(n,v) do { \
        if (u_puts(io->out_fd,(n)) != 0 || u_put_uint(io->out_fd,(v)) != 0 || \
            u_crlf(io->out_fd) != 0) { \
                rc = 1; \
                goto memstat_done; \
        } \
} while (0)
        FIELD("TOTAL ", m.total_words);
        FIELD("RESIDENT ", m.resident_words);
        FIELD("PROCESS-WORDS ", m.process_words);
        FIELD("MEMFS-USED ", m.memfs_used_words);
        FIELD("MEMFS-CAPACITY ", m.memfs_capacity_words);
        FIELD("PROC-SLOTS ", m.process_slots_used);
        FIELD("PROC-SLOTS-MAX ", m.process_slots_total);
        FIELD("FILE-SLOTS ", m.file_slots_used);
        FIELD("FILE-SLOTS-MAX ", m.file_slots_total);
#undef FIELD
memstat_done:
        if (own_sink && u_text_sink_detach() != 0)
                rc = 1;
        return rc;
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
        if (cmd_name_eq(argv[0], "DATE")) return cmd_date(argc, argv, io);
        if (cmd_name_eq(argv[0], "RMDIR")) return cmd_rmdir(argc, argv, io);
        if (cmd_name_eq(argv[0], "CP")) return cmd_cp(argc, argv, io);
        if (cmd_name_eq(argv[0], "CHMOD")) return cmd_chmod(argc, argv, io);
        if (cmd_name_eq(argv[0], "CHOWN")) return cmd_chown(argc, argv, io);
        if (cmd_name_eq(argv[0], "MKFS.DTFS")) return cmd_mkfs_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "FSCK.DTFS")) return cmd_fsck_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "MOUNT")) return cmd_mount_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "MOUNT.DTFS")) return cmd_mount_dtfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "MOUNT.TSFS")) return cmd_mount_tsfs(argc, argv, io);
        if (cmd_name_eq(argv[0], "UNMOUNT")) return cmd_unmount(argc, argv, io);
        if (cmd_name_eq(argv[0], "MV")) return cmd_mv(argc, argv, io);
        if (cmd_name_eq(argv[0], "HEXDUMP")) return cmd_hexdump(argc, argv, io);
        if (cmd_name_eq(argv[0], "PS")) return cmd_ps(argc, argv, io);
        if (cmd_name_eq(argv[0], "DEVS")) return cmd_devs(argc, argv, io);
        if (cmd_name_eq(argv[0], "MODS")) return cmd_mods(argc, argv, io);
        if (cmd_name_eq(argv[0], "MOUNTS")) return cmd_mounts(argc, argv, io);
        if (cmd_name_eq(argv[0], "FREE")) return cmd_free(argc, argv, io);
        if (cmd_name_eq(argv[0], "MEMSTAT")) return cmd_memstat(argc, argv, io);
        if (cmd_name_eq(argv[0], "SYSCTL")) return cmd_memstat(argc, argv, io);
        if (cmd_name_eq(argv[0], "DF")) return cmd_df(argc, argv, io);
        if (cmd_name_eq(argv[0], "TTYOUT")) return cmd_ttyout(argc, argv, io);
        if (cmd_name_eq(argv[0], "HALT")) return cmd_halt(argc, argv, io);
        return cmd_err(io, "DSH: UNKNOWN", argv[0]);
}
