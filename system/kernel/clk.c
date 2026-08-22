#include "clk.h"

static volatile unsigned int clk_tick_count;

int
clk_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t st;

        (void)level;
        (void)opaque;
        st = clk_coni();
        if ((st & CLK_APR_ST_FLAG) == 0)
                return PDP10_PI_NOT_HANDLED;
        ++clk_tick_count;
        clk_cono((kword_t)CLK_NATIVE_PI_LEVEL | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        return PDP10_PI_HANDLED;
}

unsigned int
clk_ticks(void)
{
        return clk_tick_count;
}
