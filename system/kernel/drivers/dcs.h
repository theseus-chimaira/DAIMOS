#ifndef DAIMON_DCS_H
#define DAIMON_DCS_H

#include "kcore.h"

#define DCS_DEVICE              0300U
#define DCSB_DEVICE             0304U
#define DCS_NATIVE_PI_LEVEL     2U
#define DCS_MAX_LINES           16U
#define DCS_DATA_MASK           0377UL
#define DCS_LINE_MASK           077UL
#define DCS_ST_SCANNER_STOPPED  000010UL
#define DCS_CO_RELEASE_SCANNER  000010UL
#define DCS_CO_RESET_SCANNER    000020UL
#define DCS_PI_MASK             000007UL

#define DCS_E_OK                0
#define DCS_E_ARG              -1
#define DCS_E_BUSY             -3

#define DCS_PACK(line,ch) \
        ((((kword_t)(line) & DCS_LINE_MASK) << 8) | \
        ((kword_t)(ch) & DCS_DATA_MASK))
#define DCS_RX_LINE(word)       (((unsigned int)((word) >> 8)) & DCS_LINE_MASK)
#define DCS_RX_CHAR(word)       ((unsigned int)((word) & DCS_DATA_MASK))

/*
 * dcs_getchar() blocks until one scanner-selected receiver supplies a byte
 * and returns DCS_PACK(line, byte).  dcs_putchar() initiates output to the
 * selected idle line described by its packed argument.  Higher-level queueing
 * and line-state policy belong to the TTY layer.
 */
kword_t dcs_getchar(void);
int dcs_putchar(kword_t line_char);
void dcs_pi_handler(void);

#endif
