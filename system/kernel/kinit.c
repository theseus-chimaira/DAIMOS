#include "kinit.h"

static void
kcore_entry(void)
{
        kinit_diag_finished();
        kinit_halt();
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

        /*
         * KCORE and every MRES are relocated and bound before any device
         * driver is initialized.  The fixed bootstrap SIXBIT helper at
         * 077760 remains available throughout this phase.
         */
        if (kinit_relocate() != 0) {
                kinit_diag_failure_poll();
                kinit_halt();
        }

        /* MINIT code remains in the opaque boot image and returns here. */
        kinit_run_minits();

        /* Final resident KCORE handoff remains the next checkpoint. */
        kcore_entry();
}
