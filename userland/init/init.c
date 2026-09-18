#include "text.h"

#define INIT_MAX_ENTRIES 8U
#define INIT_LINE_MAX 127U
#define INIT_RESPAWN 1U
#define INIT_ONCE 2U

struct init_entry {
        unsigned int tty;
        unsigned int action;
        unsigned int pid;
        kword_t path[U_PATH_WORDS];
};

static struct init_entry init_entries[INIT_MAX_ENTRIES];
static unsigned int init_count;

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
        *vp = v;
        return 0;
}

static int
parse_entry(char *line, struct init_entry *e)
{
        char *action;
        char *path;
        unsigned int i;

        if (line[0] == 0 || line[0] == '#')
                return 1;
        action = 0;
        path = 0;
        for (i = 0U; line[i] != 0; ++i) {
                if (line[i] != ':')
                        continue;
                line[i] = 0;
                if (action == 0)
                        action = &line[i + 1U];
                else if (path == 0)
                        path = &line[i + 1U];
                else
                        return -1;
        }
        if (action == 0 || path == 0 || parse_uint(line, &e->tty) != 0 ||
            e->tty > SYS_TTY_ID_MAX ||
            u_s6_pack(e->path, U_PATH_WORDS, path) != 0)
                return -1;
        if (text_eq(action, "RESPAWN"))
                e->action = INIT_RESPAWN;
        else if (text_eq(action, "ONCE"))
                e->action = INIT_ONCE;
        else
                return -1;
        e->pid = 0U;
        return 0;
}

static int
load_inittab(void)
{
        struct u_text_reader r;
        char line[INIT_LINE_MAX + 1U];
        int n;
        int rc;

        init_count = 0U;
        r.fd = -1;
        if (u_text_open(&r, "/CONFIG/INITTAB") != 0)
                return -1;
        for (;;) {
                n = u_text_getline(&r, line, sizeof(line));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0) {
                        u_text_close(&r);
                        return -1;
                }
                if (init_count >= INIT_MAX_ENTRIES) {
                        (void)u_puts(2, "INIT: TOO MANY ENTRIES");
                        (void)u_crlf(2);
                        continue;
                }
                rc = parse_entry(line, &init_entries[init_count]);
                if (rc < 0) {
                        (void)u_puts(2, "INIT: BAD INITTAB");
                        (void)u_crlf(2);
                } else if (rc == 0) {
                        ++init_count;
                }
        }
        u_text_close(&r);
        return init_count == 0U ? -1 : 0;
}

static int
spawn_entry(struct init_entry *e)
{
        kword_t block[SYS_RUN_V2_FIXED_WORDS + 2U * U_PATH_WORDS +
            3U + 3U];
        struct sys_run_v2 *run;
        kword_t tty_arg[2];
        char tty_text[3];
        unsigned int path_words;
        unsigned int tty_words;
        unsigned int arg0_off;
        unsigned int arg1_off;
        unsigned int map_off;
        unsigned int i;
        unsigned int total;
        int pid;

        path_words = 1U + ((unsigned int)e->path[0] + 5U) / 6U;
        if (path_words > U_PATH_WORDS)
                return -1;
        if (e->tty >= 10U) {
                tty_text[0] = (char)('0' + e->tty / 10U);
                tty_text[1] = (char)('0' + e->tty % 10U);
                tty_text[2] = 0;
        } else {
                tty_text[0] = (char)('0' + e->tty);
                tty_text[1] = 0;
        }
        if (u_s6_pack(tty_arg, 2U, tty_text) != 0)
                return -1;
        tty_words = 1U + ((unsigned int)tty_arg[0] + 5U) / 6U;

        arg0_off = SYS_RUN_V2_FIXED_WORDS + path_words;
        arg1_off = arg0_off + path_words;
        map_off = arg1_off + tty_words;
        total = map_off + 3U;
        for (i = 0U; i < total; ++i)
                block[i] = 0UL;

        run = (struct sys_run_v2 *)block;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, total);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = 2UL;
        run->envc = 0UL;
        for (i = 0U; i < path_words; ++i) {
                block[SYS_RUN_V2_FIXED_WORDS + i] = e->path[i];
                block[arg0_off + i] = e->path[i];
        }
        for (i = 0U; i < tty_words; ++i)
                block[arg1_off + i] = tty_arg[i];
        block[map_off] = SYS_RUN_FD_MAP(0U, 0U);
        block[map_off + 1U] = SYS_RUN_FD_MAP(1U, 1U);
        block[map_off + 2U] = SYS_RUN_FD_MAP(2U, 2U);
        pid = dsys_run(run);
        if (pid < 0)
                return -1;
        e->pid = (unsigned int)pid;
        return 0;
}

int
main(void)
{
        kword_t status;
        unsigned int i;
        int pid;

        if (dsys_getpid() != 1) {
                (void)u_puts(2, "INIT: NOT PID 1");
                (void)u_crlf(2);
                (void)dsys_exit(1);
                return 1;
        }
        (void)u_puts(1, "INIT V1");
        (void)u_crlf(1);
        if (load_inittab() != 0) {
                (void)u_puts(2, "INIT: NO INITTAB");
                (void)u_crlf(2);
                (void)dsys_exit(1);
                return 1;
        }
        for (i = 0U; i < init_count; ++i) {
                if (spawn_entry(&init_entries[i]) != 0) {
                        (void)u_puts(2, "INIT: RUN FAILED");
                        (void)u_crlf(2);
                }
        }
        for (;;) {
                pid = dsys_wait(0U, &status, 0U);
                if (pid < 0)
                        continue;
                if (SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                        continue;
                for (i = 0U; i < init_count; ++i) {
                        if (init_entries[i].pid != (unsigned int)pid)
                                continue;
                        init_entries[i].pid = 0U;
                        if (init_entries[i].action == INIT_RESPAWN &&
                            spawn_entry(&init_entries[i]) != 0) {
                                (void)u_puts(2, "INIT: RESPAWN FAILED");
                                (void)u_crlf(2);
                        }
                        break;
                }
        }
}
