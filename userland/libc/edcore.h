#ifndef DAIMOS_EDCORE_H
#define DAIMOS_EDCORE_H

#include "kcore.h"

/* A borrowed, bounded line buffer in PDP-6 packed SIXBIT format.
 * Only characters 040 through 0137 (ASCII) are represented.  No storage,
 * terminal access, history or allocation is owned by this primitive.
 * The same core can serve DSH, ED and EX command frontends. */
struct edcore_line {
        kword_t *words;
        unsigned int length;
        unsigned int capacity;
};

int edcore_get(const struct edcore_line *line, unsigned int pos);
int edcore_insert(struct edcore_line *line, unsigned int pos, int ch);
int edcore_delete(struct edcore_line *line, unsigned int pos);
int edcore_replace(struct edcore_line *line, unsigned int pos, int ch);

#endif
