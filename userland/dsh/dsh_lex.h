#ifndef DAIMON_DSH_LEX_H
#define DAIMON_DSH_LEX_H

#include "dsh.h"

#define DSH_LEX_MAX_TOKENS     32U

#define DSH_T_EOF              0
#define DSH_T_WORD             1
#define DSH_T_SEMI             2
#define DSH_T_BG               3
#define DSH_T_PIPE             4
#define DSH_T_REDIR_IN         5
#define DSH_T_REDIR_OUT        6
#define DSH_T_APPEND           7
#define DSH_T_LPAREN           8
#define DSH_T_RPAREN           9
#define DSH_T_DOLLAR_LPAREN   10

void    dsh_token_clear(struct dsh_token *t);
int     dsh_lex_s6_line(const struct dsh_line *line,
            struct dsh_token *tokens, unsigned int max_tokens,
            unsigned int *ntokens, int *errpos);

#endif /* DAIMON_DSH_LEX_H */
