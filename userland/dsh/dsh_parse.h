#ifndef DAIMON_DSH_PARSE_H
#define DAIMON_DSH_PARSE_H

#include "dsh.h"

#define DSH_PARSE_MAX_NODES     16U
#define DSH_PARSE_MAX_WORDS      8U

#define DSH_N_EMPTY              0
#define DSH_N_SIMPLE             1
#define DSH_N_LIST               2
#define DSH_N_AND                3
#define DSH_N_OR                 4
#define DSH_N_PIPE               5
#define DSH_N_NOT                6
#define DSH_N_IF                 7
#define DSH_N_FOR                8
#define DSH_N_WHILE              9
#define DSH_N_UNTIL             10
#define DSH_N_BG                11
#define DSH_N_SUBST             12
#define DSH_N_DEF               13
#define DSH_N_CASE              14
#define DSH_N_CASE_ARM          15

#define DSH_REDIR_IN          0001U
#define DSH_REDIR_OUT         0002U
#define DSH_REDIR_APPEND      0004U

#define DSH_NONE              0177777U

#define DSH_PE_NONE             0U
#define DSH_PE_COMMAND          1U
#define DSH_PE_WORD             2U
#define DSH_PE_THEN             3U
#define DSH_PE_ELSE             4U
#define DSH_PE_FI               5U
#define DSH_PE_IN               6U
#define DSH_PE_DO               7U
#define DSH_PE_DONE             8U
#define DSH_PE_END              9U
#define DSH_PE_RPAREN          10U
#define DSH_PE_ESAC            11U

struct dsh_parse_diag {
        int code;
        unsigned int token;
        unsigned int expect;
};

struct dsh_node {
        int type;
        unsigned int left;
        unsigned int right;
        unsigned int extra;
        unsigned int argc;
        unsigned int flags;
        struct dsh_s6 redir_in;
        struct dsh_s6 redir_out;
        struct dsh_s6 words[DSH_PARSE_MAX_WORDS];
        kword_t literal_mask[DSH_PARSE_MAX_WORDS];
        kword_t quote_mask[DSH_PARSE_MAX_WORDS];
};

void    dsh_node_clear(struct dsh_node *n);
void    dsh_parse_diag_clear(struct dsh_parse_diag *diag);
int     dsh_parse_tokens_diag(const struct dsh_token *tokens,
            unsigned int ntokens, struct dsh_node *nodes,
            unsigned int max_nodes, unsigned int *root,
            unsigned int *used, struct dsh_parse_diag *diag);
int     dsh_parse_tokens(const struct dsh_token *tokens,
            unsigned int ntokens, struct dsh_node *nodes,
            unsigned int max_nodes, unsigned int *root,
            unsigned int *used);
int     dsh_parse_s6_line_diag(const struct dsh_line *line,
            struct dsh_node *nodes, unsigned int max_nodes,
            unsigned int *root, unsigned int *used, int *errpos,
            struct dsh_parse_diag *diag);
int     dsh_parse_s6_line(const struct dsh_line *line,
            struct dsh_node *nodes, unsigned int max_nodes,
            unsigned int *root, unsigned int *used, int *errpos);

#endif /* DAIMON_DSH_PARSE_H */
