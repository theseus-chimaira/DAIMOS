#include "kcore.h"

void
kcore_entry_impl(void)
{
        /* Final KCORE bring-up is intentionally deferred to the next step. */
        pdp10_halt();
}
