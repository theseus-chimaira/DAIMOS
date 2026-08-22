#include "clk.h"
#include "kcore_pi.h"

static unsigned int clk_pia;
static volatile unsigned int clk_tick_count;

static int
clk_intr_pi_handler(unsigned int level, kword_t opaque)
{
        kword_t st;

        (void)level;
        (void)opaque;
        st = clk_coni();
        if ((st & CLK_APR_ST_FLAG) == 0)
                return PDP10_PI_NOT_HANDLED;
        ++clk_tick_count;
        clk_cono((kword_t)clk_pia | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        return PDP10_PI_HANDLED;
}

int
clk_init(void)
{
        if (clk_pia != 0U)
                return CLK_E_BUSY;
        clk_tick_count = 0U;
        if (pdp10_pi_register(CLK_NATIVE_PI_LEVEL, clk_intr_pi_handler, 0) != 0)
                return CLK_E_ARG;
        clk_pia = CLK_NATIVE_PI_LEVEL;
        clk_cono((kword_t)clk_pia | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CLK_NATIVE_PI_LEVEL));
        return CLK_E_OK;
}

unsigned int
clk_ticks(void)
{
        return clk_tick_count;
}
