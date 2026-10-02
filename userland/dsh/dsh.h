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
#define DSH_MAX_JOBS            4U
#define DSH_MAX_FUNCS           4U
#define DSH_FUNC_MAX_NODES     16U
#define DSH_FUNC_MAX_CALLS      4U
#define DSH_SCRIPT_MAX_TOKENS  32U

#define DSH_OK                  0
#define DSH_E_ARG              -1
#define DSH_E_RANGE            -2
#define DSH_E_CHAR             -3
#define DSH_E_LONG             -4
#define DSH_E_OVERFLOW         -5
#define DSH_E_SYNTAX           -6
#define DSH_ERROR               2
#define DSH_NOT_FOUND         127

struct dsh_s6 {
        unsigned int len;
        kword_t words[DSH_S6_MAX_WORDS];
};

struct dsh_token {
        int type;
        struct dsh_s6 text;
        kword_t literal_mask;
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

struct dsh_job {
        unsigned int used;
        unsigned int pgrp;
        unsigned int last_pid;
        unsigned int remaining;
        unsigned int status;
        unsigned int stopped;
};

struct dsh_state {
        struct dsh_var vars[DSH_MAX_VARS];
        struct dsh_alias aliases[DSH_MAX_ALIASES];
        struct dsh_job jobs[DSH_MAX_JOBS];
        struct dsh_s6 arg0;
        struct dsh_s6 args[DSH_MAX_ARGS];
        unsigned int argc;
        unsigned int status;
        unsigned int shell_pgrp;
        unsigned int tty_attached;
        unsigned int interactive;
        unsigned int exit_requested;
        unsigned int exit_status;
        unsigned int return_requested;
        unsigned int return_status;
        unsigned int call_depth;
};

struct dsh_script {
        struct dsh_token tokens[DSH_SCRIPT_MAX_TOKENS];
        unsigned int ntokens;
        unsigned int depth;
};

void dsh_s6_clear(struct dsh_s6 *s);
void dsh_line_clear(struct dsh_line *line);
int dsh_s6_append(struct dsh_s6 *s, int ch);
int dsh_s6_get(const struct dsh_s6 *s, unsigned int pos);
int dsh_s6_copy(struct dsh_s6 *dst, const struct dsh_s6 *src);
int dsh_s6_eq_text(const struct dsh_s6 *s, const char *text);
int dsh_s6_eq_packed(const struct dsh_s6 *s, unsigned int len,
    kword_t word0, kword_t word1);
int dsh_s6_pack(const struct dsh_s6 *s, kword_t *dst, unsigned int words);
int dsh_s6_from_counted(struct dsh_s6 *dst, const kword_t *src);
int dsh_s6_put(int fd, const struct dsh_s6 *s);
int dsh_line_get(const struct dsh_line *line, unsigned int pos);

void dsh_state_init(struct dsh_state *st, int argc, kword_t **argv,
    kword_t **envp);
int dsh_var_set(struct dsh_state *st, const struct dsh_s6 *name,
    const struct dsh_s6 *value, int exported);
int dsh_var_unset(struct dsh_state *st, const struct dsh_s6 *name);
const struct dsh_s6 *dsh_var_get(const struct dsh_state *st,
    const struct dsh_s6 *name);
int dsh_expand(const struct dsh_state *st, const struct dsh_s6 *in,
    struct dsh_s6 *out);
int dsh_expand_mask(const struct dsh_state *st, const struct dsh_s6 *in,
    kword_t literal_mask, struct dsh_s6 *out);
int dsh_execute_line(struct dsh_state *st, const struct dsh_line *line);
int dsh_execute_file(struct dsh_state *st, const struct dsh_s6 *path);
void dsh_script_init(struct dsh_script *script);
int dsh_script_feed(struct dsh_state *st, struct dsh_script *script,
    const struct dsh_line *line, int *status, unsigned int *need_more);
int dsh_script_finish(struct dsh_state *st, struct dsh_script *script,
    int *status);

#endif
