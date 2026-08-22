#include "kcore.h"
#include "kcore_pi.h"

kword_t kcore_boot_handoff[2];

void
kcore_init(void)
{
        pdp10_pi_init();
}

void
kcore_entry_impl(void)
{
        pdp10_halt();
}
