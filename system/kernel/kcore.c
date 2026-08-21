#include "kcore.h"

kword_t kcore_boot_handoff[2];

void
kcore_entry_impl(void)
{
        pdp10_halt();
}
