#include "kinit.h"

static struct kinit_manifest kmanifest;

struct kinit_manifest *
kinit_manifest_get(void)
{
        return &kmanifest;
}

void
kinit_save_boot_handoff(void)
{
        volatile kword_t *boot0;
        volatile kword_t *boot1;

        boot0 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD0;
        boot1 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD1;
        kmanifest.km_boot_handoff[0] = *boot0 & KINIT_WORD_MASK;
        kmanifest.km_boot_handoff[1] = *boot1 & KINIT_WORD_MASK;
}

static const kword_t *
kinit_find_image_manifest(void)
{
        const kword_t *p;
        const kword_t *end;

        p = &__kinit_image_start;
        end = &__kinit_image_end;
        while (p < end) {
                if ((*p & KINIT_WORD_MASK) == KINIT_MANIFEST_MAGIC)
                        return p;
                p++;
        }
        return 0;
}

int
kinit_build_manifest(void)
{
        const kword_t *image_begin;
        const kword_t *image_end;
        const kword_t *manifest;
        kword_t control;
        kword_t pair;
        unsigned int module_count;
        unsigned int manifest_words;
        unsigned int i;

        image_begin = &__kinit_image_start;
        image_end = &__kinit_image_end;
        if (image_end <= image_begin)
                return -1;

        manifest = kinit_find_image_manifest();
        if (manifest == 0 || manifest + KINIT_MANIFEST_HEADER_WORDS > image_end)
                return -1;

        control = manifest[KINIT_MANIFEST_WORD_CONTROL] & KINIT_WORD_MASK;
        module_count = KINIT_LH(control);
        manifest_words = KINIT_RH(control);
        if (module_count > KINIT_MAX_MODULES ||
            manifest_words != KINIT_MANIFEST_HEADER_WORDS +
            KINIT_MANIFEST_ENTRY_WORDS * module_count ||
            manifest + manifest_words > image_end)
                return -1;

        pair = manifest[KINIT_MANIFEST_WORD_KCORE] & KINIT_WORD_MASK;
        kmanifest.km_kcore_source =
            (const kword_t *)(unsigned long)KINIT_LH(pair);
        kmanifest.km_kcore_words = KINIT_RH(pair);
        if (kmanifest.km_kcore_source < image_begin ||
            kmanifest.km_kcore_source + kmanifest.km_kcore_words > image_end ||
            kmanifest.km_kcore_words == 0U)
                return -1;

        pair = manifest[KINIT_MANIFEST_WORD_KINIT] & KINIT_WORD_MASK;
        kmanifest.km_kinit_begin = (kword_t)KINIT_LH(pair);
        kmanifest.km_kinit_words = KINIT_RH(pair);
        if (kmanifest.km_kinit_words == 0U ||
            kmanifest.km_kinit_begin < (kword_t)(unsigned long)image_begin ||
            kmanifest.km_kinit_begin + kmanifest.km_kinit_words >
            (kword_t)(unsigned long)image_end)
                return -1;

        kmanifest.km_module_count = module_count;
        for (i = 0U; i < module_count; i++) {
                struct kinit_module *mp;
                struct mresr_desc desc;
                unsigned int off;

                off = KINIT_MANIFEST_WORD_MODULE0 +
                    KINIT_MANIFEST_ENTRY_WORDS * i;
                pair = manifest[off] & KINIT_WORD_MASK;
                mp = &kmanifest.km_module[i];
                mp->km_minit = (kinit_minit_fn)(unsigned long)KINIT_LH(pair);
                mp->km_mres_source =
                    (const kword_t *)(unsigned long)KINIT_RH(pair);

                pair = manifest[off + 1U] & KINIT_WORD_MASK;
                mp->km_id = KINIT_LH(pair);
                mp->km_mres_stored_words = KINIT_RH(pair);
                mp->km_mres_resident_words = 0U;
                mp->km_mres_base = 0;

                if (mp->km_minit == 0)
                        return -1;
                if (mp->km_mres_stored_words == 0U) {
                        mp->km_mres_source = 0;
                        continue;
                }
                if (mp->km_mres_source < image_begin ||
                    mp->km_mres_source + mp->km_mres_stored_words > image_end)
                        return -1;
                if (mresr_decode(mp->km_mres_source,
                    mp->km_mres_stored_words, &desc) != 0 ||
                    mresr_validate(mp->km_mres_source,
                    mp->km_mres_stored_words, &desc) != 0 ||
                    desc.mr_stored_words != mp->km_mres_stored_words)
                        return -1;
                mp->km_mres_resident_words =
                    desc.mr_image_words + desc.mr_bss_words;
        }

        return 0;
}

void
kinit_run_minits(void)
{
        unsigned int i;

        for (i = 0U; i < kmanifest.km_module_count; i++)
                (*kmanifest.km_module[i].km_minit)();
}
