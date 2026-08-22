#ifndef DAIMON_CLK_H
#define DAIMON_CLK_H

#include "kcore.h"

#define CLK_NATIVE_PI_LEVEL     7U
#define CLK_HZ                  60U

#define CLK_APR_PIA_MASK        0000007UL
#define CLK_APR_ST_FLAG         0001000UL
#define CLK_APR_ST_ENABLE       0002000UL
#define CLK_APR_CO_CLEAR_FLAG   0001000UL
#define CLK_APR_CO_ENABLE       0002000UL
#define CLK_APR_CO_DISABLE      0004000UL

#define CLK_E_OK                0
#define CLK_E_ARG              -1
#define CLK_E_BUSY             -2

int clk_init(void);
unsigned int clk_ticks(void);

kword_t clk_coni(void);
void clk_cono(kword_t word);

#endif
