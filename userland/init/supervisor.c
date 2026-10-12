#include "logevent.h"
#include "u.h"

#define INIT_MAX_ENTRIES 8U
#define INIT_RESPAWN 1U
#define INIT_ONCE 2U
struct init_entry {
        unsigned int tty, output_sink, action, pid;
        kword_t path[U_PATH_WORDS];
};
static struct init_entry init_entries[INIT_MAX_ENTRIES];
static unsigned int init_count;

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

        if (e->output_sink != SYS_TTY_SINK_NATIVE &&
            dsys_ttyctl(SYS_TTYCTL_SETOUT, e->tty, e->output_sink) < 0)
                return -1;

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
        /* TTY IDs are 0..20: encode at most two SIXBIT digits directly.
         * Keeping this bounded avoids linking the general path packer. */
        tty_arg[0] = e->tty >= 10U ? 2UL : 1UL;
        tty_arg[1] = ((kword_t)(tty_text[0] - 040)) << 30U;
        if (e->tty >= 10U)
                tty_arg[1] |= ((kword_t)(tty_text[1] - 040)) << 24U;
        tty_words = 2U;

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

/* Metadata encoding is intentionally bounded and self-validating.
 * It never re-parses /CONFIG/INITTAB or calls the startup parser. */
static int
read_entry(struct init_entry *e, const kword_t *meta, const kword_t *path)
{
        unsigned int i, c[4];
        if (meta[0] != 4U || path[0] == 0U || path[0] >
            (U_PATH_WORDS - 1U) * 6U)
                return -1;
        for (i = 0U; i < 4U; ++i)
                c[i] = (unsigned int)((meta[1] >> (30U - 6U*i)) & 077U) + 040U;
        if (c[0] < '0' || c[0] > '9' || c[1] < '0' || c[1] > '9')
                return -1;
        e->tty = (c[0] - '0') * 10U + c[1] - '0';
        if (e->tty > SYS_TTY_ID_MAX || (c[2] != 'R' && c[2] != 'O') ||
            (c[3] != 'D' && c[3] != 'N'))
                return -1;
        e->action = c[2] == 'R' ? INIT_RESPAWN : INIT_ONCE;
        e->output_sink = c[3] == 'D' ? SYS_TTY_SINK_DPY : SYS_TTY_SINK_NATIVE;
        e->pid = 0U;
        for (i = 0U; i < U_PATH_WORDS; ++i)
                e->path[i] = i < 1U + ((unsigned int)path[0] + 5U) / 6U ?
                    path[i] : 0UL;
        return 0;
}

int
main(int argc, kword_t **argv)
{
        kword_t status;
        unsigned int i;
        int pid;

        if (dsys_getpid() != 1 || argc < 2 || argc > 16 || (argc & 1)) {
                (void)u_puts(2, "SUPERVISOR: INVALID PID/CONFIG");
                (void)u_crlf(2);
                return 1;
        }
        init_count = (unsigned int)argc / 2U;
        for (i = 0U; i < init_count; ++i) {
                if (read_entry(&init_entries[i], argv[i * 2U],
                    argv[i * 2U + 1U]) != 0) {
                        (void)u_puts(2, "SUPERVISOR: INVALID INITTAB ENTRY");
                        (void)u_crlf(2);
                        return 1;
                }
        }
        (void)ulog_event(ULOG_SEV_INFO, ULOG_SRC_INIT, ULOG_INIT_READY,
            (kword_t)init_count);
        for (i = 0U; i < init_count; ++i) {
                if (spawn_entry(&init_entries[i]) != 0) {
                        (void)ulog_event(ULOG_SEV_ERROR, ULOG_SRC_INIT,
                            ULOG_INIT_RUN_FAIL, (kword_t)i);
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
                                (void)ulog_event(ULOG_SEV_ERROR, ULOG_SRC_INIT,
                                    ULOG_INIT_RESPAWN_FAIL, (kword_t)i);
                                (void)u_puts(2, "INIT: RESPAWN FAILED");
                                (void)u_crlf(2);
                        }
                        break;
                }
        }
}
