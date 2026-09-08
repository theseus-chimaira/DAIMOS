#ifndef DAIMON_MODULE_RUNTIME_H
#define DAIMON_MODULE_RUNTIME_H

#include "kcore.h"

#define MODULE_RUNTIME_MAX       19U
#define MODULE_DYNAMIC_BIND_MAX  8U
#define MODULE_HALF_MASK         0777777UL

struct module_runtime_desc {
        kword_t span;           /* LH initialized+BSS words, RH physical base. */
        kword_t reloc;          /* LH initialized words, RH relocation-map words. */
};

extern struct module_runtime_desc module_runtime_descs[MODULE_RUNTIME_MAX + 1U];
extern kword_t module_dynamic_bindings[MODULE_DYNAMIC_BIND_MAX];
extern unsigned int module_dynamic_binding_count;
extern unsigned int module_moves_enabled;

int module_runtime_move(unsigned int owner, unsigned int new_base);

#define MODULE_RUNTIME_BASE(d) ((d)->span & MODULE_HALF_MASK)
#define MODULE_RUNTIME_IMAGE_WORDS(d) (((d)->span >> 18U) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_INIT_WORDS(d) (((d)->reloc >> 18U) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_MAP_WORDS(d) ((d)->reloc & MODULE_HALF_MASK)
#define MODULE_RUNTIME_EXTENT_WORDS(d) \
        (MODULE_RUNTIME_IMAGE_WORDS(d) + MODULE_RUNTIME_MAP_WORDS(d))

#endif
