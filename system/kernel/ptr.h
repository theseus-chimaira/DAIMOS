#ifndef DAIMON_PTR_H
#define DAIMON_PTR_H

#include "kcore_pi.h"

#define PTR_DEVICE              0104U
#define PTR_NATIVE_PI_LEVEL     7U
#define PTR_ST_DONE             0010UL
#define PTR_ST_BUSY             0020UL
#define PTR_ST_BINARY           0040UL
#define PTR_ST_PI_MASK          0007UL
#define PTR_WAIT_READY          0200000U

#define PTR_E_OK                0
#define PTR_E_ARG              -1
#define PTR_E_TIMEOUT          -2
#define PTR_E_BUSY             -3

int ptr_getchar(int *cp);
int ptr_pi_handler(unsigned int level, kword_t opaque);

kword_t ptr_coni(void);
void ptr_cono(kword_t word);
kword_t ptr_datai(void);

#endif
