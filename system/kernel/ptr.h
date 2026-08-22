#ifndef DAIMON_PTR_H
#define DAIMON_PTR_H

#include "kcore.h"

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

int ptr_init(void);
int ptr_getchar(int *cp);

kword_t ptr_coni(void);
void ptr_cono(kword_t word);
kword_t ptr_datai(void);

#endif
