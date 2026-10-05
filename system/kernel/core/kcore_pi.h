/**
 * @file kcore_pi.h
 * @brief C-visible constants and resident state for PDP-6 priority interrupts.
 *
 * The PDP-6 exposes seven hardware PI levels.  KCORE owns the resident
 * dispatcher; MINIT patches each level entry directly to its first handler and
 * links any same-level continuation through a handler-owned tail word. Runtime
 * movable-module relocation is deliberately deferred; a future loader must
 * rebuild those PI chains with interrupts disabled after moving a module.
 */
#ifndef DAIMON_KCORE_PI_H
#define DAIMON_KCORE_PI_H

#include "kcore.h"

/** Number of architectural PDP-6 priority-interrupt levels. */
#define PDP10_PI_LEVELS              7U

/** Lowest valid architectural PI level number. */
#define PDP10_PI_LEVEL_MIN           1U

/** Highest valid architectural PI level number. */
#define PDP10_PI_LEVEL_MAX           7U

/** Convert architectural PI level 1..7 to its CONO/CONI enable-mask bit. */
#define PDP10_PI_MASK(level)         (0200U >> (level))

/** Patchable level-entry jumps populated by MINIT. */
extern kword_t pdp10_pi_level1_dispatch_jump;
extern kword_t pdp10_pi_level2_dispatch_jump;
extern kword_t pdp10_pi_level3_dispatch_jump;
extern kword_t pdp10_pi_level4_dispatch_jump;
extern kword_t pdp10_pi_level5_dispatch_jump;
extern kword_t pdp10_pi_level6_dispatch_jump;
extern kword_t pdp10_pi_level7_dispatch_jump;
/** Common completed-chain restore path. */
extern kword_t pdp10_pi_dispatch_done;
/** Level-7 return stub used by the optional DPY PI7 pre-handler. */
extern kword_t pdp10_pi_return_level7;

#endif
