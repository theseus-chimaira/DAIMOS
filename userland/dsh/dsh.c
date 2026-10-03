#include "dsh.h"
#include "text.h"

static struct dsh_line dsh_input_line;
static struct dsh_state dsh_main_state;

static int
dsh_input_append(int ch)
{
        unsigned int wi;
        unsigned int sh;

        if (ch < 040 || ch > 0137 ||
            dsh_input_line.len >= DSH_LINE_MAX_CHARS)
                return -1;
        wi = dsh_input_line.len / DSH_S6_CHARS_PER_WORD;
        sh = 30U - (dsh_input_line.len % DSH_S6_CHARS_PER_WORD) * 6U;
        dsh_input_line.words[wi] |=
            ((kword_t)(ch - 040) & 077UL) << sh;
        ++dsh_input_line.len;
        return 0;
}

static int
dsh_getline_tty(int echo)
{
        int ch;

        dsh_line_clear(&dsh_input_line);
        for (;;) {
                ch = dsys_readchar(0);
                if (ch == -2)
                        continue;
                if (ch < 0)
                        return -1;
                if (ch == '\r' || ch == '\n') {
                        if (echo)
                                (void)u_crlf(1);
                        return 0;
                }
                if (ch == 003) {
                        /* Interactive DSH owns a RAW/no-echo TTY while it is
                         * editing, so the kernel deliberately does not turn
                         * Ctrl-C into a process event here.  Cancel the whole
                         * logical shell input locally.  Foreground children
                         * run in signal-enabled cooked mode and therefore keep
                         * the normal kernel Ctrl-C process-group semantics. */
                        dsh_line_clear(&dsh_input_line);
                        if (echo) {
                                (void)u_puts(1, "^C");
                                (void)u_crlf(1);
                        }
                        return 1;
                }
                if (ch == 010 || ch == 0177) {
                        if (dsh_input_line.len != 0U) {
                                --dsh_input_line.len;
                                if (echo)
                                        (void)u_puts(1, "\b \b");
                        }
                        continue;
                }
                if (ch < 040 || ch > 0137)
                        continue;
                if (ch >= 'a' && ch <= 'z')
                        ch -= 'a' - 'A';
                if (dsh_input_append(ch) == 0 && echo)
                        (void)u_putc(1, ch);
        }
}

static int
dsh_getline_text(struct u_text_reader *reader)
{
        kword_t record[DSH_LINE_MAX_WORDS + 1U];
        unsigned int len;
        unsigned int words;
        unsigned int i;
        int rc;

        rc = u_text_gets6(reader, record, DSH_LINE_MAX_WORDS + 1U);
        if (rc < 0)
                return -1;
        len = (unsigned int)record[0];
        if (len > DSH_LINE_MAX_CHARS)
                return -1;
        dsh_line_clear(&dsh_input_line);
        dsh_input_line.len = len;
        words = (len + DSH_S6_CHARS_PER_WORD - 1U) /
            DSH_S6_CHARS_PER_WORD;
        for (i = 0U; i < words; ++i)
                dsh_input_line.words[i] = record[i + 1U];
        return 0;
}

static int
dsh_s6_literal(struct dsh_s6 *dst, const char *text)
{
        unsigned int i;
        int ch;

        dsh_s6_clear(dst);
        for (i = 0U; text[i] != 0; ++i) {
                ch = (unsigned char)text[i];
                if (ch >= 'a' && ch <= 'z')
                        ch -= 'a' - 'A';
                if (dsh_s6_append(dst, ch) != 0)
                        return -1;
        }
        return 0;
}

static int
dsh_line_from_counted(struct dsh_line *line, const kword_t *record)
{
        unsigned int chars;
        unsigned int words;
        unsigned int i;

        if (line == 0 || record == 0)
                return -1;
        chars = (unsigned int)(record[0] & 0777777UL);
        if (chars > DSH_LINE_MAX_CHARS)
                return -1;
        dsh_line_clear(line);
        line->len = chars;
        words = (chars + DSH_S6_CHARS_PER_WORD - 1U) /
            DSH_S6_CHARS_PER_WORD;
        for (i = 0U; i < words; ++i)
                line->words[i] = record[i + 1U];
        return 0;
}

static int
dsh_set_counted_args(struct dsh_state *st, kword_t **argv,
    unsigned int first, unsigned int count)
{
        unsigned int i;

        if (count > DSH_MAX_ARGS)
                return -1;
        st->argc = count;
        for (i = 0U; i < DSH_MAX_ARGS; ++i)
                dsh_s6_clear(&st->args[i]);
        for (i = 0U; i < count; ++i)
                if (dsh_s6_from_counted(&st->args[i],
                    argv[first + i]) != 0)
                        return -1;
        return 0;
}

static void
dsh_put_prompt(struct dsh_state *st, unsigned int more)
{
        struct dsh_s6 name;
        const struct dsh_s6 *value;

        if (dsh_s6_literal(&name, more ? "PS2" : "PS1") == 0) {
                value = dsh_var_get(st, &name);
                if (value != 0 && value->len != 0U) {
                        (void)dsh_s6_put(1, value);
                        return;
                }
        }
        (void)u_puts(1, more ? "> " : "# ");
}

static int
dsh_login_profile(struct dsh_state *st, const struct dsh_s6 *path)
{
        struct vfs_stat sb;
        kword_t packed[U_PATH_WORDS];
        int rc;

        if (dsh_s6_pack(path, packed, U_PATH_WORDS) != 0)
                return DSH_ERROR;
        if (dsys_stat(packed, &sb) != 0)
                return 0;               /* missing profiles are optional */
        rc = dsh_execute_file(st, path);
        if (rc == 0)
                return 0;
        (void)u_puts(2, "DSH: PROFILE: ");
        (void)dsh_s6_put(2, path);
        (void)u_crlf(2);
        return rc;
}

static void
dsh_login_profiles(struct dsh_state *st)
{
        struct dsh_s6 path;
        struct dsh_s6 home_name;
        const struct dsh_s6 *home;
        const char *suffix;
        unsigned int i;
        int profile_status;
        int rc;

        profile_status = 0;
        if (dsh_s6_literal(&path, "/CONFIG/DSH.PROFILE") == 0)
                profile_status = dsh_login_profile(st, &path);
        if (dsh_s6_literal(&home_name, "HOME") != 0)
                goto done;
        home = dsh_var_get(st, &home_name);
        if (home == 0 || home->len == 0U)
                goto done;
        if (dsh_s6_copy(&path, home) != 0)
                goto done;
        suffix = "/.DSH.PROFILE";
        for (i = 0U; suffix[i] != 0; ++i)
                if (dsh_s6_append(&path, suffix[i]) != 0)
                        goto done;
        rc = dsh_login_profile(st, &path);
        if (rc != 0 && profile_status == 0)
                profile_status = rc;
done:
        if (profile_status != 0)
                st->status = (unsigned int)profile_status;
}

static int
dsh_run_loop(struct dsh_state *st, int interactive, struct dsh_script *script)
{
        struct u_text_reader reader;
        unsigned int need_more;
        int rc;
        int status;

        reader.fd = -1;
        if (!interactive && u_text_open_fd(&reader, 0) != 0)
                return DSH_ERROR;
        dsh_script_init(script);
        need_more = 0U;
        while (!st->exit_requested) {
                if (interactive)
                        dsh_put_prompt(st, need_more);
                rc = interactive ? dsh_getline_tty(1) :
                    dsh_getline_text(&reader);
                if (rc < 0)
                        break;
                if (rc > 0) {
                        st->status = 1U;
                        dsh_script_init(script);
                        need_more = 0U;
                        continue;
                }
                rc = dsh_script_feed(st, script, &dsh_input_line,
                    &status, &need_more);
                if (rc != DSH_OK) {
                        (void)u_puts(2, "DSH: SYNTAX");
                        (void)u_crlf(2);
                        st->status = DSH_ERROR;
                        dsh_script_init(script);
                        need_more = 0U;
                        continue;
                }
                st->status = (unsigned int)status;
        }
        if (!st->exit_requested && script->ntokens != 0U) {
                rc = dsh_script_finish(st, script, &status);
                st->status = rc == DSH_OK ? (unsigned int)status :
                    DSH_ERROR;
        }
        if (!interactive)
                u_text_close(&reader);
        return st->exit_requested ? (int)st->exit_status : (int)st->status;
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct dsh_script script_workspace;
        struct dsh_s6 mode;
        int interactive;
        int login;
        int rc;

        dsh_script_workspace_set(&script_workspace);
        dsh_state_init(&dsh_main_state, argc, argv, envp);
        interactive = dsys_isatty(0) >= 0;
        login = 0;

        if (argc > 1) {
                if (dsh_s6_from_counted(&mode, argv[1]) != 0)
                        return DSH_ERROR;
                if (dsh_s6_eq_text(&mode, "-L")) {
                        if (argc != 2)
                                return DSH_ERROR;
                        login = 1;
                } else if (dsh_s6_eq_text(&mode, "-C")) {
                        if (argc < 3 ||
                            dsh_line_from_counted(&dsh_input_line,
                            argv[2]) != 0 ||
                            dsh_set_counted_args(&dsh_main_state, argv, 3U,
                            (unsigned int)(argc - 3)) != 0)
                                return DSH_ERROR;
                        rc = dsh_execute_line(&dsh_main_state,
                            &dsh_input_line);
                        return dsh_main_state.exit_requested ?
                            (int)dsh_main_state.exit_status : rc;
                } else {
                        if (dsh_s6_copy(&dsh_main_state.arg0, &mode) != 0 ||
                            dsh_set_counted_args(&dsh_main_state, argv, 2U,
                            (unsigned int)(argc - 2)) != 0)
                                return DSH_ERROR;
                        rc = dsh_execute_file(&dsh_main_state, &mode);
                        return dsh_main_state.exit_requested ?
                            (int)dsh_main_state.exit_status : rc;
                }
        }

        dsh_main_state.interactive = interactive ? 1U : 0U;
        if (interactive &&
            dsys_procctl(SYS_PROCCTL_TTY_SETMODE, SYS_TTY_MODE_RAW) !=
            (int)SYS_TTY_MODE_RAW)
                return 1;
        if (login)
                dsh_login_profiles(&dsh_main_state);
        if (interactive) {
                (void)u_puts(1, "DSH V1");
                (void)u_crlf(1);
        }
        rc = dsh_run_loop(&dsh_main_state, interactive, &script_workspace);
        if (interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_COOKED);
        return rc;
}
