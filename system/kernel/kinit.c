#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "mres.h"

void
mres_load(void)
{
        const kword_t *src;
        kword_t *dst;
        kword_t *init_end;
        kword_t *end;

#ifdef KINIT_DEBUG
        KINIT_TRACE(MRES_LOAD);
#endif
        src = &__resident_load_begin;
        dst = (kword_t *)(unsigned long)KINIT_KCORE_BASE;
        init_end = &__resident_low_init_end;
        end = &__resident_low_end;

        while (dst < init_end)
                *dst++ = *src++;
        while (dst < end)
                *dst++ = 0;
}

void
kinit_save_boot_handoff(void)
{
        volatile kword_t *boot0;
        volatile kword_t *boot1;

#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_SAVE_BOOT_HANDOFF);
#endif
        boot0 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD0;
        boot1 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD1;
        kcore_boot_handoff[0] = *boot0;
        kcore_boot_handoff[1] = *boot1;
}

void
module_run_minits(void)
{
        const kword_t *p;
        const kword_t *end;
        unsigned int entry;

#ifdef KINIT_DEBUG
        KINIT_TRACE(MODULE_RUN_MINITS);
#endif
        p = &__minit_table_begin;
        end = &__minit_table_end;
        while (p < end) {
                entry = KINIT_LH(*p);
                if (entry != 0U)
                        kinit_call18(entry);
                entry = KINIT_RH(*p++);
                if (entry != 0U)
                        kinit_call18(entry);
        }
}

void
kinit_enter(void)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_ENTER);
#endif
        kinit_diag_banner();
        mres_load();
        kinit_save_boot_handoff();
        kcore_init();
        kinit_diag_system();
        module_run_minits();
#ifdef KINIT_DEBUG
        kinit_diag_finished();
#endif
        kinit_call18((unsigned int)KINIT_KCORE_BASE);

        /* A returning KCORE entry is always fatal. */
        kinit_halt();
}
