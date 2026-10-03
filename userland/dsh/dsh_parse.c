#include "dsh_parse.h"
#include "dsh_lex.h"
#include "pdp10-sixbit.h"

#define S6_P2(a,b) PDP10_SIX6(a,b,' ',' ',' ',' ')
#define S6_P3(a,b,c) PDP10_SIX6(a,b,c,' ',' ',' ')
#define S6_P4(a,b,c,d) PDP10_SIX6(a,b,c,d,' ',' ')
#define S6_P5(a,b,c,d,e) PDP10_SIX6(a,b,c,d,e,' ')

#define DSH_STOP_THEN   0001U
#define DSH_STOP_ELSE   0002U
#define DSH_STOP_FI     0004U
#define DSH_STOP_DO     0010U
#define DSH_STOP_DONE   0020U
#define DSH_STOP_RPAREN 0040U
#define DSH_STOP_END    0100U
#define DSH_STOP_ESAC   0200U
#define DSH_STOP_WHEN   0400U

struct dsh_parser {
        const struct dsh_token *tokens;
        unsigned int ntokens;
        unsigned int pos;
        struct dsh_node *nodes;
        unsigned int max_nodes;
        unsigned int used;
        struct dsh_parse_diag *diag;
};

static int dsh_parse_list(struct dsh_parser *p, unsigned int stops,
        unsigned int *out);

void
dsh_parse_diag_clear(struct dsh_parse_diag *diag)
{
        if (diag == 0)
                return;
        diag->code = DSH_OK;
        diag->token = 0;
        diag->expect = DSH_PE_NONE;
}

static int
dsh_mark_error(struct dsh_parser *p, int code, unsigned int expect)
{
        if (p != 0 && p->diag != 0 && p->diag->code == DSH_OK) {
                p->diag->code = code;
                p->diag->token = p->pos;
                p->diag->expect = expect;
        }
        return code;
}

void
dsh_node_clear(struct dsh_node *n)
{
        unsigned int i;

        if (n == 0)
                return;
        n->type = DSH_N_EMPTY;
        n->left = DSH_NONE;
        n->right = DSH_NONE;
        n->extra = DSH_NONE;
        n->argc = 0;
        n->flags = 0;
        dsh_s6_clear(&n->redir_in);
        dsh_s6_clear(&n->redir_out);
        for (i = 0; i < DSH_PARSE_MAX_WORDS; i++) {
                dsh_s6_clear(&n->words[i]);
                n->literal_mask[i] = 0;
                n->quote_mask[i] = 0;
        }
}

#define DSH_K_AND      1U
#define DSH_K_BEGIN    2U
#define DSH_K_CASE     3U
#define DSH_K_DEF      4U
#define DSH_K_DO       5U
#define DSH_K_DONE     6U
#define DSH_K_ELSE     7U
#define DSH_K_END      8U
#define DSH_K_ESAC     9U
#define DSH_K_FI       10U
#define DSH_K_FOR      11U
#define DSH_K_IF       12U
#define DSH_K_IN       13U
#define DSH_K_NOT      14U
#define DSH_K_OR       15U
#define DSH_K_THEN     16U
#define DSH_K_UNTIL    17U
#define DSH_K_WHILE    18U
#define DSH_K_WHEN     19U

static unsigned int
dsh_s6_kw(const struct dsh_s6 *s)
{
        if (s == 0)
                return 0U;
        switch (s->len) {
        case 2U:
                if (dsh_s6_eq_packed(s, 2U, S6_P2('D','O'), 0))
                        return DSH_K_DO;
                if (dsh_s6_eq_packed(s, 2U, S6_P2('F','I'), 0))
                        return DSH_K_FI;
                if (dsh_s6_eq_packed(s, 2U, S6_P2('I','F'), 0))
                        return DSH_K_IF;
                if (dsh_s6_eq_packed(s, 2U, S6_P2('I','N'), 0))
                        return DSH_K_IN;
                if (dsh_s6_eq_packed(s, 2U, S6_P2('O','R'), 0))
                        return DSH_K_OR;
                return 0U;
        case 3U:
                if (dsh_s6_eq_packed(s, 3U, S6_P3('A','N','D'), 0))
                        return DSH_K_AND;
                if (dsh_s6_eq_packed(s, 3U, S6_P3('D','E','F'), 0))
                        return DSH_K_DEF;
                if (dsh_s6_eq_packed(s, 3U, S6_P3('F','O','R'), 0))
                        return DSH_K_FOR;
                if (dsh_s6_eq_packed(s, 3U, S6_P3('N','O','T'), 0))
                        return DSH_K_NOT;
                if (dsh_s6_eq_packed(s, 3U, S6_P3('E','N','D'), 0))
                        return DSH_K_END;
                return 0U;
        case 4U:
                if (dsh_s6_eq_packed(s, 4U, S6_P4('D','O','N','E'), 0))
                        return DSH_K_DONE;
                if (dsh_s6_eq_packed(s, 4U, S6_P4('E','L','S','E'), 0))
                        return DSH_K_ELSE;
                if (dsh_s6_eq_packed(s, 4U, S6_P4('E','S','A','C'), 0))
                        return DSH_K_ESAC;
                if (dsh_s6_eq_packed(s, 4U, S6_P4('C','A','S','E'), 0))
                        return DSH_K_CASE;
                if (dsh_s6_eq_packed(s, 4U, S6_P4('T','H','E','N'), 0))
                        return DSH_K_THEN;
                if (dsh_s6_eq_packed(s, 4U, S6_P4('W','H','E','N'), 0))
                        return DSH_K_WHEN;
                return 0U;
        case 5U:
                if (dsh_s6_eq_packed(s, 5U, S6_P5('B','E','G','I','N'), 0))
                        return DSH_K_BEGIN;
                if (dsh_s6_eq_packed(s, 5U, S6_P5('U','N','T','I','L'), 0))
                        return DSH_K_UNTIL;
                if (dsh_s6_eq_packed(s, 5U, S6_P5('W','H','I','L','E'), 0))
                        return DSH_K_WHILE;
                return 0U;
        default:
                return 0U;
        }
}


static unsigned int
dsh_tok_kw(const struct dsh_token *t)
{
        if (t == 0 || DSH_TOKEN_TYPE(t) != DSH_T_WORD ||
            DSH_TOKEN_QUOTED(t))
                return 0U;
        return dsh_s6_kw(&t->text);
}

static unsigned int
dsh_expect_id_kw(unsigned int kw)
{
        switch (kw) {
        case DSH_K_THEN:
                return DSH_PE_THEN;
        case DSH_K_ELSE:
                return DSH_PE_ELSE;
        case DSH_K_FI:
                return DSH_PE_FI;
        case DSH_K_IN:
                return DSH_PE_IN;
        case DSH_K_DO:
                return DSH_PE_DO;
        case DSH_K_DONE:
                return DSH_PE_DONE;
        case DSH_K_END:
                return DSH_PE_END;
        case DSH_K_ESAC:
                return DSH_PE_ESAC;
        default:
                return DSH_PE_WORD;
        }
}

static int
dsh_at_stop(const struct dsh_parser *p, unsigned int stops)
{
        const struct dsh_token *t;

        if (p->pos >= p->ntokens)
                return 1;
        t = &p->tokens[p->pos];
        if ((stops & DSH_STOP_RPAREN) != 0U &&
            DSH_TOKEN_TYPE(t) == DSH_T_RPAREN)
                return 1;
        switch (dsh_tok_kw(t)) {
        case DSH_K_THEN:
                return (stops & DSH_STOP_THEN) != 0U;
        case DSH_K_ELSE:
                return (stops & DSH_STOP_ELSE) != 0U;
        case DSH_K_FI:
                return (stops & DSH_STOP_FI) != 0U;
        case DSH_K_DO:
                return (stops & DSH_STOP_DO) != 0U;
        case DSH_K_DONE:
                return (stops & DSH_STOP_DONE) != 0U;
        case DSH_K_END:
                return (stops & DSH_STOP_END) != 0U;
        case DSH_K_ESAC:
                return (stops & DSH_STOP_ESAC) != 0U;
        case DSH_K_WHEN:
                return (stops & DSH_STOP_WHEN) != 0U;
        default:
                break;
        }
        return 0;
}

static int
dsh_alloc_node(struct dsh_parser *p, int type, unsigned int *out)
{
        if (p == 0 || out == 0)
                return DSH_E_ARG;
        if (p->used >= p->max_nodes)
                return dsh_mark_error(p, DSH_E_OVERFLOW, DSH_PE_NONE);
        *out = p->used;
        dsh_node_clear(&p->nodes[p->used]);
        p->nodes[p->used].type = type;
        p->used++;
        return DSH_OK;
}

static int
dsh_expect_word(struct dsh_parser *p, unsigned int kw)
{
        if (p->pos >= p->ntokens || dsh_tok_kw(&p->tokens[p->pos]) != kw)
                return dsh_mark_error(p, DSH_E_SYNTAX,
                    dsh_expect_id_kw(kw));
        p->pos++;
        return DSH_OK;
}

static int
dsh_expect_token(struct dsh_parser *p, int type, unsigned int expect)
{
        if (p->pos >= p->ntokens ||
            DSH_TOKEN_TYPE(&p->tokens[p->pos]) != type)
                return dsh_mark_error(p, DSH_E_SYNTAX, expect);
        p->pos++;
        return DSH_OK;
}

static int
dsh_accept_word(struct dsh_parser *p, unsigned int kw)
{
        if (p->pos < p->ntokens && dsh_tok_kw(&p->tokens[p->pos]) == kw) {
                p->pos++;
                return 1;
        }
        return 0;
}

static int
dsh_parse_simple(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        unsigned int node;
        struct dsh_node *n;
        int rc;

        rc = dsh_alloc_node(p, DSH_N_SIMPLE, &node);
        if (rc != DSH_OK)
                return rc;
        n = &p->nodes[node];
        while (p->pos < p->ntokens && !dsh_at_stop(p, stops)) {
                if (DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_REDIR_IN ||
                    DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_REDIR_OUT ||
                    DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_APPEND) {
                        int rtype;

                        rtype = DSH_TOKEN_TYPE(&p->tokens[p->pos]);
                        p->pos++;
                        if (p->pos >= p->ntokens ||
                            DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                                return dsh_mark_error(p, DSH_E_SYNTAX,
                                    DSH_PE_WORD);
                        if (rtype == DSH_T_REDIR_IN) {
                                rc = dsh_s6_copy(&n->redir_in,
                                    &p->tokens[p->pos].text);
                                if (rc != DSH_OK)
                                        return rc;
                                n->flags |= DSH_REDIR_IN;
                        } else {
                                rc = dsh_s6_copy(&n->redir_out,
                                    &p->tokens[p->pos].text);
                                if (rc != DSH_OK)
                                        return rc;
                                n->flags |= DSH_REDIR_OUT;
                                if (rtype == DSH_T_APPEND)
                                        n->flags |= DSH_REDIR_APPEND;
                        }
                        p->pos++;
                        continue;
                }
                if (DSH_TOKEN_TYPE(&p->tokens[p->pos]) ==
                    DSH_T_DOLLAR_LPAREN) {
                        rc = dsh_alloc_node(p, DSH_N_SUBST, &node);
                        if (rc != DSH_OK)
                                return rc;
                        *out = node;
                        return DSH_OK;
                }
                if (DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                        break;
                if (dsh_tok_kw(&p->tokens[p->pos]) == DSH_K_AND ||
                    dsh_tok_kw(&p->tokens[p->pos]) == DSH_K_OR)
                        break;
                if (n->argc >= DSH_PARSE_MAX_WORDS)
                        return dsh_mark_error(p, DSH_E_OVERFLOW,
                            DSH_PE_WORD);
                rc = dsh_s6_copy(&n->words[n->argc],
                    &p->tokens[p->pos].text);
                if (rc != DSH_OK)
                        return rc;
                n->literal_mask[n->argc] =
                    p->tokens[p->pos].literal_mask;
                n->quote_mask[n->argc] =
                    p->tokens[p->pos].quote_mask;
                n->argc++;
                p->pos++;
        }
        if (n->argc == 0U)
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_COMMAND);
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_if(struct dsh_parser *p, unsigned int *out)
{
        unsigned int node;
        unsigned int cond;
        unsigned int then_part;
        unsigned int else_part;
        int rc;

        p->pos++;
        rc = dsh_parse_list(p, DSH_STOP_THEN, &cond);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_THEN);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_parse_list(p, DSH_STOP_ELSE | DSH_STOP_FI, &then_part);
        if (rc != DSH_OK)
                return rc;
        else_part = DSH_NONE;
        if (dsh_accept_word(p, DSH_K_ELSE)) {
                rc = dsh_parse_list(p, DSH_STOP_FI, &else_part);
                if (rc != DSH_OK)
                        return rc;
        }
        rc = dsh_expect_word(p, DSH_K_FI);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_alloc_node(p, DSH_N_IF, &node);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].left = cond;
        p->nodes[node].right = then_part;
        p->nodes[node].extra = else_part;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_for(struct dsh_parser *p, unsigned int *out)
{
        unsigned int node;
        unsigned int body;
        struct dsh_node *n;
        int rc;

        p->pos++;
        if (p->pos >= p->ntokens ||
            DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_WORD);
        rc = dsh_alloc_node(p, DSH_N_FOR, &node);
        if (rc != DSH_OK)
                return rc;
        n = &p->nodes[node];
        rc = dsh_s6_copy(&n->words[0], &p->tokens[p->pos].text);
        if (rc != DSH_OK)
                return rc;
        n->literal_mask[0] = p->tokens[p->pos].literal_mask;
        n->quote_mask[0] = p->tokens[p->pos].quote_mask;
        n->argc = 1U;
        p->pos++;
        rc = dsh_expect_word(p, DSH_K_IN);
        if (rc != DSH_OK)
                return rc;
        while (p->pos < p->ntokens && dsh_tok_kw(&p->tokens[p->pos]) != DSH_K_DO) {
                if (DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                        return dsh_mark_error(p, DSH_E_SYNTAX,
                            DSH_PE_WORD);
                if (n->argc >= DSH_PARSE_MAX_WORDS)
                        return dsh_mark_error(p, DSH_E_OVERFLOW,
                            DSH_PE_WORD);
                rc = dsh_s6_copy(&n->words[n->argc],
                    &p->tokens[p->pos].text);
                if (rc != DSH_OK)
                        return rc;
                n->literal_mask[n->argc] =
                    p->tokens[p->pos].literal_mask;
                n->quote_mask[n->argc] =
                    p->tokens[p->pos].quote_mask;
                n->argc++;
                p->pos++;
        }
        rc = dsh_expect_word(p, DSH_K_DO);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_parse_list(p, DSH_STOP_DONE, &body);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_DONE);
        if (rc != DSH_OK)
                return rc;
        n->left = body;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_loop(struct dsh_parser *p, unsigned int *out, int type)
{
        unsigned int node;
        unsigned int cond;
        unsigned int body;
        int rc;

        p->pos++;
        rc = dsh_parse_list(p, DSH_STOP_DO, &cond);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_DO);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_parse_list(p, DSH_STOP_DONE, &body);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_DONE);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_alloc_node(p, type, &node);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].left = cond;
        p->nodes[node].right = body;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_def(struct dsh_parser *p, unsigned int *out)
{
        unsigned int node;
        unsigned int body;
        int rc;

        p->pos++;
        if (p->pos >= p->ntokens ||
            DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_WORD);
        rc = dsh_alloc_node(p, DSH_N_DEF, &node);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_s6_copy(&p->nodes[node].words[0], &p->tokens[p->pos].text);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].literal_mask[0] = p->tokens[p->pos].literal_mask;
        p->nodes[node].quote_mask[0] = p->tokens[p->pos].quote_mask;
        p->nodes[node].argc = 1U;
        p->pos++;
        rc = dsh_expect_word(p, DSH_K_DO);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_parse_list(p, DSH_STOP_DONE, &body);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_DONE);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].left = body;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_case(struct dsh_parser *p, unsigned int *out)
{
        unsigned int node;
        unsigned int arm;
        unsigned int body;
        unsigned int first_arm;
        unsigned int prev_arm;
        struct dsh_node *a;
        int rc;

        p->pos++;
        if (p->pos >= p->ntokens ||
            DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD)
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_WORD);
        rc = dsh_alloc_node(p, DSH_N_CASE, &node);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_s6_copy(&p->nodes[node].words[0], &p->tokens[p->pos].text);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].literal_mask[0] = p->tokens[p->pos].literal_mask;
        p->nodes[node].quote_mask[0] = p->tokens[p->pos].quote_mask;
        p->nodes[node].argc = 1U;
        p->pos++;
        rc = dsh_expect_word(p, DSH_K_IN);
        if (rc != DSH_OK)
                return rc;
        first_arm = DSH_NONE;
        prev_arm = DSH_NONE;
        while (p->pos < p->ntokens &&
            dsh_tok_kw(&p->tokens[p->pos]) != DSH_K_ESAC) {
                if (!dsh_accept_word(p, DSH_K_WHEN))
                        return dsh_mark_error(p, DSH_E_SYNTAX,
                            DSH_PE_WORD);
                rc = dsh_alloc_node(p, DSH_N_CASE_ARM, &arm);
                if (rc != DSH_OK)
                        return rc;
                a = &p->nodes[arm];
                while (p->pos < p->ntokens &&
                    dsh_tok_kw(&p->tokens[p->pos]) != DSH_K_DO) {
                        if (DSH_TOKEN_TYPE(&p->tokens[p->pos]) != DSH_T_WORD ||
                            a->argc >= DSH_PARSE_MAX_WORDS)
                                return dsh_mark_error(p, DSH_E_SYNTAX,
                                    DSH_PE_WORD);
                        rc = dsh_s6_copy(&a->words[a->argc],
                            &p->tokens[p->pos].text);
                        if (rc != DSH_OK)
                                return rc;
                        a->literal_mask[a->argc] =
                            p->tokens[p->pos].literal_mask;
                        a->quote_mask[a->argc] =
                            p->tokens[p->pos].quote_mask;
                        a->argc++;
                        p->pos++;
                }
                if (a->argc == 0U)
                        return dsh_mark_error(p, DSH_E_SYNTAX,
                            DSH_PE_WORD);
                rc = dsh_expect_word(p, DSH_K_DO);
                if (rc != DSH_OK)
                        return rc;
                rc = dsh_parse_list(p, DSH_STOP_WHEN | DSH_STOP_ESAC,
                    &body);
                if (rc != DSH_OK)
                        return rc;
                a->left = body;
                if (first_arm == DSH_NONE)
                        first_arm = arm;
                if (prev_arm != DSH_NONE)
                        p->nodes[prev_arm].right = arm;
                prev_arm = arm;
        }
        if (first_arm == DSH_NONE)
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_WORD);
        rc = dsh_expect_word(p, DSH_K_ESAC);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].left = first_arm;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_group(struct dsh_parser *p, unsigned int *out)
{
        unsigned int body;
        unsigned int node;
        int rc;

        p->pos++;
        rc = dsh_parse_list(p, DSH_STOP_END, &body);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_expect_word(p, DSH_K_END);
        if (rc != DSH_OK)
                return rc;
        rc = dsh_alloc_node(p, DSH_N_GROUP, &node);
        if (rc != DSH_OK)
                return rc;
        p->nodes[node].left = body;
        *out = node;
        return DSH_OK;
}

static int
dsh_parse_paren_group(struct dsh_parser *p, unsigned int *out)
{
        int rc;

        p->pos++;
        rc = dsh_parse_list(p, DSH_STOP_RPAREN, out);
        if (rc != DSH_OK)
                return rc;
        return dsh_expect_token(p, DSH_T_RPAREN, DSH_PE_RPAREN);
}

static int
dsh_parse_command(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        const struct dsh_token *t;

        if (p->pos >= p->ntokens || dsh_at_stop(p, stops))
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_COMMAND);
        t = &p->tokens[p->pos];
        if (dsh_tok_kw(t) == DSH_K_IF)
                return dsh_parse_if(p, out);
        if (dsh_tok_kw(t) == DSH_K_FOR)
                return dsh_parse_for(p, out);
        if (dsh_tok_kw(t) == DSH_K_WHILE)
                return dsh_parse_loop(p, out, DSH_N_WHILE);
        if (dsh_tok_kw(t) == DSH_K_UNTIL)
                return dsh_parse_loop(p, out, DSH_N_UNTIL);
        if (dsh_tok_kw(t) == DSH_K_DEF)
                return dsh_parse_def(p, out);
        if (dsh_tok_kw(t) == DSH_K_CASE)
                return dsh_parse_case(p, out);
        if (dsh_tok_kw(t) == DSH_K_BEGIN)
                return dsh_parse_group(p, out);
        if (DSH_TOKEN_TYPE(t) == DSH_T_LPAREN)
                return dsh_parse_paren_group(p, out);
        return dsh_parse_simple(p, stops, out);
}

static int
dsh_parse_pipeline(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        unsigned int left;
        unsigned int right;
        unsigned int node;
        int rc;

        rc = dsh_parse_command(p, stops, &left);
        if (rc != DSH_OK)
                return rc;
        while (p->pos < p->ntokens &&
            DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_PIPE) {
                p->pos++;
                rc = dsh_parse_command(p, stops, &right);
                if (rc != DSH_OK)
                        return rc;
                rc = dsh_alloc_node(p, DSH_N_PIPE, &node);
                if (rc != DSH_OK)
                        return rc;
                p->nodes[node].left = left;
                p->nodes[node].right = right;
                left = node;
        }
        *out = left;
        return DSH_OK;
}

static int
dsh_parse_notpipe(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        unsigned int child;
        unsigned int node;
        int rc;

        if (p->pos < p->ntokens && dsh_tok_kw(&p->tokens[p->pos]) == DSH_K_NOT) {
                p->pos++;
                rc = dsh_parse_pipeline(p, stops, &child);
                if (rc != DSH_OK)
                        return rc;
                rc = dsh_alloc_node(p, DSH_N_NOT, &node);
                if (rc != DSH_OK)
                        return rc;
                p->nodes[node].left = child;
                *out = node;
                return DSH_OK;
        }
        return dsh_parse_pipeline(p, stops, out);
}

static int
dsh_parse_andor(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        unsigned int left;
        unsigned int right;
        unsigned int node;
        int type;
        int rc;

        rc = dsh_parse_notpipe(p, stops, &left);
        if (rc != DSH_OK)
                return rc;
        while (p->pos < p->ntokens) {
                if (dsh_at_stop(p, stops))
                        break;
                if (dsh_tok_kw(&p->tokens[p->pos]) == DSH_K_AND)
                        type = DSH_N_AND;
                else if (dsh_tok_kw(&p->tokens[p->pos]) == DSH_K_OR)
                        type = DSH_N_OR;
                else
                        break;
                p->pos++;
                rc = dsh_parse_notpipe(p, stops, &right);
                if (rc != DSH_OK)
                        return rc;
                rc = dsh_alloc_node(p, type, &node);
                if (rc != DSH_OK)
                        return rc;
                p->nodes[node].left = left;
                p->nodes[node].right = right;
                left = node;
        }
        *out = left;
        return DSH_OK;
}

static int
dsh_parse_list(struct dsh_parser *p, unsigned int stops, unsigned int *out)
{
        unsigned int result;
        unsigned int current;
        unsigned int node;
        int rc;

        while (p->pos < p->ntokens &&
            (DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_SEMI ||
             DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_BG))
                p->pos++;
        if (p->pos >= p->ntokens || dsh_at_stop(p, stops))
                return dsh_mark_error(p, DSH_E_SYNTAX, DSH_PE_COMMAND);
        rc = dsh_parse_andor(p, stops, &current);
        if (rc != DSH_OK)
                return rc;
        result = DSH_NONE;
        while (p->pos < p->ntokens &&
            (DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_SEMI ||
             DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_BG)) {
                int sep;

                sep = DSH_TOKEN_TYPE(&p->tokens[p->pos]);
                p->pos++;
                if (sep == DSH_T_BG) {
                        rc = dsh_alloc_node(p, DSH_N_BG, &node);
                        if (rc != DSH_OK)
                                return rc;
                        p->nodes[node].left = current;
                        current = node;
                }
                if (result == DSH_NONE) {
                        result = current;
                } else {
                        rc = dsh_alloc_node(p, DSH_N_LIST, &node);
                        if (rc != DSH_OK)
                                return rc;
                        p->nodes[node].left = result;
                        p->nodes[node].right = current;
                        result = node;
                }
                while (p->pos < p->ntokens &&
                    (DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_SEMI ||
                     DSH_TOKEN_TYPE(&p->tokens[p->pos]) == DSH_T_BG))
                        p->pos++;
                if (p->pos >= p->ntokens || dsh_at_stop(p, stops))
                        break;
                rc = dsh_parse_andor(p, stops, &current);
                if (rc != DSH_OK)
                        return rc;
        }
        if (p->pos >= p->ntokens || dsh_at_stop(p, stops)) {
                if (result == DSH_NONE) {
                        result = current;
                } else if (result != current) {
                        /* A trailing separator already committed current. */
                        if (p->nodes[result].type != DSH_N_LIST ||
                            p->nodes[result].right != current) {
                                rc = dsh_alloc_node(p, DSH_N_LIST, &node);
                                if (rc != DSH_OK)
                                        return rc;
                                p->nodes[node].left = result;
                                p->nodes[node].right = current;
                                result = node;
                        }
                }
        }
        *out = result;
        return DSH_OK;
}

int
dsh_parse_tokens_diag(const struct dsh_token *tokens, unsigned int ntokens, struct dsh_node *nodes, unsigned int max_nodes, unsigned int *root, unsigned int *used, struct dsh_parse_diag *diag)
{
        struct dsh_parser p;
        unsigned int i;
        int rc;

        if (tokens == 0 || nodes == 0 || root == 0)
                return DSH_E_ARG;
        dsh_parse_diag_clear(diag);
        for (i = 0; i < max_nodes; i++)
                dsh_node_clear(&nodes[i]);
        p.tokens = tokens;
        p.ntokens = ntokens;
        p.pos = 0;
        p.nodes = nodes;
        p.max_nodes = max_nodes;
        p.used = 0;
        p.diag = diag;
        if (ntokens == 0U) {
                rc = dsh_alloc_node(&p, DSH_N_EMPTY, root);
                if (rc != DSH_OK)
                        return rc;
                if (used != 0)
                        *used = p.used;
                return DSH_OK;
        }
        rc = dsh_parse_list(&p, 0U, root);
        if (rc != DSH_OK)
                return rc;
        if (p.pos != ntokens)
                return dsh_mark_error(&p, DSH_E_SYNTAX, DSH_PE_NONE);
        if (used != 0)
                *used = p.used;
        return DSH_OK;
}

int
dsh_parse_tokens(const struct dsh_token *tokens, unsigned int ntokens, struct dsh_node *nodes, unsigned int max_nodes, unsigned int *root, unsigned int *used)
{
        return dsh_parse_tokens_diag(tokens, ntokens, nodes, max_nodes,
            root, used, 0);
}

int
dsh_parse_s6_line_diag(const struct dsh_line *line, struct dsh_node *nodes, unsigned int max_nodes, unsigned int *root, unsigned int *used, int *errpos, struct dsh_parse_diag *diag)
{
        struct dsh_token tokens[DSH_LEX_MAX_TOKENS];
        unsigned int ntokens;
        int rc;

        rc = dsh_lex_s6_line(line, tokens, DSH_LEX_MAX_TOKENS,
            &ntokens, errpos);
        if (rc != DSH_OK) {
                if (diag != 0) {
                        diag->code = rc;
                        diag->token = 0;
                        diag->expect = DSH_PE_NONE;
                }
                return rc;
        }
        return dsh_parse_tokens_diag(tokens, ntokens, nodes, max_nodes,
            root, used, diag);
}

int
dsh_parse_s6_line(const struct dsh_line *line, struct dsh_node *nodes, unsigned int max_nodes, unsigned int *root, unsigned int *used, int *errpos)
{
        return dsh_parse_s6_line_diag(line, nodes, max_nodes, root, used,
            errpos, 0);
}
