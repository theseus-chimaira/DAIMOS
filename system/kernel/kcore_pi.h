#ifndef DAIMON_KCORE_PI_H
#define DAIMON_KCORE_PI_H

#include "kcore.h"

#define PDP10_PI_LEVELS              7U
#define PDP10_PI_LEVEL_MIN           1U
#define PDP10_PI_LEVEL_MAX           7U
#define PDP10_PI_HANDLER_CAPACITY    8U
#define PDP10_PI_LOW_SPAN_COUNT       3U
#define PDP10_PI_RESIDENT_SPAN_COUNT  (PDP10_PI_LEVELS - PDP10_PI_LOW_SPAN_COUNT)
#define PDP10_PI_MASK(level)         (0200U >> (level))

/* Compact resident dispatch state.  MINIT builds these tables once.
 * PI1..PI3 span words live in reusable low core 037, 040, and 041. */
extern kword_t pdp10_pi_handlers[PDP10_PI_HANDLER_CAPACITY];
extern kword_t pdp10_pi_level_span[PDP10_PI_RESIDENT_SPAN_COUNT];

#endif
