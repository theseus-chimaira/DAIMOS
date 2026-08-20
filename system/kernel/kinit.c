#include "kinit.h"
#include "bootinfo_hdd_v1.h"
#include "dboot_v1.h"
#include "machine.h"

#ifdef DAIMON_BOOT_TRACE
static void
kinit_boot_trace6(int c0, int c1, int c2, int c3, int c4, int c5)
{
        (void)pdp10_cty_putc(c0);
        (void)pdp10_cty_putc(c1);
        (void)pdp10_cty_putc(c2);
        (void)pdp10_cty_putc(c3);
        (void)pdp10_cty_putc(c4);
        (void)pdp10_cty_putc(c5);
        (void)pdp10_cty_putc(' ');
}
#endif

void
kinit_enter(kword_t *bootinfo_words)
{
        kword_t init_base;
        kword_t init_words;
        kword_t kcore_words;
        kword_t member_summary;

        kinit_diag_banner();
#ifdef DAIMON_BOOT_TRACE
        kinit_boot_trace6('K', '1', 'B', 'A', 'N', 'N');
#endif
        {
                int error;

                error = kinit_import_bootinfo(bootinfo_words);
                if (error != 0) {
                        kinit_diag_error(KINIT_DIAG_BAD_BOOTINFO, error);
                        return;
                }
#ifdef DAIMON_BOOT_TRACE
                kinit_boot_trace6('K', '2', 'B', 'I', 'N', 'F');
#endif
                error = kinit_import_boot_badmaps();
                if (error != 0) {
                        kinit_diag_error(KINIT_DIAG_BAD_BADMAP, error);
                        return;
                }
#ifdef DAIMON_BOOT_TRACE
                kinit_boot_trace6('K', '3', 'B', 'M', 'A', 'P');
#endif
        }

        init_base = bootinfo_words[BOOTINFO_WORD_INIT_START] &
            DBOOT_HALF_MASK;
        init_words = bootinfo_words[BOOTINFO_WORD_INIT_WORDS] &
            DBOOT_HALF_MASK;
        kcore_words = bootinfo_words[BOOTINFO_WORD_KCORE_MEMORY] &
            DBOOT_HALF_MASK;
        {
                int error;

                error = kinit_load_payload(init_base, init_words,
                    DBOOT_KERNEL_LOAD_BASE, kcore_words);
                if (error != 0) {
                        kinit_diag_error(KINIT_DIAG_BAD_PAYLOAD, error);
                        return;
                }
#ifdef DAIMON_BOOT_TRACE
                kinit_boot_trace6('K', '4', 'P', 'A', 'Y', 'L');
#endif
        }

        /* The boot channel is idle now; discover non-member disks without
           disturbing DBOOT or payload reads. */
        kinit_probe_disks();
#ifdef DAIMON_BOOT_TRACE
        kinit_boot_trace6('K', '5', 'D', 'I', 'S', 'K');
#endif

        /* Report only after the complete boot payload has been validated. */
        kinit_diag_bootinfo(bootinfo_words);
        member_summary = bootinfo_words[BOOTINFO_WORD_MEMBER_SUMMARY] &
            DBOOT_WORD_MASK;
        {
                int error;

                error = kinit_diag_modules(init_base, init_words,
                    BOOTINFO_MEMBER_MASK(member_summary),
                    kinit_boot_source()->ks_dsk_present_mask);
                if (error != 0) {
                        kinit_diag_error(KINIT_DIAG_BAD_DMANIF, error);
                        return;
                }
#ifdef DAIMON_BOOT_TRACE
                kinit_boot_trace6('K', '6', 'M', 'O', 'D', 'S');
#endif
        }
        kinit_diag_handoff();
#ifdef DAIMON_BOOT_TRACE
        kinit_boot_trace6('K', '7', 'C', 'O', 'R', 'E');
#endif
        mach_enter_kcore(kinit_boot_source());
}
