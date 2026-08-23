#ifndef DAIMON_CLK_H
#define DAIMON_CLK_H

#include "kcore_pi.h"

#define CLK_NATIVE_PI_LEVEL     6U
#define CLK_HZ                  60U

#define CLK_APR_PIA_MASK        0000007UL
#define CLK_APR_ST_FLAG         0001000UL
#define CLK_APR_ST_ENABLE       0002000UL
#define CLK_APR_CO_CLEAR_FLAG   0001000UL
#define CLK_APR_CO_ENABLE       0002000UL
#define CLK_APR_CO_DISABLE      0004000UL

unsigned int clk_ticks(void);
void clk_pi_handler(void);


#endif
