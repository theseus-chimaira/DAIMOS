#include "dsh.h"
#include "dsh_lex.h"
#include "dsh_parse.h"
#include "cmdmap.h"
#include "text.h"

#define DSH_REC_WORDS (DSH_S6_MAX_WORDS + 1U)
#define DSH_RUN_WORDS 192U

static kword_t dsh_run_block[DSH_RUN_WORDS];
static kword_t dsh_path_record[U_PATH_WORDS];
static struct dsh_node dsh_nodes[DSH_PARSE_MAX_NODES];

struct dsh_function_store {
        unsigned int used;
        struct dsh_s6 name;
        struct dsh_node nodes[DSH_FUNC_MAX_NODES];
        unsigned int root;
        unsigned int used_nodes;
};

static struct dsh_function_store dsh_functions[DSH_MAX_FUNCS];

static int dsh_s6_same(const struct dsh_s6 *a, const struct dsh_s6 *b);
static int dsh_job_store(struct dsh_state *st, unsigned int pgrp,
    unsigned int last_pid, unsigned int remaining, unsigned int status,
    unsigned int stopped);
static int dsh_builtin_jobs(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv);
static int dsh_builtin_wait(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv);
static int dsh_builtin_fg(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv);
static int dsh_builtin_bg(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv);
static int dsh_exec_node(struct dsh_state *st,
    const struct dsh_node *nodes, unsigned int node);

static const struct dsh_function_store *
dsh_find_function(const struct dsh_s6 *name)
{
        unsigned int i;

        for (i = 0U; i < DSH_MAX_FUNCS; ++i)
                if (dsh_functions[i].used &&
                    dsh_s6_same(&dsh_functions[i].name, name))
                        return &dsh_functions[i];
        return 0;
}

static int
dsh_copy_function_node(struct dsh_node *dst, unsigned int *usedp,
    const struct dsh_node *src, unsigned int index, unsigned int *out)
{
        unsigned int here;
        int rc;

        if (index == DSH_NONE || index >= DSH_PARSE_MAX_NODES ||
            *usedp >= DSH_FUNC_MAX_NODES)
                return DSH_E_OVERFLOW;
        here = (*usedp)++;
        dst[here] = src[index];
        if (src[index].left != DSH_NONE) {
                rc = dsh_copy_function_node(dst, usedp, src,
                    src[index].left, &dst[here].left);
                if (rc != DSH_OK)
                        return rc;
        }
        if (src[index].right != DSH_NONE) {
                rc = dsh_copy_function_node(dst, usedp, src,
                    src[index].right, &dst[here].right);
                if (rc != DSH_OK)
                        return rc;
        }
        if (src[index].extra != DSH_NONE) {
                rc = dsh_copy_function_node(dst, usedp, src,
                    src[index].extra, &dst[here].extra);
                if (rc != DSH_OK)
                        return rc;
        }
        *out = here;
        return DSH_OK;
}

static int
dsh_define_function(const struct dsh_node *nodes, const struct dsh_node *n)
{
        unsigned int i;
        unsigned int slot;
        int found;
        int rc;

        if (n->argc == 0U || n->left == DSH_NONE)
                return DSH_ERROR;
        found = 0;
        slot = 0U;
        for (i = 0U; i < DSH_MAX_FUNCS; ++i)
                if (dsh_functions[i].used &&
                    dsh_s6_same(&dsh_functions[i].name, &n->words[0])) {
                        slot = i;
                        found = 1;
                        break;
                }
        if (!found)
                for (i = 0U; i < DSH_MAX_FUNCS; ++i)
                        if (!dsh_functions[i].used) {
                                slot = i;
                                found = 1;
                                break;
                        }
        if (!found)
                return DSH_ERROR;
        dsh_functions[slot].used = 1U;
        if (dsh_s6_copy(&dsh_functions[slot].name, &n->words[0]) != 0)
                return DSH_ERROR;
        dsh_functions[slot].used_nodes = 0U;
        dsh_functions[slot].root = DSH_NONE;
        rc = dsh_copy_function_node(dsh_functions[slot].nodes,
            &dsh_functions[slot].used_nodes, nodes, n->left,
            &dsh_functions[slot].root);
        if (rc != DSH_OK) {
                dsh_functions[slot].used = 0U;
                return DSH_ERROR;
        }
        return 0;
}

static int
dsh_exec_function(struct dsh_state *st, const struct dsh_function_store *fn,
    unsigned int argc, struct dsh_s6 *argv)
{
        struct dsh_s6 saved_arg0;
        struct dsh_s6 saved_args[DSH_MAX_ARGS];
        unsigned int saved_argc;
        unsigned int i;
        int status;

        if (st->call_depth >= DSH_FUNC_MAX_CALLS || argc == 0U)
                return DSH_ERROR;
        (void)dsh_s6_copy(&saved_arg0, &st->arg0);
        saved_argc = st->argc;
        for (i = 0U; i < DSH_MAX_ARGS; ++i)
                (void)dsh_s6_copy(&saved_args[i], &st->args[i]);
        (void)dsh_s6_copy(&st->arg0, &argv[0]);
        st->argc = argc - 1U;
        for (i = 0U; i < DSH_MAX_ARGS; ++i) {
                if (i < st->argc)
                        (void)dsh_s6_copy(&st->args[i], &argv[i + 1U]);
                else
                        dsh_s6_clear(&st->args[i]);
        }
        ++st->call_depth;
        st->return_requested = 0U;
        st->return_status = 0U;
        status = dsh_exec_node(st, fn->nodes, fn->root);
        if (st->return_requested) {
                status = (int)st->return_status;
                st->return_requested = 0U;
        }
        --st->call_depth;
        (void)dsh_s6_copy(&st->arg0, &saved_arg0);
        st->argc = saved_argc;
        for (i = 0U; i < DSH_MAX_ARGS; ++i)
                (void)dsh_s6_copy(&st->args[i], &saved_args[i]);
        return status;
}

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
dsh_parse_uint(const struct dsh_s6 *s, unsigned int *vp)
{
        unsigned int i;
        unsigned int v;
        int ch;

        if (s->len == 0U)
                return -1;
        v = 0U;
        for (i = 0U; i < s->len; ++i) {
                ch = dsh_s6_get(s, i);
                if (ch < '0' || ch > '9')
                        return -1;
                if (v > 077777U)
                        return -1;
                v = v * 10U + (unsigned int)(ch - '0');
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
        unsigned int code;
        int eq;

        if (dsh_s6_eq_text(&argv[0], "EXIT")) {
                if (argc > 2U)
                        return 2;
                code = st->status;
                if (argc == 2U && dsh_parse_uint(&argv[1], &code) != 0)
                        return 2;
                st->exit_requested = 1U;
                st->exit_status = code & 0377U;
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "CD")) {
                if (argc != 2U || dsh_s6_pack(&argv[1], dsh_path_record,
                    U_PATH_WORDS) != 0 || dsys_chdir(dsh_path_record) != 0)
                        return 1;
                return 0;
        }
        if (dsh_s6_eq_text(&argv[0], "SET")) {
                if (argc == 2U) {
                        eq = dsh_find_equal(&argv[1]);
                        if (eq <= 0 ||
                            dsh_copy_range(&name, &argv[1], 0U,
                            (unsigned int)eq) != 0 ||
                            dsh_copy_range(&value, &argv[1],
                            (unsigned int)eq + 1U, argv[1].len) != 0)
                                return 2;
                        return dsh_var_set(st, &name, &value, 0) != 0;
                }
                if (argc == 3U)
                        return dsh_var_set(st, &argv[1], &argv[2], 0) != 0;
                if (argc != 1U)
                        return 2;
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
                unsigned int count;

                if (argc > 2U)
                        return 2;
                count = 1U;
                if (argc == 2U && dsh_parse_uint(&argv[1], &count) != 0)
                        return 2;
                if (count > st->argc)
                        return 1;
                for (i = count; i < st->argc; ++i)
                        (void)dsh_s6_copy(&st->args[i - count],
                            &st->args[i]);
                st->argc -= count;
                for (i = st->argc; i < DSH_MAX_ARGS; ++i)
                        dsh_s6_clear(&st->args[i]);
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
                struct dsh_s6 saved[DSH_MAX_ARGS];
                unsigned int savedc;
                unsigned int j;
                int src_rc;

                if (argc < 2U)
                        return 2;
                savedc = st->argc;
                for (j = 0U; j < DSH_MAX_ARGS; ++j)
                        (void)dsh_s6_copy(&saved[j], &st->args[j]);
                if (argc > 2U) {
                        st->argc = argc - 2U;
                        for (j = 0U; j < st->argc; ++j)
                                (void)dsh_s6_copy(&st->args[j], &argv[j + 2U]);
                }
                src_rc = dsh_execute_file(st, &argv[1]);
                st->argc = savedc;
                for (j = 0U; j < DSH_MAX_ARGS; ++j)
                        (void)dsh_s6_copy(&st->args[j], &saved[j]);
                return src_rc;
        }
        if (dsh_s6_eq_text(&argv[0], "RETURN")) {
                if (st->call_depth == 0U || argc > 2U)
                        return 2;
                code = st->status;
                if (argc == 2U && dsh_parse_uint(&argv[1], &code) != 0)
                        return 2;
                st->return_requested = 1U;
                st->return_status = code & 0377U;
                return (int)st->return_status;
        }
        if (dsh_s6_eq_text(&argv[0], "JOBS"))
                return dsh_builtin_jobs(st, argc, argv);
        if (dsh_s6_eq_text(&argv[0], "WAIT"))
                return dsh_builtin_wait(st, argc, argv);
        if (dsh_s6_eq_text(&argv[0], "FG"))
                return dsh_builtin_fg(st, argc, argv);
        if (dsh_s6_eq_text(&argv[0], "BG"))
                return dsh_builtin_bg(st, argc, argv);
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
                for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                        if (st->aliases[i].used &&
                            dsh_s6_same(&st->aliases[i].name, &name))
                                break;
                if (i == DSH_MAX_ALIASES)
                        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                                if (!st->aliases[i].used)
                                        break;
                if (i != DSH_MAX_ALIASES) {
                        st->aliases[i].used = 1U;
                        (void)dsh_s6_copy(&st->aliases[i].name, &name);
                        (void)dsh_s6_copy(&st->aliases[i].value, &value);
                        return 0;
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

int
dsh_execute_file(struct dsh_state *st, const struct dsh_s6 *path)
{
        struct u_text_reader reader;
        struct dsh_script script;
        struct dsh_line line;
        char buf[DSH_LINE_MAX_CHARS + 1U];
        unsigned int i;
        unsigned int need_more;
        int fd;
        int n;
        int rc;
        int status;
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
        dsh_script_init(&script);
        rc = 0;
        status = 0;
        for (;;) {
                n = u_text_getline(&reader, buf, sizeof(buf));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0) {
                        rc = 1;
                        break;
                }
                dsh_line_clear(&line);
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
                need_more = 0U;
                rc = dsh_script_feed(st, &script, &line, &status,
                    &need_more);
                if (rc != DSH_OK) {
                        rc = DSH_ERROR;
                        break;
                }
                st->status = (unsigned int)status;
                if (st->exit_requested)
                        break;
        }
        if (rc == 0 && !st->exit_requested) {
                rc = dsh_script_finish(st, &script, &status);
                if (rc == DSH_OK)
                        rc = status;
                else
                        rc = DSH_ERROR;
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
dsh_append_env_record(kword_t *block, unsigned int *used,
    const struct dsh_s6 *name, const struct dsh_s6 *value)
{
        kword_t rec[U_ARG_WORDS];
        unsigned int len;
        unsigned int i;
        unsigned int pos;
        unsigned int wi;
        unsigned int sh;
        int ch;

        len = name->len + 1U + value->len;
        if (len > SYS_RUN_ARG_MAX_CHARS || len > (U_ARG_WORDS - 1U) * 6U)
                return -1;
        for (i = 0U; i < U_ARG_WORDS; ++i)
                rec[i] = 0;
        rec[0] = (kword_t)len;
        pos = 0U;
        for (i = 0U; i < name->len; ++i) {
                ch = dsh_s6_get(name, i);
                wi = 1U + pos / 6U;
                sh = 30U - (pos % 6U) * 6U;
                rec[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
                ++pos;
        }
        wi = 1U + pos / 6U;
        sh = 30U - (pos % 6U) * 6U;
        rec[wi] |= ((kword_t)('=' - 040) & 077UL) << sh;
        ++pos;
        for (i = 0U; i < value->len; ++i) {
                ch = dsh_s6_get(value, i);
                wi = 1U + pos / 6U;
                sh = 30U - (pos % 6U) * 6U;
                rec[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
                ++pos;
        }
        len = 1U + (len + 5U) / 6U;
        if (*used + len > DSH_RUN_WORDS)
                return -1;
        for (i = 0U; i < len; ++i)
                block[(*used)++] = rec[i];
        return 0;
}

static int
dsh_launch_path(struct dsh_state *st, const struct dsh_s6 *path,
    unsigned int argc, struct dsh_s6 *argv, int infd, int outfd,
    unsigned int pgrp_mode, unsigned int pgrp, int *pidp)
{
        struct sys_run_v2 *run;
        unsigned int used;
        unsigned int envc;
        unsigned int i;
        int pid;

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
                if (dsh_append_env_record(dsh_run_block, &used,
                    &st->vars[i].name, &st->vars[i].value) != 0)
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
        run->flags = pgrp_mode;
        run->pgrp = (kword_t)pgrp;
        run->fdmap_count = 3UL;
        run->argc = argc;
        run->envc = envc;
        pid = dsys_run(run);
        if (pid < 0)
                return DSH_NOT_FOUND;
        if (pidp != 0)
                *pidp = pid;
        return 0;
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
dsh_launch_external(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd, unsigned int pgrp_mode,
    unsigned int pgrp, int *pidp)
{
        struct dsh_s6 path;
        struct dsh_s6 mapped;
        struct dsh_s6 dir;
        struct dsh_s6 pname;
        const struct dsh_s6 *pval;
        unsigned int i;
        unsigned int start;
        int rc;

        for (i = 0U; i < argv[0].len; ++i)
                if (dsh_s6_get(&argv[0], i) == '/') {
                        rc = dsh_launch_path(st, &argv[0], argc, argv,
                            infd, outfd, pgrp_mode, pgrp, pidp);
                        if (rc != DSH_NOT_FOUND)
                                return rc;
                        if (dsh_s6_pack(&argv[0], dsh_path_record,
                            U_PATH_WORDS) == 0 &&
                            u_cmd_resolve(dsh_path_record, dsh_path_record,
                            U_PATH_WORDS) == 0 &&
                            dsh_s6_from_counted(&mapped, dsh_path_record) == 0)
                                return dsh_launch_path(st, &mapped, argc, argv,
                                    infd, outfd, pgrp_mode, pgrp, pidp);
                        return DSH_NOT_FOUND;
                }
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
                rc = dsh_launch_path(st, &path, argc, argv, infd, outfd,
                    pgrp_mode, pgrp, pidp);
                if (rc != DSH_NOT_FOUND)
                        return rc;
                start = i + 1U;
        }
        if (dsh_s6_pack(&argv[0], dsh_path_record, U_PATH_WORDS) == 0 &&
            u_cmd_resolve(dsh_path_record, dsh_path_record,
            U_PATH_WORDS) == 0 &&
            dsh_s6_from_counted(&mapped, dsh_path_record) == 0)
                return dsh_launch_path(st, &mapped, argc, argv, infd, outfd,
                    pgrp_mode, pgrp, pidp);
        return DSH_NOT_FOUND;
}

static int
dsh_run_external(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd)
{
        kword_t status;
        unsigned int kind;
        int pid;
        int rc;

        rc = dsh_launch_external(st, argc, argv, infd, outfd,
            SYS_RUN_PGRP_NEW, 0U, &pid);
        if (rc != 0)
                return rc;
        if (st->interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_COOKED);
        if (st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                    (unsigned int)pid);
        rc = dsys_wait((unsigned int)pid, &status, 0U);
        if (st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                    st->shell_pgrp);
        if (st->interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_RAW);
        if (rc != pid)
                return DSH_ERROR;
        kind = SYS_WAIT_STATUS_KIND(status);
        if (kind == SYS_WAIT_EXITED)
                return (int)SYS_WAIT_STATUS_VALUE(status);
        if (kind == SYS_WAIT_STOPPED) {
                if (dsh_job_store(st, (unsigned int)pid, (unsigned int)pid,
                    1U, 0U, 1U) < 0)
                        return DSH_ERROR;
                return 1;
        }
        return DSH_ERROR;
}

static int
dsh_s6_same(const struct dsh_s6 *a, const struct dsh_s6 *b)
{
        unsigned int i;

        if (a == 0 || b == 0 || a->len != b->len)
                return 0;
        for (i = 0U; i < a->len; ++i)
                if (dsh_s6_get(a, i) != dsh_s6_get(b, i))
                        return 0;
        return 1;
}

static int
dsh_alias_expand(struct dsh_state *st, struct dsh_s6 *argv,
    unsigned int *argcp)
{
        struct dsh_s6 repl[DSH_MAX_ARGS];
        unsigned int replc;
        unsigned int rest;
        unsigned int i;
        unsigned int p;
        int ch;

        if (*argcp == 0U || dsh_s6_eq_text(&argv[0], "ALIAS"))
                return 0;
        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                if (st->aliases[i].used &&
                    dsh_s6_same(&st->aliases[i].name, &argv[0]))
                        break;
        if (i == DSH_MAX_ALIASES)
                return 0;
        replc = 0U;
        p = 0U;
        while (p < st->aliases[i].value.len) {
                while (p < st->aliases[i].value.len &&
                    dsh_s6_get(&st->aliases[i].value, p) == ' ')
                        ++p;
                if (p >= st->aliases[i].value.len)
                        break;
                if (replc >= DSH_MAX_ARGS)
                        return -1;
                dsh_s6_clear(&repl[replc]);
                while (p < st->aliases[i].value.len &&
                    (ch = dsh_s6_get(&st->aliases[i].value, p)) != ' ') {
                        if (dsh_s6_append(&repl[replc], ch) != 0)
                                return -1;
                        ++p;
                }
                ++replc;
        }
        if (replc == 0U || replc + *argcp - 1U > DSH_MAX_ARGS)
                return -1;
        rest = *argcp - 1U;
        while (rest != 0U) {
                --rest;
                (void)dsh_s6_copy(&argv[replc + rest], &argv[rest + 1U]);
        }
        for (i = 0U; i < replc; ++i)
                (void)dsh_s6_copy(&argv[i], &repl[i]);
        *argcp += replc - 1U;
        return 0;
}

static int
dsh_builtin_fds(int infd, int outfd, int *saveinp, int *saveoutp)
{
        *saveinp = -1;
        *saveoutp = -1;
        if (infd != 0) {
                *saveinp = dsys_dup(0);
                if (*saveinp < 0 || dsys_dup2(infd, 0) < 0)
                        goto bad;
        }
        if (outfd != 1) {
                *saveoutp = dsys_dup(1);
                if (*saveoutp < 0 || dsys_dup2(outfd, 1) < 0)
                        goto bad;
        }
        return 0;
bad:
        if (*saveinp >= 0) {
                (void)dsys_dup2(*saveinp, 0);
                (void)dsys_close(*saveinp);
                *saveinp = -1;
        }
        if (*saveoutp >= 0) {
                (void)dsys_dup2(*saveoutp, 1);
                (void)dsys_close(*saveoutp);
                *saveoutp = -1;
        }
        return -1;
}

static void
dsh_builtin_fds_restore(int savein, int saveout)
{
        if (savein >= 0) {
                (void)dsys_dup2(savein, 0);
                (void)dsys_close(savein);
        }
        if (saveout >= 0) {
                (void)dsys_dup2(saveout, 1);
                (void)dsys_close(saveout);
        }
}

static int
dsh_open_node_fds(const struct dsh_node *n, int basein, int baseout,
    int *infdp, int *outfdp)
{
        int infd;
        int outfd;

        infd = basein;
        outfd = baseout;
        if ((n->flags & DSH_REDIR_IN) != 0U) {
                if (dsh_s6_pack(&n->redir_in, dsh_path_record,
                    U_PATH_WORDS) != 0)
                        return -1;
                infd = dsys_open(dsh_path_record, SYS_O_RDONLY);
                if (infd < 0)
                        return -1;
        }
        if ((n->flags & DSH_REDIR_OUT) != 0U) {
                if (dsh_s6_pack(&n->redir_out, dsh_path_record,
                    U_PATH_WORDS) != 0) {
                        if (infd != basein)
                                (void)dsys_close(infd);
                        return -1;
                }
                outfd = dsys_open(dsh_path_record, SYS_O_WRONLY |
                    SYS_O_CREAT |
                    ((n->flags & DSH_REDIR_APPEND) != 0U ?
                    SYS_O_APPEND : SYS_O_TRUNC));
                if (outfd < 0) {
                        if (infd != basein)
                                (void)dsys_close(infd);
                        return -1;
                }
        }
        *infdp = infd;
        *outfdp = outfd;
        return 0;
}

static void
dsh_close_node_fds(const struct dsh_node *n, int basein, int baseout,
    int infd, int outfd)
{
        if ((n->flags & DSH_REDIR_IN) != 0U && infd != basein)
                (void)dsys_close(infd);
        if ((n->flags & DSH_REDIR_OUT) != 0U && outfd != baseout)
                (void)dsys_close(outfd);
}

static int
dsh_is_builtin_name(const struct dsh_s6 *name)
{
        return dsh_s6_eq_text(name, "EXIT") ||
            dsh_s6_eq_text(name, "CD") ||
            dsh_s6_eq_text(name, "SET") ||
            dsh_s6_eq_text(name, "UNSET") ||
            dsh_s6_eq_text(name, "EXPORT") ||
            dsh_s6_eq_text(name, "UMASK") ||
            dsh_s6_eq_text(name, "READ") ||
            dsh_s6_eq_text(name, "SHIFT") ||
            dsh_s6_eq_text(name, "RETURN") ||
            dsh_s6_eq_text(name, "SOURCE") ||
            dsh_s6_eq_text(name, ".") ||
            dsh_s6_eq_text(name, "WAIT") ||
            dsh_s6_eq_text(name, "JOBS") ||
            dsh_s6_eq_text(name, "FG") ||
            dsh_s6_eq_text(name, "BG") ||
            dsh_s6_eq_text(name, "ALIAS") ||
            dsh_s6_eq_text(name, "UNALIAS");
}

static int
dsh_expand_node_argv(struct dsh_state *st, const struct dsh_node *n,
    struct dsh_s6 *argv, unsigned int *argcp)
{
        unsigned int i;

        if (n->argc > DSH_MAX_ARGS)
                return -1;
        for (i = 0U; i < n->argc; ++i)
                if (dsh_expand_mask(st, &n->words[i],
                    n->literal_mask[i], &argv[i]) != 0)
                        return -1;
        *argcp = n->argc;
        return dsh_alias_expand(st, argv, argcp);
}

static int
dsh_exec_simple_node(struct dsh_state *st, const struct dsh_node *n,
    int basein, int baseout, int launch_only, unsigned int pgrp_mode,
    unsigned int pgrp, int *pidp)
{
        struct dsh_s6 argv[DSH_MAX_ARGS];
        struct dsh_s6 name;
        struct dsh_s6 value;
        unsigned int argc;
        int infd;
        int outfd;
        int savein;
        int saveout;
        int eq;
        int rc;
        const struct dsh_function_store *fn;

        if (dsh_expand_node_argv(st, n, argv, &argc) != 0)
                return DSH_ERROR;
        if (argc == 0U)
                return 0;
        fn = dsh_find_function(&argv[0]);
        if (fn != 0) {
                if (launch_only)
                        return 126;
                return dsh_exec_function(st, fn, argc, argv);
        }
        if (argc == 1U && (eq = dsh_find_equal(&argv[0])) > 0) {
                if (launch_only)
                        return 126;
                if (dsh_copy_range(&name, &argv[0], 0U, (unsigned int)eq) != 0 ||
                    dsh_copy_range(&value, &argv[0], (unsigned int)eq + 1U,
                    argv[0].len) != 0)
                        return DSH_ERROR;
                return dsh_var_set(st, &name, &value, 0) != 0;
        }
        if (dsh_open_node_fds(n, basein, baseout, &infd, &outfd) != 0)
                return 1;
        if (launch_only) {
                if (dsh_is_builtin_name(&argv[0]))
                        rc = 126;
                else
                        rc = dsh_launch_external(st, argc, argv, infd, outfd,
                            pgrp_mode, pgrp, pidp);
        } else {
                if (dsh_builtin_fds(infd, outfd, &savein, &saveout) != 0)
                        rc = DSH_ERROR;
                else {
                        rc = dsh_builtin(st, argc, argv);
                        dsh_builtin_fds_restore(savein, saveout);
                }
                if (rc < 0)
                        rc = dsh_run_external(st, argc, argv, infd, outfd);
        }
        dsh_close_node_fds(n, basein, baseout, infd, outfd);
        if (rc == DSH_NOT_FOUND) {
                (void)u_puts(2, "DSH: NOT FOUND: ");
                (void)dsh_s6_put(2, &argv[0]);
                (void)u_crlf(2);
        }
        return rc;
}

static int
dsh_job_store(struct dsh_state *st, unsigned int pgrp,
    unsigned int last_pid, unsigned int remaining, unsigned int status,
    unsigned int stopped)
{
        unsigned int i;

        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                if (st->jobs[i].used && st->jobs[i].pgrp == pgrp)
                        break;
        if (i == DSH_MAX_JOBS)
                for (i = 0U; i < DSH_MAX_JOBS; ++i)
                        if (!st->jobs[i].used)
                                break;
        if (i == DSH_MAX_JOBS)
                return -1;
        st->jobs[i].used = 1U;
        st->jobs[i].pgrp = pgrp;
        st->jobs[i].last_pid = last_pid;
        st->jobs[i].remaining = remaining;
        st->jobs[i].status = status;
        st->jobs[i].stopped = stopped;
        return (int)i;
}

static int
dsh_wait_pgrp(struct dsh_state *st, unsigned int pgrp,
    unsigned int last_pid, unsigned int remaining, unsigned int last_status,
    int save_stopped)
{
        kword_t status;
        int pid;
        unsigned int kind;

        while (remaining != 0U) {
                pid = dsys_wait(SYS_WAIT_PGRP_FLAG | pgrp, &status, 0U);
                if (pid < 0)
                        return DSH_ERROR;
                kind = SYS_WAIT_STATUS_KIND(status);
                if (kind == SYS_WAIT_EXITED) {
                        --remaining;
                        if ((unsigned int)pid == last_pid)
                                last_status = SYS_WAIT_STATUS_VALUE(status);
                        continue;
                }
                if (kind == SYS_WAIT_STOPPED) {
                        if (save_stopped &&
                            dsh_job_store(st, pgrp, last_pid, remaining,
                            last_status, 1U) < 0)
                                return DSH_ERROR;
                        return 1;
                }
        }
        return (int)last_status;
}

static int
dsh_job_arg(const struct dsh_s6 *arg, unsigned int *pgrpp)
{
        struct dsh_s6 number;
        unsigned int i;
        unsigned int first;

        if (arg == 0 || arg->len == 0U)
                return -1;
        first = dsh_s6_get(arg, 0U) == '%' ? 1U : 0U;
        if (first >= arg->len)
                return -1;
        dsh_s6_clear(&number);
        for (i = first; i < arg->len; ++i)
                if (dsh_s6_append(&number, dsh_s6_get(arg, i)) != 0)
                        return -1;
        return dsh_parse_uint(&number, pgrpp);
}

static int
dsh_job_find(struct dsh_state *st, unsigned int argc, struct dsh_s6 *argv)
{
        unsigned int pgrp;
        int i;

        if (argc > 2U)
                return -2;
        if (argc == 2U) {
                if (dsh_job_arg(&argv[1], &pgrp) != 0)
                        return -2;
                for (i = 0; i < (int)DSH_MAX_JOBS; ++i)
                        if (st->jobs[i].used && st->jobs[i].pgrp == pgrp)
                                return i;
                return -1;
        }
        for (i = (int)DSH_MAX_JOBS - 1; i >= 0; --i)
                if (st->jobs[i].used)
                        return i;
        return -1;
}

static void
dsh_job_poll_one(struct dsh_state *st, unsigned int slot)
{
        struct dsh_job *job;
        kword_t status;
        int pid;
        unsigned int kind;

        job = &st->jobs[slot];
        if (!job->used)
                return;
        for (;;) {
                pid = dsys_wait(SYS_WAIT_PGRP_FLAG | job->pgrp, &status,
                    SYS_WAIT_NOHANG);
                if (pid <= 0)
                        return;
                kind = SYS_WAIT_STATUS_KIND(status);
                if (kind == SYS_WAIT_EXITED) {
                        if (job->remaining != 0U)
                                --job->remaining;
                        if ((unsigned int)pid == job->last_pid)
                                job->status = SYS_WAIT_STATUS_VALUE(status);
                        if (job->remaining == 0U) {
                                job->used = 0U;
                                return;
                        }
                } else if (kind == SYS_WAIT_STOPPED)
                        job->stopped = 1U;
                else if (kind == SYS_WAIT_CONTINUED)
                        job->stopped = 0U;
        }
}

static int
dsh_builtin_jobs(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv)
{
        unsigned int i;

        (void)argv;
        if (argc != 1U)
                return 2;
        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                dsh_job_poll_one(st, i);
        for (i = 0U; i < DSH_MAX_JOBS; ++i) {
                if (!st->jobs[i].used)
                        continue;
                (void)u_putc(1, '[');
                (void)u_put_uint(1, (kword_t)st->jobs[i].pgrp);
                (void)u_puts(1, "] ");
                (void)u_puts(1, st->jobs[i].stopped ?
                    "STOPPED" : "RUNNING");
                (void)u_crlf(1);
        }
        return 0;
}

static int
dsh_wait_job_slot(struct dsh_state *st, unsigned int slot, int foreground)
{
        struct dsh_job *job;
        int rc;

        job = &st->jobs[slot];
        if (!job->used)
                return 0;
        dsh_job_poll_one(st, slot);
        if (!job->used)
                return (int)job->status;
        if (foreground && st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, job->pgrp);
        if (job->stopped && !foreground)
                return 1;
        if (job->stopped) {
                if (dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                    SYS_EVENT_ARG(job->pgrp, SYS_EVENT_CONT)) < 0) {
                        if (foreground && st->tty_attached)
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                                    st->shell_pgrp);
                        return 1;
                }
                job->stopped = 0U;
        }
        rc = dsh_wait_pgrp(st, job->pgrp, job->last_pid,
            job->remaining, job->status, foreground);
        if (foreground && st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, st->shell_pgrp);
        if (rc != 1 || !job->stopped)
                job->used = 0U;
        return rc;
}

static int
dsh_builtin_wait(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv)
{
        unsigned int i;
        int slot;
        int status;

        if (argc > 2U)
                return 2;
        if (argc == 2U) {
                slot = dsh_job_find(st, argc, argv);
                if (slot == -2)
                        return 2;
                if (slot < 0)
                        return 1;
                return dsh_wait_job_slot(st, (unsigned int)slot, 0);
        }
        status = 0;
        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                if (st->jobs[i].used)
                        status = dsh_wait_job_slot(st, i, 0);
        return status;
}

static int
dsh_builtin_fg(struct dsh_state *st, unsigned int argc, struct dsh_s6 *argv)
{
        int slot;

        slot = dsh_job_find(st, argc, argv);
        if (slot == -2)
                return 2;
        if (slot < 0)
                return 1;
        return dsh_wait_job_slot(st, (unsigned int)slot, 1);
}

static int
dsh_builtin_bg(struct dsh_state *st, unsigned int argc, struct dsh_s6 *argv)
{
        struct dsh_job *job;
        int slot;

        slot = dsh_job_find(st, argc, argv);
        if (slot == -2)
                return 2;
        if (slot < 0)
                return 1;
        job = &st->jobs[slot];
        if (dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
            SYS_EVENT_ARG(job->pgrp, SYS_EVENT_CONT)) < 0)
                return 1;
        job->stopped = 0U;
        return 0;
}

static int
dsh_pipeline_collect(const struct dsh_node *nodes, unsigned int node,
    unsigned int *stages, unsigned int *countp)
{
        const struct dsh_node *n;

        n = &nodes[node];
        if (n->type == DSH_N_PIPE) {
                if (dsh_pipeline_collect(nodes, n->left, stages, countp) != 0 ||
                    dsh_pipeline_collect(nodes, n->right, stages, countp) != 0)
                        return -1;
                return 0;
        }
        if (n->type != DSH_N_SIMPLE || *countp >= DSH_MAX_ARGS)
                return -1;
        stages[(*countp)++] = node;
        return 0;
}

static void
dsh_close_pipe_set(int pipes[][2], unsigned int count)
{
        unsigned int i;

        for (i = 0U; i < count; ++i) {
                if (pipes[i][0] >= 0) {
                        (void)dsys_close(pipes[i][0]);
                        pipes[i][0] = -1;
                }
                if (pipes[i][1] >= 0) {
                        (void)dsys_close(pipes[i][1]);
                        pipes[i][1] = -1;
                }
        }
}

static int
dsh_exec_pipeline(struct dsh_state *st, const struct dsh_node *nodes,
    unsigned int root, int background)
{
        unsigned int stages[DSH_MAX_ARGS];
        int pipes[DSH_MAX_ARGS - 1U][2];
        unsigned int count;
        unsigned int i;
        unsigned int pgrp;
        int pid;
        int last_pid;
        int rc;
        kword_t pair;

        count = 0U;
        if (dsh_pipeline_collect(nodes, root, stages, &count) != 0 ||
            count == 0U)
                return 126;
        for (i = 0U; i + 1U < DSH_MAX_ARGS; ++i) {
                pipes[i][0] = -1;
                pipes[i][1] = -1;
        }
        for (i = 0U; i + 1U < count; ++i) {
                pair = dsys_pipe();
                if (pair == (kword_t)-1) {
                        dsh_close_pipe_set(pipes, i);
                        return DSH_ERROR;
                }
                pipes[i][0] = (int)((pair >> 18U) & 0777777UL);
                pipes[i][1] = (int)(pair & 0777777UL);
        }
        pgrp = 0U;
        last_pid = -1;
        for (i = 0U; i < count; ++i) {
                int infd = i == 0U ? 0 : pipes[i - 1U][0];
                int outfd = i + 1U == count ? 1 : pipes[i][1];
                unsigned int mode = i == 0U ?
                    SYS_RUN_PGRP_NEW : SYS_RUN_PGRP_JOIN;

                rc = dsh_exec_simple_node(st, &nodes[stages[i]], infd,
                    outfd, 1, mode, pgrp, &pid);
                if (rc != 0) {
                        dsh_close_pipe_set(pipes, count - 1U);
                        if (pgrp != 0U)
                                (void)dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                                    SYS_EVENT_ARG(pgrp, SYS_EVENT_TERM));
                        if (!background && st->tty_attached)
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                                    st->shell_pgrp);
                        return rc;
                }
                if (i == 0U) {
                        pgrp = (unsigned int)pid;
                        if (!background && st->tty_attached)
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                                    pgrp);
                }
                last_pid = pid;
        }
        dsh_close_pipe_set(pipes, count - 1U);
        if (background) {
                rc = dsh_job_store(st, pgrp, (unsigned int)last_pid,
                    count, 0U, 0U);
                if (rc < 0)
                        return DSH_ERROR;
                (void)u_putc(1, '[');
                (void)u_put_uint(1, (kword_t)pgrp);
                (void)u_putc(1, ']');
                (void)u_crlf(1);
                return 0;
        }
        if (st->interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_COOKED);
        rc = dsh_wait_pgrp(st, pgrp, (unsigned int)last_pid, count, 0U, 1);
        if (st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, st->shell_pgrp);
        if (st->interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_RAW);
        return rc;
}

static int
dsh_exec_loop(struct dsh_state *st, const struct dsh_node *nodes,
    const struct dsh_node *n, int until)
{
        unsigned int limit;
        int cond;
        int status;

        limit = 64U;
        status = 0;
        while (limit-- != 0U) {
                cond = dsh_exec_node(st, nodes, n->left);
                if ((!until && cond != 0) || (until && cond == 0))
                        return status;
                status = dsh_exec_node(st, nodes, n->right);
                if (st->exit_requested || st->return_requested)
                        return status;
        }
        return DSH_ERROR;
}

static int
dsh_exec_node(struct dsh_state *st, const struct dsh_node *nodes,
    unsigned int node)
{
        const struct dsh_node *n;
        struct dsh_s6 value;
        struct dsh_s6 pattern;
        unsigned int i;
        int status;

        if (node == DSH_NONE)
                return DSH_ERROR;
        n = &nodes[node];
        switch (n->type) {
        case DSH_N_EMPTY:
                return 0;
        case DSH_N_SIMPLE:
                return dsh_exec_simple_node(st, n, 0, 1, 0,
                    SYS_RUN_PGRP_INHERIT, 0U, 0);
        case DSH_N_LIST:
                status = dsh_exec_node(st, nodes, n->left);
                if (st->exit_requested || st->return_requested)
                        return status;
                return dsh_exec_node(st, nodes, n->right);
        case DSH_N_AND:
                status = dsh_exec_node(st, nodes, n->left);
                if (st->exit_requested || st->return_requested || status != 0)
                        return status;
                return dsh_exec_node(st, nodes, n->right);
        case DSH_N_OR:
                status = dsh_exec_node(st, nodes, n->left);
                if (st->exit_requested || st->return_requested || status == 0)
                        return status;
                return dsh_exec_node(st, nodes, n->right);
        case DSH_N_NOT:
                return dsh_exec_node(st, nodes, n->left) == 0 ? 1 : 0;
        case DSH_N_IF:
                status = dsh_exec_node(st, nodes, n->left);
                if (st->exit_requested || st->return_requested)
                        return status;
                if (status == 0)
                        return dsh_exec_node(st, nodes, n->right);
                if (n->extra != DSH_NONE)
                        return dsh_exec_node(st, nodes, n->extra);
                return 0;
        case DSH_N_FOR:
                status = 0;
                for (i = 1U; i < n->argc; ++i) {
                        if (dsh_expand_mask(st, &n->words[i],
                            n->literal_mask[i], &value) != 0 ||
                            dsh_var_set(st, &n->words[0], &value, 0) != 0)
                                return DSH_ERROR;
                        status = dsh_exec_node(st, nodes, n->left);
                        if (st->exit_requested || st->return_requested)
                                return status;
                }
                return status;
        case DSH_N_WHILE:
                return dsh_exec_loop(st, nodes, n, 0);
        case DSH_N_UNTIL:
                return dsh_exec_loop(st, nodes, n, 1);
        case DSH_N_CASE:
                if (n->argc < 2U ||
                    dsh_expand_mask(st, &n->words[0],
                    n->literal_mask[0], &value) != 0 ||
                    dsh_expand_mask(st, &n->words[1],
                    n->literal_mask[1], &pattern) != 0)
                        return DSH_ERROR;
                if (dsh_s6_same(&value, &pattern))
                        return dsh_exec_node(st, nodes, n->left);
                if (n->right != DSH_NONE)
                        return dsh_exec_node(st, nodes, n->right);
                return 1;
        case DSH_N_PIPE:
                return dsh_exec_pipeline(st, nodes, node, 0);
        case DSH_N_BG:
                if (nodes[n->left].type == DSH_N_PIPE)
                        return dsh_exec_pipeline(st, nodes, n->left, 1);
                if (nodes[n->left].type == DSH_N_SIMPLE) {
                        struct dsh_node fake[2];
                        dsh_node_clear(&fake[0]);
                        dsh_node_clear(&fake[1]);
                        fake[0] = nodes[n->left];
                        return dsh_exec_pipeline(st, fake, 0U, 1);
                }
                return 126;
        case DSH_N_DEF:
                return dsh_define_function(nodes, n);
        case DSH_N_SUBST:
                return 126;
        default:
                return DSH_ERROR;
        }
}

static int
dsh_script_depth(struct dsh_script *script, const struct dsh_token *tokens,
    unsigned int ntokens)
{
        unsigned int i;
        int depth;

        depth = (int)script->depth;
        for (i = 0U; i < ntokens; ++i) {
                if (tokens[i].type != DSH_T_WORD)
                        continue;
                if (dsh_s6_eq_text(&tokens[i].text, "IF") ||
                    dsh_s6_eq_text(&tokens[i].text, "FOR") ||
                    dsh_s6_eq_text(&tokens[i].text, "WHILE") ||
                    dsh_s6_eq_text(&tokens[i].text, "UNTIL") ||
                    dsh_s6_eq_text(&tokens[i].text, "DEF") ||
                    dsh_s6_eq_text(&tokens[i].text, "CASE") ||
                    dsh_s6_eq_text(&tokens[i].text, "BEGIN"))
                        ++depth;
                else if (dsh_s6_eq_text(&tokens[i].text, "FI") ||
                    dsh_s6_eq_text(&tokens[i].text, "DONE") ||
                    dsh_s6_eq_text(&tokens[i].text, "ESAC") ||
                    dsh_s6_eq_text(&tokens[i].text, "END")) {
                        --depth;
                        if (depth < 0)
                                return DSH_E_SYNTAX;
                }
        }
        script->depth = (unsigned int)depth;
        return DSH_OK;
}

void
dsh_script_init(struct dsh_script *script)
{
        unsigned int i;

        if (script == 0)
                return;
        for (i = 0U; i < DSH_SCRIPT_MAX_TOKENS; ++i)
                dsh_token_clear(&script->tokens[i]);
        script->ntokens = 0U;
        script->depth = 0U;
}

static int
dsh_script_append(struct dsh_script *script, const struct dsh_token *tokens,
    unsigned int ntokens)
{
        unsigned int i;

        if (ntokens == 0U)
                return DSH_OK;
        if (script->ntokens != 0U) {
                if (script->ntokens >= DSH_SCRIPT_MAX_TOKENS)
                        return DSH_E_OVERFLOW;
                dsh_token_clear(&script->tokens[script->ntokens]);
                script->tokens[script->ntokens].type = DSH_T_SEMI;
                ++script->ntokens;
        }
        if (script->ntokens + ntokens > DSH_SCRIPT_MAX_TOKENS)
                return DSH_E_OVERFLOW;
        for (i = 0U; i < ntokens; ++i)
                script->tokens[script->ntokens++] = tokens[i];
        return DSH_OK;
}

static int
dsh_script_execute(struct dsh_state *st, struct dsh_script *script,
    int *status)
{
        unsigned int root;
        unsigned int used;
        int rc;

        if (script->ntokens == 0U) {
                *status = 0;
                return DSH_OK;
        }
        rc = dsh_parse_tokens(script->tokens, script->ntokens, dsh_nodes,
            DSH_PARSE_MAX_NODES, &root, &used);
        if (rc != DSH_OK)
                return rc;
        *status = dsh_exec_node(st, dsh_nodes, root);
        st->status = (unsigned int)*status;
        dsh_script_init(script);
        return DSH_OK;
}

int
dsh_script_feed(struct dsh_state *st, struct dsh_script *script,
    const struct dsh_line *line, int *status, unsigned int *need_more)
{
        struct dsh_token tokens[DSH_LEX_MAX_TOKENS];
        unsigned int ntokens;
        int rc;

        if (st == 0 || script == 0 || line == 0 || status == 0 ||
            need_more == 0)
                return DSH_E_ARG;
        *need_more = 0U;
        rc = dsh_lex_s6_line(line, tokens, DSH_LEX_MAX_TOKENS, &ntokens, 0);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_script_depth(script, tokens, ntokens);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_script_append(script, tokens, ntokens);
        if (rc != DSH_OK)
                return rc;
        if (script->depth != 0U) {
                *need_more = 1U;
                return DSH_OK;
        }
        return dsh_script_execute(st, script, status);
}

int
dsh_script_finish(struct dsh_state *st, struct dsh_script *script,
    int *status)
{
        if (st == 0 || script == 0 || status == 0)
                return DSH_E_ARG;
        if (script->depth != 0U)
                return DSH_E_SYNTAX;
        return dsh_script_execute(st, script, status);
}

int
dsh_execute_line(struct dsh_state *st, const struct dsh_line *line)
{
        unsigned int root;
        unsigned int used;
        int errpos;
        int rc;

        rc = dsh_parse_s6_line(line, dsh_nodes, DSH_PARSE_MAX_NODES,
            &root, &used, &errpos);
        if (rc != DSH_OK) {
                (void)u_puts(2, "DSH: SYNTAX");
                (void)u_crlf(2);
                return DSH_ERROR;
        }
        return dsh_exec_node(st, dsh_nodes, root);
}
