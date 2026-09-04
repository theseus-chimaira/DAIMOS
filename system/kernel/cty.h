#ifndef DAIMON_CTY_H
#define DAIMON_CTY_H

#include "kcore.h"

#define CTY_DEVICE              0120U
#define CTY_ST_OUTPUT_READY     0010UL
#define CTY_ST_OUTPUT_BUSY      0020UL
#define CTY_ST_INPUT_READY      0040UL
#define CTY_ST_INPUT_BUSY       0100UL
#define CTY_ST_PI_MASK          0007UL
#define CTY_CO_CLR_OUTPUT_READY (CTY_ST_OUTPUT_READY << 4)
#define CTY_CO_CLR_INPUT_READY  (CTY_ST_INPUT_READY << 4)
#define CTY_NATIVE_PI_LEVEL     4U
#define CTY_WAIT_READY          0200000U

#define CTY_E_OK                0
#define CTY_E_ARG              -1
#define CTY_E_TIMEOUT          -2
#define CTY_E_BUSY             -3

int cty_putchar(int c);
int cty_getchar(void);
void cty_pi_handler(void);


#endif
