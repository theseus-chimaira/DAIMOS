#include "kinit.h"
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
        if (cty_init() != CTY_E_OK)
                minit_fail_poll((kword_t)SIXBIT("CTY   "));
        if (cty_put6((kword_t)SIXBIT("CTY   ")) != CTY_E_OK ||
            cty_put6_spaces(5U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("CTY0  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}

void
clk_minit(void)
{
        if (clk_init() != CLK_E_OK)
                minit_fail_cty((kword_t)SIXBIT("HZ    "));
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
        if (ptr_init() != PTR_E_OK)
                minit_fail_cty((kword_t)SIXBIT("PTR   "));
        if (cty_put6((kword_t)SIXBIT("PTR   ")) != CTY_E_OK ||
            cty_put6_spaces(5U) != CTY_E_OK ||
            cty_put6((kword_t)SIXBIT("PTR0  ")) != CTY_E_OK ||
            cty_newline() != CTY_E_OK)
                kinit_halt();
}
