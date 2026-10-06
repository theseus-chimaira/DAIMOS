/**
 * @file clk.h
 * @brief PDP-6 APR 60 Hz line-clock interface and register definitions.
 *
 * CLK is the resident monotonic clock used for scheduler quanta, sleeps,
 * storage timeouts, and elapsed-time accounting.  KINIT probes the APR line
 * clock, installs the CLK MRES package when the clock is usable, registers its
 * PI level 6 handler, and then re-enables the hardware.  The installed MRES
 * remains resident after KINIT is reclaimed because interrupt delivery and
 * the tick counter are needed for the lifetime of the kernel.
 *
 * The line clock is deliberately separate from the Stanford Petit PCLK wall
 * clock.  PCLK supplies civil UTC on demand; CLK is the monotonic 60 Hz source
 * and must not depend on calendar time.  APR and the line clock share the APR
 * PIA, and the Type 340 display may also share PI level 6, so the resident
 * service qualifies the APR clock flag before claiming a tick.
 */
#ifndef DAIMON_CLK_H
#define DAIMON_CLK_H

#include "kcore_pi.h"

/** Architectural PI level used by the APR line clock. */
#define CLK_NATIVE_PI_LEVEL     6U

/** Nominal APR line-clock frequency in ticks per second. */
#define CLK_HZ                  60U

/** APR PIA field: PDP-10 bits 33..35, containing PI level 0..7. */
#define CLK_APR_PIA_MASK        0000007UL

/** APR CONI clock flag: PDP-10 bit 26 (octal 001000). */
#define CLK_APR_ST_FLAG         0001000UL

/** APR CONI clock-enable state: PDP-10 bit 25 (octal 002000). */
#define CLK_APR_ST_ENABLE       0002000UL

/** APR CONO command to clear the line-clock flag: bit 26. */
#define CLK_APR_CO_CLEAR_FLAG   0001000UL

/** APR CONO command to enable the line clock: bit 25. */
#define CLK_APR_CO_ENABLE       0002000UL

/** APR CONO command to disable the line clock: bit 24 (octal 004000). */
#define CLK_APR_CO_DISABLE      0004000UL

/** APR CONI non-existent-memory condition: bit 23. */
#define CLK_APR_ST_NXM          0010000UL

/** APR CONI illegal/protected-address condition: bit 22. */
#define CLK_APR_ST_PROTECT      0020000UL

/** APR CONI pushdown-list-overflow condition: bit 19. */
#define CLK_APR_ST_PDL_OV       0200000UL

/** APR CONO command to clear non-existent-memory: bit 23. */
#define CLK_APR_CO_CLEAR_NXM    0010000UL

/** APR CONO command to clear illegal/protected-address: bit 22. */
#define CLK_APR_CO_CLEAR_PROTECT 0020000UL

/** APR CONO command to clear pushdown-list overflow: bit 18. */
#define CLK_APR_CO_CLEAR_PDL_OV 0400000UL

#define CLK_APR_ST_FAULTS \
        (CLK_APR_ST_NXM | CLK_APR_ST_PROTECT | CLK_APR_ST_PDL_OV)
#define CLK_APR_CO_CLEAR_FAULTS \
        (CLK_APR_CO_CLEAR_NXM | CLK_APR_CO_CLEAR_PROTECT | \
        CLK_APR_CO_CLEAR_PDL_OV)

/**
 * @brief Return the resident monotonic line-clock tick count.
 *
 * @return Number of qualified APR line-clock interrupts since CLK MRES was
 *         installed.  The counter is a native 36-bit word and wraps naturally.
 */
unsigned int clk_ticks(void);

/**
 * @brief PI level 6 entry for the resident APR line-clock service.
 *
 * The generic PI dispatcher saves the interrupted AC1..AC3 and AC17 state.
 * This entry qualifies and services the clock through clk_pi_service(), then
 * transfers to the common PI-handler continuation rather than returning as a
 * normal C function.
 */
void clk_pi_handler(void);


#endif
