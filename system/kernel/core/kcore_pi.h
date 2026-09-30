/**
 * @file kcore_pi.h
 * @brief C-visible constants and resident state for PDP-6 priority interrupts.
 *
 * The PDP-6 exposes seven hardware PI levels.  KCORE owns the resident
 * dispatcher and handler table; MINIT constructs the compact per-level spans
 * once during boot and runtime module management may later retarget handler
 * addresses without changing the dispatcher ABI.
 *
 * A span word is encoded as -count,,start.  The left half is the negative
 * number of handlers registered for that level and the right half is the
 * starting index in pdp10_pi_handlers[].  A zero span means no handlers.
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

/** Maximum number of resident PI handlers across all levels. */
#define PDP10_PI_HANDLER_CAPACITY    13U

/** Convert architectural PI level 1..7 to its CONO/CONI enable-mask bit. */
#define PDP10_PI_MASK(level)         (0200U >> (level))

/** Compact resident handler-address table, populated by MINIT. */
extern kword_t pdp10_pi_handlers[PDP10_PI_HANDLER_CAPACITY];

/** One packed -count,,start dispatch span for each architectural PI level. */
extern kword_t pdp10_pi_level_span[PDP10_PI_LEVELS];

/** Resident generic PI dispatcher entry used when a level has multiple handlers. */
extern kword_t pdp10_pi_dispatch;

/** Patchable level-entry jumps used to bypass generic dispatch for one handler. */
extern kword_t pdp10_pi_level1_dispatch_jump;
extern kword_t pdp10_pi_level2_dispatch_jump;
extern kword_t pdp10_pi_level3_dispatch_jump;
extern kword_t pdp10_pi_level4_dispatch_jump;
extern kword_t pdp10_pi_level5_dispatch_jump;
extern kword_t pdp10_pi_level6_dispatch_jump;

#endif
