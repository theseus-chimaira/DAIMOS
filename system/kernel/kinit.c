#include "kinit.h"

static void
kcore_entry(void)
{
        /* Temporary V0.1 handoff stub.  Replace with relocated KCORE entry. */
        kinit_diag_finished();
        kinit_halt();
}

void
kinit_cty_init(void)
{
        kinit_cty_hw_init();
}

void
kinit_enter(void)
{
        kinit_diag_banner();
        kinit_save_boot_handoff();

        if (kinit_build_manifest() != 0) {
                kinit_diag_failure_poll();
                kinit_halt();
        }

        /* From here on the polling helper at 060 is no longer required. */
        kinit_cty_init();
        kinit_diag_system();

        if (kinit_relocate() != 0) {
                kinit_diag_failure();
                kinit_halt();
        }

        kinit_run_minits();
        kcore_entry();
}
