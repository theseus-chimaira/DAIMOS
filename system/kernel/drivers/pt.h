#ifndef DAIMON_PT_H
#define DAIMON_PT_H

#include "kcore_pi.h"

#define PTP_DEVICE              0100U
#define PTR_DEVICE              0104U
#define PT_NATIVE_PI_LEVEL      7U

#define PT_ST_DONE              0010UL
#define PT_ST_BUSY              0020UL
#define PT_ST_BINARY            0040UL
#define PTP_ST_NO_TAPE          0100UL
#define PT_ST_PI_MASK           0007UL
#define PT_WAIT_READY           0200000U

#define PT_E_OK                 0
#define PT_E_ARG               -1
#define PT_E_TIMEOUT           -2
#define PT_E_BUSY              -3
#define PT_E_IO                -4

int ptr_getchar(int *cp);
void ptr_pi_handler(void);
int ptp_putchar(int c);
void ptp_pi_handler(void);


#endif
