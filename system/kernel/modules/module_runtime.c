/**
 * @file module_runtime.c
 * @brief Shared state for deferred runtime movable-module relocation.
 *
 * This file defines the descriptor/binding state consumed by the compact
 * PDP-6 relocation primitive in module_runtime_pdp6.s. The facility is not
 * currently wired into MM compaction; it remains groundwork for the deferred
 * post-overlay movable-module stage.
 */
#include "module_runtime.h"
#include "fs_mres.h"
#include "kcore_pi.h"
#include "mres_reloc.h"

#define MODULE_BIND_OWNER_SHIFT 18U

kword_t module_runtime_descs[MODULE_RUNTIME_MAX + 1U];
kword_t module_dynamic_bindings[MODULE_DYNAMIC_BIND_MAX];
