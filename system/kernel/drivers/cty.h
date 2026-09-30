/**
 * @file cty.h
 * @brief PDP-6 console teletype hardware and resident service interface.
 *
 * CTY is the primary console terminal at device 0120. KINIT probes the device,
 * installs its MRES package, registers PI level 4, and then publishes the
 * resident getchar/putchar services to the generic TTY layer. The installed
 * package remains resident because it owns asynchronous input/output state and
 * is also the operator escape path for real-time scheduling.
 *
 * Input is interrupt driven and blocks through the process event mechanism;
 * output is synchronous to the caller but completion is acknowledged by PI4.
 * All characters are reduced to the PDP-6 console's 7-bit data path.
 */
#ifndef DAIMON_CTY_H
#define DAIMON_CTY_H

#include "kcore.h"

/** PDP-6 console teletype I/O device number. */
#define CTY_DEVICE              0120U
/** CONI bit 32: output character completed and transmitter ready. */
#define CTY_ST_OUTPUT_READY     0010UL
/** CONI bit 31: transmitter currently busy. */
#define CTY_ST_OUTPUT_BUSY      0020UL
/** CONI bit 30: one input character is ready for DATAI. */
#define CTY_ST_INPUT_READY      0040UL
/** CONI bit 29: keyboard/input side busy. */
#define CTY_ST_INPUT_BUSY       0100UL
/** CONI/CONO bits 33..35: assigned PI level. */
#define CTY_ST_PI_MASK          0007UL
/** CONO bit 28: clear OUTPUT READY (CONI bit 32). */
#define CTY_CO_CLR_OUTPUT_READY (CTY_ST_OUTPUT_READY << 4)
/** CONO bit 26: clear INPUT READY (CONI bit 30). */
#define CTY_CO_CLR_INPUT_READY  (CTY_ST_INPUT_READY << 4)
/** Architectural PI level used by CTY. */
#define CTY_NATIVE_PI_LEVEL     4U
/** Bounded polling count used by synchronous output waits. */
#define CTY_WAIT_READY          0200000U

/** Successful CTY operation. */
#define CTY_E_OK                0
/** Console hardware did not make progress before the bounded wait expired. */
#define CTY_E_TIMEOUT          -2
/** Another output operation already owns the transmitter. */
#define CTY_E_BUSY             -3

/**
 * @brief Write one character to the physical console.
 * @param c Character value; only the low seven bits are transmitted.
 * @return CTY_E_OK, CTY_E_BUSY, or CTY_E_TIMEOUT.
 */
int cty_putchar(int c);

/**
 * @brief Read one seven-bit console character, sleeping if necessary.
 * @return Character value 0..0177, or an interrupt/error status propagated by
 *         the process event wait path.
 */
int cty_getchar(void);

/** @brief Resident PI4 entry for CTY input and output completion. */
void cty_pi_handler(void);


#endif
