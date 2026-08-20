#include "mres_reloc.h"

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
mresr_patch_addr18(kword_t *wordp, kword_t target)
{
        if (wordp == 0 || target > MRESR_ADDR18_MASK)
                return MRESR_ERANGE;
        *wordp = (*wordp & ~MRESR_ADDR18_MASK) | target;
        return 0;
}

int
mresr_preflight(const struct mresr_input *inputs, unsigned int count,
    kword_t resident_start, kword_t resident_limit, kword_t *load_bases,
    kword_t *resident_endp)
{
        struct mresr_desc desc;
        unsigned long cursor;
        unsigned long extent;
        unsigned int i;
        int error;

        if ((count != 0U && inputs == 0) || resident_endp == 0 ||
            resident_start > MRESR_ADDR18_MASK ||
            resident_limit > MRESR_ADDR18_MASK + 1UL ||
            resident_start > resident_limit)
                return MRESR_EINVAL;

        cursor = resident_start;
        for (i = 0; i < count; i++) {
                error = mresr_decode(inputs[i].mr_payload,
                    inputs[i].mr_payload_words, &desc);
                if (error != 0)
                        return error;
                error = mresr_validate(inputs[i].mr_payload,
                    inputs[i].mr_payload_words, &desc);
                if (error != 0)
                        return error;
                extent = (unsigned long)desc.mr_image_words +
                    desc.mr_bss_words;
                if (extent > resident_limit - cursor)
                        return MRESR_E2BIG;
                cursor += extent;
        }

        cursor = resident_start;
        for (i = 0; i < count; i++) {
                error = mresr_decode(inputs[i].mr_payload,
                    inputs[i].mr_payload_words, &desc);
                if (error != 0)
                        return error;
                if (load_bases != 0)
                        load_bases[i] = (kword_t)cursor;
                cursor += (unsigned long)desc.mr_image_words +
                    desc.mr_bss_words;
        }
        *resident_endp = (kword_t)cursor;
        return 0;
}

int
mresb_decode(const kword_t *payload, unsigned int payload_words,
    struct mresb_desc *dp)
{
        unsigned int kcore_words;
        unsigned int bitmap_words;
        unsigned long stored_words;

        if (payload == 0 || dp == 0 || payload_words < MRESB_HEADER_WORDS)
                return MRESR_EINVAL;
        if ((payload[0] & MRESR_WORD_MASK) != MRESB_MAGIC)
                return MRESR_EINVAL;
        kcore_words = MRESR_LH(payload[1]);
        if (kcore_words == 0U)
                return MRESR_EINVAL;
        if ((payload[2] & ~0177UL) != 0)
                return MRESR_EINVAL;
        bitmap_words = MRESR_BITMAP_WORDS(kcore_words);
        stored_words = (unsigned long)MRESB_HEADER_WORDS + bitmap_words;
        if (stored_words > payload_words || stored_words > MRESR_ADDR18_MASK)
                return MRESR_EINVAL;
        if (!mresr_bitmap_tail_ok(payload + MRESB_HEADER_WORDS,
            kcore_words, bitmap_words))
                return MRESR_EINVAL;
        dp->mb_kcore_words = kcore_words;
        dp->mb_bitmap_words = bitmap_words;
        dp->mb_provider_link_base = (kword_t)MRESR_RH(payload[1]);
        dp->mb_provider_id = (unsigned int)(payload[2] & 0177UL);
        dp->mb_stored_words = (unsigned int)stored_words;
        return 0;
}

int
mresb_apply(const kword_t *payload, unsigned int payload_words,
    kword_t *kcore, unsigned int kcore_words, unsigned int provider_id,
    kword_t provider_base, unsigned int provider_words)
{
        struct mresb_desc desc;
        const kword_t *bitmap;
        unsigned int i;
        unsigned long old_target;
        unsigned long offset;
        kword_t mask;
        const kword_t *bp;
        int error;

        if (kcore == 0 || provider_base > MRESR_ADDR18_MASK)
                return MRESR_EINVAL;
        error = mresb_decode(payload, payload_words, &desc);
        if (error != 0)
                return error;
        if (desc.mb_stored_words != payload_words ||
            desc.mb_kcore_words != kcore_words ||
            desc.mb_provider_id != provider_id)
                return MRESR_EINVAL;
        if (provider_words == 0U ||
            provider_base + (unsigned long)provider_words - 1UL >
            MRESR_ADDR18_MASK)
                return MRESR_ERANGE;
        bitmap = payload + MRESB_HEADER_WORDS;

        bp = bitmap;
        mask = MRESR_BITMAP_BIT(0);
        for (i = 0U; i < kcore_words; i++) {
                if ((*bp & mask) != 0) {
                        old_target = MRESR_RH(kcore[i]);
                        if (old_target < desc.mb_provider_link_base)
                                return MRESR_ERANGE;
                        offset = old_target - desc.mb_provider_link_base;
                        if (offset >= provider_words)
                                return MRESR_ERANGE;
                }
                mask >>= 1;
                if (mask == 0) {
                        mask = MRESR_BITMAP_BIT(0);
                        bp++;
                }
        }

        bp = bitmap;
        mask = MRESR_BITMAP_BIT(0);
        for (i = 0U; i < kcore_words; i++) {
                if ((*bp & mask) != 0) {
                        offset = MRESR_RH(kcore[i]) -
                            desc.mb_provider_link_base;
                        kcore[i] = (kcore[i] & ~MRESR_ADDR18_MASK) |
                            (kword_t)(provider_base + offset);
                }
                mask >>= 1;
                if (mask == 0) {
                        mask = MRESR_BITMAP_BIT(0);
                        bp++;
                }
        }
        return 0;
}
