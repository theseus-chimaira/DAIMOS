/**
 * @file ge.h
 * @brief PDP-6 General Electric GE/GTY multi-console interface.
 *
 * Four logical GE terminals share GTYI device 0070 for input and GTYO device
 * 0750 for output. KINIT probes both halves, installs one resident MRES package,
 * and publishes the line-oriented services to the generic TTY layer only when
 * both devices respond on PI4.
 *
 * Input uses one resident mailbox/event pair regardless of console count;
 * characters received for another logical GE line are deferred into that TTY's
 * existing process-session pending field. Output serializes one complete GE
 * framed message through a single resident ownership word.
 */
#ifndef DAIMON_GE_H
#define DAIMON_GE_H

#include "kcore.h"

/** GE keyboard/input device number. */
#define GE_GTYI_DEVICE          0070U
/** GE display/output device number. */
#define GE_GTYO_DEVICE          0750U
/** Number of physical GE logical consoles. */
#define GE_CONSOLES             4U
/** Native PI level used by GTYI and probed on GTYO. */
#define GE_NATIVE_PI_LEVEL      4U

/** GTYI CONI bit 32: input word ready for DATAI. */
#define GTYI_ST_DONE            00010UL
/** GTYI CONI/CONO bits 33..35: PI assignment. */
#define GTYI_PI_MASK            00007UL
/** GTYI DATAI bits 16..17: two-bit GE console number. */
#define GTYI_PORT_MASK          000003000000UL
/** Shift from GTYI port field to integer console number. */
#define GTYI_PORT_SHIFT         18U
/** GTYI low seven-bit character mask. */
#define GTYI_CHAR_MASK          0177UL

/** GTYO CONI bit 29: transmitter ready for next DATAO byte. */
#define GTYO_ST_DONE            00100UL
/** GTYO CONO bit 28: force transmitter into ready state. */
#define GTYO_CO_FROB            00200UL
/** GTYO CONI/CONO bits 33..35: PI assignment. */
#define GTYO_PI_MASK            00007UL

#define GE_SOH                  001U
#define GE_STX                  002U
#define GE_ETX                  003U
#define GE_DATA_MASK            0177UL
#define GE_LINE_MASK            03UL

/** Successful GE output operation. */
#define GE_E_OK                 0
/** Invalid GE line or packed output argument. */
#define GE_E_ARG               -1
/** Another complete GE output frame is already being sent. */
#define GE_E_BUSY              -3

#define GE_PACK(line,ch) \
        ((((kword_t)(line) & GE_LINE_MASK) << 8) | \
        ((kword_t)(ch) & GE_DATA_MASK))
#define GE_RX_LINE(word) \
        (((unsigned int)((word) >> 8)) & (unsigned int)GE_LINE_MASK)
#define GE_RX_CHAR(word)        ((unsigned int)((word) & GE_DATA_MASK))

/**
 * @brief Read one character from a selected GE console.
 * @param line GE console 0..3.
 * @return Character 0..0177, or a negative wait/error status.
 *
 * Bytes received for another console are preserved in that logical TTY's
 * existing pending-byte field before scanning resumes.
 */
kword_t ge_getchar(unsigned int line);

/**
 * @brief Send one character through the GE framed-output protocol.
 * @param line_char GE_PACK(line, character), with line 0..3.
 * @return GE_E_OK, GE_E_ARG, or GE_E_BUSY.
 */
int ge_putchar(kword_t line_char);

/** @brief Resident PI4 entry for GTYI input-ready events. */
void ge_pi_handler(void);

#endif
