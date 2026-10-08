#include "u.h"
#include "text.h"

/*
 * Native MAKE deliberately implements the compact, traditional part of BSD
 * make.  DAIMOS is a 36-bit machine with bounded resident processes, so the
 * dependency graph and all text live in fixed arenas rather than a heap.
 * Recipes are interpreted by DSH -C; duplicating shell syntax here would cost
 * memory and create a second, subtly different command language.
 */

#define MAKE_LINE_MAX       256U
#define MAKE_NAME_MAX       103U
#define MAKE_RECIPE_MAX     SYS_RUN_ARG_MAX_CHARS
#define MAKE_MAX_RULES       96U
#define MAKE_MAX_DEPS       384U
#define MAKE_MAX_RECIPES    192U
#define MAKE_MAX_VARS        64U
#define MAKE_MAX_LHS          8U
#define MAKE_MAX_GOALS       16U
#define MAKE_ARENA_CHARS    6144U
#define MAKE_RUN_WORDS       400U
#define MAKE_MAX_DEPTH        32U
#define MAKE_NONE        0777777U

#define MAKE_RULE_PHONY       0001U
#define MAKE_RULE_SUFFIX      0002U
#define MAKE_RULE_CHANGED     0004U

#define MAKE_STATE_IDLE       0U
#define MAKE_STATE_ACTIVE     1U
#define MAKE_STATE_DONE       2U
#define MAKE_STATE_FAILED     3U

#define MAKE_VAR_COMMAND      0001U
#define MAKE_VAR_ENV          0002U

struct make_rule {
        unsigned int name;
        unsigned int dep_head;
        unsigned int dep_tail;
        unsigned int recipe_head;
        unsigned int recipe_tail;
        unsigned int flags;
        unsigned int state;
};

struct make_link {
        unsigned int text;
        unsigned int next;
};

struct make_var {
        unsigned int name;
        unsigned int value;
        unsigned int flags;
};

struct make_auto {
        const char *target;
        const char *first;
        const char *newer;
        const char *stem;
};

struct make_result {
        kword_t mtime;
        unsigned int exists;
        unsigned int valid;
        unsigned int changed;
};

struct make_implicit {
        int rule;
        char source[MAKE_NAME_MAX + 1U];
        char stem[MAKE_NAME_MAX + 1U];
};

static struct make_rule make_rules[MAKE_MAX_RULES];
static struct make_link make_deps[MAKE_MAX_DEPS];
static struct make_link make_recipes[MAKE_MAX_RECIPES];
static struct make_var make_vars[MAKE_MAX_VARS];
static char make_arena[MAKE_ARENA_CHARS];
static unsigned int make_rule_count;
static unsigned int make_dep_count;
static unsigned int make_recipe_count;
static unsigned int make_var_count;
static unsigned int make_arena_used;
static unsigned int make_default_rule = MAKE_NONE;
static unsigned int make_current[MAKE_MAX_LHS];
static unsigned int make_current_count;
static kword_t **make_envp;

static int make_dry_run;
static int make_silent;
static int make_keep_going;
static int make_always;
static int make_question;

static char make_line[MAKE_LINE_MAX + 1U];
static char make_work[MAKE_LINE_MAX + 1U];
static char make_expand_buf[MAKE_LINE_MAX + 1U];
static char make_newer_buf[MAKE_LINE_MAX + 1U];
static kword_t make_path[U_PATH_WORDS];
static kword_t make_run[MAKE_RUN_WORDS];

static int make_expand(const char *src, char *dst, unsigned int cap,
    const struct make_auto *automatic, unsigned int depth);

static unsigned int
make_strlen(const char *s)
{
        unsigned int n;

        n = 0U;
        while (s != 0 && s[n] != 0)
                ++n;
        return n;
}

static int
make_streq(const char *a, const char *b)
{
        unsigned int i;

        if (a == 0 || b == 0)
                return 0;
        for (i = 0U; a[i] != 0 && b[i] != 0; ++i)
                if (a[i] != b[i])
                        return 0;
        return a[i] == b[i];
}

static int
make_append(char *dst, unsigned int cap, unsigned int *used, const char *src)
{
        unsigned int i;

        if (dst == 0 || used == 0 || src == 0 || *used >= cap)
                return -1;
        for (i = 0U; src[i] != 0; ++i) {
                if (*used + 1U >= cap)
                        return -1;
                dst[(*used)++] = src[i];
        }
        dst[*used] = 0;
        return 0;
}

static unsigned int
make_store(const char *s)
{
        unsigned int n;
        unsigned int off;
        unsigned int i;

        n = make_strlen(s) + 1U;
        if (n > MAKE_ARENA_CHARS - make_arena_used)
                return MAKE_NONE;
        off = make_arena_used;
        for (i = 0U; i < n; ++i)
                make_arena[make_arena_used++] = s[i];
        return off;
}

static char *
make_text(unsigned int off)
{
        return off == MAKE_NONE || off >= make_arena_used ? 0 :
            &make_arena[off];
}

static char *
make_trim(char *s)
{
        char *e;

        while (*s == ' ')
                ++s;
        e = s + make_strlen(s);
        while (e != s && e[-1] == ' ')
                --e;
        *e = 0;
        return s;
}

static char *
make_word(char **pp)
{
        char *p;
        char *s;

        p = *pp;
        while (*p == ' ')
                ++p;
        if (*p == 0) {
                *pp = p;
                return 0;
        }
        s = p;
        while (*p != 0 && *p != ' ')
                ++p;
        if (*p != 0)
                *p++ = 0;
        *pp = p;
        return s;
}

static void
make_diag(const char *msg, const char *name)
{
        (void)u_puts(2, "MAKE: ");
        (void)u_puts(2, msg);
        if (name != 0) {
                (void)u_puts(2, ": ");
                (void)u_puts(2, name);
        }
        (void)u_crlf(2);
}

static int
make_counted_text(const kword_t *s, char *dst, unsigned int cap)
{
        unsigned int n;
        unsigned int i;
        unsigned int wi;
        unsigned int sh;

        if (s == 0 || dst == 0 || cap == 0U)
                return -1;
        n = (unsigned int)(s[0] & 0777777UL);
        if (n + 1U > cap)
                return -1;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                dst[i] = (char)(((s[wi] >> sh) & 077UL) + 040UL);
        }
        dst[n] = 0;
        return 0;
}

static int
make_find_var(const char *name)
{
        unsigned int i;

        for (i = 0U; i < make_var_count; ++i)
                if (make_streq(make_text(make_vars[i].name), name))
                        return (int)i;
        return -1;
}

static int
make_set_var(const char *name, const char *value, int op, unsigned int flags)
{
        char joined[MAKE_LINE_MAX + 1U];
        unsigned int noff;
        unsigned int voff;
        unsigned int used;
        int vi;

        vi = make_find_var(name);
        if (vi >= 0 && (make_vars[vi].flags & MAKE_VAR_COMMAND) != 0U &&
            (flags & MAKE_VAR_COMMAND) == 0U)
                return 0;
        if (op == '?' && vi >= 0)
                return 0;
        if (op == '+') {
                used = 0U;
                joined[0] = 0;
                if (vi >= 0 && make_text(make_vars[vi].value)[0] != 0 &&
                    (make_append(joined, sizeof(joined), &used,
                    make_text(make_vars[vi].value)) != 0 ||
                    make_append(joined, sizeof(joined), &used, " ") != 0))
                        return -1;
                if (make_append(joined, sizeof(joined), &used, value) != 0)
                        return -1;
                value = joined;
        } else if (op == ':') {
                if (make_expand(value, joined, sizeof(joined), 0, 0U) != 0)
                        return -1;
                value = joined;
        }
        voff = make_store(value);
        if (voff == MAKE_NONE)
                return -1;
        if (vi >= 0) {
                make_vars[vi].value = voff;
                make_vars[vi].flags = flags;
                return 0;
        }
        if (make_var_count >= MAKE_MAX_VARS)
                return -1;
        noff = make_store(name);
        if (noff == MAKE_NONE)
                return -1;
        make_vars[make_var_count].name = noff;
        make_vars[make_var_count].value = voff;
        make_vars[make_var_count].flags = flags;
        ++make_var_count;
        return 0;
}

static int
make_expand_value(const char *name, char *dst, unsigned int cap,
    unsigned int *used, const struct make_auto *automatic, unsigned int depth)
{
        int vi;
        unsigned int before;

        vi = make_find_var(name);
        if (vi < 0)
                return 0;
        before = *used;
        if (make_expand(make_text(make_vars[vi].value), &dst[*used],
            cap - *used, automatic, depth + 1U) != 0)
                return -1;
        *used += make_strlen(&dst[before]);
        return 0;
}

static int
make_expand(const char *src, char *dst, unsigned int cap,
    const struct make_auto *automatic, unsigned int depth)
{
        char name[64];
        const char *auto_text;
        unsigned int used;
        unsigned int i;
        unsigned int n;
        char close;

        if (src == 0 || dst == 0 || cap == 0U || depth > 8U)
                return -1;
        used = 0U;
        dst[0] = 0;
        for (i = 0U; src[i] != 0; ++i) {
                if (src[i] != '$') {
                        if (used + 1U >= cap)
                                return -1;
                        dst[used++] = src[i];
                        dst[used] = 0;
                        continue;
                }
                ++i;
                if (src[i] == 0)
                        break;
                if (src[i] == '$') {
                        if (used + 1U >= cap)
                                return -1;
                        dst[used++] = '$';
                        dst[used] = 0;
                        continue;
                }
                auto_text = 0;
                if (automatic != 0) {
                        if (src[i] == '@') auto_text = automatic->target;
                        else if (src[i] == '<') auto_text = automatic->first;
                        else if (src[i] == '?') auto_text = automatic->newer;
                        else if (src[i] == '*') auto_text = automatic->stem;
                }
                if (auto_text != 0) {
                        if (make_append(dst, cap, &used, auto_text) != 0)
                                return -1;
                        continue;
                }
                if (src[i] == '(' || src[i] == '{') {
                        close = src[i] == '(' ? ')' : '}';
                        n = 0U;
                        ++i;
                        while (src[i] != 0 && src[i] != close) {
                                if (n + 1U >= sizeof(name))
                                        return -1;
                                name[n++] = src[i++];
                        }
                        if (src[i] != close)
                                return -1;
                        name[n] = 0;
                } else {
                        name[0] = src[i];
                        name[1] = 0;
                }
                if (make_expand_value(name, dst, cap, &used, automatic,
                    depth) != 0)
                        return -1;
        }
        return 0;
}

static int
make_import_environment(kword_t **envp)
{
        char entry[MAKE_LINE_MAX + 1U];
        char *eq;
        unsigned int i;

        if (envp == 0)
                return 0;
        for (i = 0U; envp[i] != 0 && i < SYS_RUN_ENV_MAX; ++i) {
                if (make_counted_text(envp[i], entry, sizeof(entry)) != 0)
                        continue;
                eq = entry;
                while (*eq != 0 && *eq != '=')
                        ++eq;
                if (*eq != '=')
                        continue;
                *eq++ = 0;
                if (entry[0] != 0 &&
                    make_set_var(entry, eq, '=', MAKE_VAR_ENV) != 0)
                        return -1;
        }
        return 0;
}

static int
make_find_rule(const char *name)
{
        unsigned int i;

        for (i = 0U; i < make_rule_count; ++i)
                if (make_streq(make_text(make_rules[i].name), name))
                        return (int)i;
        return -1;
}

static int
make_get_rule(const char *name)
{
        unsigned int off;
        int ri;

        ri = make_find_rule(name);
        if (ri >= 0)
                return ri;
        if (make_rule_count >= MAKE_MAX_RULES)
                return -1;
        off = make_store(name);
        if (off == MAKE_NONE)
                return -1;
        ri = (int)make_rule_count++;
        make_rules[ri].name = off;
        make_rules[ri].dep_head = MAKE_NONE;
        make_rules[ri].dep_tail = MAKE_NONE;
        make_rules[ri].recipe_head = MAKE_NONE;
        make_rules[ri].recipe_tail = MAKE_NONE;
        make_rules[ri].flags = 0U;
        make_rules[ri].state = MAKE_STATE_IDLE;
        return ri;
}

static int
make_add_dep(unsigned int ri, const char *name)
{
        unsigned int di;
        unsigned int off;

        if (ri >= make_rule_count || make_dep_count >= MAKE_MAX_DEPS)
                return -1;
        off = make_store(name);
        if (off == MAKE_NONE)
                return -1;
        di = make_dep_count++;
        make_deps[di].text = off;
        make_deps[di].next = MAKE_NONE;
        if (make_rules[ri].dep_tail == MAKE_NONE)
                make_rules[ri].dep_head = di;
        else
                make_deps[make_rules[ri].dep_tail].next = di;
        make_rules[ri].dep_tail = di;
        return 0;
}

static int
make_add_recipe(unsigned int ri, unsigned int text)
{
        unsigned int ci;

        if (ri >= make_rule_count || make_recipe_count >= MAKE_MAX_RECIPES)
                return -1;
        ci = make_recipe_count++;
        make_recipes[ci].text = text;
        make_recipes[ci].next = MAKE_NONE;
        if (make_rules[ri].recipe_tail == MAKE_NONE)
                make_rules[ri].recipe_head = ci;
        else
                make_recipes[make_rules[ri].recipe_tail].next = ci;
        make_rules[ri].recipe_tail = ci;
        return 0;
}

static int
make_assignment(char *line, unsigned int flags)
{
        char *eq;
        char *colon;
        char *name;
        char *value;
        char op;

        eq = line;
        colon = 0;
        while (*eq != 0 && *eq != '=') {
                if (*eq == ':' && colon == 0)
                        colon = eq;
                ++eq;
        }
        if (*eq != '=')
                return 0;
        op = '=';
        if (eq != line && (eq[-1] == '?' || eq[-1] == '+' || eq[-1] == ':')) {
                op = eq[-1];
                --eq;
        } else if (colon != 0)
                return 0;
        *eq = 0;
        value = eq + 1;
        if (op != '=')
                ++value;
        name = make_trim(line);
        value = make_trim(value);
        if (*name == 0 || make_set_var(name, value, op, flags) != 0)
                return -1;
        return 1;
}

static int
make_suffix_rule_name(const char *name)
{
        if (name == 0 || name[0] != '.' || name[1] == 0)
                return 0;
        if (make_streq(name, ".PHONY") || make_streq(name, ".SUFFIXES"))
                return 0;
        return 1;
}

static int
make_parse_rule(char *line)
{
        char *colon;
        char *lhs;
        char *rhs;
        char *word;
        char *p;
        unsigned int lhs_count;
        unsigned int i;
        int ri;

        if (make_expand(line, make_expand_buf, sizeof(make_expand_buf), 0,
            0U) != 0)
                return -1;
        colon = make_expand_buf;
        while (*colon != 0 && *colon != ':')
                ++colon;
        if (*colon != ':')
                return -1;
        if (colon[1] == ':')
                return -1;
        *colon++ = 0;
        lhs = make_trim(make_expand_buf);
        rhs = make_trim(colon);
        lhs_count = 0U;
        p = lhs;
        while ((word = make_word(&p)) != 0) {
                if (lhs_count >= MAKE_MAX_LHS)
                        return -1;
                ri = make_get_rule(word);
                if (ri < 0)
                        return -1;
                make_current[lhs_count++] = (unsigned int)ri;
                if (make_suffix_rule_name(word))
                        make_rules[ri].flags |= MAKE_RULE_SUFFIX;
                if (make_default_rule == MAKE_NONE && word[0] != '.')
                        make_default_rule = (unsigned int)ri;
        }
        if (lhs_count == 0U)
                return -1;
        make_current_count = lhs_count;
        p = rhs;
        while ((word = make_word(&p)) != 0) {
                if (make_streq(lhs, ".PHONY")) {
                        ri = make_get_rule(word);
                        if (ri < 0)
                                return -1;
                        make_rules[ri].flags |= MAKE_RULE_PHONY;
                        continue;
                }
                if (make_streq(lhs, ".SUFFIXES"))
                        continue;
                for (i = 0U; i < lhs_count; ++i)
                        if (make_add_dep(make_current[i], word) != 0)
                                return -1;
        }
        if (make_streq(lhs, ".PHONY") || make_streq(lhs, ".SUFFIXES"))
                make_current_count = 0U;
        return 0;
}

static int
make_parse_file(const char *path)
{
        struct u_text_reader reader;
        char part[MAKE_LINE_MAX + 1U];
        char *line;
        char *hash;
        unsigned int used;
        unsigned int n;
        unsigned int i;
        unsigned int recipe_text;
        int rc;
        int ar;

        if (u_text_open(&reader, path) != 0)
                return -1;
        for (;;) {
                rc = u_text_getline(&reader, part, sizeof(part));
                if (rc == U_TEXT_EOF)
                        break;
                if (rc < 0) {
                        u_text_close(&reader);
                        return -1;
                }
                used = 0U;
                make_line[0] = 0;
                for (;;) {
                        n = make_strlen(part);
                        if (n != 0U && part[n - 1U] == '\\') {
                                part[n - 1U] = 0;
                                if (make_append(make_line, sizeof(make_line),
                                    &used, part) != 0 ||
                                    make_append(make_line, sizeof(make_line),
                                    &used, " ") != 0) {
                                        u_text_close(&reader);
                                        return -1;
                                }
                                rc = u_text_getline(&reader, part,
                                    sizeof(part));
                                if (rc < 0) {
                                        u_text_close(&reader);
                                        return -1;
                                }
                                continue;
                        }
                        if (make_append(make_line, sizeof(make_line), &used,
                            part) != 0) {
                                u_text_close(&reader);
                                return -1;
                        }
                        break;
                }
                line = make_trim(make_line);
                if (*line == 0 || *line == '#')
                        continue;
                if (*line == '>') {
                        if (make_current_count == 0U) {
                                u_text_close(&reader);
                                return -1;
                        }
                        ++line;
                        while (*line == ' ')
                                ++line;
                        recipe_text = make_store(line);
                        if (recipe_text == MAKE_NONE) {
                                u_text_close(&reader);
                                return -1;
                        }
                        for (i = 0U; i < make_current_count; ++i)
                                if (make_add_recipe(make_current[i],
                                    recipe_text) != 0) {
                                        u_text_close(&reader);
                                        return -1;
                                }
                        continue;
                }
                make_current_count = 0U;
                hash = line;
                while (*hash != 0 && *hash != '#')
                        ++hash;
                *hash = 0;
                line = make_trim(line);
                if (*line == 0)
                        continue;
                for (i = 0U; line[i] != 0; ++i)
                        make_work[i] = line[i];
                make_work[i] = 0;
                ar = make_assignment(make_work, 0U);
                if (ar < 0 || (ar == 0 && make_parse_rule(line) != 0)) {
                        u_text_close(&reader);
                        return -1;
                }
        }
        u_text_close(&reader);
        return 0;
}

static unsigned int
make_days(unsigned int month, unsigned int year)
{
        static const unsigned int days[12] = {
                31U, 28U, 31U, 30U, 31U, 30U,
                31U, 31U, 30U, 31U, 30U, 31U
        };

        if (month == 2U && ((year % 4U) == 0U) &&
            (((year % 100U) != 0U) || ((year % 400U) == 0U)))
                return 29U;
        return days[month - 1U];
}

static int
make_time_valid(kword_t t)
{
        unsigned int century;
        unsigned int bcd;
        unsigned int yy;
        unsigned int year;
        unsigned int month;
        unsigned int day;
        unsigned int hour;
        unsigned int minute;
        unsigned int second;

        if (t == 0UL || (t & (1UL << 35U)) != 0UL)
                return 0;
        century = (unsigned int)((t >> 34U) & 1UL);
        bcd = (unsigned int)((t >> 26U) & 0377UL);
        if (((bcd >> 4U) & 017U) > 9U || (bcd & 017U) > 9U)
                return 0;
        yy = ((bcd >> 4U) & 017U) * 10U + (bcd & 017U);
        if ((!century && yy < 26U) || (century && yy > 25U))
                return 0;
        year = century ? 2100U + yy : 2000U + yy;
        month = (unsigned int)((t >> 22U) & 017UL);
        day = (unsigned int)((t >> 17U) & 037UL);
        hour = (unsigned int)((t >> 12U) & 037UL);
        minute = (unsigned int)((t >> 6U) & 077UL);
        second = (unsigned int)(t & 077UL);
        if (month < 1U || month > 12U || day < 1U ||
            day > make_days(month, year) || hour > 23U || minute > 59U ||
            second > 59U)
                return 0;
        return 1;
}

static int
make_stat(const char *name, struct make_result *result)
{
        struct vfs_stat st;

        result->mtime = 0UL;
        result->exists = 0U;
        result->valid = 0U;
        result->changed = 0U;
        if (u_s6_pack(make_path, U_PATH_WORDS, name) != 0)
                return -1;
        if (dsys_stat(make_path, &st) != 0)
                return 0;
        result->exists = 1U;
        result->mtime = st.mtime;
        result->valid = make_time_valid(st.mtime) ? 1U : 0U;
        return 0;
}

static int
make_source_rule(const char *target, struct make_implicit *imp)
{
        const char *dot;
        const char *rn;
        const char *second;
        const char *source_end;
        unsigned int stem_len;
        unsigned int src_len;
        unsigned int target_len;
        unsigned int i;
        struct make_result sr;

        dot = 0;
        for (i = 0U; target[i] != 0; ++i) {
                if (target[i] == '/')
                        dot = 0;
                else if (target[i] == '.')
                        dot = &target[i];
        }
        for (i = 0U; i < make_rule_count; ++i) {
                if ((make_rules[i].flags & MAKE_RULE_SUFFIX) == 0U ||
                    make_rules[i].recipe_head == MAKE_NONE)
                        continue;
                rn = make_text(make_rules[i].name);
                second = rn + 1;
                while (*second != 0 && *second != '.')
                        ++second;
                if (*second == '.') {
                        if (dot == 0 || !make_streq(second, dot))
                                continue;
                        stem_len = (unsigned int)(dot - target);
                        source_end = second;
                } else {
                        /* Classic single-suffix rule: .S: builds FOO from
                         * FOO.S.  It applies only to a target with no suffix
                         * in its final path component. */
                        if (dot != 0)
                                continue;
                        target_len = make_strlen(target);
                        stem_len = target_len;
                        source_end = second;
                }
                src_len = (unsigned int)(source_end - rn);
                if (stem_len + src_len > MAKE_NAME_MAX)
                        continue;
                for (src_len = 0U; src_len < stem_len; ++src_len) {
                        imp->source[src_len] = target[src_len];
                        imp->stem[src_len] = target[src_len];
                }
                imp->stem[stem_len] = 0;
                for (src_len = 0U; rn[src_len] != 0 &&
                    &rn[src_len] < source_end; ++src_len)
                        imp->source[stem_len + src_len] = rn[src_len];
                imp->source[stem_len + src_len] = 0;
                if (make_find_rule(imp->source) >= 0 ||
                    (make_stat(imp->source, &sr) == 0 && sr.exists)) {
                        imp->rule = (int)i;
                        return 0;
                }
        }
        return -1;
}

/*
 * A zero mtime is the native representation used by immutable image-seeded
 * files and by filesystems which cannot supply a creation time.  Such a
 * prerequisite cannot be ordered against a real timestamp, but repeatedly
 * rebuilding forever is worse and makes source trees shipped on D6FS/TSFS
 * unusable with INSTALL.  Once a target has a valid timestamp, treat a zero
 * prerequisite as older.  Malformed nonzero timestamps remain conservative
 * and force a rebuild.
 */
static int
make_dep_requires_update(const struct make_result *dep,
    const struct make_result *target)
{
        if (!target->exists || !target->valid)
                return 1;
        if (dep->valid)
                return dep->mtime > target->mtime;
        return dep->mtime != 0UL;
}

static unsigned int
make_record_words(const kword_t *record)
{
        unsigned int chars;

        chars = (unsigned int)(record[0] & 0777777UL);
        return 1U + (chars + 5U) / 6U;
}

static int
make_run_append(unsigned int *used, const kword_t *record)
{
        unsigned int words;
        unsigned int i;

        words = make_record_words(record);
        if (*used + words > MAKE_RUN_WORDS)
                return -1;
        for (i = 0U; i < words; ++i)
                make_run[(*used)++] = record[i];
        return 0;
}

static int
make_run_recipe(const char *command)
{
        kword_t path[U_PATH_WORDS];
        kword_t arg0[U_ARG_WORDS];
        kword_t arg1[U_ARG_WORDS];
        kword_t arg2[U_ARG_WORDS];
        struct sys_run_v2 *run;
        kword_t status;
        unsigned int used;
        unsigned int envc;
        int pid;

        if (u_s6_pack(path, U_PATH_WORDS, "/SYSTEM/EXEC/DSH") != 0 ||
            u_s6_pack(arg0, U_ARG_WORDS, "DSH") != 0 ||
            u_s6_pack(arg1, U_ARG_WORDS, "-C") != 0 ||
            u_s6_pack(arg2, U_ARG_WORDS, command) != 0)
                return 126;
        used = SYS_RUN_V2_FIXED_WORDS;
        if (make_run_append(&used, path) != 0 ||
            make_run_append(&used, arg0) != 0 ||
            make_run_append(&used, arg1) != 0 ||
            make_run_append(&used, arg2) != 0)
                return 126;
        envc = 0U;
        if (make_envp != 0) {
                while (make_envp[envc] != 0 && envc < SYS_RUN_ENV_MAX) {
                        if (make_run_append(&used, make_envp[envc]) != 0)
                                return 126;
                        ++envc;
                }
        }
        if (used + 3U > MAKE_RUN_WORDS)
                return 126;
        make_run[used++] = SYS_RUN_FD_MAP(0U, 0U);
        make_run[used++] = SYS_RUN_FD_MAP(1U, 1U);
        make_run[used++] = SYS_RUN_FD_MAP(2U, 2U);
        run = (struct sys_run_v2 *)make_run;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, used);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = 3UL;
        run->envc = envc;
        pid = dsys_run(run);
        if (pid < 0)
                return 126;
        if (dsys_wait((unsigned int)pid, &status, 0U) != pid ||
            SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                return 126;
        return (int)SYS_WAIT_STATUS_VALUE(status);
}

static int
make_run_commands(unsigned int head, const struct make_auto *automatic)
{
        char *p;
        unsigned int ci;
        int ignore;
        int quiet;
        int rc;

        for (ci = head; ci != MAKE_NONE; ci = make_recipes[ci].next) {
                if (make_expand(make_text(make_recipes[ci].text),
                    make_expand_buf, sizeof(make_expand_buf), automatic,
                    0U) != 0 || make_strlen(make_expand_buf) > MAKE_RECIPE_MAX)
                        return 1;
                p = make_trim(make_expand_buf);
                ignore = 0;
                quiet = 0;
                while (*p == '@' || *p == '-') {
                        if (*p == '@') quiet = 1;
                        else ignore = 1;
                        ++p;
                }
                p = make_trim(p);
                if (!make_silent && !quiet && !make_question) {
                        (void)u_puts(1, p);
                        (void)u_crlf(1);
                }
                if (make_dry_run || *p == 0)
                        continue;
                rc = make_run_recipe(p);
                if (rc != 0 && !ignore)
                        return rc;
        }
        return 0;
}

static int make_build(const char *name, struct make_result *out,
    unsigned int depth);

static int
make_newer_add(const char *name)
{
        unsigned int used;

        used = make_strlen(make_newer_buf);
        if (used != 0U && make_append(make_newer_buf, sizeof(make_newer_buf),
            &used, " ") != 0)
                return -1;
        return make_append(make_newer_buf, sizeof(make_newer_buf), &used,
            name);
}

static int
make_build(const char *name, struct make_result *out, unsigned int depth)
{
        struct make_result target;
        struct make_result dep;
        struct make_implicit imp;
        struct make_auto automatic;
        struct make_rule *rule;
        const char *depname;
        const char *first;
        unsigned int di;
        unsigned int recipe;
        unsigned int need;
        int ri;
        int ir;
        int failed;
        int rc;

        if (depth > MAKE_MAX_DEPTH)
                return 1;
        ri = make_find_rule(name);
        if (ri >= 0) {
                rule = &make_rules[ri];
                if (rule->state == MAKE_STATE_ACTIVE) {
                        make_diag("DEPENDENCY CYCLE", name);
                        return 1;
                }
                if (rule->state == MAKE_STATE_DONE) {
                        rc = make_stat(name, out);
                        if (rc == 0 && (rule->flags & (MAKE_RULE_CHANGED |
                            MAKE_RULE_PHONY)) != 0U)
                                out->changed = 1U;
                        return rc;
                }
                if (rule->state == MAKE_STATE_FAILED)
                        return 1;
                rule->state = MAKE_STATE_ACTIVE;
        } else
                rule = 0;
        if (make_stat(name, &target) != 0)
                goto fail;
        imp.rule = -1;
        imp.source[0] = 0;
        imp.stem[0] = 0;
        if ((rule == 0 || rule->recipe_head == MAKE_NONE) &&
            make_source_rule(name, &imp) == 0)
                ir = imp.rule;
        else
                ir = -1;
        if (rule == 0 && !target.exists && ir < 0) {
                make_diag("DON'T KNOW HOW TO MAKE", name);
                return 1;
        }
        if (rule == 0 && target.exists && ir < 0) {
                *out = target;
                return 0;
        }
        need = make_always || !target.exists || !target.valid;
        if (rule != 0 && (rule->flags & MAKE_RULE_PHONY) != 0U)
                need = 1U;
        failed = 0;
        first = 0;
        if (rule != 0) {
                for (di = rule->dep_head; di != MAKE_NONE;
                    di = make_deps[di].next) {
                        depname = make_text(make_deps[di].text);
                        if (first == 0)
                                first = depname;
                        rc = make_build(depname, &dep, depth + 1U);
                        if (rc != 0) {
                                failed = 1;
                                if (!make_keep_going)
                                        goto fail;
                                continue;
                        }
                        if (dep.changed ||
                            make_dep_requires_update(&dep, &target))
                                need = 1U;
                }
        }
        if (ir >= 0) {
                if (first == 0)
                        first = imp.source;
                rc = make_build(imp.source, &dep, depth + 1U);
                if (rc != 0)
                        goto fail;
                if (dep.changed ||
                    make_dep_requires_update(&dep, &target))
                        need = 1U;
        }
        if (failed)
                goto fail;
        /* Build recursion above uses the same small scratch buffer.  Form $?
         * only after all children are complete, when no deeper call can
         * overwrite it. */
        make_newer_buf[0] = 0;
        if (rule != 0) {
                for (di = rule->dep_head; di != MAKE_NONE;
                    di = make_deps[di].next) {
                        int dri;
                        depname = make_text(make_deps[di].text);
                        if (make_stat(depname, &dep) != 0)
                                goto fail;
                        dri = make_find_rule(depname);
                        if ((dri >= 0 && (make_rules[dri].flags &
                            (MAKE_RULE_CHANGED | MAKE_RULE_PHONY)) != 0U) ||
                            make_dep_requires_update(&dep, &target))
                                if (make_newer_add(depname) != 0)
                                        goto fail;
                }
        }
        if (ir >= 0) {
                int sri;
                if (make_stat(imp.source, &dep) != 0)
                        goto fail;
                sri = make_find_rule(imp.source);
                if ((sri >= 0 && (make_rules[sri].flags &
                    (MAKE_RULE_CHANGED | MAKE_RULE_PHONY)) != 0U) ||
                    make_dep_requires_update(&dep, &target))
                        if (make_newer_add(imp.source) != 0)
                                goto fail;
        }
        recipe = rule != 0 && rule->recipe_head != MAKE_NONE ?
            rule->recipe_head : (ir >= 0 ? make_rules[ir].recipe_head :
            MAKE_NONE);
        if (need && recipe != MAKE_NONE) {
                automatic.target = name;
                automatic.first = first == 0 ? "" : first;
                automatic.newer = make_newer_buf;
                automatic.stem = ir >= 0 ? imp.stem : "";
                rc = make_run_commands(recipe, &automatic);
                if (rc != 0) {
                        make_diag("RECIPE FAILED", name);
                        goto fail;
                }
                target.changed = 1U;
                if (rule != 0)
                        rule->flags |= MAKE_RULE_CHANGED;
                if (!make_dry_run) {
                        struct make_result after;
                        if (make_stat(name, &after) == 0 && after.exists) {
                                after.changed = 1U;
                                target = after;
                        }
                }
        } else if (need) {
                target.changed = 1U;
                if (rule != 0)
                        rule->flags |= MAKE_RULE_CHANGED;
                if (!target.exists && rule != 0 &&
                    rule->dep_head == MAKE_NONE &&
                    (rule->flags & MAKE_RULE_PHONY) == 0U) {
                        make_diag("NO RECIPE FOR", name);
                        goto fail;
                }
        }
        if (rule != 0)
                rule->state = MAKE_STATE_DONE;
        *out = target;
        return 0;

fail:
        if (rule != 0)
                rule->state = MAKE_STATE_FAILED;
        return 1;
}

static int
make_command_assignment(const char *arg)
{
        unsigned int i;

        for (i = 0U; arg[i] != 0; ++i)
                if (arg[i] == '=')
                        return i != 0U;
        return 0;
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        char arg[MAKE_LINE_MAX + 1U];
        char makefile[MAKE_NAME_MAX + 1U];
        char goals[MAKE_MAX_GOALS][MAKE_NAME_MAX + 1U];
        char *eq;
        struct make_result result;
        unsigned int goal_count;
        unsigned int i;
        unsigned int j;
        int rc;
        int failed;

        make_envp = envp;
        if (make_import_environment(envp) != 0 ||
            make_set_var("MAKE", "MAKE", '=', 0U) != 0) {
                make_diag("VARIABLE STORAGE EXHAUSTED", 0);
                return 2;
        }
        makefile[0] = 'M'; makefile[1] = 'A'; makefile[2] = 'K';
        makefile[3] = 'E'; makefile[4] = 'F'; makefile[5] = 'I';
        makefile[6] = 'L'; makefile[7] = 'E'; makefile[8] = 0;
        goal_count = 0U;
        for (i = 1U; i < (unsigned int)argc; ++i) {
                if (make_counted_text(argv[i], arg, sizeof(arg)) != 0) {
                        make_diag("INVALID ARGUMENT", 0);
                        return 2;
                }
                if ((make_streq(arg, "-F") || make_streq(arg, "-f") ||
                    make_streq(arg, "-C")) && i + 1U < (unsigned int)argc) {
                        int chdir_opt;
                        chdir_opt = make_streq(arg, "-C");
                        if (make_counted_text(argv[++i], arg,
                            sizeof(arg)) != 0)
                                return 2;
                        if (chdir_opt) {
                                if (u_s6_pack(make_path, U_PATH_WORDS, arg) !=
                                    0 || dsys_chdir(make_path) != 0) {
                                        make_diag("CANNOT CHDIR", arg);
                                        return 2;
                                }
                        } else {
                                if (make_strlen(arg) > MAKE_NAME_MAX)
                                        return 2;
                                for (j = 0U; arg[j] != 0; ++j)
                                        makefile[j] = arg[j];
                                makefile[j] = 0;
                        }
                        continue;
                }
                if (arg[0] == '-' && arg[1] != 0) {
                        for (j = 1U; arg[j] != 0; ++j) {
                                if (arg[j] == 'N' || arg[j] == 'n')
                                        make_dry_run = 1;
                                else if (arg[j] == 'S' || arg[j] == 's')
                                        make_silent = 1;
                                else if (arg[j] == 'K' || arg[j] == 'k')
                                        make_keep_going = 1;
                                else if (arg[j] == 'Q' || arg[j] == 'q') {
                                        make_question = 1;
                                        make_dry_run = 1;
                                } else if (arg[j] == 'B' || arg[j] == 'b')
                                        make_always = 1;
                                else {
                                        make_diag("UNKNOWN OPTION", arg);
                                        return 2;
                                }
                        }
                        continue;
                }
                if (make_command_assignment(arg)) {
                        eq = arg;
                        while (*eq != '=')
                                ++eq;
                        *eq++ = 0;
                        if (make_set_var(arg, eq, '=', MAKE_VAR_COMMAND) != 0) {
                                make_diag("VARIABLE STORAGE EXHAUSTED", arg);
                                return 2;
                        }
                        continue;
                }
                if (goal_count >= MAKE_MAX_GOALS ||
                    make_strlen(arg) > MAKE_NAME_MAX)
                        return 2;
                for (j = 0U; arg[j] != 0; ++j)
                        goals[goal_count][j] = arg[j];
                goals[goal_count][j] = 0;
                ++goal_count;
        }
        if (make_parse_file(makefile) != 0) {
                make_diag("CANNOT PARSE", makefile);
                return 1;
        }
        if (goal_count == 0U) {
                if (make_default_rule == MAKE_NONE) {
                        make_diag("NO TARGET", 0);
                        return 1;
                }
                if (make_strlen(make_text(make_rules[make_default_rule].name)) >
                    MAKE_NAME_MAX)
                        return 1;
                i = 0U;
                while (make_text(make_rules[make_default_rule].name)[i] != 0) {
                        goals[0][i] =
                            make_text(make_rules[make_default_rule].name)[i];
                        ++i;
                }
                goals[0][i] = 0;
                goal_count = 1U;
        }
        failed = 0;
        for (i = 0U; i < goal_count; ++i) {
                rc = make_build(goals[i], &result, 0U);
                if (rc != 0) {
                        failed = 1;
                        if (!make_keep_going)
                                break;
                } else if (make_question && result.changed)
                        failed = 1;
        }
        return failed ? 1 : 0;
}
