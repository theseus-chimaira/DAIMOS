/* Configuration field/key parsing is intentionally separate from S6REC
 * I/O: DSH needs the reader but never uses these config parsers. */
#include "text.h"

static int
u_text_space(char ch)
{
        return ch == ' ' || ch == '\t';
}

static char *
u_text_trim(char *s)
{
        char *end;

        while (u_text_space(*s))
                ++s;
        end = s;
        while (*end != 0)
                ++end;
        while (end != s && u_text_space(end[-1]))
                --end;
        *end = 0;
        return s;
}

int
u_text_fields(char *line, char **field, unsigned int max_fields)
{
        char *p;
        char *start;
        unsigned int n;

        if (line == 0 || field == 0 || max_fields == 0U)
                return -1;
        p = u_text_trim(line);
        if (*p == 0 || *p == '#')
                return 0;

        n = 0U;
        start = p;
        for (;;) {
                if (*p == ':' || *p == 0) {
                        if (n >= max_fields)
                                return -1;
                        if (*p == ':') {
                                *p = 0;
                                field[n++] = u_text_trim(start);
                                start = ++p;
                                continue;
                        }
                        field[n++] = u_text_trim(start);
                        return (int)n;
                }
                ++p;
        }
}

int
u_text_key(char *line, char **key, char **value, unsigned int max_values)
{
        char *p;
        char *rhs;
        unsigned int n;

        if (line == 0 || key == 0 ||
            (max_values != 0U && value == 0))
                return -1;
        p = u_text_trim(line);
        if (*p == 0 || *p == '#')
                return 0;

        rhs = p;
        while (*rhs != 0 && *rhs != '=')
                ++rhs;
        if (*rhs != '=')
                return -1;
        *rhs++ = 0;
        *key = u_text_trim(p);
        if (**key == 0)
                return -1;
        rhs = u_text_trim(rhs);

        n = 0U;
        while (*rhs != 0) {
                if (n >= max_values)
                        return -1;
                value[n++] = rhs;
                while (*rhs != 0 && !u_text_space(*rhs))
                        ++rhs;
                if (*rhs == 0)
                        break;
                *rhs++ = 0;
                while (u_text_space(*rhs))
                        ++rhs;
        }
        return (int)n;
}
