#include "kinit.h"
#include "kcore_pi.h"
#include "cty.h"
#include "clk.h"
#include "pt.h"
#include "card.h"
#include "joy.h"

static void
minit_fail_poll(kword_t name)
{
        kinit_put6(name);
        kinit_put6_spaces(5U);
        kinit_put6((kword_t)SIXBIT("FAIL  "));
        kinit_newline();
        kinit_halt();
}

static void
minit_fail_cty(kword_t name)
{
        (void)cty_put6(name);
        (void)cty_put6_spaces(5U);
        (void)cty_put6((kword_t)SIXBIT("FAIL  "));
        (void)cty_newline();
        kinit_halt();
}

static void
minit_ok_cty(kword_t name)
{
        if (cty_put6(name) != CTY_E_OK ||
            cty_put6_spaces(5U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("  OK  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}

void
cty_minit(void)
{
        if (pdp10_pi_register(CTY_NATIVE_PI_LEVEL, cty_pi_handler, 0) != 0)
                minit_fail_poll((kword_t)SIXBIT("CTY   "));
        cty_cono(CTY_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CTY_NATIVE_PI_LEVEL));
        minit_ok_cty((kword_t)SIXBIT("CTY   "));
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
        if (pdp10_pi_register(PT_NATIVE_PI_LEVEL, ptr_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("PTR   "));
        ptr_cono(PT_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(PT_NATIVE_PI_LEVEL));
        minit_ok_cty((kword_t)SIXBIT("PTR   "));
}

void
ptp_minit(void)
{
        if (pdp10_pi_register(PT_NATIVE_PI_LEVEL, ptp_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("PTP   "));
        ptp_cono(PT_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(PT_NATIVE_PI_LEVEL));
        minit_ok_cty((kword_t)SIXBIT("PTP   "));
}

void
cr_minit(void)
{
        if (pdp10_pi_register(CARD_NATIVE_PI_LEVEL, cr_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("CR    "));
        cr_cono((kword_t)CARD_NATIVE_PI_LEVEL | CR_CO_CLR_DRDY |
            CR_CO_CLR_END_CARD | CR_CO_CLR_DATA_MISS);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CARD_NATIVE_PI_LEVEL));
        minit_ok_cty((kword_t)SIXBIT("CR    "));
}

void
cp_minit(void)
{
        if (pdp10_pi_register(CARD_NATIVE_PI_LEVEL, cp_pi_handler, 0) != 0)
                minit_fail_cty((kword_t)SIXBIT("CP    "));
        cp_cono(CARD_NATIVE_PI_LEVEL);
        pdp10_pi_hw_enable(PDP10_PI_MASK(CARD_NATIVE_PI_LEVEL));
        minit_ok_cty((kword_t)SIXBIT("CP    "));
}

void
wcnsls_minit(void)
{
        wcnsls_cono(WCNSLS_CO_SPACEWAR);
        minit_ok_cty((kword_t)SIXBIT("WCNSLS"));
}

void
ocnsls_minit(void)
{
        minit_ok_cty((kword_t)SIXBIT("OCNSLS"));
}

void
slv_minit(void)
{
        if (cty_put6((kword_t)SIXBIT("SLV   ")) != CTY_E_OK ||
            cty_put6_spaces(4U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("    NO")) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT(" DRV  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}
