/**
 * @file kfmt.h
 * @brief Resident kernel formatting helpers shared by MonitorFS producers.
 *
 * Kernel-generated pseudo-files should avoid temporary text buffers whenever
 * practical.  The formatter interface therefore exposes a read-one-character
 * operation: callers provide a numeric value and logical character offset,
 * and the resident formatter computes only the requested character.
 */
#ifndef DAIMON_KFMT_H
#define DAIMON_KFMT_H

#include "kcore.h"

/**
 * @brief Read one character from an unsigned 18-bit decimal value plus CR/LF.
 * @param value Unsigned value representable in one PDP-6/PDP-10 18-bit halfword.
 * @param off Zero-based character offset in the generated decimal line.
 * @param chp Receives the 7-bit character when @p off names one.
 * @return 1 when a character is produced, 0 when @p off lies past the final
 *         line feed, and -1 when @p chp is null.
 *
 * Leading zeroes are suppressed except for the value zero itself.  The text
 * representation is the shortest decimal form followed by carriage return
 * and line feed.  No output buffer or persistent formatting state is used.
 */
int kfmt_u18_decimal_readchar(kword_t value, kword_t off, unsigned int *chp);

#endif
