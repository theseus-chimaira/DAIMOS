#ifndef DAIMOS_USER_CMDMAP_H
#define DAIMOS_USER_CMDMAP_H

#include "u.h"

/* Resolve one counted SIXBIT command name through /SYSTEM/EXEC/MAP. */
int u_cmd_resolve(const kword_t *command, kword_t *path,
    unsigned int path_words);

#endif
