#ifndef DAIMON_MODULE_RUNTIME_H
#define DAIMON_MODULE_RUNTIME_H

#include "kcore.h"

#define MODULE_RUNTIME_MAX       19U
#define MODULE_DYNAMIC_BIND_MAX  8U
#define MODULE_HALF_MASK         0777777UL

/* One word per module: LH initialized words, RH current physical base.
 * Boot MRES extents are pinned and do not retain relocation maps.  A future
 * movable module must retain its map after the image; MODULE_RUNTIME_MAP_WORDS
 * describes that movable-module representation. */
extern kword_t module_runtime_descs[MODULE_RUNTIME_MAX + 1U];
extern kword_t module_dynamic_bindings[MODULE_DYNAMIC_BIND_MAX];
#define module_moves_enabled module_runtime_descs[0]

int module_runtime_move(unsigned int owner, unsigned int new_base,
    unsigned int total_words);

#define MODULE_RUNTIME_BASE(d) ((d) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_INIT_WORDS(d) (((d) >> 18U) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_MAP_WORDS(d) \
        ((MODULE_RUNTIME_INIT_WORDS(d) + 17U) / 18U)

#endif
