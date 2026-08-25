#ifndef DAIMON_GE_H
#define DAIMON_GE_H

#include "kcore.h"

#define GE_GTYI_DEVICE          0070U
#define GE_GTYO_DEVICE          0750U
#define GE_CONSOLES             4U
#define GE_NATIVE_PI_LEVEL      4U

#define GTYI_ST_DONE            00010UL
#define GTYI_PI_MASK            00007UL
#define GTYI_PORT_MASK          000003000000UL
#define GTYI_PORT_SHIFT         18U
#define GTYI_CHAR_MASK          0177UL

#define GTYO_ST_DONE            00100UL
#define GTYO_CO_FROB            00200UL
#define GTYO_PI_MASK            00007UL

#define GE_SOH                  001U
#define GE_STX                  002U
#define GE_ETX                  003U
#define GE_DATA_MASK            0177UL
#define GE_LINE_MASK            03UL

#define GE_E_OK                 0
#define GE_E_ARG               -1
#define GE_E_BUSY              -3

#define GE_PACK(line,ch) \
        ((((kword_t)(line) & GE_LINE_MASK) << 8) | \
        ((kword_t)(ch) & GE_DATA_MASK))
#define GE_RX_LINE(word) \
        (((unsigned int)((word) >> 8)) & (unsigned int)GE_LINE_MASK)
#define GE_RX_CHAR(word)        ((unsigned int)((word) & GE_DATA_MASK))

/*
 * Four GE/GTY terminals use PI4.  ge_getchar() returns one
 * packed console/character pair.  ge_putchar() sends one character using the
 * GE framed-output protocol; higher-level buffering belongs to the TTY core.
 */
kword_t ge_getchar(void);
int ge_putchar(kword_t line_char);
void ge_pi_handler(void);

#endif
