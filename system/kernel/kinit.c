#include "kinit.h"
#include "module.h"
#include "mres.h"

void
kinit_enter(void)
{
        KINIT_TRACE("KENTER");
        kinit_diag_banner();

        mres_load();
        kinit_save_boot_handoff();
        kinit_diag_system();
        module_run_minits();

        kinit_diag_finished();
        kinit_call18((unsigned int)KINIT_KCORE_BASE);
        kinit_halt();
}
