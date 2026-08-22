#include "kcore.h"
#include "clk.h"
#include "kcore_pi.h"

#ifdef KCORE_DEVICE_TEST
#include <pdp10-sixbit.h>
#include "pt.h"
#include "card.h"
#include "joy.h"
#include "cty.h"
static kword_t kcore_test_card[CARD_COLUMNS];

static void
kcore_device_test_fail(void)
{
        (void)cty_put6((kword_t)SIXBIT("DEVTST"));
        (void)cty_put6_spaces(5U);
        (void)cty_put6((kword_t)SIXBIT("FAIL  "));
        (void)cty_newline();
        pdp10_halt();
}

static void
kcore_device_test(void)
{
        if (ptp_putchar(0252) != PT_E_OK)
                kcore_device_test_fail();
        if (cr_read_card(kcore_test_card) != (int)CARD_COLUMNS)
                kcore_device_test_fail();
        if (cp_punch_card(kcore_test_card) != (int)CARD_COLUMNS)
                kcore_device_test_fail();
        if ((wcnsls_read() & WCNSLS_WORD_MASK) != WCNSLS_WORD_MASK)
                kcore_device_test_fail();
        if ((ocnsls_read() & OCNSLS_WORD_MASK) != 0)
                kcore_device_test_fail();
        (void)cty_put6((kword_t)SIXBIT("DEVTST"));
        (void)cty_put6_spaces(5U);
        (void)cty_put6((kword_t)SIXBIT("PASS  "));
        (void)cty_newline();
}
#endif

kword_t kcore_boot_handoff[2];

void
kcore_init(void)
{
        pdp10_pi_init();
}

void
kcore_entry_impl(void)
{
        while (clk_ticks() == 0U)
                ;
#ifdef KCORE_DEVICE_TEST
        kcore_device_test();
#endif
        pdp10_halt();
}
