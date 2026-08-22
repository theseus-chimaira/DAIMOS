#include "module.h"

void
module_run_minits(void)
{
        const kword_t *p;
        const kword_t *end;
        unsigned int entry;

        KINIT_TRACE("KRUNMI");
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
