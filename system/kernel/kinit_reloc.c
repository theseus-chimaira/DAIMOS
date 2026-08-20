#include "kinit.h"

#define MRESR_EINVAL   (-1)
#define MRESR_E2BIG    (-2)
#define MRESR_ERANGE   (-3)
#define MRESR_ENOENT   (-4)

static const struct mresr_provider *
mresr_provider_find(const struct mresr_provider *providers,
    unsigned int provider_count, unsigned int module_id)
{
        unsigned int k;

        for (k = 0U; k < provider_count; k++) {
                if (providers[k].mp_id == module_id)
                        return &providers[k];
        }
        return 0;
}

static int
mresr_bitmap_tail_ok(const kword_t *bitmap, unsigned int image_words,
    unsigned int bitmap_words)
{
        unsigned int used;
        kword_t valid_mask;

        used = image_words % 36U;
        if (used == 0U || bitmap_words == 0U)
                return 1;
        valid_mask = MRESR_WORD_MASK &
            ~(((kword_t)1UL << (36U - used)) - 1UL);
        return (bitmap[bitmap_words - 1U] & ~valid_mask) == 0;
}

int
mresr_decode(const kword_t *payload, unsigned int payload_words,
    struct mresr_desc *dp)
{
        unsigned int image_words;
        unsigned int bss_words;
        unsigned int bitmap_words;
        unsigned int import_count;
        unsigned int header_words;
        unsigned long stored_words;
        kword_t magic;

        if (payload == 0 || dp == 0 || payload_words < MRESR_HEADER_WORDS)
                return MRESR_EINVAL;
        magic = payload[0] & MRESR_WORD_MASK;
        if (magic == MRESR_MAGIC) {
                header_words = MRESR_HEADER_WORDS;
                import_count = 0U;
        } else if (magic == MRESR_MAGIC2) {
                if (payload_words < MRESR2_HEADER_WORDS)
                        return MRESR_EINVAL;
                if ((payload[2] & ~MRESR_ADDR18_MASK) != 0)
                        return MRESR_EINVAL;
                import_count = MRESR_RH(payload[2]);
                if (import_count == 0U || import_count > MRESR_MAX_IMPORTS)
                        return MRESR_EINVAL;
                header_words = MRESR2_HEADER_WORDS;
        } else {
                return MRESR_EINVAL;
        }

        image_words = MRESR_LH(payload[1]);
        bss_words = MRESR_RH(payload[1]);
        if (image_words > MRESR_MAX_IMAGE_WORDS ||
            bss_words > MRESR_MAX_BSS_WORDS)
                return MRESR_E2BIG;
        bitmap_words = MRESR_BITMAP_WORDS(image_words);
        stored_words = (unsigned long)header_words + image_words + bitmap_words;
        if (import_count != 0U)
                stored_words += (unsigned long)import_count *
                    (1UL + bitmap_words);
        if (stored_words > payload_words || stored_words > MRESR_ADDR18_MASK)
                return MRESR_EINVAL;

        dp->mr_image_words = image_words;
        dp->mr_bss_words = bss_words;
        dp->mr_bitmap_words = bitmap_words;
        dp->mr_import_count = import_count;
        dp->mr_header_words = header_words;
        dp->mr_stored_words = (unsigned int)stored_words;
        return 0;
}

int
mresr_validate(const kword_t *payload, unsigned int payload_words,
    const struct mresr_desc *dp)
{
        const kword_t *image;
        const kword_t *bitmap;
        const kword_t *ip;
        const kword_t *imap;
        unsigned long extent;
        unsigned int i;
        unsigned int j;
        unsigned int k;
        unsigned int module_id;
        kword_t mask;
        const kword_t *bp;

        if (payload == 0 || dp == 0 || dp->mr_stored_words > payload_words)
                return MRESR_EINVAL;
        image = payload + dp->mr_header_words;
        bitmap = image + dp->mr_image_words;
        extent = (unsigned long)dp->mr_image_words + dp->mr_bss_words;
        if (extent > MRESR_ADDR18_MASK + 1UL)
                return MRESR_E2BIG;

        bp = bitmap;
        mask = MRESR_BITMAP_BIT(0);
        for (i = 0; i < dp->mr_image_words; i++) {
                if ((*bp & mask) != 0 && MRESR_RH(image[i]) >= extent)
                        return MRESR_ERANGE;
                mask >>= 1;
                if (mask == 0) {
                        mask = MRESR_BITMAP_BIT(0);
                        bp++;
                }
        }
        if (!mresr_bitmap_tail_ok(bitmap, dp->mr_image_words,
            dp->mr_bitmap_words))
                return MRESR_EINVAL;

        ip = bitmap + dp->mr_bitmap_words;
        for (j = 0; j < dp->mr_import_count; j++) {
                if ((ip[0] & ~0177UL) != 0)
                        return MRESR_EINVAL;
                module_id = (unsigned int)(ip[0] & 0177UL);
                for (k = 0; k < j; k++) {
                        const kword_t *oldp;

                        oldp = bitmap + dp->mr_bitmap_words +
                            k * (1U + dp->mr_bitmap_words);
                        if ((unsigned int)(oldp[0] & 0177UL) == module_id)
                                return MRESR_EINVAL;
                }
                imap = ip + 1;
                if (!mresr_bitmap_tail_ok(imap, dp->mr_image_words,
                    dp->mr_bitmap_words))
                        return MRESR_EINVAL;
                for (i = 0; i < dp->mr_bitmap_words; i++) {
                        if ((imap[i] & bitmap[i]) != 0)
                                return MRESR_EINVAL;
                        for (k = 0; k < j; k++) {
                                const kword_t *oldmap;

                                oldmap = bitmap + dp->mr_bitmap_words +
                                    k * (1U + dp->mr_bitmap_words) + 1U;
                                if ((imap[i] & oldmap[i]) != 0)
                                        return MRESR_EINVAL;
                        }
                }
                ip += 1U + dp->mr_bitmap_words;
        }
        return 0;
}

int
mresr_load(const kword_t *payload, unsigned int payload_words,
    kword_t load_base, kword_t *destination, unsigned int destination_words)
{
        struct mresr_desc desc;
        const kword_t *image;
        const kword_t *bitmap;
        unsigned long extent;
        unsigned long relocated;
        unsigned int i;
        kword_t mask;
        const kword_t *bp;
        int error;

        if (destination == 0 || load_base > MRESR_ADDR18_MASK)
                return MRESR_EINVAL;
        error = mresr_decode(payload, payload_words, &desc);
        if (error != 0)
                return error;
        error = mresr_validate(payload, payload_words, &desc);
        if (error != 0)
                return error;

        extent = (unsigned long)desc.mr_image_words + desc.mr_bss_words;
        if (extent > destination_words)
                return MRESR_E2BIG;
        if (extent != 0UL && load_base + extent - 1UL > MRESR_ADDR18_MASK)
                return MRESR_ERANGE;

        image = payload + desc.mr_header_words;
        bitmap = image + desc.mr_image_words;
        bp = bitmap;
        mask = MRESR_BITMAP_BIT(0);
        for (i = 0; i < desc.mr_image_words; i++) {
                if ((*bp & mask) != 0) {
                        relocated = load_base + MRESR_RH(image[i]);
                        destination[i] = (image[i] & ~MRESR_ADDR18_MASK) |
                            (kword_t)relocated;
                } else {
                        destination[i] = image[i] & MRESR_WORD_MASK;
                }
                mask >>= 1;
                if (mask == 0) {
                        mask = MRESR_BITMAP_BIT(0);
                        bp++;
                }
        }
        for (; i < (unsigned int)extent; i++)
                destination[i] = 0;
        return 0;
}

int
mresr_bind(const kword_t *payload, unsigned int payload_words,
    kword_t *destination, unsigned int destination_words,
    const struct mresr_provider *providers, unsigned int provider_count)
{
        struct mresr_desc desc;
        const kword_t *bitmap;
        const kword_t *ip;
        const kword_t *imap;
        const struct mresr_provider *provider;
        unsigned int i;
        unsigned int j;
        unsigned int module_id;
        unsigned long target;
        kword_t mask;
        const kword_t *bp;
        int error;

        if (destination == 0)
                return MRESR_EINVAL;
        error = mresr_decode(payload, payload_words, &desc);
        if (error != 0)
                return error;
        error = mresr_validate(payload, payload_words, &desc);
        if (error != 0)
                return error;
        if (destination_words < desc.mr_image_words)
                return MRESR_E2BIG;
        if (desc.mr_import_count == 0U)
                return 0;
        if (providers == 0)
                return MRESR_ENOENT;

        bitmap = payload + desc.mr_header_words + desc.mr_image_words;
        ip = bitmap + desc.mr_bitmap_words;
        for (j = 0U; j < desc.mr_import_count; j++) {
                module_id = (unsigned int)(ip[0] & 0177UL);
                provider = mresr_provider_find(providers, provider_count,
                    module_id);
                if (provider == 0)
                        return MRESR_ENOENT;
                imap = ip + 1;
                bp = imap;
                mask = MRESR_BITMAP_BIT(0);
                for (i = 0U; i < desc.mr_image_words; i++) {
                        if ((*bp & mask) != 0) {
                                if (MRESR_RH(destination[i]) >=
                                    provider->mp_resident_words)
                                        return MRESR_ERANGE;
                                target = provider->mp_base +
                                    MRESR_RH(destination[i]);
                                if (target > MRESR_ADDR18_MASK)
                                        return MRESR_ERANGE;
                        }
                        mask >>= 1;
                        if (mask == 0) {
                                mask = MRESR_BITMAP_BIT(0);
                                bp++;
                        }
                }
                ip += 1U + desc.mr_bitmap_words;
        }

        ip = bitmap + desc.mr_bitmap_words;
        for (j = 0U; j < desc.mr_import_count; j++) {
                module_id = (unsigned int)(ip[0] & 0177UL);
                provider = mresr_provider_find(providers, provider_count,
                    module_id);
                imap = ip + 1;
                bp = imap;
                mask = MRESR_BITMAP_BIT(0);
                for (i = 0U; i < desc.mr_image_words; i++) {
                        if ((*bp & mask) != 0) {
                                target = provider->mp_base +
                                    MRESR_RH(destination[i]);
                                destination[i] =
                                    (destination[i] & ~MRESR_ADDR18_MASK) |
                                    (kword_t)target;
                        }
                        mask >>= 1;
                        if (mask == 0) {
                                mask = MRESR_BITMAP_BIT(0);
                                bp++;
                        }
                }
                ip += 1U + desc.mr_bitmap_words;
        }
        return 0;
}



int
kinit_relocate(void)
{
        struct kinit_manifest *manifest;
        struct mresr_provider providers[KINIT_MAX_MODULES];
        kword_t cursor;
        unsigned int provider_count;
        unsigned int i;

        manifest = kinit_manifest_get();
        cursor = KINIT_KCORE_BASE;

        for (i = 0U; i < manifest->km_kcore_words; i++)
                ((kword_t *)(unsigned long)cursor)[i] =
                    manifest->km_kcore_source[i] & KINIT_WORD_MASK;
        cursor += manifest->km_kcore_words;

        provider_count = 0U;
        for (i = 0U; i < manifest->km_module_count; i++) {
                struct kinit_module *mp;

                mp = &manifest->km_module[i];
                mp->km_mres_base = cursor;
                if (mp->km_mres_resident_words != 0U) {
                        if (cursor + mp->km_mres_resident_words >
                            manifest->km_kinit_begin)
                                return -1;
                        providers[provider_count].mp_id = mp->km_id;
                        providers[provider_count].mp_base = cursor;
                        providers[provider_count].mp_resident_words =
                            mp->km_mres_resident_words;
                        provider_count++;
                        cursor += mp->km_mres_resident_words;
                }
        }

        for (i = 0U; i < manifest->km_module_count; i++) {
                struct kinit_module *mp;

                mp = &manifest->km_module[i];
                if (mp->km_mres_resident_words == 0U)
                        continue;
                if (mresr_load(mp->km_mres_source,
                    mp->km_mres_stored_words, mp->km_mres_base,
                    (kword_t *)(unsigned long)mp->km_mres_base,
                    mp->km_mres_resident_words) != 0)
                        return -1;
        }

        for (i = 0U; i < manifest->km_module_count; i++) {
                struct kinit_module *mp;

                mp = &manifest->km_module[i];
                if (mp->km_mres_resident_words == 0U)
                        continue;
                if (mresr_bind(mp->km_mres_source,
                    mp->km_mres_stored_words,
                    (kword_t *)(unsigned long)mp->km_mres_base,
                    mp->km_mres_resident_words, providers,
                    provider_count) != 0)
                        return -1;
        }

        return 0;
}
