/**
 * @file dcs.h
 * @brief PDP-6 Type 630 Data Communications System interface.
 *
 * The Type 630 DCS provides sixteen asynchronous terminal lines through one
 * shared receiver scanner. DCSA (device 0300) controls the scanner and carries
 * character data; DCSB (0304) reports the stopped receiver line and selects a
 * transmit line. KINIT probes DCSA, installs one resident MRES package, and
 * publishes the input/output services to the generic TTY layer.
 *
 * The resident driver intentionally keeps only one receive word and one event
 * word regardless of line count. If the hardware scanner stops on a line other
 * than the requesting reader, the byte is deferred into that logical TTY's
 * existing process-session record rather than allocating per-line DCS storage.
 */
#ifndef DAIMON_DCS_H
#define DAIMON_DCS_H

#include "kcore.h"

/** Type 630 DCSA device: receiver scanner control and character data. */
#define DCS_DEVICE              0300U
/** Type 630 DCSB device: scanner-line readout and transmit-line selection. */
#define DCSB_DEVICE             0304U
/** Native Type 630 receiver PI level used by DAIMOS. */
#define DCS_NATIVE_PI_LEVEL     2U
/** Number of DCS terminal lines exposed by the PDP-6 configuration. */
#define DCS_MAX_LINES           16U
/** Low eight bits of a received/transmitted terminal character. */
#define DCS_DATA_MASK           0377UL
/** Six-bit hardware scanner/send-buffer line field. */
#define DCS_LINE_MASK           077UL
/** DCSA CONI bit 32: receiver scanner stopped on a ready line. */
#define DCS_ST_SCANNER_STOPPED  000010UL
/** DCSA CONO bit 32: release/restart the receiver scanner. */
#define DCS_CO_RELEASE_SCANNER  000010UL
/** DCSA CONO bit 31: reset scanner to line zero and release it. */
#define DCS_CO_RESET_SCANNER    000020UL
/** DCSA CONI/CONO bits 33..35: receiver PI assignment. */
#define DCS_PI_MASK             000007UL

/** Successful DCS operation. */
#define DCS_E_OK                0
/** Invalid DCS line number or packed line/character argument. */
#define DCS_E_ARG              -1

/** Pack a six-bit DCS line and eight-bit byte into the driver ABI word. */
#define DCS_PACK(line,ch) \
        ((((kword_t)(line) & DCS_LINE_MASK) << 8) | \
        ((kword_t)(ch) & DCS_DATA_MASK))
/** Extract the six-bit line field from a packed DCS word. */
#define DCS_RX_LINE(word)       (((unsigned int)((word) >> 8)) & DCS_LINE_MASK)
/** Extract the low eight-bit character from a packed DCS word. */
#define DCS_RX_CHAR(word)       ((unsigned int)((word) & DCS_DATA_MASK))

/**
 * @brief Read one byte from a selected DCS line.
 * @return Packed DCS_PACK(line, byte), or a negative wait/error status.
 *
 * The operation blocks until that exact logical line has data; scanner results
 * for other lines are deferred without loss.
 */
kword_t dcs_getchar(unsigned int line);

/**
 * @brief Send one byte to a selected DCS line.
 * @param line_char DCS_PACK(line, byte).
 * @return DCS_E_OK or DCS_E_ARG.
 */
int dcs_putchar(kword_t line_char);

/** @brief Resident PI2 entry for Type 630 receiver-scanner events. */
void dcs_pi_handler(void);

#endif
