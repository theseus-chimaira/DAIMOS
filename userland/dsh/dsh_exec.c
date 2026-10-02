#include "dsh.h"
#include "text.h"

#define DSH_REC_WORDS (DSH_S6_MAX_WORDS + 1U)
#define DSH_RUN_WORDS 192U

struct dsh_word {
        struct dsh_s6 text;
        unsigned int expand;
};

static kword_t dsh_run_block[DSH_RUN_WORDS];
static kword_t dsh_path_record[U_PATH_WORDS];

static int dsh_source_file(struct dsh_state *st, const struct dsh_s6 *path);

static int
dsh_copy_range(struct dsh_s6 *dst, const struct dsh_s6 *src,
    unsigned int first, unsigned int last)
{
        unsigned int i;

        dsh_s6_clear(dst);
        for (i = first; i < last; ++i)
                if (dsh_s6_append(dst, dsh_s6_get(src, i)) != 0)
                        return -1;
        return 0;
}

static int
dsh_find_equal(const struct dsh_s6 *s)
{
        unsigned int i;

        for (i = 0U; i < s->len; ++i)
                if (dsh_s6_get(s, i) == '=')
                        return (int)i;
        return -1;
}

static int
dsh_tokenize(const struct dsh_line *line, struct dsh_word *words,
    unsigned int *argcp, int *in_at, int *out_at, int *append)
{
        unsigned int p;
        unsigned int argc;
        int ch;
        int quote;

        p = 0U;
        argc = 0U;
        *in_at = -1;
        *out_at = -1;
        *append = 0;
        while (p < line->len) {
                while (p < line->len) {
                        unsigned int wi = p / 6U;
                        unsigned int sh = 30U - (p % 6U) * 6U;
                        ch = (int)(((line->words[wi] >> sh) & 077UL) + 040U);
                        if (ch != ' ')
                                break;
                        ++p;
                }
                if (p >= line->len)
                        break;
                if (argc >= DSH_MAX_ARGS)
                        return -1;
                dsh_s6_clear(&words[argc].text);
                words[argc].expand = 1U;
                quote = 0;
                while (p < line->len) {
                        unsigned int wi = p / 6U;
                        unsigned int sh = 30U - (p % 6U) * 6U;
                        ch = (int)(((line->words[wi] >> sh) & 077UL) + 040U);
                        if (quote == 0 && ch == ' ')
                                break;
                        ++p;
                        if (quote == 0 && (ch == '\'' || ch == '"')) {
                                quote = ch;
                                if (quote == '\'')
                                        words[argc].expand = 0U;
                                continue;
                        }
                        if (quote != 0 && ch == quote) {
                                quote = 0;
                                continue;
                        }
                        if (ch == '\\' && p < line->len) {
                                wi = p / 6U;
                                sh = 30U - (p % 6U) * 6U;
                                ch = (int)(((line->words[wi] >> sh) & 077UL) +
                                    040U);
                                ++p;
                        }
                        if (dsh_s6_append(&words[argc].text, ch) != 0)
                                return -1;
                }
                if (quote != 0)
                        return -1;
                if (dsh_s6_eq_text(&words[argc].text, "<"))
                        *in_at = (int)argc;
                else if (dsh_s6_eq_text(&words[argc].text, ">")) {
                        *out_at = (int)argc;
                        *append = 0;
                } else if (dsh_s6_eq_text(&words[argc].text, ">>")) {
                        *out_at = (int)argc;
                        *append = 1;
                }
                ++argc;
        }
        *argcp = argc;
        return 0;
}

static int
dsh_parse_octal(const struct dsh_s6 *s, unsigned int *vp)
{
        unsigned int i;
        unsigned int v;
        int ch;

        if (s->len == 0U || s->len > 3U)
                return -1;
        v = 0U;
        for (i = 0U; i < s->len; ++i) {
                ch = dsh_s6_get(s, i);
                if (ch < '0' || ch > '7')
                        return -1;
                v = (v << 3) | (unsigned int)(ch - '0');
        }
        *vp = v;
        return 0;
}

static int
dsh_builtin(struct dsh_state *st, unsigned int argc, struct dsh_s6 *argv)
{
        struct dsh_s6 name;
        struct dsh_s6 value;
        const struct dsh_s6 *v;
        unsigned int i;
        unsigned int mask;
        int eq;

        if (dsh_s6_eq_text(&argv[0], "EXIT")) {
                st->exit_requested = 1U;
                st->exit_status = argc > 1U ? st->status : 0U;
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "CD")) {
                if (argc != 2U || dsh_s6_pack(&argv[1], dsh_path_record,
                    U_PATH_WORDS) != 0 || dsys_chdir(dsh_path_record) != 0)
                        return 1;
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "SET")) {
                for (i = 0U; i < DSH_MAX_VARS; ++i) {
                        if (!st->vars[i].used)
                                continue;
                        if (dsh_s6_put(1, &st->vars[i].name) != 0 ||
                            u_putc(1, '=') != 0 ||
                            dsh_s6_put(1, &st->vars[i].value) != 0 ||
                            u_crlf(1) != 0)
                                return 1;
                }
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "UNSET"))
                return argc == 2U ? dsh_var_unset(st, &argv[1]) != 0 : 2;
        if (dsh_s6_eq_text(&argv[0], "EXPORT")) {
                if (argc != 2U)
                        return 2;
                eq = dsh_find_equal(&argv[1]);
                if (eq >= 0) {
                        if (dsh_copy_range(&name, &argv[1], 0U,
                            (unsigned int)eq) != 0 ||
                            dsh_copy_range(&value, &argv[1],
                            (unsigned int)eq + 1U, argv[1].len) != 0)
                                return 2;
                        return dsh_var_set(st, &name, &value, 1) != 0;
                }
                v = dsh_var_get(st, &argv[1]);
                dsh_s6_clear(&value);
                if (v != 0)
                        (void)dsh_s6_copy(&value, v);
                return dsh_var_set(st, &argv[1], &value, 1) != 0;
        }
        if (dsh_s6_eq_text(&argv[0], "UMASK")) {
                if (argc == 1U) {
                        int old = dsys_umask(0U);
                        if (old < 0)
                                return 1;
                        (void)dsys_umask((unsigned int)old);
                        return u_put_octal(1, (kword_t)old, 3U) != 0 ||
                            u_crlf(1) != 0;
                }
                if (argc != 2U || dsh_parse_octal(&argv[1], &mask) != 0)
                        return 2;
                return dsys_umask(mask & 0777U) < 0;
        }
        if (dsh_s6_eq_text(&argv[0], "SHIFT")) {
                if (argc != 1U || st->argc == 0U)
                        return 1;
                for (i = 1U; i < st->argc; ++i)
                        (void)dsh_s6_copy(&st->args[i - 1U], &st->args[i]);
                --st->argc;
                dsh_s6_clear(&st->args[st->argc]);
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "READ")) {
                int ch;

                if (argc != 2U)
                        return 2;
                dsh_s6_clear(&value);
                for (;;) {
                        ch = dsys_readchar(0);
                        if (ch == -2)
                                continue;
                        if (ch < 0)
                                return 1;
                        if (ch == '\r' || ch == '\n')
                                break;
                        if (ch >= 'a' && ch <= 'z')
                                ch -= 'a' - 'A';
                        if (ch >= 040 && ch <= 0137 &&
                            dsh_s6_append(&value, ch) != 0)
                                return 1;
                }
                return dsh_var_set(st, &argv[1], &value, 0) != 0;
        }
        if (dsh_s6_eq_text(&argv[0], "SOURCE") ||
            dsh_s6_eq_text(&argv[0], ".")) {
                if (argc != 2U)
                        return 2;
                return dsh_source_file(st, &argv[1]);
        }
        if (dsh_s6_eq_text(&argv[0], "RETURN"))
                return 2;
        if (dsh_s6_eq_text(&argv[0], "JOBS"))
                return argc == 1U ? 0 : 2;
        if (dsh_s6_eq_text(&argv[0], "WAIT")) {
                kword_t status;
                int pid;

                if (argc != 1U)
                        return 2;
                pid = dsys_wait(0U, &status, 0U);
                if (pid < 0)
                        return 0;
                if (SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                        return 1;
                return (int)SYS_WAIT_STATUS_VALUE(status);
        }
        if (dsh_s6_eq_text(&argv[0], "FG") ||
            dsh_s6_eq_text(&argv[0], "BG"))
                return 1;
        if (dsh_s6_eq_text(&argv[0], "ALIAS")) {
                if (argc == 1U) {
                        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                                if (st->aliases[i].used) {
                                        (void)dsh_s6_put(1,
                                            &st->aliases[i].name);
                                        (void)u_putc(1, '=');
                                        (void)dsh_s6_put(1,
                                            &st->aliases[i].value);
                                        (void)u_crlf(1);
                                }
                        return 0;
                }
                if (argc != 2U || (eq = dsh_find_equal(&argv[1])) <= 0)
                        return 2;
                if (dsh_copy_range(&name, &argv[1], 0U,
                    (unsigned int)eq) != 0 ||
                    dsh_copy_range(&value, &argv[1], (unsigned int)eq + 1U,
                    argv[1].len) != 0)
                        return 2;
                for (i = 0U; i < DSH_MAX_ALIASES; ++i) {
                        if (!st->aliases[i].used) {
                                st->aliases[i].used = 1U;
                                (void)dsh_s6_copy(&st->aliases[i].name, &name);
                                (void)dsh_s6_copy(&st->aliases[i].value, &value);
                                return 0;
                        }
                }
                return 1;
        }
        if (dsh_s6_eq_text(&argv[0], "UNALIAS")) {
                if (argc != 2U)
                        return 2;
                for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                        if (st->aliases[i].used &&
                            st->aliases[i].name.len == argv[1].len) {
                                unsigned int j;
                                for (j = 0U; j < argv[1].len; ++j)
                                        if (dsh_s6_get(&st->aliases[i].name,
                                            j) != dsh_s6_get(&argv[1], j))
                                                break;
                                if (j == argv[1].len) {
                                        st->aliases[i].used = 0U;
                                        return 0;
                                }
                        }
                return 0;
        }
        return -1;
}

static int
dsh_source_file(struct dsh_state *st, const struct dsh_s6 *path)
{
        struct u_text_reader reader;
        struct dsh_line line;
        char buf[DSH_LINE_MAX_CHARS + 1U];
        unsigned int i;
        int fd;
        int n;
        int rc;
        int ch;

        if (dsh_s6_pack(path, dsh_path_record, U_PATH_WORDS) != 0)
                return 1;
        fd = dsys_open(dsh_path_record, SYS_O_RDONLY);
        if (fd < 0)
                return 1;
        if (u_text_open_fd(&reader, fd) != 0) {
                (void)dsys_close(fd);
                return 1;
        }
        rc = 0;
        for (;;) {
                n = u_text_getline(&reader, buf, sizeof(buf));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0) {
                        rc = 1;
                        break;
                }
                line.len = 0U;
                for (i = 0U; i < DSH_LINE_MAX_WORDS; ++i)
                        line.words[i] = 0;
                for (i = 0U; i < (unsigned int)n; ++i) {
                        unsigned int wi;
                        unsigned int sh;

                        ch = (unsigned char)buf[i];
                        if (ch >= 'a' && ch <= 'z')
                                ch -= 'a' - 'A';
                        if (ch < 040 || ch > 0137 ||
                            line.len >= DSH_LINE_MAX_CHARS) {
                                rc = 2;
                                break;
                        }
                        wi = line.len / 6U;
                        sh = 30U - (line.len % 6U) * 6U;
                        line.words[wi] |=
                            ((kword_t)(ch - 040) & 077UL) << sh;
                        ++line.len;
                }
                if (rc != 0)
                        break;
                rc = dsh_execute_line(st, &line);
                st->status = (unsigned int)rc;
                if (st->exit_requested)
                        break;
        }
        u_text_close(&reader);
        return rc;
}

static unsigned int
dsh_record_words(const kword_t *p)
{
        return 1U + ((unsigned int)p[0] + 5U) / 6U;
}

static int
dsh_append_record(kword_t *block, unsigned int *used,
    const struct dsh_s6 *s)
{
        kword_t rec[DSH_REC_WORDS];
        unsigned int i;
        unsigned int n;

        if (dsh_s6_pack(s, rec, DSH_REC_WORDS) != 0)
                return -1;
        n = dsh_record_words(rec);
        if (*used + n > DSH_RUN_WORDS)
                return -1;
        for (i = 0U; i < n; ++i)
                block[(*used)++] = rec[i];
        return 0;
}

static int
dsh_run_path(struct dsh_state *st, const struct dsh_s6 *path,
    unsigned int argc, struct dsh_s6 *argv, int infd, int outfd)
{
        struct sys_run_v2 *run;
        struct dsh_s6 env;
        unsigned int used;
        unsigned int envc;
        unsigned int i;
        unsigned int j;
        int pid;
        kword_t status;

        used = SYS_RUN_V2_FIXED_WORDS;
        if (dsh_append_record(dsh_run_block, &used, path) != 0)
                return DSH_ERROR;
        for (i = 0U; i < argc; ++i)
                if (dsh_append_record(dsh_run_block, &used, &argv[i]) != 0)
                        return DSH_ERROR;
        envc = 0U;
        for (i = 0U; i < DSH_MAX_VARS; ++i) {
                if (!st->vars[i].used || !st->vars[i].exported)
                        continue;
                dsh_s6_clear(&env);
                for (j = 0U; j < st->vars[i].name.len; ++j)
                        if (dsh_s6_append(&env,
                            dsh_s6_get(&st->vars[i].name, j)) != 0)
                                return DSH_ERROR;
                if (dsh_s6_append(&env, '=') != 0)
                        return DSH_ERROR;
                for (j = 0U; j < st->vars[i].value.len; ++j)
                        if (dsh_s6_append(&env,
                            dsh_s6_get(&st->vars[i].value, j)) != 0)
                                return DSH_ERROR;
                if (dsh_append_record(dsh_run_block, &used, &env) != 0)
                        return DSH_ERROR;
                ++envc;
        }
        if (used + 3U > DSH_RUN_WORDS)
                return DSH_ERROR;
        dsh_run_block[used++] = SYS_RUN_FD_MAP(0U, (unsigned int)infd);
        dsh_run_block[used++] = SYS_RUN_FD_MAP(1U, (unsigned int)outfd);
        dsh_run_block[used++] = SYS_RUN_FD_MAP(2U, 2U);
        run = (struct sys_run_v2 *)dsh_run_block;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, used);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = argc;
        run->envc = envc;
        pid = dsys_run(run);
        if (pid < 0)
                return DSH_NOT_FOUND;
        if (dsys_wait((unsigned int)pid, &status, 0U) != pid ||
            SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                return DSH_ERROR;
        return (int)SYS_WAIT_STATUS_VALUE(status);
}

static int
dsh_make_path(struct dsh_s6 *out, const struct dsh_s6 *dir,
    const struct dsh_s6 *cmd)
{
        unsigned int i;

        dsh_s6_clear(out);
        for (i = 0U; i < dir->len; ++i)
                if (dsh_s6_append(out, dsh_s6_get(dir, i)) != 0)
                        return -1;
        if (out->len != 0U && dsh_s6_get(out, out->len - 1U) != '/' &&
            dsh_s6_append(out, '/') != 0)
                return -1;
        for (i = 0U; i < cmd->len; ++i)
                if (dsh_s6_append(out, dsh_s6_get(cmd, i)) != 0)
                        return -1;
        return 0;
}

static int
dsh_run_external(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd)
{
        struct dsh_s6 path;
        struct dsh_s6 dir;
        struct dsh_s6 pname;
        const struct dsh_s6 *pval;
        unsigned int i;
        unsigned int start;
        int rc;

        for (i = 0U; i < argv[0].len; ++i)
                if (dsh_s6_get(&argv[0], i) == '/')
                        return dsh_run_path(st, &argv[0], argc, argv,
                            infd, outfd);
        dsh_s6_clear(&pname);
        (void)dsh_s6_append(&pname, 'P');
        (void)dsh_s6_append(&pname, 'A');
        (void)dsh_s6_append(&pname, 'T');
        (void)dsh_s6_append(&pname, 'H');
        pval = dsh_var_get(st, &pname);
        if (pval == 0)
                return DSH_NOT_FOUND;
        start = 0U;
        for (i = 0U; i <= pval->len; ++i) {
                if (i != pval->len && dsh_s6_get(pval, i) != ':')
                        continue;
                if (dsh_copy_range(&dir, pval, start, i) != 0 ||
                    dsh_make_path(&path, &dir, &argv[0]) != 0)
                        return DSH_ERROR;
                rc = dsh_run_path(st, &path, argc, argv, infd, outfd);
                if (rc != DSH_NOT_FOUND)
                        return rc;
                start = i + 1U;
        }
        return DSH_NOT_FOUND;
}

int
dsh_execute_line(struct dsh_state *st, const struct dsh_line *line)
{
        struct dsh_word words[DSH_MAX_ARGS];
        struct dsh_s6 argv[DSH_MAX_ARGS];
        struct dsh_s6 name;
        struct dsh_s6 value;
        unsigned int argc;
        unsigned int i;
        unsigned int outc;
        int in_at;
        int out_at;
        int append;
        int infd;
        int outfd;
        int eq;
        int rc;

        if (dsh_tokenize(line, words, &argc, &in_at, &out_at, &append) != 0)
                return DSH_ERROR;
        if (argc == 0U)
                return 0;
        for (i = 0U; i < argc; ++i) {
                if (words[i].expand) {
                        if (dsh_expand(st, &words[i].text, &argv[i]) != 0)
                                return DSH_ERROR;
                } else
                        (void)dsh_s6_copy(&argv[i], &words[i].text);
        }
        if (argc == 1U && (eq = dsh_find_equal(&argv[0])) > 0) {
                if (dsh_copy_range(&name, &argv[0], 0U, (unsigned int)eq) != 0 ||
                    dsh_copy_range(&value, &argv[0], (unsigned int)eq + 1U,
                    argv[0].len) != 0)
                        return DSH_ERROR;
                return dsh_var_set(st, &name, &value, 0) != 0;
        }
        infd = 0;
        outfd = 1;
        outc = argc;
        if (in_at >= 0) {
                if ((unsigned int)in_at + 1U >= argc)
                        return DSH_ERROR;
                if (dsh_s6_pack(&argv[in_at + 1], dsh_path_record,
                    U_PATH_WORDS) != 0)
                        return DSH_ERROR;
                infd = dsys_open(dsh_path_record, SYS_O_RDONLY);
                if (infd < 0)
                        return 1;
                if ((unsigned int)in_at < outc)
                        outc = (unsigned int)in_at;
        }
        if (out_at >= 0) {
                if ((unsigned int)out_at + 1U >= argc) {
                        if (infd != 0) (void)dsys_close(infd);
                        return DSH_ERROR;
                }
                if (dsh_s6_pack(&argv[out_at + 1], dsh_path_record,
                    U_PATH_WORDS) != 0) {
                        if (infd != 0) (void)dsys_close(infd);
                        return DSH_ERROR;
                }
                outfd = dsys_open(dsh_path_record, SYS_O_WRONLY |
                    SYS_O_CREAT | (append ? SYS_O_APPEND : SYS_O_TRUNC));
                if (outfd < 0) {
                        if (infd != 0) (void)dsys_close(infd);
                        return 1;
                }
                if ((unsigned int)out_at < outc)
                        outc = (unsigned int)out_at;
        }
        rc = dsh_builtin(st, outc, argv);
        if (rc < 0)
                rc = dsh_run_external(st, outc, argv, infd, outfd);
        if (infd != 0)
                (void)dsys_close(infd);
        if (outfd != 1)
                (void)dsys_close(outfd);
        if (rc == DSH_NOT_FOUND) {
                (void)u_puts(2, "DSH: NOT FOUND: ");
                (void)dsh_s6_put(2, &argv[0]);
                (void)u_crlf(2);
        }
        return rc;
}
