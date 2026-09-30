/**
 * @file lpt.h
 * @brief PDP-6 LP10-compatible line-printer service interface.
 *
 * LPT device 0124 is a small synchronous permanent-KCORE driver rather than an
 * optional MRES package. KINIT primes DONE through CONO, verifies the device,
 * and publishes lpt_putchar() directly. The runtime path deliberately polls:
 * there is no resident PI state, queue, or buffer in DAIMOS.
 */
#ifndef DAIMON_LPT_H
#define DAIMON_LPT_H

#include "kcore.h"

/** LP10-compatible line-printer I/O device number. */
#define LPT_DEVICE         0124U
/** CONI/CONO bit 29: DATAO accepted/completed; software-settable for probe. */
#define LPT_ST_DONE        0000100UL
/** CONI bit 28: printer hardware is processing a DATAO/clear operation. */
#define LPT_ST_BUSY        0000200UL
/** CONI bit 27: printer error or unattached output medium. */
#define LPT_ST_ERROR       0000400UL
/** CONO bit 26: controller clear/form-feed style operation. */
#define LPT_CO_CLEAR       0002000UL
/** Bounded synchronous polling count for ready and completion waits. */
#define LPT_WAIT_READY     0200000U

/**
 * @brief Write one seven-bit character to the physical line printer.
 * @param c Character value; only the low seven bits are transmitted.
 * @return 0 on success, -2 on timeout, or -4 on printer error.
 *
 * The LP10 DATAO format holds five seven-bit characters. DAIMOS places one
 * character in the first slot and leaves the remaining slots zero, which the
 * hardware ignores.
 */
int lpt_putchar(int c);

#endif
