/**
 * @file pclk.h
 * @brief Stanford Phil Petit calendar-clock TIME36 interface.
 *
 * PCLK is the PDP-6 wall-clock source at I/O device 0730. It is separate from
 * the APR 60 Hz monotonic clock: scheduler accounting, sleeps, and timeouts use
 * CLK, while PCLK is read on demand for civil UTC such as filesystem mtimes
 * and GETTIME. The reader is permanent KCORE code, keeps no mutable state, and
 * allocates no buffers.
 */
#ifndef DAIMOS_PCLK_H
#define DAIMOS_PCLK_H

#include "kcore.h"

/**
 * @brief Packed 36-bit UTC representation returned by pclk_time36().
 *
 * DAIMOS TIME36 is a chronologically sortable packed UTC timestamp:
 *
 *   bit 35       always zero
 *   bit 34       0 = 20xx, 1 = 21xx
 *   bits 33..26  two BCD year digits
 *   bits 25..22  month 1..12
 *   bits 21..17  day 1..31
 *   bits 16..12  hour 0..23
 *   bits 11..6   minute 0..59
 *   bits 5..0    second 0..59
 *
 * Supported dates are 2026-01-01 through 2125-12-31; PCLK year values
 * 26..99 mean 2026..2099 and 00..25 mean 2100..2125.
 */
#define PCLK_TIME_CENTURY_21      0200000000000UL
#define PCLK_TIME_YEAR_BCD_SHIFT  26U
#define PCLK_TIME_MONTH_SHIFT     22U
#define PCLK_TIME_DAY_SHIFT       17U
#define PCLK_TIME_HOUR_SHIFT      12U
#define PCLK_TIME_MINUTE_SHIFT    6U
#define PCLK_TIME_SECOND_SHIFT    0U
#define PCLK_TIME_YEAR_BCD_MASK   0377UL
#define PCLK_TIME_MONTH_MASK      017UL
#define PCLK_TIME_DAY_MASK        037UL
#define PCLK_TIME_HOUR_MASK       037UL
#define PCLK_TIME_MINUTE_MASK     077UL
#define PCLK_TIME_SECOND_MASK     077UL

/**
 * @brief Read a coherent PCLK UTC timestamp and convert it to TIME36.
 * @return Packed TIME36 value for the current supported UTC date and time.
 *
 * The routine retries when the two DATAI samples surrounding CONI differ, so
 * fields from different minutes can never be combined.
 */
kword_t pclk_time36(void);

#endif
