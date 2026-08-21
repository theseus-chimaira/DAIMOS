#ifndef DAIMON_KINIT_H
#define DAIMON_KINIT_H

#include <pdp10-sixbit.h>

/* PDP-6/PDP-10 C uses one 36-bit word for unsigned long. */
typedef unsigned long kword_t;
typedef void (*kinit_minit_fn)(void);

#define KINIT_WORD_MASK          0777777777777UL
#define KINIT_HALF_MASK          0777777UL
#define KINIT_KCORE_BASE         000060UL
#define KINIT_KCORE_ENTRY         (KINIT_KCORE_BASE + 0UL)
#define KINIT_KCORE_EARLY_INIT    (KINIT_KCORE_BASE + 1UL)
#define KINIT_KCORE_PUTCHAR       (KINIT_KCORE_BASE + 2UL)
#define KINIT_BOOT_WORD0         000040UL
#define KINIT_BOOT_WORD1         000041UL
#define KINIT_MAX_MODULES        32U

#define KINIT_LH(w) \
        ((unsigned int)(((w) >> 18) & KINIT_HALF_MASK))
#define KINIT_RH(w) \
        ((unsigned int)((w) & KINIT_HALF_MASK))
#define KINIT_PAIR(lh, rh) \
        (((((kword_t)(lh)) & KINIT_HALF_MASK) << 18) | \
        (((kword_t)(rh)) & KINIT_HALF_MASK))

#define KINIT_S6(c)              ((((kword_t)(c)) - 040UL) & 077UL)
#define KINIT_S6_W6(a,b,c,d,e,f) \
        ((KINIT_S6(a) << 30) | (KINIT_S6(b) << 24) | \
        (KINIT_S6(c) << 18) | (KINIT_S6(d) << 12) | \
        (KINIT_S6(e) << 6) | KINIT_S6(f))

/*
 * KINIT-owned image manifest.  Stage1 never interprets this structure.
 * KINIT finds it in its loaded image and turns it into the runtime table
 * below.
 *
 *   0  SIXBIT /KMAN01/
 *   1  module_count,,manifest_words
 *   2  kcore_source,,kcore_words
 *   3  kinit_begin,,kinit_words
 *   4+ two words per module:
 *        minit_entry,,mres_source
 *        module_id,,mres_stored_words
 */
#define KINIT_MANIFEST_MAGIC             SIXBIT("KMAN01")
#define KINIT_MANIFEST_HEADER_WORDS      4U
#define KINIT_MANIFEST_ENTRY_WORDS       2U
#define KINIT_MANIFEST_WORD_MAGIC        0U
#define KINIT_MANIFEST_WORD_CONTROL      1U
#define KINIT_MANIFEST_WORD_KCORE        2U
#define KINIT_MANIFEST_WORD_KINIT        3U
#define KINIT_MANIFEST_WORD_MODULE0      4U

struct kinit_module {
        kinit_minit_fn km_minit;
        const kword_t *km_mres_source;
        unsigned int km_mres_stored_words;
        unsigned int km_mres_resident_words;
        kword_t km_mres_base;
        unsigned int km_id;
};

struct kinit_manifest {
        kword_t km_boot_handoff[2];
        const kword_t *km_kcore_source;
        unsigned int km_kcore_words;
        kword_t km_kinit_begin;
        unsigned int km_kinit_words;
        unsigned int km_module_count;
        struct kinit_module km_module[KINIT_MAX_MODULES];
};

/* MREL1/MREL2 relocation format used by stored MRES payloads. */
#define MRESR_MAGIC              SIXBIT("MREL1 ")
#define MRESR_MAGIC2             SIXBIT("MREL2 ")
#define MRESR_HEADER_WORDS       2U
#define MRESR2_HEADER_WORDS      3U
#define MRESR_ADDR18_MASK        0777777UL
#define MRESR_WORD_MASK          KINIT_WORD_MASK
#define MRESR_MAX_IMAGE_WORDS    0777777U
#define MRESR_MAX_BSS_WORDS      0777777U
#define MRESR_MAX_IMPORTS        0177U
#define MRESR_BITMAP_WORDS(words) (((words) + 35U) / 36U)
#define MRESR_BITMAP_BIT(bit)    ((kword_t)1UL << (35U - (bit)))
#define MRESR_LH(w)              KINIT_LH(w)
#define MRESR_RH(w)              KINIT_RH(w)

struct mresr_desc {
        unsigned int mr_image_words;
        unsigned int mr_bss_words;
        unsigned int mr_bitmap_words;
        unsigned int mr_import_count;
        unsigned int mr_header_words;
        unsigned int mr_stored_words;
};

struct mresr_provider {
        unsigned int mp_id;
        kword_t mp_base;
        unsigned int mp_resident_words;
};

extern kword_t __kinit_image_start;
extern kword_t __kinit_image_end;

struct kinit_manifest *kinit_manifest_get(void);
void kinit_save_boot_handoff(void);
int kinit_build_manifest(void);
int kinit_relocate(void);
void kinit_run_minits(void);

void kinit_diag_banner(void);
void kinit_diag_system(void);
void kinit_diag_failure_poll(void);
void kinit_diag_failure(void);
void kinit_diag_finished(void);

int kinit_cty_init(void);
int kinit_cty_putchar(int c);
void kinit_poll_put6(kword_t word);
void kinit_halt(void);

int mresr_decode(const kword_t *payload, unsigned int payload_words,
        struct mresr_desc *dp);
int mresr_validate(const kword_t *payload, unsigned int payload_words,
        const struct mresr_desc *dp);
int mresr_load(const kword_t *payload, unsigned int payload_words,
        kword_t load_base, kword_t *destination,
        unsigned int destination_words);
int mresr_bind(const kword_t *payload, unsigned int payload_words,
        kword_t *destination, unsigned int destination_words,
        const struct mresr_provider *providers, unsigned int provider_count);

#endif
