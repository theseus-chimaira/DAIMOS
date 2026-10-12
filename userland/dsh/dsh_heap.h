#ifndef DAIMOS_DSH_HEAP_H
#define DAIMOS_DSH_HEAP_H
#include "u.h"
kword_t *dsh_heap_alloc(unsigned int words);
void dsh_heap_free(kword_t *payload);
#endif
