#include "cmd.h"

#define DSH_LINE_MAX 95U

static char dsh_line[DSH_LINE_MAX + 1U];
static kword_t dsh_args[U_MAX_ARGS][U_ARG_WORDS];
static kword_t *dsh_argv[U_MAX_ARGS];

static int
dsh_getline(void)
{
        unsigned int n;
        int ch;

        n = 0U;
        for (;;) {
                ch = dsys_readchar(0);
                if (ch == -2)
                        continue;
                if (ch < 0)
                        return -1;
                if (ch == '\r' || ch == '\n') {
                        (void)u_crlf(1);
                        dsh_line[n] = 0;
                        return (int)n;
                }
                if (ch == 010 || ch == 0177) {
                        if (n != 0U) {
                                --n;
                                (void)u_puts(1, "\b \b");
                        }
                        continue;
                }
                if (ch < 040 || ch > 0176 || n >= DSH_LINE_MAX)
                        continue;
                if (ch >= 'a' && ch <= 'z')
                        ch -= 'a' - 'A';
                dsh_line[n++] = (char)ch;
                (void)u_putc(1, ch);
        }
}

static int
dsh_parse(void)
{
        unsigned int pos;
        unsigned int start;
        unsigned int out;
        int argc;
        int quote;
        char tmp[DSH_LINE_MAX + 1U];

        pos = 0U;
        argc = 0;
        while (dsh_line[pos] != 0) {
                while (dsh_line[pos] == ' ' || dsh_line[pos] == '\t') ++pos;
                if (dsh_line[pos] == 0) break;
                if (argc >= (int)U_MAX_ARGS) return -1;
                start = pos;
                out = 0U;
                quote = 0;
                if (dsh_line[pos] == '"') { quote = 1; ++pos; }
                while (dsh_line[pos] != 0) {
                        if (quote) {
                                if (dsh_line[pos] == '"') { ++pos; break; }
                        } else if (dsh_line[pos] == ' ' || dsh_line[pos] == '\t') {
                                break;
                        }
                        tmp[out++] = dsh_line[pos++];
                }
                (void)start;
                tmp[out] = 0;
                if (u_s6_pack(dsh_args[argc], U_ARG_WORDS, tmp) != 0) return -1;
                dsh_argv[argc] = dsh_args[argc];
                ++argc;
        }
        return argc;
}

static int
dsh_run(int argc)
{
        struct u_io io;
        int outfd;
        int append;
        int i;
        int rc;
        kword_t *outpath;

        if (argc == 0) return 0;
        if (u_s6_eq(dsh_argv[0], "EXIT")) return 1000;
        if (u_s6_eq(dsh_argv[0], "CD")) {
                if (argc != 2 || dsys_chdir(dsh_argv[1]) != 0) {
                        (void)u_puts(2, "CD");
                        (void)u_crlf(2);
                        return 1;
                }
                return 0;
        }
        io.in_fd = 0;
        io.out_fd = 1;
        io.err_fd = 2;
        outfd = -1;
        outpath = 0;
        for (i = 1; i < argc; ++i) {
                append = 0;
                if (u_s6_eq(dsh_argv[i], ">")) append = 1;
                else if (u_s6_eq(dsh_argv[i], ">>")) append = 2;
                if (append == 0) continue;
                if (i + 1 >= argc) return 1;
                outfd = dsys_open(dsh_argv[i + 1], SYS_O_WRONLY |
                    SYS_O_CREAT | (append == 2 ? SYS_O_APPEND : SYS_O_TRUNC));
                if (outfd < 0) return 1;
                io.out_fd = outfd;
                outpath = dsh_argv[i + 1];
                argc = i;
                break;
        }
        rc = cmd_dispatch(argc, dsh_argv, &io);
        if (outfd >= 0 && dsys_close(outfd) != 0) rc = 1;
        if (rc != 0 && outpath != 0) {
                (void)u_put_s6(io.err_fd, dsh_argv[0]);
                (void)u_puts(io.err_fd, ": ");
                (void)u_put_s6(io.err_fd, outpath);
                (void)u_crlf(io.err_fd);
        }
        return rc;
}

int
main(void)
{
        int argc;
        int rc;

        if (dsys_procctl(SYS_PROCCTL_TTY_SETMODE, SYS_TTY_MODE_RAW) !=
            (int)SYS_TTY_MODE_RAW)
                return 1;
        (void)u_puts(1, "DSH V1");
        (void)u_crlf(1);
        for (;;) {
                if (u_puts(1, "# ") != 0) return 1;
                if (dsh_getline() < 0) return 1;
                argc = dsh_parse();
                if (argc < 0) { (void)u_puts(2, "?PARSE"); (void)u_crlf(2); continue; }
                rc = dsh_run(argc);
                if (rc == 1000) break;
        }
        (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE, SYS_TTY_MODE_COOKED);
        (void)dsys_exit(0);
        return 0;
}
