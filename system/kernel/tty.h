#ifndef DAIMON_TTY_H
#define DAIMON_TTY_H

#include "kcore.h"

#define TTY_ID_CTY              0U
#define TTY_ID_DCS_BASE         1U
#define TTY_ID_DCS_COUNT        16U
#define TTY_ID_MASK             077U
#define TTY_DATA_MASK           0377UL

#define TTY_E_OK                0
#define TTY_E_INVALID          -1

#define TTY_PACK(id,ch) \
        ((((kword_t)(id) & TTY_ID_MASK) << 8) | \
        ((kword_t)(ch) & TTY_DATA_MASK))
#define TTY_ID(word)            (((unsigned int)((word) >> 8)) & TTY_ID_MASK)
#define TTY_CHAR(word)          ((unsigned int)((word) & TTY_DATA_MASK))

/*
 * tty_putchar() is the minimal resident terminal-output dispatcher.
 * TTY 0 is the CTY; TTY 1..16 map to DCS lines 0..15.  Physical-device
 * addresses are bound by MINIT after the corresponding MRES modules load.
 */
int tty_putchar(kword_t tty_char);

#endif
