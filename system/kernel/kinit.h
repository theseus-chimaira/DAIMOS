#ifndef DAIMON_KINIT_H
#define DAIMON_KINIT_H

#include "types.h"
#include "dboot_v1.h"
#include "boot_source.h"
#include "d6stripe.h"

/*
 * Native PDP-6 KINIT needs boot-only state after KCORE is resident.  Keep it
 * in the otherwise-unused gap immediately above the 4KW KCORE ceiling.  The
 * stack above MRES-COLD becomes the KCORE stack at handoff, so Stage1 memory
 * cannot remain KINIT scratch once KCORE starts making calls.
 */
#define KINIT_SCRATCH_BASE          012000UL

struct kinit_stage1_scratch {
        struct kinit_boot_source ks_boot_source;
        struct d6stripe_map ks_fs_map;
        kword_t ks_sector[DBOOT_SECTOR_WORDS];
        kword_t ks_bad_run[DBOOT_MAX_BAD_RUNS];
};

#ifdef DAIMON_KINIT_STAGE1_SCRATCH
#define KINIT_STAGE1_SCRATCH \
        ((struct kinit_stage1_scratch *)(unsigned long)KINIT_SCRATCH_BASE)
#endif

/*
 * KINIT is the reclaimable DAIMON kernel initializer.  HDD/DBOOT V1 Stage1
 * jumps to the KINIT entry with AC1 containing BOOTINFO_BASE.
 */
#define KINIT_DIAG_BAD_BOOTINFO  1U
#define KINIT_DIAG_BAD_BADMAP    2U
#define KINIT_DIAG_BAD_PAYLOAD   3U
#define KINIT_DIAG_BAD_DMANIF    4U

void    kinit_diag_banner(void);
void    kinit_diag_error(unsigned int which, int error);
void    kinit_diag_bootinfo(const kword_t *bootinfo_words);
int     kinit_diag_modules(kword_t init_base, kword_t init_words,
            kword_t member_mask, kword_t present_mask);
void    kinit_diag_handoff(void);

void    kinit_enter(kword_t *bootinfo_words);
int     kinit_import_bootinfo(kword_t *bootinfo_words);
int     kinit_import_boot_badmaps(void);
void    kinit_probe_disks(void);
int     kinit_load_payload(kword_t init_base, kword_t init_words,
            kword_t kcore_base, kword_t kcore_words);
struct kinit_boot_source *kinit_boot_source(void);

#endif /* DAIMON_KINIT_H */
