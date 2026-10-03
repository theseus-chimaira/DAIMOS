#include "dsh.h"
#include "dsh_lex.h"
#include "dsh_parse.h"
#include "cmdmap.h"
#include "text.h"

#define DSH_REC_WORDS (DSH_S6_MAX_WORDS + 1U)
#define DSH_RUN_WORDS 192U

#define DSH_JOB_STATUS_MASK      0377UL
#define DSH_JOB_STATE_SHIFT      8U
#define DSH_JOB_STATE_MASK       03UL
#define DSH_JOB_TTY_SHIFT       10U
#define DSH_JOB_TTY_MASK         017UL
#define DSH_JOB_TTY_NONE         017U
#define DSH_JOB_SERIAL_SHIFT    14U
#define DSH_JOB_SERIAL_MASK 0777777UL

#define DSH_JOB_RUNNING           0U
#define DSH_JOB_STOPPED           1U
#define DSH_JOB_DONE              2U

static kword_t dsh_run_block[DSH_RUN_WORDS];
static kword_t dsh_path_record[U_PATH_WORDS];
static struct dsh_node dsh_nodes[DSH_PARSE_MAX_NODES];
static unsigned int dsh_nodes_active;
static struct dsh_script *dsh_script_workspace;

struct dsh_function_store {
        unsigned int used;
        struct dsh_s6 name;
        unsigned int base;
        unsigned int root;
        unsigned int used_nodes;
};

static struct dsh_function_store dsh_functions[DSH_MAX_FUNCS];
static struct dsh_node dsh_function_nodes[DSH_FUNC_MAX_NODES];
static unsigned int dsh_function_nodes_used;

static int dsh_s6_same(const struct dsh_s6 *a, const struct dsh_s6 *b);
static void dsh_s6_swap(struct dsh_s6 *a, struct dsh_s6 *b);
static int dsh_wild_match(const struct dsh_s6 *value,
    const struct dsh_s6 *pattern, kword_t quote_mask);
static int dsh_job_store(struct dsh_state *st, unsigned int pgrp,
    unsigned int last_pid, unsigned int remaining, unsigned int status,
    unsigned int state, unsigned int tty_mode, const struct dsh_s6 *command);
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
dsh_count_function_nodes(const struct dsh_node *src, unsigned int index,
    unsigned int *countp)
{
        int rc;

        if (index == DSH_NONE || index >= DSH_PARSE_MAX_NODES ||
            *countp >= DSH_FUNC_MAX_NODES)
                return DSH_E_OVERFLOW;
        ++*countp;
        if (src[index].left != DSH_NONE) {
                rc = dsh_count_function_nodes(src, src[index].left, countp);
                if (rc != DSH_OK)
                        return rc;
        }
        if (src[index].right != DSH_NONE) {
                rc = dsh_count_function_nodes(src, src[index].right, countp);
                if (rc != DSH_OK)
                        return rc;
        }
        if (src[index].extra != DSH_NONE) {
                rc = dsh_count_function_nodes(src, src[index].extra, countp);
                if (rc != DSH_OK)
                        return rc;
        }
        return DSH_OK;
}

static void
dsh_remove_function_nodes(unsigned int slot)
{
        unsigned int base;
        unsigned int count;
        unsigned int i;
        unsigned int j;

        base = dsh_functions[slot].base;
        count = dsh_functions[slot].used_nodes;
        for (i = base + count; i < dsh_function_nodes_used; ++i)
                dsh_function_nodes[i - count] = dsh_function_nodes[i];
        dsh_function_nodes_used -= count;
        for (j = 0U; j < DSH_MAX_FUNCS; ++j)
                if (j != slot && dsh_functions[j].used &&
                    dsh_functions[j].base > base)
                        dsh_functions[j].base -= count;
}

static int
dsh_define_function(struct dsh_state *st, const struct dsh_node *nodes,
    const struct dsh_node *n)
{
        unsigned int needed;
        unsigned int old_count;
        unsigned int i;
        unsigned int slot;
        int found;
        int rc;

        if (n->argc == 0U || n->left == DSH_NONE)
                return DSH_ERROR;
        found = 0;
        old_count = 0U;
        slot = 0U;
        for (i = 0U; i < DSH_MAX_FUNCS; ++i)
                if (dsh_functions[i].used &&
                    dsh_s6_same(&dsh_functions[i].name, &n->words[0])) {
                        slot = i;
                        found = 1;
                        old_count = dsh_functions[i].used_nodes;
                        break;
                }
        needed = 0U;
        if (dsh_count_function_nodes(nodes, n->left, &needed) != DSH_OK ||
            dsh_function_nodes_used - old_count + needed >
            DSH_FUNC_MAX_NODES)
                return DSH_ERROR;
        if (found && st->call_depth != 0U)
                return DSH_ERROR;
        if (!found)
                for (i = 0U; i < DSH_MAX_FUNCS; ++i)
                        if (!dsh_functions[i].used) {
                                slot = i;
                                found = 1;
                                break;
                        }
        if (!found)
                return DSH_ERROR;
        if (old_count != 0U)
                dsh_remove_function_nodes(slot);
        dsh_functions[slot].used = 1U;
        if (dsh_s6_copy(&dsh_functions[slot].name, &n->words[0]) != 0)
                return DSH_ERROR;
        dsh_functions[slot].base = dsh_function_nodes_used;
        dsh_functions[slot].used_nodes = 0U;
        dsh_functions[slot].root = DSH_NONE;
        rc = dsh_copy_function_node(
            &dsh_function_nodes[dsh_functions[slot].base],
            &dsh_functions[slot].used_nodes, nodes, n->left,
            &dsh_functions[slot].root);
        if (rc != DSH_OK) {
                dsh_functions[slot].used = 0U;
                return DSH_ERROR;
        }
        dsh_function_nodes_used += dsh_functions[slot].used_nodes;
        return 0;
}

static int
dsh_exec_function(struct dsh_state *st, const struct dsh_function_store *fn,
    unsigned int argc, struct dsh_s6 *argv)
{
        unsigned int saved_argc;
        unsigned int i;
        int status;

        if (st->call_depth >= DSH_FUNC_MAX_CALLS || argc == 0U)
                return DSH_ERROR;
        saved_argc = st->argc;
        /* argv remains live for the duration of this call.  Swap the old
         * positional frame into those otherwise-dead argument records rather
         * than stacking another 9 SIXBIT records for every recursion level. */
        dsh_s6_swap(&st->arg0, &argv[0]);
        st->argc = argc - 1U;
        for (i = 0U; i < st->argc; ++i)
                dsh_s6_swap(&st->args[i], &argv[i + 1U]);
        ++st->call_depth;
        st->return_requested = 0U;
        st->return_status = 0U;
        status = dsh_exec_node(st, &dsh_function_nodes[fn->base], fn->root);
        if (st->return_requested) {
                status = (int)st->return_status;
                st->return_requested = 0U;
        }
        --st->call_depth;
        for (i = 0U; i < st->argc; ++i)
                dsh_s6_swap(&st->args[i], &argv[i + 1U]);
        dsh_s6_swap(&st->arg0, &argv[0]);
        st->argc = saved_argc;
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

static void
dsh_s6_swap(struct dsh_s6 *a, struct dsh_s6 *b)
{
        kword_t word;
        unsigned int len;
        unsigned int i;

        len = a->len;
        a->len = b->len;
        b->len = len;
        for (i = 0U; i < DSH_S6_MAX_WORDS; ++i) {
                word = a->words[i];
                a->words[i] = b->words[i];
                b->words[i] = word;
        }
}

static int
dsh_builtin_source(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv)
{
        unsigned int savedc;
        unsigned int sourcec;
        unsigned int restorec;
        unsigned int i;
        int rc;

        if (argc < 2U)
                return 2;
        savedc = st->argc;
        sourcec = argc - 2U;
        if (argc > 2U) {
                /* The source-argument slots are dead once copied into the
                 * positional frame.  Swap the overwritten old values into
                 * those existing slots instead of allocating an eight-word
                 * frame copy on the small PDP-6 user stack. */
                for (i = 0U; i < sourcec; ++i) {
                        dsh_s6_swap(&st->args[i], &argv[i + 2U]);
                }
                st->argc = sourcec;
        }
        rc = dsh_execute_file(st, &argv[1]);
        restorec = savedc < sourcec ? savedc : sourcec;
        for (i = 0U; i < restorec; ++i)
                dsh_s6_swap(&st->args[i], &argv[i + 2U]);
        st->argc = savedc;
        return rc;
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
            dsh_s6_eq_text(&argv[0], "."))
                return dsh_builtin_source(st, argc, argv);
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

static int
dsh_execute_file_with_script(struct dsh_state *st, const struct dsh_s6 *path,
    struct dsh_script *script)
{
        struct u_text_reader reader;
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
        dsh_script_init(script);
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
                rc = dsh_script_feed(st, script, &line, &status,
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
                rc = dsh_script_finish(st, script, &status);
                if (rc == DSH_OK)
                        rc = status;
                else
                        rc = DSH_ERROR;
        }
        u_text_close(&reader);
        return rc;
}

static int
dsh_execute_file_outer(struct dsh_state *st, const struct dsh_s6 *path)
{
        struct dsh_script script;

        return dsh_execute_file_with_script(st, path, &script);
}

int
dsh_execute_file(struct dsh_state *st, const struct dsh_s6 *path)
{
        /* Parsing is complete before evaluation starts, so an executing
         * script's token arena is free while SOURCE runs.  Reuse that arena
         * for nested SOURCE calls rather than stacking another 32-token
         * workspace on the PDP-6's fixed 02000-word user stack. */
        if (dsh_script_workspace != 0)
                return dsh_execute_file_with_script(st, path,
                    dsh_script_workspace);
        return dsh_execute_file_outer(st, path);
}

void
dsh_script_workspace_set(struct dsh_script *script)
{
        dsh_script_workspace = script;
}

static int
dsh_append_record(kword_t *block, unsigned int *used,
    const struct dsh_s6 *s)
{
        unsigned int n;

        n = 1U + (s->len + 5U) / 6U;
        if (*used + n > DSH_RUN_WORDS)
                return -1;
        if (dsh_s6_pack(s, &block[*used], n) != 0)
                return -1;
        *used += n;
        return 0;
}

static int
dsh_append_env_record(kword_t *block, unsigned int *used,
    const struct dsh_s6 *name, const struct dsh_s6 *value)
{
        unsigned int len;
        unsigned int words;
        unsigned int i;
        unsigned int pos;
        unsigned int wi;
        unsigned int sh;
        int ch;

        len = name->len + 1U + value->len;
        if (len > SYS_RUN_ARG_MAX_CHARS || len > (U_ARG_WORDS - 1U) * 6U)
                return -1;
        words = 1U + (len + 5U) / 6U;
        if (*used + words > DSH_RUN_WORDS)
                return -1;
        for (i = 0U; i < words; ++i)
                block[*used + i] = 0;
        block[*used] = (kword_t)len;
        pos = 0U;
        for (i = 0U; i < name->len; ++i) {
                ch = dsh_s6_get(name, i);
                wi = *used + 1U + pos / 6U;
                sh = 30U - (pos % 6U) * 6U;
                block[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
                ++pos;
        }
        wi = *used + 1U + pos / 6U;
        sh = 30U - (pos % 6U) * 6U;
        block[wi] |= ((kword_t)('=' - 040) & 077UL) << sh;
        ++pos;
        for (i = 0U; i < value->len; ++i) {
                ch = dsh_s6_get(value, i);
                wi = *used + 1U + pos / 6U;
                sh = 30U - (pos % 6U) * 6U;
                block[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
                ++pos;
        }
        *used += words;
        return 0;
}

static int
dsh_launch_path(struct dsh_state *st, const struct dsh_s6 *path,
    unsigned int argc, struct dsh_s6 *argv, int infd, int outfd,
    unsigned int pgrp_mode, unsigned int pgrp, int *pidp)
{
        struct sys_run_v2 *run;
        struct vfs_stat statbuf;
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
        if (pid < 0) {
                /* RUN deliberately has one compact failure return.  Preserve
                 * the shell's 127=not-found / 126=found-but-not-runnable
                 * distinction by checking namespace existence only on this
                 * slow failure path. */
                if (dsh_s6_pack(path, dsh_path_record, U_PATH_WORDS) == 0 &&
                    dsys_stat(dsh_path_record, &statbuf) == 0)
                        return 126;
                return DSH_NOT_FOUND;
        }
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

static const struct dsh_s6 *
dsh_path_value(const struct dsh_state *st)
{
        unsigned int i;

        for (i = 0U; i < DSH_MAX_VARS; ++i)
                if (st->vars[i].used &&
                    dsh_s6_eq_text(&st->vars[i].name, "PATH"))
                        return &st->vars[i].value;
        return 0;
}

static int
dsh_launch_mapped(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd, unsigned int pgrp_mode,
    unsigned int pgrp, int *pidp)
{
        struct dsh_s6 mapped;
        int rc;

        rc = dsh_s6_pack(&argv[0], dsh_run_block, U_PATH_WORDS);
        if (rc == 0)
                rc = u_cmd_resolve(dsh_run_block, dsh_path_record,
                    U_PATH_WORDS);
        if (rc == 0)
                rc = dsh_s6_from_counted(&mapped, dsh_path_record);
        if (rc != 0)
                return DSH_NOT_FOUND;
        return dsh_launch_path(st, &mapped, argc, argv, infd, outfd,
            pgrp_mode, pgrp, pidp);
}

static int
dsh_launch_path_search(struct dsh_state *st, const struct dsh_s6 *pval,
    unsigned int argc, struct dsh_s6 *argv, int infd, int outfd,
    unsigned int pgrp_mode, unsigned int pgrp, int *pidp)
{
        struct dsh_s6 path;
        struct dsh_s6 dir;
        unsigned int i;
        unsigned int start;
        int rc;

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
        return DSH_NOT_FOUND;
}

static int
dsh_launch_external(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd, unsigned int pgrp_mode,
    unsigned int pgrp, int *pidp)
{
        const struct dsh_s6 *pval;
        unsigned int i;
        int rc;

        for (i = 0U; i < argv[0].len; ++i)
                if (dsh_s6_get(&argv[0], i) == '/') {
                        rc = dsh_launch_path(st, &argv[0], argc, argv,
                            infd, outfd, pgrp_mode, pgrp, pidp);
                        if (rc != DSH_NOT_FOUND)
                                return rc;
                        return dsh_launch_mapped(st, argc, argv, infd, outfd,
                            pgrp_mode, pgrp, pidp);
                }
        pval = dsh_path_value(st);
        if (pval == 0)
                return DSH_NOT_FOUND;
        rc = dsh_launch_path_search(st, pval, argc, argv, infd, outfd,
            pgrp_mode, pgrp, pidp);
        if (rc != DSH_NOT_FOUND)
                return rc;
        return dsh_launch_mapped(st, argc, argv, infd, outfd,
            pgrp_mode, pgrp, pidp);
}

static int
dsh_run_external(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv, int infd, int outfd)
{
        kword_t status;
        unsigned int tty_mode;
        unsigned int kind;
        int mode;
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
        tty_mode = DSH_JOB_TTY_NONE;
        if (rc == pid && st->tty_attached) {
                mode = dsys_procctl(SYS_PROCCTL_TTY_GETMODE, 0U);
                if (mode >= 0)
                        tty_mode = (unsigned int)mode & 07U;
        }
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
                    1U, 0U, DSH_JOB_STOPPED, tty_mode, &argv[0]) < 0)
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

/* Match the deliberately small DSH wildcard language.  A quote-mask bit
 * suppresses wildcard meaning for the corresponding pattern character. */
static int
dsh_wild_match(const struct dsh_s6 *value, const struct dsh_s6 *pattern,
    kword_t quote_mask)
{
        unsigned int vi;
        unsigned int pi;
        unsigned int star;
        unsigned int retry;
        int pc;

        vi = 0U;
        pi = 0U;
        star = DSH_NONE;
        retry = 0U;
        while (vi < value->len) {
                if (pi < pattern->len) {
                        pc = dsh_s6_get(pattern, pi);
                        if ((quote_mask & ((kword_t)1 << pi)) == 0 &&
                            pc == '*') {
                                star = pi++;
                                retry = vi;
                                continue;
                        }
                        if (((quote_mask & ((kword_t)1 << pi)) == 0 &&
                            pc == '?') || pc == dsh_s6_get(value, vi)) {
                                ++pi;
                                ++vi;
                                continue;
                        }
                }
                if (star == DSH_NONE)
                        return 0;
                pi = star + 1U;
                vi = ++retry;
        }
        while (pi < pattern->len && dsh_s6_get(pattern, pi) == '*' &&
            (quote_mask & ((kword_t)1 << pi)) == 0)
                ++pi;
        return pi == pattern->len;
}

static int
dsh_alias_expand(struct dsh_state *st, struct dsh_s6 *argv,
    kword_t *quote_masks, unsigned int *argcp)
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
                quote_masks[replc + rest] = quote_masks[rest + 1U];
        }
        for (i = 0U; i < replc; ++i) {
                (void)dsh_s6_copy(&argv[i], &repl[i]);
                quote_masks[i] = 0;
        }
        *argcp += replc - 1U;
        return 0;
}

static int
dsh_alias_needed(const struct dsh_state *st, const struct dsh_s6 *command)
{
        unsigned int i;

        if (dsh_s6_eq_text(command, "ALIAS"))
                return 0;
        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                if (st->aliases[i].used &&
                    dsh_s6_same(&st->aliases[i].name, command))
                        return 1;
        return 0;
}

static int
dsh_has_wild(const struct dsh_s6 *s, kword_t quote_mask)
{
        unsigned int i;
        int ch;

        for (i = 0U; i < s->len; ++i) {
                ch = dsh_s6_get(s, i);
                if ((ch == '*' || ch == '?') &&
                    (quote_mask & ((kword_t)1 << i)) == 0)
                        return 1;
        }
        return 0;
}

static int
dsh_path_join(struct dsh_s6 *out, const struct dsh_s6 *prefix,
    const struct dsh_s6 *name)
{
        unsigned int i;

        dsh_s6_clear(out);
        for (i = 0U; i < prefix->len; ++i)
                if (dsh_s6_append(out, dsh_s6_get(prefix, i)) != 0)
                        return -1;
        if (prefix->len != 0U &&
            !(prefix->len == 1U && dsh_s6_get(prefix, 0U) == '/') &&
            dsh_s6_append(out, '/') != 0)
                return -1;
        for (i = 0U; i < name->len; ++i)
                if (dsh_s6_append(out, dsh_s6_get(name, i)) != 0)
                        return -1;
        return 0;
}

static int
dsh_glob_walk(const struct dsh_s6 *pattern, kword_t quote_mask,
    unsigned int pos, const struct dsh_s6 *prefix, int saw_wild,
    struct dsh_s6 *out, unsigned int max_out, unsigned int *countp)
{
        struct dsh_s6 component;
        struct dsh_s6 next;
        struct dsh_s6 name;
        struct dsh_s6 dir;
        struct vfs_dirent ent;
        struct vfs_stat stbuf;
        kword_t component_mask;
        kword_t record[U_PATH_WORDS];
        unsigned int bit;
        int wildcard;
        int fd;
        int rc;

        while (pos < pattern->len && dsh_s6_get(pattern, pos) == '/')
                ++pos;
        if (pos >= pattern->len) {
                if (saw_wild) {
                        if (dsh_s6_pack(prefix, record, U_PATH_WORDS) != 0 ||
                            dsys_stat(record, &stbuf) != 0)
                                return 0;
                }
                if (*countp >= max_out)
                        return -1;
                if (dsh_s6_copy(&out[*countp], prefix) != 0)
                        return -1;
                ++*countp;
                return 0;
        }

        dsh_s6_clear(&component);
        component_mask = 0;
        bit = 0U;
        while (pos < pattern->len && dsh_s6_get(pattern, pos) != '/') {
                if (dsh_s6_append(&component,
                    dsh_s6_get(pattern, pos)) != 0)
                        return -1;
                if ((quote_mask & ((kword_t)1 << pos)) != 0)
                        component_mask |= (kword_t)1 << bit;
                ++bit;
                ++pos;
        }
        wildcard = dsh_has_wild(&component, component_mask);
        if (!wildcard) {
                if (dsh_path_join(&next, prefix, &component) != 0)
                        return -1;
                return dsh_glob_walk(pattern, quote_mask, pos, &next,
                    saw_wild, out, max_out, countp);
        }

        if (prefix->len == 0U) {
                dsh_s6_clear(&dir);
                if (dsh_s6_append(&dir, '.') != 0)
                        return -1;
        } else if (dsh_s6_copy(&dir, prefix) != 0) {
                return -1;
        }
        if (dsh_s6_pack(&dir, record, U_PATH_WORDS) != 0)
                return 0;
        fd = dsys_open(record, SYS_O_RDONLY);
        if (fd < 0)
                return 0;
        for (;;) {
                rc = dsys_dirread(fd, &ent);
                if (rc <= 0)
                        break;
                if (u_s6_from_dirent(record, U_PATH_WORDS, &ent) != 0 ||
                    dsh_s6_from_counted(&name, record) != 0)
                        continue;
                if (!dsh_wild_match(&name, &component, component_mask))
                        continue;
                if (dsh_path_join(&next, prefix, &name) != 0 ||
                    dsh_glob_walk(pattern, quote_mask, pos, &next, 1,
                    out, max_out, countp) != 0) {
                        (void)dsys_close(fd);
                        return -1;
                }
        }
        (void)dsys_close(fd);
        return 0;
}

static int
dsh_glob_argv(struct dsh_s6 *argv, kword_t *quote_masks,
    unsigned int *argcp)
{
        struct dsh_s6 expanded[DSH_MAX_ARGS];
        struct dsh_s6 prefix;
        unsigned int outc;
        unsigned int matches;
        unsigned int i;
        unsigned int j;

        outc = 0U;
        for (i = 0U; i < *argcp; ++i) {
                if (!dsh_has_wild(&argv[i], quote_masks[i])) {
                        if (outc >= DSH_MAX_ARGS ||
                            dsh_s6_copy(&expanded[outc], &argv[i]) != 0)
                                return -1;
                        ++outc;
                        continue;
                }
                dsh_s6_clear(&prefix);
                j = 0U;
                if (argv[i].len != 0U && dsh_s6_get(&argv[i], 0U) == '/') {
                        if (dsh_s6_append(&prefix, '/') != 0)
                                return -1;
                        j = 1U;
                }
                matches = outc;
                if (dsh_glob_walk(&argv[i], quote_masks[i], j, &prefix, 0,
                    expanded, DSH_MAX_ARGS, &outc) != 0)
                        return -1;
                if (outc == matches) {
                        if (outc >= DSH_MAX_ARGS ||
                            dsh_s6_copy(&expanded[outc], &argv[i]) != 0)
                                return -1;
                        ++outc;
                }
        }
        for (i = 0U; i < outc; ++i) {
                if (dsh_s6_copy(&argv[i], &expanded[i]) != 0)
                        return -1;
                quote_masks[i] = 0;
        }
        *argcp = outc;
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
        kword_t quote_masks[DSH_MAX_ARGS];
        int need_glob;
        unsigned int i;

        if (n->argc > DSH_MAX_ARGS)
                return -1;
        for (i = 0U; i < n->argc; ++i) {
                if (dsh_expand_quoted(st, &n->words[i],
                    n->literal_mask[i], n->quote_mask[i], &argv[i],
                    &quote_masks[i]) != 0)
                        return -1;
        }
        *argcp = n->argc;
        if (*argcp != 0U && dsh_alias_needed(st, &argv[0]) &&
            dsh_alias_expand(st, argv, quote_masks, argcp) != 0)
                return -1;
        need_glob = 0;
        for (i = 0U; i < *argcp; ++i)
                if (dsh_has_wild(&argv[i], quote_masks[i])) {
                        need_glob = 1;
                        break;
                }
        if (!need_glob)
                return 0;
        return dsh_glob_argv(argv, quote_masks, argcp);
}

static int
dsh_exec_assignment(struct dsh_state *st, const struct dsh_s6 *word,
    unsigned int eq, int launch_only)
{
        struct dsh_s6 name;
        struct dsh_s6 value;

        if (launch_only)
                return 126;
        if (dsh_copy_range(&name, word, 0U, eq) != 0 ||
            dsh_copy_range(&value, word, eq + 1U, word->len) != 0)
                return DSH_ERROR;
        return dsh_var_set(st, &name, &value, 0) != 0;
}

static int
dsh_exec_simple_node(struct dsh_state *st, const struct dsh_node *n,
    int basein, int baseout, int launch_only, unsigned int pgrp_mode,
    unsigned int pgrp, int *pidp)
{
        struct dsh_s6 argv[DSH_MAX_ARGS];
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
                return dsh_exec_assignment(st, &argv[0], (unsigned int)eq,
                    launch_only);
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

static unsigned int
dsh_job_status(const struct dsh_job *job)
{
        return (unsigned int)(job->meta & DSH_JOB_STATUS_MASK);
}

static unsigned int
dsh_job_state(const struct dsh_job *job)
{
        return (unsigned int)((job->meta >> DSH_JOB_STATE_SHIFT) &
            DSH_JOB_STATE_MASK);
}

static unsigned int
dsh_job_tty_mode(const struct dsh_job *job)
{
        return (unsigned int)((job->meta >> DSH_JOB_TTY_SHIFT) &
            DSH_JOB_TTY_MASK);
}

static unsigned int
dsh_job_serial(const struct dsh_job *job)
{
        return (unsigned int)((job->meta >> DSH_JOB_SERIAL_SHIFT) &
            DSH_JOB_SERIAL_MASK);
}

static void
dsh_job_meta_set(struct dsh_job *job, unsigned int status,
    unsigned int state, unsigned int tty_mode, unsigned int serial)
{
        job->meta = (kword_t)(status & DSH_JOB_STATUS_MASK) |
            ((kword_t)(state & DSH_JOB_STATE_MASK) << DSH_JOB_STATE_SHIFT) |
            ((kword_t)(tty_mode & DSH_JOB_TTY_MASK) << DSH_JOB_TTY_SHIFT) |
            ((kword_t)(serial & DSH_JOB_SERIAL_MASK) << DSH_JOB_SERIAL_SHIFT);
}

static void
dsh_job_set_state(struct dsh_job *job, unsigned int state)
{
        job->meta &= ~((kword_t)DSH_JOB_STATE_MASK << DSH_JOB_STATE_SHIFT);
        job->meta |= (kword_t)(state & DSH_JOB_STATE_MASK) <<
            DSH_JOB_STATE_SHIFT;
}

static void
dsh_job_set_status(struct dsh_job *job, unsigned int status)
{
        job->meta &= ~(kword_t)DSH_JOB_STATUS_MASK;
        job->meta |= (kword_t)(status & DSH_JOB_STATUS_MASK);
}

static void
dsh_job_set_tty(struct dsh_job *job, unsigned int tty_mode)
{
        job->meta &= ~((kword_t)DSH_JOB_TTY_MASK << DSH_JOB_TTY_SHIFT);
        job->meta |= (kword_t)(tty_mode & DSH_JOB_TTY_MASK) <<
            DSH_JOB_TTY_SHIFT;
}

static int
dsh_job_store(struct dsh_state *st, unsigned int pgrp,
    unsigned int last_pid, unsigned int remaining, unsigned int status,
    unsigned int state, unsigned int tty_mode, const struct dsh_s6 *command)
{
        unsigned int i;
        unsigned int serial;
        int was_used;

        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                if (st->jobs[i].used && st->jobs[i].pgrp == pgrp)
                        break;
        if (i == DSH_MAX_JOBS)
                for (i = 0U; i < DSH_MAX_JOBS; ++i)
                        if (!st->jobs[i].used)
                                break;
        if (i == DSH_MAX_JOBS)
                return -1;
        was_used = st->jobs[i].used != 0U;
        serial = was_used ? dsh_job_serial(&st->jobs[i]) :
            (++st->job_serial & DSH_JOB_SERIAL_MASK);
        if (serial == 0U)
                serial = ++st->job_serial & DSH_JOB_SERIAL_MASK;
        st->jobs[i].used = 1U;
        st->jobs[i].pgrp = pgrp;
        st->jobs[i].last_pid = last_pid;
        st->jobs[i].remaining = remaining;
        dsh_job_meta_set(&st->jobs[i], status, state, tty_mode, serial);
        if (command != 0)
                (void)dsh_s6_copy(&st->jobs[i].command, command);
        else if (!was_used)
                dsh_s6_clear(&st->jobs[i].command);
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
                            last_status, DSH_JOB_STOPPED, DSH_JOB_TTY_NONE,
                            0) < 0)
                                return DSH_ERROR;
                        return 1;
                }
        }
        return (int)last_status;
}

static int
dsh_job_number_arg(const struct dsh_s6 *arg, unsigned int first,
    unsigned int *valuep)
{
        struct dsh_s6 number;
        unsigned int i;

        if (arg == 0 || first >= arg->len)
                return -1;
        dsh_s6_clear(&number);
        for (i = first; i < arg->len; ++i)
                if (dsh_s6_append(&number, dsh_s6_get(arg, i)) != 0)
                        return -1;
        return dsh_parse_uint(&number, valuep);
}

static int
dsh_job_find(struct dsh_state *st, unsigned int argc, struct dsh_s6 *argv,
    int stopped_only, int include_done)
{
        unsigned int best_serial;
        unsigned int number;
        unsigned int pgrp;
        unsigned int state;
        int i;
        int best;

        if (argc > 2U)
                return -2;
        if (argc == 2U) {
                if (dsh_s6_get(&argv[1], 0U) == '%') {
                        if (dsh_job_number_arg(&argv[1], 1U, &number) != 0 ||
                            number == 0U || number > DSH_MAX_JOBS)
                                return -2;
                        i = (int)number - 1;
                        if (!st->jobs[i].used)
                                return -1;
                        state = dsh_job_state(&st->jobs[i]);
                        if ((!include_done && state == DSH_JOB_DONE) ||
                            (stopped_only && state != DSH_JOB_STOPPED))
                                return -1;
                        return i;
                }
                if (dsh_job_number_arg(&argv[1], 0U, &pgrp) != 0)
                        return -2;
                for (i = 0; i < (int)DSH_MAX_JOBS; ++i) {
                        if (!st->jobs[i].used || st->jobs[i].pgrp != pgrp)
                                continue;
                        state = dsh_job_state(&st->jobs[i]);
                        if ((!include_done && state == DSH_JOB_DONE) ||
                            (stopped_only && state != DSH_JOB_STOPPED))
                                return -1;
                        return i;
                }
                return -1;
        }
        best = -1;
        best_serial = 0U;
        for (i = 0; i < (int)DSH_MAX_JOBS; ++i) {
                if (!st->jobs[i].used)
                        continue;
                state = dsh_job_state(&st->jobs[i]);
                if ((!include_done && state == DSH_JOB_DONE) ||
                    (stopped_only && state != DSH_JOB_STOPPED))
                        continue;
                if (best < 0 || dsh_job_serial(&st->jobs[i]) > best_serial) {
                        best = i;
                        best_serial = dsh_job_serial(&st->jobs[i]);
                }
        }
        return best;
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
                                dsh_job_set_status(job,
                                    SYS_WAIT_STATUS_VALUE(status));
                        if (job->remaining == 0U) {
                                dsh_job_set_state(job, DSH_JOB_DONE);
                                return;
                        }
                } else if (kind == SYS_WAIT_STOPPED)
                        dsh_job_set_state(job, DSH_JOB_STOPPED);
                else if (kind == SYS_WAIT_CONTINUED)
                        dsh_job_set_state(job, DSH_JOB_RUNNING);
        }
}

static int
dsh_builtin_jobs(struct dsh_state *st, unsigned int argc,
    struct dsh_s6 *argv)
{
        unsigned int i;
        unsigned int state;

        (void)argv;
        if (argc != 1U)
                return 2;
        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                dsh_job_poll_one(st, i);
        for (i = 0U; i < DSH_MAX_JOBS; ++i) {
                if (!st->jobs[i].used)
                        continue;
                state = dsh_job_state(&st->jobs[i]);
                (void)u_putc(1, '%');
                (void)u_put_uint(1, (kword_t)(i + 1U));
                (void)u_putc(1, ' ');
                (void)u_puts(1, state == DSH_JOB_STOPPED ? "STOPPED" :
                    state == DSH_JOB_DONE ? "DONE" : "RUNNING");
                if (state == DSH_JOB_DONE) {
                        (void)u_putc(1, ' ');
                        (void)u_put_uint(1,
                            (kword_t)dsh_job_status(&st->jobs[i]));
                }
                if (st->jobs[i].command.len != 0U) {
                        (void)u_putc(1, ' ');
                        (void)dsh_s6_put(1, &st->jobs[i].command);
                }
                (void)u_crlf(1);
                if (state == DSH_JOB_DONE)
                        st->jobs[i].used = 0U;
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
                return 0;
        if (dsh_job_state(job) == DSH_JOB_DONE) {
                rc = (int)dsh_job_status(job);
                job->used = 0U;
                return rc;
        }
        if (foreground && st->tty_attached) {
                /* TTY_SETMODE is permitted only to the current foreground
                 * process group.  Restore the stopped job's terminal-global
                 * mode while DSH still owns the TTY, then hand foreground
                 * ownership to the job before CONT. */
                if (dsh_job_tty_mode(job) != DSH_JOB_TTY_NONE)
                        (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                            dsh_job_tty_mode(job));
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, job->pgrp);
        }
        if (dsh_job_state(job) == DSH_JOB_STOPPED && !foreground)
                return 1;
        if (dsh_job_state(job) == DSH_JOB_STOPPED) {
                if (dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                    SYS_EVENT_ARG(job->pgrp, SYS_EVENT_CONT)) < 0) {
                        if (foreground && st->tty_attached)
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                                    st->shell_pgrp);
                        return 1;
                }
                dsh_job_set_state(job, DSH_JOB_RUNNING);
        }
        rc = dsh_wait_pgrp(st, job->pgrp, job->last_pid,
            job->remaining, dsh_job_status(job), 1);
        if (foreground && st->tty_attached) {
                int mode;

                mode = dsys_procctl(SYS_PROCCTL_TTY_GETMODE, 0U);
                if (mode >= 0 && job->used &&
                    dsh_job_state(job) == DSH_JOB_STOPPED)
                        dsh_job_set_tty(job, (unsigned int)mode & 07U);
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, st->shell_pgrp);
                if (st->interactive)
                        (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                            SYS_TTY_MODE_RAW);
        }
        if (rc != 1 || dsh_job_state(job) != DSH_JOB_STOPPED)
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
                slot = dsh_job_find(st, argc, argv, 0, 1);
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

        slot = dsh_job_find(st, argc, argv, 0, 0);
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

        slot = dsh_job_find(st, argc, argv, 1, 0);
        if (slot == -2)
                return 2;
        if (slot < 0)
                return 1;
        job = &st->jobs[slot];
        if (dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
            SYS_EVENT_ARG(job->pgrp, SYS_EVENT_CONT)) < 0)
                return 1;
        dsh_job_set_state(job, DSH_JOB_RUNNING);
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
dsh_pipeline_label(const struct dsh_node *nodes, const unsigned int *stages,
    unsigned int count, struct dsh_s6 *label)
{
        const struct dsh_s6 *word;
        unsigned int i;
        unsigned int j;

        dsh_s6_clear(label);
        for (i = 0U; i < count && label->len < DSH_S6_MAX_CHARS; ++i) {
                if (i != 0U) {
                        if (dsh_s6_append(label, ' ') != 0 ||
                            dsh_s6_append(label, '!') != 0 ||
                            dsh_s6_append(label, ' ') != 0)
                                return;
                }
                word = &nodes[stages[i]].words[0];
                for (j = 0U; j < word->len; ++j)
                        if (dsh_s6_append(label, dsh_s6_get(word, j)) != 0)
                                return;
        }
}

struct dsh_emit {
        int fd;
        unsigned int chars;
};

static int
dsh_emit_char(struct dsh_emit *out, int ch)
{
        if (ch < 040 || ch > 0137 || out->chars >= DSH_LINE_MAX_CHARS)
                return -1;
        if (u_putc(out->fd, ch) != 0)
                return -1;
        ++out->chars;
        return 0;
}

static int
dsh_emit_text(struct dsh_emit *out, const char *text)
{
        unsigned int i;

        for (i = 0U; text[i] != 0; ++i)
                if (dsh_emit_char(out, (unsigned char)text[i]) != 0)
                        return -1;
        return 0;
}

static int
dsh_emit_record(struct dsh_emit *out)
{
        if (u_crlf(out->fd) != 0)
                return -1;
        out->chars = 0U;
        return 0;
}

/* Recreate one parsed word without changing expansion or wildcard semantics.
 * literal_mask characters are escaped.  quote_mask-only runs are emitted in
 * double quotes so variables still expand while wildcard/space meaning stays
 * suppressed.  Empty words are explicitly quoted. */
static int
dsh_emit_word(struct dsh_emit *out, const struct dsh_s6 *word,
    kword_t literal_mask, kword_t quote_mask)
{
        unsigned int i;
        int quoted;
        int ch;

        if (word->len == 0U)
                return dsh_emit_text(out, "''");
        quoted = 0;
        for (i = 0U; i < word->len; ++i) {
                ch = dsh_s6_get(word, i);
                if ((literal_mask & ((kword_t)1 << i)) != 0) {
                        if (quoted) {
                                if (dsh_emit_char(out, '"') != 0)
                                        return -1;
                                quoted = 0;
                        }
                        if (dsh_emit_char(out, '\\') != 0 ||
                            dsh_emit_char(out, ch) != 0)
                                return -1;
                        continue;
                }
                if ((quote_mask & ((kword_t)1 << i)) != 0) {
                        if (!quoted) {
                                if (dsh_emit_char(out, '"') != 0)
                                        return -1;
                                quoted = 1;
                        }
                        if (dsh_emit_char(out, ch) != 0)
                                return -1;
                        continue;
                }
                if (quoted) {
                        if (dsh_emit_char(out, '"') != 0)
                                return -1;
                        quoted = 0;
                }
                if (dsh_emit_char(out, ch) != 0)
                        return -1;
        }
        if (quoted && dsh_emit_char(out, '"') != 0)
                return -1;
        return 0;
}

/* Redirection paths are currently literal in the executor.  Quote the whole
 * path so reparsing in the worker cannot introduce variable/glob semantics. */
static int
dsh_emit_literal_word(struct dsh_emit *out, const struct dsh_s6 *word)
{
        unsigned int i;
        int ch;

        if (dsh_emit_char(out, '\'') != 0)
                return -1;
        for (i = 0U; i < word->len; ++i) {
                ch = dsh_s6_get(word, i);
                if (ch == '\'') {
                        if (dsh_emit_char(out, '\'') != 0 ||
                            dsh_emit_char(out, '\\') != 0 ||
                            dsh_emit_char(out, '\'') != 0 ||
                            dsh_emit_char(out, '\'') != 0)
                                return -1;
                } else if (dsh_emit_char(out, ch) != 0) {
                        return -1;
                }
        }
        return dsh_emit_char(out, '\'');
}

static int dsh_emit_node(struct dsh_emit *out, const struct dsh_node *nodes,
    unsigned int node);

static int
dsh_emit_simple(struct dsh_emit *out, const struct dsh_node *n)
{
        unsigned int i;

        for (i = 0U; i < n->argc; ++i) {
                if (i != 0U && dsh_emit_char(out, ' ') != 0)
                        return -1;
                if (dsh_emit_word(out, &n->words[i], n->literal_mask[i],
                    n->quote_mask[i]) != 0)
                        return -1;
        }
        if ((n->flags & DSH_REDIR_IN) != 0U) {
                if (dsh_emit_text(out, " < ") != 0 ||
                    dsh_emit_literal_word(out, &n->redir_in) != 0)
                        return -1;
        }
        if ((n->flags & DSH_REDIR_OUT) != 0U) {
                if (dsh_emit_text(out,
                    (n->flags & DSH_REDIR_APPEND) != 0U ? " >> " : " > ") !=
                    0 || dsh_emit_literal_word(out, &n->redir_out) != 0)
                        return -1;
        }
        return 0;
}

static int
dsh_emit_case(struct dsh_emit *out, const struct dsh_node *nodes,
    const struct dsh_node *n)
{
        const struct dsh_node *arm;
        unsigned int index;
        unsigned int i;

        if (dsh_emit_text(out, "CASE ") != 0 ||
            dsh_emit_word(out, &n->words[0], n->literal_mask[0],
            n->quote_mask[0]) != 0 || dsh_emit_text(out, " IN ") != 0)
                return -1;
        index = n->left;
        while (index != DSH_NONE) {
                arm = &nodes[index];
                if (arm->type != DSH_N_CASE_ARM ||
                    dsh_emit_text(out, "WHEN ") != 0)
                        return -1;
                for (i = 0U; i < arm->argc; ++i) {
                        if (i != 0U && dsh_emit_char(out, ' ') != 0)
                                return -1;
                        if (dsh_emit_word(out, &arm->words[i],
                            arm->literal_mask[i], arm->quote_mask[i]) != 0)
                                return -1;
                }
                if (dsh_emit_text(out, " DO ") != 0 ||
                    dsh_emit_node(out, nodes, arm->left) != 0 ||
                    dsh_emit_char(out, ' ') != 0)
                        return -1;
                index = arm->right;
        }
        return dsh_emit_text(out, "ESAC");
}

static int
dsh_emit_node(struct dsh_emit *out, const struct dsh_node *nodes,
    unsigned int node)
{
        const struct dsh_node *n;
        unsigned int i;
        const char *word;

        if (node == DSH_NONE)
                return -1;
        n = &nodes[node];
        switch (n->type) {
        case DSH_N_EMPTY:
                return dsh_emit_text(out, "TRUE");
        case DSH_N_SIMPLE:
                return dsh_emit_simple(out, n);
        case DSH_N_LIST:
                if (dsh_emit_node(out, nodes, n->left) != 0 ||
                    dsh_emit_char(out, ';') != 0 || dsh_emit_record(out) != 0)
                        return -1;
                return dsh_emit_node(out, nodes, n->right);
        case DSH_N_AND:
        case DSH_N_OR:
        case DSH_N_PIPE:
                if (dsh_emit_node(out, nodes, n->left) != 0)
                        return -1;
                word = n->type == DSH_N_AND ? " AND " :
                    (n->type == DSH_N_OR ? " OR " : " ! ");
                if (dsh_emit_text(out, word) != 0)
                        return -1;
                return dsh_emit_node(out, nodes, n->right);
        case DSH_N_NOT:
                if (dsh_emit_text(out, "NOT ") != 0)
                        return -1;
                return dsh_emit_node(out, nodes, n->left);
        case DSH_N_IF:
                if (dsh_emit_text(out, "IF ") != 0 ||
                    dsh_emit_node(out, nodes, n->left) != 0 ||
                    dsh_emit_text(out, " THEN ") != 0 ||
                    dsh_emit_node(out, nodes, n->right) != 0)
                        return -1;
                if (n->extra != DSH_NONE &&
                    (dsh_emit_text(out, " ELSE ") != 0 ||
                    dsh_emit_node(out, nodes, n->extra) != 0))
                        return -1;
                return dsh_emit_text(out, " FI");
        case DSH_N_FOR:
                if (dsh_emit_text(out, "FOR ") != 0 ||
                    dsh_emit_word(out, &n->words[0], n->literal_mask[0],
                    n->quote_mask[0]) != 0 || dsh_emit_text(out, " IN") != 0)
                        return -1;
                for (i = 1U; i < n->argc; ++i)
                        if (dsh_emit_char(out, ' ') != 0 ||
                            dsh_emit_word(out, &n->words[i],
                            n->literal_mask[i], n->quote_mask[i]) != 0)
                                return -1;
                if (dsh_emit_text(out, " DO ") != 0 ||
                    dsh_emit_node(out, nodes, n->left) != 0)
                        return -1;
                return dsh_emit_text(out, " DONE");
        case DSH_N_WHILE:
        case DSH_N_UNTIL:
                word = n->type == DSH_N_WHILE ? "WHILE " : "UNTIL ";
                if (dsh_emit_text(out, word) != 0 ||
                    dsh_emit_node(out, nodes, n->left) != 0 ||
                    dsh_emit_text(out, " DO ") != 0 ||
                    dsh_emit_node(out, nodes, n->right) != 0)
                        return -1;
                return dsh_emit_text(out, " DONE");
        case DSH_N_CASE:
                return dsh_emit_case(out, nodes, n);
        case DSH_N_GROUP:
                if (dsh_emit_text(out, "BEGIN ") != 0 ||
                    dsh_emit_node(out, nodes, n->left) != 0)
                        return -1;
                return dsh_emit_text(out, " END");
        case DSH_N_DEF:
                if (dsh_emit_text(out, "DEF ") != 0 ||
                    dsh_emit_word(out, &n->words[0], n->literal_mask[0],
                    n->quote_mask[0]) != 0 || dsh_emit_text(out, " DO ") != 0 ||
                    dsh_emit_node(out, nodes, n->left) != 0)
                        return -1;
                return dsh_emit_text(out, " DONE");
        case DSH_N_BG:
                if (dsh_emit_node(out, nodes, n->left) != 0)
                        return -1;
                return dsh_emit_text(out, " &");
        default:
                return -1;
        }
}

static int
dsh_s6_from_text(struct dsh_s6 *s, const char *text)
{
        unsigned int i;

        dsh_s6_clear(s);
        for (i = 0U; text[i] != 0; ++i)
                if (dsh_s6_append(s, (unsigned char)text[i]) != 0)
                        return -1;
        return 0;
}

static int
dsh_exec_background_group(struct dsh_state *st, const struct dsh_node *nodes,
    const struct dsh_node *group)
{
        struct dsh_s6 path;
        struct dsh_s6 argv[1];
        struct dsh_s6 label;
        struct dsh_emit out;
        kword_t pair;
        int read_fd;
        int write_fd;
        int pid;
        int slot;
        int rc;

        if (dsh_s6_from_text(&path, "/SYSTEM/EXEC/DSH") != 0 ||
            dsh_s6_from_text(&argv[0], "DSH") != 0 ||
            dsh_s6_from_text(&label, "BEGIN") != 0)
                return DSH_ERROR;
        pair = dsys_pipe();
        if (pair == (kword_t)-1)
                return DSH_ERROR;
        read_fd = (int)((pair >> 18U) & 0777777UL);
        write_fd = (int)(pair & 0777777UL);
        rc = dsh_launch_path(st, &path, 1U, argv, read_fd, 1,
            SYS_RUN_PGRP_NEW, 0U, &pid);
        (void)dsys_close(read_fd);
        if (rc != 0) {
                (void)dsys_close(write_fd);
                return rc;
        }
        out.fd = write_fd;
        out.chars = 0U;
        rc = u_text_sink_attach(write_fd);
        if (rc == 0) {
                rc = dsh_emit_node(&out, nodes, group->left);
                if (rc == 0)
                        rc = dsh_emit_record(&out);
                if (u_text_sink_detach() != 0)
                        rc = -1;
        }
        (void)dsys_close(write_fd);
        if (rc != 0) {
                (void)dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                    SYS_EVENT_ARG((unsigned int)pid, SYS_EVENT_TERM));
                (void)dsh_wait_pgrp(st, (unsigned int)pid, (unsigned int)pid,
                    1U, 0U, 0);
                return DSH_ERROR;
        }
        slot = dsh_job_store(st, (unsigned int)pid, (unsigned int)pid, 1U,
            0U, DSH_JOB_RUNNING, DSH_JOB_TTY_NONE, &label);
        if (slot < 0) {
                (void)u_puts(2, "DSH: JOB TABLE FULL");
                (void)u_crlf(2);
                (void)dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                    SYS_EVENT_ARG((unsigned int)pid, SYS_EVENT_TERM));
                (void)dsh_wait_pgrp(st, (unsigned int)pid, (unsigned int)pid,
                    1U, 0U, 0);
                return DSH_ERROR;
        }
        (void)u_putc(1, '%');
        (void)u_put_uint(1, (kword_t)((unsigned int)slot + 1U));
        (void)u_crlf(1);
        return 0;
}

static int
dsh_exec_pipeline(struct dsh_state *st, const struct dsh_node *nodes,
    unsigned int root, int background)
{
        struct dsh_s6 label;
        unsigned int stages[DSH_MAX_ARGS];
        unsigned int count;
        unsigned int i;
        unsigned int pgrp;
        int prev_read;
        int next_read;
        int next_write;
        int pid;
        int last_pid;
        int rc;
        kword_t pair;

        count = 0U;
        if (dsh_pipeline_collect(nodes, root, stages, &count) != 0 ||
            count == 0U)
                return 126;
        dsh_pipeline_label(nodes, stages, count, &label);
        pgrp = 0U;
        last_pid = -1;
        prev_read = -1;
        for (i = 0U; i < count; ++i) {
                int infd;
                int outfd;
                unsigned int mode = i == 0U ?
                    SYS_RUN_PGRP_NEW : SYS_RUN_PGRP_JOIN;

                next_read = -1;
                next_write = -1;
                if (i + 1U < count) {
                        pair = dsys_pipe();
                        if (pair == (kword_t)-1) {
                                rc = DSH_ERROR;
                                goto pipeline_launch_fail;
                        }
                        next_read = (int)((pair >> 18U) & 0777777UL);
                        next_write = (int)(pair & 0777777UL);
                }
                infd = prev_read >= 0 ? prev_read : 0;
                outfd = next_write >= 0 ? next_write : 1;
                rc = dsh_exec_simple_node(st, &nodes[stages[i]], infd,
                    outfd, 1, mode, pgrp, &pid);
                if (rc != 0) {
                        goto pipeline_launch_fail;
                }
                if (prev_read >= 0)
                        (void)dsys_close(prev_read);
                if (next_write >= 0)
                        (void)dsys_close(next_write);
                prev_read = next_read;
                if (i == 0U) {
                        pgrp = (unsigned int)pid;
                        if (!background && st->tty_attached) {
                                /* As with FG, mode must be changed while DSH
                                 * is still the foreground owner. */
                                if (st->interactive)
                                        (void)dsys_procctl(
                                            SYS_PROCCTL_TTY_SETMODE,
                                            SYS_TTY_MODE_COOKED);
                                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                                    pgrp);
                        }
                }
                last_pid = pid;
        }
        if (background) {
                rc = dsh_job_store(st, pgrp, (unsigned int)last_pid,
                    count, 0U, DSH_JOB_RUNNING, DSH_JOB_TTY_NONE, &label);
                if (rc < 0) {
                        (void)u_puts(2, "DSH: JOB TABLE FULL");
                        (void)u_crlf(2);
                        (void)dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                            SYS_EVENT_ARG(pgrp, SYS_EVENT_TERM));
                        (void)dsh_wait_pgrp(st, pgrp,
                            (unsigned int)last_pid, count, 0U, 0);
                        return DSH_ERROR;
                }
                (void)u_putc(1, '%');
                (void)u_put_uint(1, (kword_t)((unsigned int)rc + 1U));
                (void)u_crlf(1);
                return 0;
        }
        rc = dsh_wait_pgrp(st, pgrp, (unsigned int)last_pid, count, 0U, 1);
        if (rc == 1) {
                for (i = 0U; i < DSH_MAX_JOBS; ++i)
                        if (st->jobs[i].used && st->jobs[i].pgrp == pgrp) {
                                (void)dsh_s6_copy(&st->jobs[i].command,
                                    &label);
                                if (st->tty_attached) {
                                        int mode;

                                        mode = dsys_procctl(
                                            SYS_PROCCTL_TTY_GETMODE, 0U);
                                        if (mode >= 0)
                                                dsh_job_set_tty(&st->jobs[i],
                                                    (unsigned int)mode & 07U);
                                }
                                break;
                        }
        }
        if (st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG, st->shell_pgrp);
        if (st->interactive)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETMODE,
                    SYS_TTY_MODE_RAW);
        return rc;

pipeline_launch_fail:
        if (prev_read >= 0)
                (void)dsys_close(prev_read);
        if (next_read >= 0)
                (void)dsys_close(next_read);
        if (next_write >= 0)
                (void)dsys_close(next_write);
        if (pgrp != 0U)
                (void)dsys_procctl(SYS_PROCCTL_EVENT_PGRP,
                    SYS_EVENT_ARG(pgrp, SYS_EVENT_TERM));
        if (!background && st->tty_attached)
                (void)dsys_procctl(SYS_PROCCTL_TTY_SETFG,
                    st->shell_pgrp);
        return rc;
}

static int
dsh_exec_loop(struct dsh_state *st, const struct dsh_node *nodes,
    const struct dsh_node *n, int until)
{
        int cond;
        int status;

        status = 0;
        for (;;) {
                cond = dsh_exec_node(st, nodes, n->left);
                if ((!until && cond != 0) || (until && cond == 0))
                        return status;
                status = dsh_exec_node(st, nodes, n->right);
                if (st->exit_requested || st->return_requested)
                        return status;
        }
}

static int
dsh_exec_node_raw(struct dsh_state *st, const struct dsh_node *nodes,
    unsigned int node)
{
        const struct dsh_node *n;
        struct dsh_s6 value;
        struct dsh_s6 pattern;
        kword_t pattern_quote;
        unsigned int arm;
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
                if (n->argc != 1U ||
                    dsh_expand_mask(st, &n->words[0],
                    n->literal_mask[0], &value) != 0)
                        return DSH_ERROR;
                arm = n->left;
                while (arm != DSH_NONE) {
                        const struct dsh_node *a;

                        a = &nodes[arm];
                        if (a->type != DSH_N_CASE_ARM)
                                return DSH_ERROR;
                        for (i = 0U; i < a->argc; ++i) {
                                if (dsh_expand_quoted(st, &a->words[i],
                                    a->literal_mask[i], a->quote_mask[i],
                                    &pattern, &pattern_quote) != 0)
                                        return DSH_ERROR;
                                if (dsh_wild_match(&value, &pattern,
                                    pattern_quote))
                                        return dsh_exec_node(st, nodes,
                                            a->left);
                        }
                        arm = a->right;
                }
                return 0;
        case DSH_N_GROUP:
                return dsh_exec_node(st, nodes, n->left);
        case DSH_N_PIPE:
                return dsh_exec_pipeline(st, nodes, node, 0);
        case DSH_N_BG:
                if (nodes[n->left].type == DSH_N_PIPE ||
                    nodes[n->left].type == DSH_N_SIMPLE)
                        return dsh_exec_pipeline(st, nodes, n->left, 1);
                if (nodes[n->left].type == DSH_N_GROUP)
                        return dsh_exec_background_group(st, nodes,
                            &nodes[n->left]);
                return 126;
        case DSH_N_DEF:
                return dsh_define_function(st, nodes, n);
        case DSH_N_SUBST:
                return 126;
        default:
                return DSH_ERROR;
        }
}

/* $? is the status of the most recently executed command, not merely the
 * status of the last complete input record.  Keep the update at the evaluator
 * boundary so LIST/AND/OR/loop/function recursion all observe the immediately
 * preceding command without duplicating status assignments in every AST case. */
static int
dsh_exec_node(struct dsh_state *st, const struct dsh_node *nodes,
    unsigned int node)
{
        int status;

        status = dsh_exec_node_raw(st, nodes, node);
        st->status = (unsigned int)status;
        return status;
}

static int
dsh_script_depth(struct dsh_script *script, const struct dsh_token *tokens,
    unsigned int ntokens)
{
        unsigned int i;
        int depth;

        depth = (int)script->depth;
        for (i = 0U; i < ntokens; ++i) {
                if (DSH_TOKEN_TYPE(&tokens[i]) != DSH_T_WORD ||
                    DSH_TOKEN_QUOTED(&tokens[i]))
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
dsh_script_execute(struct dsh_state *st, struct dsh_script *script,
    int *status)
{
        struct dsh_script *saved_workspace;
        struct dsh_node *arena;
        unsigned int base;
        unsigned int root;
        unsigned int used;
        int rc;

        if (script->ntokens == 0U) {
                *status = 0;
                return DSH_OK;
        }
        base = dsh_nodes_active;
        if (base >= DSH_PARSE_MAX_NODES)
                return DSH_E_OVERFLOW;
        arena = &dsh_nodes[base];
        rc = dsh_parse_tokens(script->tokens, script->ntokens, arena,
            DSH_PARSE_MAX_NODES - base, &root, &used);
        if (rc != DSH_OK)
                return rc;
        dsh_script_init(script);
        dsh_nodes_active = base + used;
        saved_workspace = dsh_script_workspace;
        dsh_script_workspace = script;
        *status = dsh_exec_node(st, arena, root);
        dsh_script_workspace = saved_workspace;
        dsh_nodes_active = base;
        st->status = (unsigned int)*status;
        return DSH_OK;
}

int
dsh_script_feed(struct dsh_state *st, struct dsh_script *script,
    const struct dsh_line *line, int *status, unsigned int *need_more)
{
        unsigned int base;
        unsigned int room;
        unsigned int ntokens;
        int rc;

        if (st == 0 || script == 0 || line == 0 || status == 0 ||
            need_more == 0)
                return DSH_E_ARG;
        *need_more = 0U;
        base = script->ntokens;
        if (base != 0U) {
                if (base >= DSH_SCRIPT_MAX_TOKENS)
                        return DSH_E_OVERFLOW;
                dsh_token_clear(&script->tokens[base]);
                script->tokens[base].type = DSH_T_SEMI;
                ++base;
        }
        room = DSH_SCRIPT_MAX_TOKENS - base;
        rc = dsh_lex_s6_line(line, &script->tokens[base], room,
            &ntokens, 0);
        if (rc != DSH_OK)
                return rc;
        if (ntokens == 0U)
                return DSH_OK;
        rc = dsh_script_depth(script, &script->tokens[base], ntokens);
        if (rc != DSH_OK)
                return rc;
        script->ntokens = base + ntokens;
        if (script->depth != 0U) {
                *need_more = 1U;
                return DSH_OK;
        }
        rc = dsh_script_execute(st, script, status);
        return rc;
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
        struct dsh_node *arena;
        unsigned int base;
        unsigned int root;
        unsigned int used;
        int errpos;
        int status;
        int rc;

        base = dsh_nodes_active;
        if (base >= DSH_PARSE_MAX_NODES)
                return DSH_ERROR;
        arena = &dsh_nodes[base];
        rc = dsh_parse_s6_line(line, arena, DSH_PARSE_MAX_NODES - base,
            &root, &used, &errpos);
        if (rc != DSH_OK) {
                (void)u_puts(2, "DSH: SYNTAX");
                (void)u_crlf(2);
                return DSH_ERROR;
        }
        dsh_nodes_active = base + used;
        status = dsh_exec_node(st, arena, root);
        dsh_nodes_active = base;
        return status;
}
