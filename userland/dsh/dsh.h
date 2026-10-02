#ifndef DAIMOS_DSH_H
#define DAIMOS_DSH_H

#include "u.h"

#define DSH_S6_CHARS_PER_WORD 6U
#define DSH_S6_MAX_CHARS      36U
#define DSH_S6_MAX_WORDS       6U
#define DSH_LINE_MAX_CHARS    180U
#define DSH_LINE_MAX_WORDS     30U
#define DSH_MAX_ARGS            8U
#define DSH_MAX_VARS           16U
#define DSH_MAX_ALIASES         8U

#define DSH_OK                  0
#define DSH_ERROR               2
#define DSH_NOT_FOUND         127

struct dsh_s6 {
        unsigned int len;
        kword_t words[DSH_S6_MAX_WORDS];
};

struct dsh_line {
        unsigned int len;
        kword_t words[DSH_LINE_MAX_WORDS];
};

struct dsh_var {
        unsigned int used;
        unsigned int exported;
        struct dsh_s6 name;
        struct dsh_s6 value;
};

struct dsh_alias {
        unsigned int used;
        struct dsh_s6 name;
        struct dsh_s6 value;
};

struct dsh_state {
        struct dsh_var vars[DSH_MAX_VARS];
        struct dsh_alias aliases[DSH_MAX_ALIASES];
        struct dsh_s6 arg0;
        struct dsh_s6 args[DSH_MAX_ARGS];
        unsigned int argc;
        unsigned int status;
        unsigned int exit_requested;
        unsigned int exit_status;
};

void dsh_s6_clear(struct dsh_s6 *s);
int dsh_s6_append(struct dsh_s6 *s, int ch);
int dsh_s6_get(const struct dsh_s6 *s, unsigned int pos);
int dsh_s6_copy(struct dsh_s6 *dst, const struct dsh_s6 *src);
int dsh_s6_eq_text(const struct dsh_s6 *s, const char *text);
int dsh_s6_pack(const struct dsh_s6 *s, kword_t *dst, unsigned int words);
int dsh_s6_from_counted(struct dsh_s6 *dst, const kword_t *src);
int dsh_s6_put(int fd, const struct dsh_s6 *s);

void dsh_state_init(struct dsh_state *st, int argc, kword_t **argv,
    kword_t **envp);
int dsh_var_set(struct dsh_state *st, const struct dsh_s6 *name,
    const struct dsh_s6 *value, int exported);
int dsh_var_unset(struct dsh_state *st, const struct dsh_s6 *name);
const struct dsh_s6 *dsh_var_get(const struct dsh_state *st,
    const struct dsh_s6 *name);
int dsh_expand(const struct dsh_state *st, const struct dsh_s6 *in,
    struct dsh_s6 *out);
int dsh_execute_line(struct dsh_state *st, const struct dsh_line *line);

#endif
