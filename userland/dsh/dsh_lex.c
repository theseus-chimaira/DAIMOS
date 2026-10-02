#include "dsh_lex.h"

static int
dsh_space(int ch)
{
        return ch == ' ';
}

static int
dsh_line_ch(const struct dsh_line *line, unsigned int pos)
{
        if (line == 0 || pos >= line->len)
                return 0;
        return dsh_line_get(line, pos);
}

static int
dsh_word_stop(int ch)
{
        return ch == 0 || dsh_space(ch) || ch == ';' || ch == '&' ||
            ch == '!' || ch == '<' || ch == '>' || ch == '(' ||
            ch == ')';
}

void
dsh_token_clear(struct dsh_token *t)
{
        if (t == 0)
                return;
        t->type = DSH_T_EOF;
        dsh_s6_clear(&t->text);
        t->literal_mask = 0;
        t->quote_mask = 0;
}

static int
dsh_add_op(struct dsh_token *tokens, unsigned int max_tokens, unsigned int *ntokens, int type)
{
        if (*ntokens >= max_tokens)
                return DSH_E_OVERFLOW;
        dsh_token_clear(&tokens[*ntokens]);
        tokens[*ntokens].type = type;
        (*ntokens)++;
        return DSH_OK;
}

static int
dsh_add_word(const struct dsh_line *line, unsigned int *posp, struct dsh_token *tokens, unsigned int max_tokens, unsigned int *ntokens)
{
        struct dsh_token *t;
        unsigned int pos;
        unsigned int had_fragment;
        int quote;
        int rc;
        int ch;

        if (*ntokens >= max_tokens)
                return DSH_E_OVERFLOW;
        t = &tokens[*ntokens];
        dsh_token_clear(t);
        t->type = DSH_T_WORD;
        pos = *posp;
        had_fragment = 0U;

        while ((ch = dsh_line_ch(line, pos)) != 0) {
                if (ch == '\'' || ch == '"') {
                        quote = ch;
                        t->type |= DSH_T_QUOTED;
                        had_fragment = 1U;
                        pos++;
                        while ((ch = dsh_line_ch(line, pos)) != 0 &&
                            ch != quote) {
                                if (quote == '"' && ch == '\\') {
                                        int next;

                                        next = dsh_line_ch(line, pos + 1U);
                                        if (next == '"' || next == '\\' ||
                                            next == '$') {
                                                ch = next;
                                                pos += 2U;
                                                t->literal_mask |=
                                                    (kword_t)1 << t->text.len;
                                                t->quote_mask |=
                                                    (kword_t)1 << t->text.len;
                                        } else {
                                                pos++;
                                                t->literal_mask |=
                                                    (kword_t)1 << t->text.len;
                                                t->quote_mask |=
                                                    (kword_t)1 << t->text.len;
                                                rc = dsh_s6_append(&t->text,
                                                    '\\');
                                                if (rc != DSH_OK)
                                                        return rc;
                                                continue;
                                        }
                                } else {
                                        pos++;
                                        t->quote_mask |=
                                            (kword_t)1 << t->text.len;
                                        if (quote == '\'')
                                                t->literal_mask |=
                                                    (kword_t)1 << t->text.len;
                                }
                                rc = dsh_s6_append(&t->text, ch);
                                if (rc != DSH_OK)
                                        return rc;
                        }
                        if (dsh_line_ch(line, pos) != quote)
                                return DSH_E_CHAR;
                        pos++;
                        continue;
                }
                if (ch == '\\') {
                        had_fragment = 1U;
                        pos++;
                        ch = dsh_line_ch(line, pos);
                        if (ch == 0)
                                return DSH_E_CHAR;
                        pos++;
                        t->literal_mask |= (kword_t)1 << t->text.len;
                        t->quote_mask |= (kword_t)1 << t->text.len;
                        rc = dsh_s6_append(&t->text, ch);
                        if (rc != DSH_OK)
                                return rc;
                        continue;
                }
                if (dsh_word_stop(ch))
                        break;
                had_fragment = 1U;
                pos++;
                rc = dsh_s6_append(&t->text, ch);
                if (rc != DSH_OK)
                        return rc;
        }

        if (t->text.len == 0U && !had_fragment)
                return DSH_E_CHAR;
        *posp = pos;
        (*ntokens)++;
        return DSH_OK;
}

int
dsh_lex_s6_line(const struct dsh_line *line, struct dsh_token *tokens, unsigned int max_tokens, unsigned int *ntokens, int *errpos)
{
        unsigned int pos;
        int rc;
        int ch;

        if (line == 0 || tokens == 0 || ntokens == 0)
                return DSH_E_ARG;
        *ntokens = 0;
        if (errpos != 0)
                *errpos = -1;
        pos = 0;

        while ((ch = dsh_line_ch(line, pos)) != 0) {
                if (dsh_space(ch)) {
                        pos++;
                        continue;
                }
                if (ch == '#')
                        break;
                if (ch == '$' && dsh_line_ch(line, pos + 1U) == '(') {
                        rc = dsh_add_op(tokens, max_tokens, ntokens,
                            DSH_T_DOLLAR_LPAREN);
                        if (rc != DSH_OK)
                                goto bad;
                        pos += 2U;
                        continue;
                }
                if (ch == ';' || ch == '&' || ch == '!' || ch == '<' ||
                    ch == '>' || ch == '(' || ch == ')') {
                        if (ch == ';')
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_SEMI);
                        else if (ch == '&')
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_BG);
                        else if (ch == '!')
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_PIPE);
                        else if (ch == '<')
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_REDIR_IN);
                        else if (ch == '>') {
                                if (dsh_line_ch(line, pos + 1U) == '>') {
                                        rc = dsh_add_op(tokens, max_tokens,
                                            ntokens, DSH_T_APPEND);
                                        if (rc == DSH_OK)
                                                pos++;
                                } else {
                                        rc = dsh_add_op(tokens, max_tokens,
                                            ntokens, DSH_T_REDIR_OUT);
                                }
                        } else if (ch == '(')
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_LPAREN);
                        else
                                rc = dsh_add_op(tokens, max_tokens, ntokens,
                                    DSH_T_RPAREN);
                        if (rc != DSH_OK)
                                goto bad;
                        pos++;
                        continue;
                }
                rc = dsh_add_word(line, &pos, tokens, max_tokens, ntokens);
                if (rc != DSH_OK)
                        goto bad;
        }

        return DSH_OK;

bad:
        if (errpos != 0)
                *errpos = (int)pos;
        return rc;
}
