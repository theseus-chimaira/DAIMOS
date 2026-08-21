#include "kinit.h"
#include "kcore.h"

void
kinit_save_boot_handoff(void)
{
        volatile kword_t *boot0;
        volatile kword_t *boot1;

        KINIT_TRACE(PDP10_SIXBIT6('K','S','A','V','E',' '));
        boot0 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD0;
        boot1 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD1;
        kcore_boot_handoff[0] = *boot0 & KINIT_WORD_MASK;
        kcore_boot_handoff[1] = *boot1 & KINIT_WORD_MASK;
}
