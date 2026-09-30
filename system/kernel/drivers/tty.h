/**
 * @file tty.h
 * @brief Logical terminal numbering and compact physical-dispatch interface.
 *
 * DAIMOS exposes one contiguous logical TTY namespace over the installed
 * physical terminal devices: CTY is id 0, DCS lines are ids 1..16, and GE
 * consoles are ids 17..20. The resident TTY MRES contains only range mapping
 * and six backend tail jumps patched by MINIT after physical drivers load.
 *
 * Line discipline, controlling-terminal ownership, canonical buffering, and
 * job control live in the process layer; this interface deliberately remains
 * a very small physical I/O dispatcher.
 */
#ifndef DAIMON_TTY_H
#define DAIMON_TTY_H

#include "kcore.h"

/** Physical console terminal logical id. */
#define TTY_ID_CTY              0U
/** First logical id backed by DCS line 0. */
#define TTY_ID_DCS_BASE         1U
/** Number of DCS logical terminals. */
#define TTY_ID_DCS_COUNT        16U
/** First logical id backed by GE console 0. */
#define TTY_ID_GE_BASE          17U
/** Number of GE logical terminals. */
#define TTY_ID_GE_COUNT         4U
/** Six-bit packed logical-terminal id mask. */
#define TTY_ID_MASK             077U
/** Eight-bit packed character mask. */
#define TTY_DATA_MASK           0377UL

#define TTY_E_OK                0
#define TTY_E_INVALID          -1

#define TTY_PACK(id,ch) \
        ((((kword_t)(id) & TTY_ID_MASK) << 8) | \
        ((kword_t)(ch) & TTY_DATA_MASK))
#define TTY_ID(word)            (((unsigned int)((word) >> 8)) & TTY_ID_MASK)
#define TTY_CHAR(word)          ((unsigned int)((word) & TTY_DATA_MASK))

/**
 * @brief Write one packed logical-terminal character.
 * @param tty_char TTY_PACK(logical_id, character).
 * @return Backend status, or TTY_E_INVALID when the id/backend is unavailable.
 */
int tty_putchar(kword_t tty_char);

/**
 * @brief Read one character from a logical terminal.
 * @param tty Logical terminal id 0..20.
 * @return Character/backend status, or TTY_E_INVALID for invalid/unbound ids.
 */
int tty_getchar(unsigned int tty);

/** Render exactly one complete S6REC text record on the controlling TTY. */
int tty_write_s6rec(const kword_t *words, unsigned int nwords);

#endif
