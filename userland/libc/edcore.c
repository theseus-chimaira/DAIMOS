#include "edcore.h"

/* Six SIXBIT characters per 36-bit word.  Word storage is provided by the
 * caller, allowing fixed-size DSH input and a future dynamic ED line store
 * to use identical editing semantics. */
int
edcore_get(const struct edcore_line *line, unsigned int pos)
{
        unsigned int sh;

        if (line == 0 || line->words == 0 || pos >= line->length)
                return -1;
        sh = 30U - (pos % 6U) * 6U;
        return (int)((line->words[pos / 6U] >> sh) & 077UL) + 040;
}

static void
edcore_put(struct edcore_line *line, unsigned int pos, int ch)
{
        unsigned int sh;
        kword_t mask;

        sh = 30U - (pos % 6U) * 6U;
        mask = (kword_t)077UL << sh;
        line->words[pos / 6U] = (line->words[pos / 6U] & ~mask) |
            ((kword_t)(ch - 040) << sh);
}

int
edcore_insert(struct edcore_line *line, unsigned int pos, int ch)
{
        unsigned int i;

        if (line == 0 || line->words == 0 || pos > line->length ||
            line->length >= line->capacity || ch < 040 || ch > 0137)
                return -1;
        /* Work backward so the source is never overwritten before use. */
        for (i = line->length; i > pos; --i)
                edcore_put(line, i, edcore_get(line, i - 1U));
        edcore_put(line, pos, ch);
        ++line->length;
        return 0;
}

int
edcore_delete(struct edcore_line *line, unsigned int pos)
{
        unsigned int i;

        if (line == 0 || line->words == 0 || pos >= line->length)
                return -1;
        for (i = pos + 1U; i < line->length; ++i)
                edcore_put(line, i - 1U, edcore_get(line, i));
        --line->length;
        edcore_put(line, line->length, 040);
        return 0;
}

int
edcore_replace(struct edcore_line *line, unsigned int pos, int ch)
{
        if (line == 0 || line->words == 0 || pos >= line->length ||
            ch < 040 || ch > 0137)
                return -1;
        edcore_put(line, pos, ch);
        return 0;
}
