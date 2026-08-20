#include "kinit.h"

typedef int (*kinit_kcore_init_fn)(void);
typedef int (*kinit_kcore_putchar_fn)(int c);

static void
kcore_entry(void)
{
        /* Final handoff remains disabled during the V0.1 bring-up. */
        kinit_diag_finished();
        kinit_halt();
}

int
kinit_cty_init(void)
{
        kinit_kcore_init_fn initfn;

        initfn = (kinit_kcore_init_fn)(unsigned long)KINIT_KCORE_EARLY_INIT;
        return (*initfn)();
}

int
kinit_cty_putchar(int c)
{
        kinit_kcore_putchar_fn putfn;

        putfn = (kinit_kcore_putchar_fn)(unsigned long)KINIT_KCORE_PUTCHAR;
        return (*putfn)(c);
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
         * Relocate resident KCORE first.  The copy overwrites the temporary
         * Stage1 SIXBIT helper at 060, so all later output comes from KCORE.
         */
        if (kinit_relocate() != 0)
                kinit_halt();

        /* Borrow KCORE's real PI/CTY implementation, then return to KINIT. */
        if (kinit_cty_init() != 0)
                kinit_halt();

        kinit_diag_system();

        /* V0.1 checkpoint: MINITs and final KCORE entry come next. */
        kinit_halt();

        kinit_run_minits();
        kcore_entry();
}
