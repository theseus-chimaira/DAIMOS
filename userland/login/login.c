#include "text.h"

#define LOGIN_LINE_MAX 127U
#define LOGIN_NAME_MAX 31U
#define LOGIN_PASS_MAX 63U

struct login_account {
        unsigned int uid;
        unsigned int gid;
        char password[LOGIN_PASS_MAX + 1U];
        kword_t home[U_PATH_WORDS];
        kword_t shell[U_PATH_WORDS];
};

static int
text_eq(const char *a, const char *b)
{
        unsigned int i;

        i = 0U;
        while (a[i] != 0 && b[i] != 0) {
                if (a[i] != b[i])
                        return 0;
                ++i;
        }
        return a[i] == b[i];
}

static int
text_copy(char *dst, unsigned int size, const char *src)
{
        unsigned int i;

        if (size == 0U)
                return -1;
        for (i = 0U; src[i] != 0; ++i) {
                if (i + 1U >= size)
                        return -1;
                dst[i] = src[i];
        }
        dst[i] = 0;
        return 0;
}

static int
parse_uint(const char *s, unsigned int *vp)
{
        unsigned int v;
        unsigned int i;

        if (s == 0 || s[0] == 0)
                return -1;
        v = 0U;
        for (i = 0U; s[i] != 0; ++i) {
                if (s[i] < '0' || s[i] > '9')
                        return -1;
                if (v > 077777U)
                        return -1;
                v = v * 10U + (unsigned int)(s[i] - '0');
        }
        if (v > 0777777U)
                return -1;
        *vp = v;
        return 0;
}

static int
parse_s6_uint(const kword_t *s, unsigned int *vp)
{
        unsigned int i;
        unsigned int n;
        unsigned int wi;
        unsigned int sh;
        unsigned int ch;
        unsigned int v;

        if (s == 0 || vp == 0)
                return -1;
        n = (unsigned int)(s[0] & 0777777UL);
        if (n == 0U || n > 6U)
                return -1;
        v = 0U;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(((s[wi] >> sh) & 077UL) + 040UL);
                if (ch < '0' || ch > '9')
                        return -1;
                v = v * 10U + ch - '0';
                if (v > SYS_TTY_ID_MAX)
                        return -1;
        }
        *vp = v;
        return 0;
}

static int
split_passwd(char *line, char **field)
{
        unsigned int n;
        unsigned int i;

        field[0] = line;
        n = 1U;
        for (i = 0U; line[i] != 0; ++i) {
                if (line[i] != ':')
                        continue;
                if (n >= 6U)
                        return -1;
                line[i] = 0;
                field[n++] = &line[i + 1U];
        }
        return n == 6U ? 0 : -1;
}

static int
login_getline(char *buf, unsigned int size, int upper)
{
        unsigned int n;
        int ch;

        n = 0U;
        for (;;) {
                ch = dsys_readchar(0);
                if (ch == -2)
                        return -1;
                if (ch < 0)
                        return -1;
                if (ch == '\r' || ch == '\n') {
                        buf[n] = 0;
                        return (int)n;
                }
                if (ch < 040 || ch > 0176 || n + 1U >= size)
                        continue;
                if (upper && ch >= 'a' && ch <= 'z')
                        ch -= 'a' - 'A';
                buf[n++] = (char)ch;
        }
}

/* Return 0 for a valid matching account and -1 otherwise. */
static int
find_account(const char *name, struct login_account *account)
{
        struct u_text_reader r;
        char line[LOGIN_LINE_MAX + 1U];
        char *f[6];
        int n;
        int result;

        r.fd = -1;
        if (u_text_open(&r, "/CONFIG/PASSWD") != 0)
                return -1;
        result = -1;
        for (;;) {
                n = u_text_getline(&r, line, sizeof(line));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0) {
                        result = -1;
                        break;
                }
                if (line[0] == 0 || line[0] == '#')
                        continue;
                if (split_passwd(line, f) != 0 || !text_eq(f[0], name))
                        continue;
                if (parse_uint(f[2], &account->uid) != 0 ||
                    parse_uint(f[3], &account->gid) != 0 ||
                    text_copy(account->password, sizeof(account->password),
                    f[1]) != 0 ||
                    u_s6_pack(account->home, U_PATH_WORDS, f[4]) != 0 ||
                    u_s6_pack(account->shell, U_PATH_WORDS, f[5]) != 0) {
                        result = -1;
                        break;
                }
                result = 0;
                break;
        }
        u_text_close(&r);
        return result;
}

static int
exec_shell(const kword_t *path)
{
        kword_t block[SYS_EXEC_V1_FIXED_WORDS + 2U * U_PATH_WORDS];
        struct sys_exec_v1 *exec;
        unsigned int path_words;
        unsigned int total;
        unsigned int i;

        path_words = 1U + ((unsigned int)path[0] + 5U) / 6U;
        if (path_words > U_PATH_WORDS)
                return -1;
        total = SYS_EXEC_V1_FIXED_WORDS + 2U * path_words;
        for (i = 0U; i < total; ++i)
                block[i] = 0UL;
        exec = (struct sys_exec_v1 *)block;
        exec->version_words = SYS_RUN_HEADER(SYS_EXEC_VERSION_1, total);
        exec->argc = 1UL;
        exec->envc = 0UL;
        for (i = 0U; i < path_words; ++i) {
                block[SYS_EXEC_V1_FIXED_WORDS + i] = path[i];
                block[SYS_EXEC_V1_FIXED_WORDS + path_words + i] = path[i];
        }
        return dsys_exec(exec);
}

static int
setup_tty(unsigned int tty)
{
        int pgrp;

        if (tty > SYS_TTY_ID_MAX)
                return -1;
        if (dsys_procctl(SYS_PROCCTL_NEWSESSION, 0U) < 0)
                return -1;
        if (dsys_procctl(SYS_PROCCTL_TTY_ATTACH, tty) != (int)tty)
                return -1;
        pgrp = dsys_procctl(SYS_PROCCTL_GETPGRP, 0U);
        if (pgrp < 0 || dsys_procctl(SYS_PROCCTL_TTY_SETFG,
            (unsigned int)pgrp) != pgrp)
                return -1;
        if (dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
            SYS_TTY_MODE_COOKED) != (int)SYS_TTY_MODE_COOKED)
                return -1;
        return 0;
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct login_account account;
        char name[LOGIN_NAME_MAX + 1U];
        char password[LOGIN_PASS_MAX + 1U];
        unsigned int tty;
        int auth;

        (void)envp;
        if (argc < 2 || argv == 0 || parse_s6_uint(argv[1], &tty) != 0 ||
            setup_tty(tty) != 0) {
                (void)u_puts(2, "LOGIN: TTY FAILED");
                (void)u_crlf(2);
                (void)dsys_exit(1);
                return 1;
        }
        for (;;) {
                (void)u_puts(1, "LOGIN: ");
                if (login_getline(name, sizeof(name), 1) < 0)
                        break;
                auth = find_account(name, &account);
                if (auth != 0) {
                        (void)u_puts(1, "LOGIN INCORRECT");
                        (void)u_crlf(1);
                        continue;
                }
                if (account.password[0] != 0) {
                        (void)u_puts(1, "PASSWORD: ");
                        if (dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                            SYS_TTY_MODE_CANONICAL | SYS_TTY_MODE_SIGNALS) < 0)
                                break;
                        if (login_getline(password, sizeof(password), 0) < 0) {
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                                    SYS_TTY_MODE_COOKED);
                                break;
                        }
                        (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                            SYS_TTY_MODE_COOKED);
                        (void)u_crlf(1);
                        if (!text_eq(password, account.password)) {
                                (void)u_puts(1, "LOGIN INCORRECT");
                                (void)u_crlf(1);
                                continue;
                        }
                }
                if (dsys_procctl(SYS_PROCCTL_SETGID, account.gid) < 0 ||
                    dsys_procctl(SYS_PROCCTL_SETUID, account.uid) < 0 ||
                    dsys_chdir(account.home) != 0) {
                        (void)u_puts(2, "LOGIN: SESSION FAILED");
                        (void)u_crlf(2);
                        break;
                }
                if (exec_shell(account.shell) != 0) {
                        (void)u_puts(2, "LOGIN: EXEC FAILED");
                        (void)u_crlf(2);
                }
                break;
        }
        (void)dsys_exit(1);
        return 1;
}
