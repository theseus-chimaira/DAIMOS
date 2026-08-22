#include "kinit.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "ptr.h"

static void
minit_fail_poll(kword_t name)
{
        kinit_put6(name);
        kinit_put6_spaces(4U);
        kinit_put6((kword_t)SIXBIT("  FAIL"));
        kinit_newline();
        kinit_halt();
}

static void
minit_fail_cty(kword_t name)
{
        (void)cty_put6(name);
        (void)cty_put6_spaces(4U);
        (void)cty_put6((kword_t)SIXBIT("  FAIL"));
        (void)cty_newline();
        kinit_halt();
}

void
cty_minit(void)
{
        if (pdp10_pi_register(CTY_NATIVE_PI_LEVEL, cty_pi_handler, 0) != 0)
                minit_fail_poll((kword_t)SIXBIT("CTY   "));
        cty_cono(CTY_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CTY_NATIVE_PI_LEVEL));
        if (cty_put6((kword_t)SIXBIT("CTY   ")) != CTY_E_OK ||
            cty_put6_spaces(5U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("CTY0  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}

void
clk_minit(void)
{
        if (pdp10_pi_register(CLK_NATIVE_PI_LEVEL, clk_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("HZ    "));
        clk_cono((kword_t)CLK_NATIVE_PI_LEVEL | CLK_APR_CO_CLEAR_FLAG |
            CLK_APR_CO_ENABLE);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CLK_NATIVE_PI_LEVEL));
        if (cty_put6((kword_t)SIXBIT("HZ    ")) != CTY_E_OK ||
            cty_put6_spaces(4U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("   60 ")) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("LINE  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}

void
ptr_minit(void)
{
        if (pdp10_pi_register(PTR_NATIVE_PI_LEVEL, ptr_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("PTR   "));
        ptr_cono(PTR_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(PTR_NATIVE_PI_LEVEL));
        if (cty_put6((kword_t)SIXBIT("PTR   ")) != CTY_E_OK ||
            cty_put6_spaces(5U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("PTR0  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}
