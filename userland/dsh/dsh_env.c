#include "dsh.h"

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
                st->vars[i].used = 0U;
        for (i = 0U; i < DSH_MAX_ALIASES; ++i)
                st->aliases[i].used = 0U;
        dsh_s6_clear(&st->arg0);
        for (i = 0U; i < DSH_MAX_ARGS; ++i)
                dsh_s6_clear(&st->args[i]);
        st->argc = 0U;
        st->status = 0U;
        st->exit_requested = 0U;
        st->exit_status = 0U;
        if (argc > 0 && argv != 0)
                (void)dsh_s6_from_counted(&st->arg0, argv[0]);
        if (envp != 0) {
                for (i = 0U; envp[i] != 0 && i < DSH_MAX_VARS; ++i)
                        if (dsh_split_env(envp[i], &name, &value) == 0)
                                (void)dsh_var_set(st, &name, &value, 1);
        }
}

int
dsh_var_set(struct dsh_state *st, const struct dsh_s6 *name,
    const struct dsh_s6 *value, int exported)
{
        unsigned int i;
        int free_slot;

        if (name == 0 || value == 0 || name->len == 0U)
                return -1;
        free_slot = -1;
        for (i = 0U; i < DSH_MAX_VARS; ++i) {
                if (!st->vars[i].used) {
                        if (free_slot < 0)
                                free_slot = (int)i;
                        continue;
                }
                if (st->vars[i].name.len == name->len) {
                        unsigned int j;
                        for (j = 0U; j < name->len; ++j)
                                if (dsh_s6_get(&st->vars[i].name, j) !=
                                    dsh_s6_get(name, j))
                                        break;
                        if (j == name->len) {
                                (void)dsh_s6_copy(&st->vars[i].value, value);
                                if (exported >= 0)
                                        st->vars[i].exported =
                                            (unsigned int)exported;
                                return 0;
                        }
                }
        }
        if (free_slot < 0)
                return -1;
        st->vars[free_slot].used = 1U;
        st->vars[free_slot].exported = exported > 0;
        (void)dsh_s6_copy(&st->vars[free_slot].name, name);
        (void)dsh_s6_copy(&st->vars[free_slot].value, value);
        return 0;
}

const struct dsh_s6 *
dsh_var_get(const struct dsh_state *st, const struct dsh_s6 *name)
{
        unsigned int i;
        unsigned int j;

        for (i = 0U; i < DSH_MAX_VARS; ++i) {
                if (!st->vars[i].used || st->vars[i].name.len != name->len)
                        continue;
                for (j = 0U; j < name->len; ++j)
                        if (dsh_s6_get(&st->vars[i].name, j) !=
                            dsh_s6_get(name, j))
                                break;
                if (j == name->len)
                        return &st->vars[i].value;
        }
        return 0;
}

int
dsh_var_unset(struct dsh_state *st, const struct dsh_s6 *name)
{
        unsigned int i;
        unsigned int j;

        for (i = 0U; i < DSH_MAX_VARS; ++i) {
                if (!st->vars[i].used || st->vars[i].name.len != name->len)
                        continue;
                for (j = 0U; j < name->len; ++j)
                        if (dsh_s6_get(&st->vars[i].name, j) !=
                            dsh_s6_get(name, j))
                                break;
                if (j == name->len) {
                        st->vars[i].used = 0U;
                        return 0;
                }
        }
        return 0;
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

int
dsh_expand(const struct dsh_state *st, const struct dsh_s6 *in,
    struct dsh_s6 *out)
{
        struct dsh_s6 name;
        const struct dsh_s6 *value;
        unsigned int i;
        unsigned int n;
        int ch;

        dsh_s6_clear(out);
        for (i = 0U; i < in->len;) {
                ch = dsh_s6_get(in, i++);
                if (ch != '$' || i >= in->len) {
                        if (dsh_s6_append(out, ch) != 0)
                                return -1;
                        continue;
                }
                ch = dsh_s6_get(in, i);
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
                        continue;
                }
                if (ch == '?') {
                        unsigned int v = st->status;
                        char digits[4];
                        unsigned int k = 0U;
                        ++i;
                        do {
                                digits[k++] = (char)('0' + v % 10U);
                                v /= 10U;
                        } while (v != 0U && k < sizeof(digits));
                        while (k != 0U)
                                if (dsh_s6_append(out, digits[--k]) != 0)
                                        return -1;
                        continue;
                }
                dsh_s6_clear(&name);
                if (ch == '[') {
                        ++i;
                        while (i < in->len &&
                            (ch = dsh_s6_get(in, i++)) != ']')
                                if (dsh_s6_append(&name, ch) != 0)
                                        return -1;
                } else {
                        while (i < in->len) {
                                ch = dsh_s6_get(in, i);
                                if (!dsh_name_char(ch, name.len == 0U))
                                        break;
                                ++i;
                                if (dsh_s6_append(&name, ch) != 0)
                                        return -1;
                        }
                }
                value = dsh_var_get(st, &name);
                if (value != 0 && dsh_append_s6(out, value) != 0)
                        return -1;
        }
        return 0;
}
