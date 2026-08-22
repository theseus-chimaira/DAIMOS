#include "kinit.h"
#include "cty.h"
#include "clk.h"

static void
minit_fail(kword_t name)
{
        kinit_put6(name);
        kinit_put6_spaces(4U);
        kinit_put6((kword_t)SIXBIT("  FAIL"));
        kinit_newline();
        kinit_halt();
}

void
cty_minit(void)
{
        if (cty_init() != CTY_E_OK || cty_putchar(0) != CTY_E_OK)
                minit_fail((kword_t)SIXBIT("CTY   "));
        kinit_put6((kword_t)SIXBIT("CTY   "));
        kinit_put6_spaces(5U);
        kinit_put6((kword_t)SIXBIT("CTY0  "));
        kinit_newline();
}

void
clk_minit(void)
{
        if (clk_init() != CLK_E_OK)
                minit_fail((kword_t)SIXBIT("HZ    "));
        kinit_put6((kword_t)SIXBIT("HZ    "));
        kinit_put6_spaces(4U);
        kinit_put6((kword_t)SIXBIT("   60 "));
        kinit_put6((kword_t)SIXBIT("LINE  "));
        kinit_newline();
}
