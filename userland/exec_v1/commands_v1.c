#include "cmd_v1.h"

static int
cmd_name_eq(const kword_t *s, const char *name)
{
        kword_t base[U_V1_ARG_WORDS];
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
        for (i = 0U; i < U_V1_ARG_WORDS; ++i)
                base[i] = 0;
        out = 0U;
        for (i = start; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((s[wi] >> sh) & 077UL) + 040U);
                wi = 1U + out / 6U;
                if (wi >= U_V1_ARG_WORDS)
                        return 0;
                sh = 30U - (out % 6U) * 6U;
                base[wi] |= ((kword_t)((ch - 040U) & 077U)) << sh;
                ++out;
        }
        base[0] = out;
        return u_v1_s6_eq(base, name);
}

static int
cmd_err(struct u_v1_io *io, const char *name, kword_t *path)
{
        if (u_v1_puts(io->err_fd, name) != 0 ||
            u_v1_puts(io->err_fd, ": ") != 0)
                return 1;
        if (path != 0 && u_v1_put_s6(io->err_fd, path) != 0)
                return 1;
        (void)u_v1_crlf(io->err_fd);
        return 1;
}

static int
cmd_echo(int argc, kword_t **argv, struct u_v1_io *io)
{
        int i;
        for (i = 1; i < argc; ++i) {
                if (i != 1 && u_v1_putc(io->out_fd, ' ') != 0) return 1;
                if (u_v1_put_s6(io->out_fd, argv[i]) != 0) return 1;
        }
        return u_v1_crlf(io->out_fd);
}

static int
cmd_cat(int argc, kword_t **argv, struct u_v1_io *io)
{
        int i;
        int fd;
        int ch;
        int rc;
        if (argc < 2) return cmd_err(io, "CAT", 0);
        rc = 0;
        for (i = 1; i < argc; ++i) {
                fd = dsys_v1_open(argv[i], SYS_V1_O_RDONLY);
                if (fd < 0) { rc = cmd_err(io, "CAT", argv[i]); continue; }
                for (;;) {
                        ch = dsys_v1_readchar(fd);
                        if (ch == -2) break;
                        if (ch < 0 || u_v1_putc(io->out_fd, ch) != 0) { rc = 1; break; }
                }
                if (dsys_v1_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_ls_one(kword_t *path, struct u_v1_io *io)
{
        struct sys_v1_stat st;
        struct sys_v1_dirent ent;
        kword_t name[U_V1_ARG_WORDS];
        int fd;
        int r;
        if (dsys_v1_stat(path, &st) != 0) return cmd_err(io, "LS", path);
        if (st.type != VFS_V1_TYPE_DIR) {
                if (u_v1_put_s6(io->out_fd, path) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
                return 0;
        }
        fd = dsys_v1_open(path, SYS_V1_O_RDONLY);
        if (fd < 0) return cmd_err(io, "LS", path);
        for (;;) {
                r = dsys_v1_dirread(fd, &ent);
                if (r == 0) break;
                if (r < 0) { (void)dsys_v1_close(fd); return 1; }
                if (u_v1_s6_from_dirent(name, U_V1_ARG_WORDS, &ent) != 0) return 1;
                if (ent.type == VFS_V1_TYPE_DIR) {
                        if (u_v1_putc(io->out_fd, 'D') != 0) return 1;
                } else if (ent.type == VFS_V1_TYPE_MOUNTSRC) {
                        if (u_v1_putc(io->out_fd, 'M') != 0) return 1;
                } else if (ent.type == VFS_V1_TYPE_BLOCK) {
                        if (u_v1_putc(io->out_fd, 'B') != 0) return 1;
                } else if (ent.type == VFS_V1_TYPE_CHAR) {
                        if (u_v1_putc(io->out_fd, 'C') != 0) return 1;
                } else if (u_v1_putc(io->out_fd, 'F') != 0) return 1;
                if (u_v1_putc(io->out_fd, ' ') != 0 || u_v1_put_s6(io->out_fd, name) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
        }
        return dsys_v1_close(fd) != 0;
}

static int
cmd_ls(int argc, kword_t **argv, struct u_v1_io *io)
{
        static kword_t dot[2] = { 1UL, VFS_V1_SIX6('.',' ',' ',' ',' ',' ') };
        int i;
        int rc;
        if (argc < 2) return cmd_ls_one(dot, io);
        rc = 0;
        for (i = 1; i < argc; ++i) if (cmd_ls_one(argv[i], io) != 0) rc = 1;
        return rc;
}

static int
cmd_mkdir(int argc, kword_t **argv, struct u_v1_io *io)
{
        int i, rc = 0;
        if (argc < 2) return cmd_err(io, "MKDIR", 0);
        for (i = 1; i < argc; ++i) if (dsys_v1_mkdir(argv[i]) != 0) rc = cmd_err(io, "MKDIR", argv[i]);
        return rc;
}

static int
cmd_rm(int argc, kword_t **argv, struct u_v1_io *io)
{
        int i, rc = 0;
        if (argc < 2) return cmd_err(io, "RM", 0);
        for (i = 1; i < argc; ++i) if (dsys_v1_unlink(argv[i]) != 0) rc = cmd_err(io, "RM", argv[i]);
        return rc;
}

static int
cmd_pwd(int argc, kword_t **argv, struct u_v1_io *io)
{
        kword_t path[U_V1_PATH_WORDS];
        (void)argc; (void)argv;
        if (dsys_v1_getcwd(path, U_V1_PATH_WORDS) != 0) return cmd_err(io, "PWD", 0);
        return u_v1_put_s6(io->out_fd, path) != 0 || u_v1_crlf(io->out_fd) != 0;
}

static int
cmd_stat(int argc, kword_t **argv, struct u_v1_io *io)
{
        struct sys_v1_stat st;
        int i, rc = 0;
        if (argc < 2) return cmd_err(io, "STAT", 0);
        for (i = 1; i < argc; ++i) {
                if (dsys_v1_stat(argv[i], &st) != 0) { rc = cmd_err(io, "STAT", argv[i]); continue; }
                if (u_v1_put_s6(io->out_fd, argv[i]) != 0 || u_v1_puts(io->out_fd, " TYPE ") != 0 ||
                    u_v1_put_uint(io->out_fd, st.type) != 0 || u_v1_puts(io->out_fd, " CHARS ") != 0 ||
                    u_v1_put_uint(io->out_fd, st.size_chars) != 0 || u_v1_puts(io->out_fd, " WORDS ") != 0 ||
                    u_v1_put_uint(io->out_fd, st.size_words) != 0 || u_v1_puts(io->out_fd, " MODE ") != 0 ||
                    u_v1_put_octal(io->out_fd, st.mode, 4U) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
        }
        return rc;
}

static int
cmd_touch(int argc, kword_t **argv, struct u_v1_io *io)
{
        int i, fd, rc = 0;
        if (argc < 2) return cmd_err(io, "TOUCH", 0);
        for (i = 1; i < argc; ++i) {
                fd = dsys_v1_open(argv[i], SYS_V1_O_WRONLY | SYS_V1_O_CREAT | SYS_V1_O_APPEND);
                if (fd < 0) rc = cmd_err(io, "TOUCH", argv[i]);
                else if (dsys_v1_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_cp(int argc, kword_t **argv, struct u_v1_io *io)
{
        int in, out, ch, rc;
        if (argc != 3) return cmd_err(io, "CP", 0);
        in = dsys_v1_open(argv[1], SYS_V1_O_RDONLY);
        if (in < 0) return cmd_err(io, "CP", argv[1]);
        out = dsys_v1_open(argv[2], SYS_V1_O_WRONLY | SYS_V1_O_CREAT | SYS_V1_O_TRUNC);
        if (out < 0) { (void)dsys_v1_close(in); return cmd_err(io, "CP", argv[2]); }
        rc = 0;
        for (;;) {
                ch = dsys_v1_readchar(in);
                if (ch == -2) break;
                if (ch < 0 || dsys_v1_writechar(out, ch) != 0) { rc = 1; break; }
        }
        if (dsys_v1_close(in) != 0 || dsys_v1_close(out) != 0) rc = 1;
        return rc;
}

static int
cmd_mv(int argc, kword_t **argv, struct u_v1_io *io)
{
        if (argc != 3) return cmd_err(io, "MV", 0);
        return dsys_v1_rename(argv[1], argv[2]) == 0 ? 0 : cmd_err(io, "MV", argv[1]);
}

static int
cmd_hexdump(int argc, kword_t **argv, struct u_v1_io *io)
{
        kword_t w[8];
        kword_t off;
        int fd, n, i, j, rc;
        if (argc < 2) return cmd_err(io, "HEXDUMP", 0);
        rc = 0;
        for (j = 1; j < argc; ++j) {
                fd = dsys_v1_open(argv[j], SYS_V1_O_RDONLY);
                if (fd < 0) { rc = cmd_err(io, "HEXDUMP", argv[j]); continue; }
                off = 0;
                for (;;) {
                        n = dsys_v1_read_words(fd, w, 8U);
                        if (n < 0) { rc = 1; break; }
                        if (n == 0) break;
                        for (i = 0; i < n; ++i) {
                                if (u_v1_put_octal(io->out_fd, off++, 6U) != 0 || u_v1_putc(io->out_fd, ' ') != 0 ||
                                    u_v1_put_octal(io->out_fd, w[i], 12U) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
                        }
                }
                if (dsys_v1_close(fd) != 0) rc = 1;
        }
        return rc;
}

static int
cmd_ps(int argc, kword_t **argv, struct u_v1_io *io)
{
        struct sys_v1_procinfo p;
        unsigned int i;
        (void)argc; (void)argv;
        if (u_v1_puts(io->out_fd, "PID PPID S WORDS COMM") != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
        for (i = 0U; i < SYS_V1_PROC_SLOTS; ++i) {
                if (dsys_v1_procinfo(i, &p) != 0) continue;
                if (u_v1_put_uint(io->out_fd, p.pid) != 0 || u_v1_putc(io->out_fd, ' ') != 0 ||
                    u_v1_put_uint(io->out_fd, p.ppid) != 0 || u_v1_putc(io->out_fd, ' ') != 0 ||
                    u_v1_put_uint(io->out_fd, p.state) != 0 || u_v1_putc(io->out_fd, ' ') != 0 ||
                    u_v1_put_uint(io->out_fd, p.words) != 0 || u_v1_putc(io->out_fd, ' ') != 0) return 1;
                { kword_t s6[2]; s6[0] = 6U; s6[1] = p.comm; if (u_v1_put_s6(io->out_fd, s6) != 0) return 1; }
                if (u_v1_crlf(io->out_fd) != 0) return 1;
        }
        return 0;
}

static int
cmd_devs(int argc, kword_t **argv, struct u_v1_io *io)
{
        static kword_t dev[3] = { 7UL, VFS_V1_SIX6('/','D','E','V','I','C'), VFS_V1_SIX6('E',' ',' ',' ',' ',' ') };
        (void)argc; (void)argv;
        return cmd_ls_one(dev, io);
}

static int
cmd_free(int argc, kword_t **argv, struct u_v1_io *io)
{
        struct sys_v1_meminfo m;
        kword_t accounted;
        (void)argc; (void)argv;
        if (dsys_v1_meminfo(&m) != 0) return cmd_err(io, "FREE", 0);
        accounted = m.resident_words + m.process_words + m.ramfs_used_words;
        if (u_v1_puts(io->out_fd, "TOTAL ") != 0 || u_v1_put_uint(io->out_fd, m.total_words) != 0 || u_v1_crlf(io->out_fd) != 0 ||
            u_v1_puts(io->out_fd, "RESIDENT ") != 0 || u_v1_put_uint(io->out_fd, m.resident_words) != 0 || u_v1_crlf(io->out_fd) != 0 ||
            u_v1_puts(io->out_fd, "PROCESS ") != 0 || u_v1_put_uint(io->out_fd, m.process_words) != 0 || u_v1_crlf(io->out_fd) != 0 ||
            u_v1_puts(io->out_fd, "RAMFS ") != 0 || u_v1_put_uint(io->out_fd, m.ramfs_used_words) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
        if (m.total_words != 0 && m.total_words >= accounted) {
                if (u_v1_puts(io->out_fd, "UNACCOUNTED ") != 0 || u_v1_put_uint(io->out_fd, m.total_words - accounted) != 0 || u_v1_crlf(io->out_fd) != 0) return 1;
        }
        return 0;
}

static int
cmd_df(int argc, kword_t **argv, struct u_v1_io *io)
{
        struct sys_v1_meminfo m;
        kword_t free_words;

        (void)argc;
        (void)argv;
        if (dsys_v1_meminfo(&m) != 0)
                return cmd_err(io, "DF", 0);
        free_words = m.ramfs_capacity_words >= m.ramfs_used_words ?
            m.ramfs_capacity_words - m.ramfs_used_words : 0;
        if (u_v1_puts(io->out_fd, "RAMFS0 USED ") != 0 ||
            u_v1_put_uint(io->out_fd, m.ramfs_used_words) != 0 ||
            u_v1_puts(io->out_fd, " CAPACITY ") != 0 ||
            u_v1_put_uint(io->out_fd, m.ramfs_capacity_words) != 0 ||
            u_v1_puts(io->out_fd, " FREE ") != 0 ||
            u_v1_put_uint(io->out_fd, free_words) != 0 ||
            u_v1_crlf(io->out_fd) != 0)
                return 1;
        return 0;
}

static int
cmd_halt(int argc, kword_t **argv, struct u_v1_io *io)
{
        (void)argc;
        (void)argv;
        (void)io;
        return dsys_v1_halt() == 0 ? 0 : 1;
}

static int
cmd_memstat(int argc, kword_t **argv, struct u_v1_io *io)
{
        struct sys_v1_meminfo m;
        (void)argc; (void)argv;
        if (dsys_v1_meminfo(&m) != 0) return cmd_err(io, "MEMSTAT", 0);
#define FIELD(n,v) do { if (u_v1_puts(io->out_fd,(n)) != 0 || u_v1_put_uint(io->out_fd,(v)) != 0 || u_v1_crlf(io->out_fd) != 0) return 1; } while (0)
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
cmd_v1_dispatch(int argc, kword_t **argv, struct u_v1_io *io)
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
