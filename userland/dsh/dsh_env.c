#include "dsh.h"

struct dsh_s6 dsh_var_scratch;

static int
dsh_name_char(int ch, int first)
{
        if (ch >= 'A' && ch <= 'Z')
                return 1;
        if (!first && ch >= '0' && ch <= '9')
                return 1;
        return ch == '_';
}

static int
dsh_split_env(const kword_t *record, struct dsh_s6 *name,
    struct dsh_s6 *value)
{
        struct dsh_s6 all;
        unsigned int i;
        int ch;
        int seen;

        if (dsh_s6_from_counted(&all, record) != 0)
                return -1;
        dsh_s6_clear(name);
        dsh_s6_clear(value);
        seen = 0;
        for (i = 0U; i < all.len; ++i) {
                ch = dsh_s6_get(&all, i);
                if (!seen && ch == '=') {
                        seen = 1;
                        continue;
                }
                if (dsh_s6_append(seen ? value : name, ch) != 0)
                        return -1;
        }
        return seen ? 0 : -1;
}

void
dsh_state_init(struct dsh_state *st, int argc, kword_t **argv,
    kword_t **envp)
{
        struct dsh_s6 name;
        struct dsh_s6 value;
        unsigned int i;

        for (i = 0U; i < DSH_MAX_VARS; ++i)
                st->vars[i].meta = 0U;
        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                st->aliases[i].used = 0U;
        for (i = 0U; i < DSH_MAX_JOBS; ++i)
                st->jobs[i].used = 0U;
        dsh_s6_clear(&st->arg0);
        for (i = 0U; i < DSH_MAX_ARGS; ++i)
                dsh_s6_clear(&st->args[i]);
        st->argc = 0U;
        st->status = 0U;
        st->shell_pgrp = 0U;
        st->tty_attached = 0U;
        st->interactive = 0U;
        st->exit_requested = 0U;
        st->exit_status = 0U;
        st->return_requested = 0U;
        st->return_status = 0U;
        st->call_depth = 0U;
        st->loop_depth = 0U;
        st->loop_floor = 0U;
        st->loop_action = DSH_LOOP_NONE;
        st->loop_levels = 0U;
        st->job_serial = 0U;
        if (argc > 0 && argv != 0)
                (void)dsh_s6_from_counted(&st->arg0, argv[0]);
        {
                int pgrp;
                int tty;

                pgrp = dsys_procctl(SYS_PROCCTL_GETPGRP, 0U);
                if (pgrp > 0)
                        st->shell_pgrp = (unsigned int)pgrp;
                tty = dsys_procctl(SYS_PROCCTL_GETTTY, 0U);
                if (tty >= 0 && SYS_TTY_IS_ATTACHED((unsigned int)tty))
                        st->tty_attached = 1U;
        }
        if (envp != 0) {
                for (i = 0U; envp[i] != 0 && i < DSH_MAX_VARS; ++i)
                        if (dsh_split_env(envp[i], &name, &value) == 0)
                                (void)dsh_var_set(st, &name, &value, 1);
        }
}

static int
dsh_append_s6(struct dsh_s6 *dst, const struct dsh_s6 *src)
{
        unsigned int i;

        for (i = 0U; i < src->len; ++i)
                if (dsh_s6_append(dst, dsh_s6_get(src, i)) != 0)
                        return -1;
        return 0;
}

static int
dsh_append_uint(struct dsh_s6 *out, unsigned int value)
{
        char digits[12];
        unsigned int n;

        n = 0U;
        do {
                digits[n++] = (char)('0' + value % 10U);
                value /= 10U;
        } while (value != 0U && n < sizeof(digits));
        while (n != 0U)
                if (dsh_s6_append(out, digits[--n]) != 0)
                        return -1;
        return 0;
}

static const struct dsh_s6 *
dsh_var_get_range(const struct dsh_state *st, const struct dsh_s6 *in,
    unsigned int first, unsigned int last)
{
        struct dsh_s6 name;
        unsigned int i;

        dsh_s6_clear(&name);
        for (i = first; i < last; ++i)
                if (dsh_s6_append(&name, dsh_s6_get(in, i)) != 0)
                        return 0;
        return dsh_var_get(st, &name);
}

/* Expand exactly one nonrecursive $<NAME[:OPLITERAL]> expression.  The
 * fallback is copied to a local SIXBIT record so := cannot invalidate the
 * global scratch record returned by the compact assembler variable getter.
 * No heap allocation or persistent state is needed beyond := itself. */
static int
dsh_expand_angle(struct dsh_state *st, const struct dsh_s6 *in,
    unsigned int *index, struct dsh_s6 *out)
{
        struct dsh_s6 name;
        struct dsh_s6 fallback;
        const struct dsh_s6 *value;
        unsigned int i;
        int op;
        int ch;
        int active;

        dsh_s6_clear(&name);
        dsh_s6_clear(&fallback);
        i = *index;
        while (i < in->len &&
            dsh_name_char(dsh_s6_get(in, i), name.len == 0U)) {
                if (dsh_s6_append(&name, dsh_s6_get(in, i)) != 0)
                        return -1;
                ++i;
        }
        if (name.len == 0U || i >= in->len)
                return -1;
        op = 0;
        ch = dsh_s6_get(in, i);
        if (ch == ':') {
                if (++i >= in->len)
                        return -1;
                op = dsh_s6_get(in, i++);
                if (op != '-' && op != '=' && op != '+' && op != '?')
                        return -1;
                while (i < in->len && (ch = dsh_s6_get(in, i)) != '>') {
                        /* Defaults are literal SIXBIT, not recursively
                         * expanded or interpreted as shell syntax. */
                        if (ch == '$' || ch == '<' || ch == '\\' ||
                            ch == '"' || ch == '\'')
                                return -1;
                        if (dsh_s6_append(&fallback, ch) != 0)
                                return -1;
                        ++i;
                }
        }
        if (i >= in->len || dsh_s6_get(in, i) != '>')
                return -1;
        *index = i + 1U;
        value = dsh_var_get(st, &name);
        active = value != 0 && value->len != 0U;
        if (op == '-' && !active)
                value = &fallback;
        else if (op == '=' && !active) {
                /* Negative export mode preserves an existing export bit,
                 * including when an exported variable was empty. */
                if (dsh_var_set(st, &name, &fallback, -1) != 0)
                        return -1;
                value = &fallback;
        } else if (op == '+')
                value = active ? &fallback : 0;
        else if (op == '?' && !active) {
                (void)u_puts(2, "DSH: PARAMETER: ");
                if (fallback.len != 0U)
                        (void)dsh_s6_put(2, &fallback);
                else
                        (void)dsh_s6_put(2, &name);
                (void)u_crlf(2);
                return -1;
        }
        return value == 0 ? 0 : dsh_append_s6(out, value);
}

int
dsh_expand_quoted(struct dsh_state *st, const struct dsh_s6 *in,
    kword_t literal_mask, kword_t quote_mask, struct dsh_s6 *out,
    kword_t *out_quote_mask)
{
        const struct dsh_s6 *value;
        unsigned int begin;
        unsigned int first;
        unsigned int i;
        unsigned int last;
        unsigned int n;
        int ch;
        int quoted;

        dsh_s6_clear(out);
        if (out_quote_mask != 0)
                *out_quote_mask = 0;
        for (i = 0U; i < in->len;) {
                ch = dsh_s6_get(in, i++);
                quoted = (quote_mask & ((kword_t)1 << (i - 1U))) != 0;
                if (ch != '$' || (literal_mask & ((kword_t)1 << (i - 1U))) ||
                    i >= in->len) {
                        begin = out->len;
                        if (dsh_s6_append(out, ch) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                *out_quote_mask |= (kword_t)1 << begin;
                        continue;
                }
                begin = out->len;
                ch = dsh_s6_get(in, i);
                if (ch == '<') {
                        ++i;
                        if (dsh_expand_angle(st, in, &i, out) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch >= '0' && ch <= '9') {
                        ++i;
                        n = (unsigned int)(ch - '0');
                        if (n == 0U)
                                value = &st->arg0;
                        else if (n <= st->argc)
                                value = &st->args[n - 1U];
                        else
                                value = 0;
                        if (value != 0 && dsh_append_s6(out, value) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch == '?') {
                        ++i;
                        if (dsh_append_uint(out, st->status) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch == '#') {
                        ++i;
                        if (dsh_append_uint(out, st->argc) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch == '$') {
                        int pid;

                        ++i;
                        pid = dsys_getpid();
                        if (pid < 0 ||
                            dsh_append_uint(out, (unsigned int)pid) != 0)
                                return -1;
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch == '@') {
                        unsigned int a;

                        ++i;
                        for (a = 0U; a < st->argc; ++a) {
                                if (a != 0U &&
                                    dsh_s6_append(out, ' ') != 0)
                                        return -1;
                                if (dsh_append_s6(out, &st->args[a]) != 0)
                                        return -1;
                        }
                        if (quoted && out_quote_mask != 0)
                                for (n = begin; n < out->len; ++n)
                                        *out_quote_mask |= (kword_t)1 << n;
                        continue;
                }
                if (ch == '[') {
                        ++i;
                        first = i;
                        while (i < in->len && dsh_s6_get(in, i) != ']') {
                                if (!dsh_name_char(dsh_s6_get(in, i),
                                    i == first))
                                        return -1;
                                ++i;
                        }
                        /* An empty or unterminated bracketed variable is
                         * invalid.  Never silently use the truncated name. */
                        if (i == first || i >= in->len)
                                return -1;
                        last = i;
                        ++i;
                } else {
                        first = i;
                        while (i < in->len) {
                                ch = dsh_s6_get(in, i);
                                if (!dsh_name_char(ch, i == first))
                                        break;
                                ++i;
                        }
                        last = i;
                }
                value = dsh_var_get_range(st, in, first, last);
                if (value != 0 && dsh_append_s6(out, value) != 0)
                        return -1;
                if (quoted && out_quote_mask != 0)
                        for (n = begin; n < out->len; ++n)
                                *out_quote_mask |= (kword_t)1 << n;
        }
        return 0;
}

int
dsh_expand_mask(struct dsh_state *st, const struct dsh_s6 *in,
    kword_t literal_mask, struct dsh_s6 *out)
{
        return dsh_expand_quoted(st, in, literal_mask, 0, out, 0);
}

int
dsh_expand(struct dsh_state *st, const struct dsh_s6 *in,
    struct dsh_s6 *out)
{
        return dsh_expand_mask(st, in, 0, out);
}
