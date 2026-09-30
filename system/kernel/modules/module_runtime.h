/**
 * @file module_runtime.h
 * @brief Deferred runtime movable-module relocation interface.
 *
 * Current boot MRES packages are permanent and pinned after installation; they
 * discard their relocation maps. A future movable runtime module retains its
 * two-bit relocation map after the image so software can rebase it when MM
 * moves the extent. Current MM compaction moves processes only, so this is
 * groundwork for the post-overlay runtime-module stage and is not active in
 * today's permanent KCORE.
 */
#ifndef DAIMON_MODULE_RUNTIME_H
#define DAIMON_MODULE_RUNTIME_H

#include "kcore.h"

#define MODULE_RUNTIME_MAX       20U
#define MODULE_DYNAMIC_BIND_MAX  8U
#define MODULE_HALF_MASK         0777777UL

/** One word per module: LH initialized words, RH current physical base.
 * Boot MRES extents are pinned and do not retain relocation maps.  A future
 * movable module must retain its map after the image; MODULE_RUNTIME_MAP_WORDS
 * describes that movable-module representation. */
extern kword_t module_runtime_descs[MODULE_RUNTIME_MAX + 1U];
/** Binding locations whose referenced target may need retargeting after a move. */
extern kword_t module_dynamic_bindings[MODULE_DYNAMIC_BIND_MAX];
#define module_moves_enabled module_runtime_descs[0]

/**
 * Move and software-relocate one already validated movable module.
 *
 * The future MM caller owns owner/range/state validation and must keep PI
 * disabled across copy, relocation, binding retargeting, descriptor rebasing,
 * and publication. total_words includes the retained relocation map.
 */
int module_runtime_move(unsigned int owner, unsigned int new_base,
    unsigned int total_words);

#define MODULE_RUNTIME_BASE(d) ((d) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_INIT_WORDS(d) (((d) >> 18U) & MODULE_HALF_MASK)
#define MODULE_RUNTIME_MAP_WORDS(d) \
        ((MODULE_RUNTIME_INIT_WORDS(d) + 17U) / 18U)

#endif
