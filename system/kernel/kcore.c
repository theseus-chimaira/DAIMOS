#include "kcore.h"
#include "kcore_cty.h"
#include "kcore_pi.h"

/*
 * KCORE is already resident at 060 when these routines are called by KINIT.
 * Early initialization is deliberately separate from the final KCORE entry:
 * KINIT may borrow resident KCORE services and then continue bootstrap work.
 */
int
kcore_early_init_impl(void)
{
        int error;

        kcore_pi_low_init();
        pdp10_pi_init();
        error = cty_intr_connect_pi(CTY_NATIVE_PI_LEVEL);
        if (error != CTY_E_OK)
                return error;
        return 0;
}

int
kcore_putchar_impl(int c)
{
        return cty_putchar_intr(c);
}

void
kcore_entry_impl(void)
{
        /* Final KCORE bring-up is intentionally deferred to the next step. */
        pdp10_halt();
}
