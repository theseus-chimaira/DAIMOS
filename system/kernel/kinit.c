#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "mres.h"
#include "kboot.h"
#include "mm.h"

int kfs_boot_prepare(void);

extern kword_t sys_resident_words_immediate;
extern kword_t __kinit_image_end;

static unsigned int mres_next_addr;
static unsigned int mres_owner_next;
static const kword_t *module_mres_package;
static unsigned int module_services[MODULE_SERVICE_COUNT];
kword_t kinit_boot_handoff[2];

static unsigned int
mres_reloc_code(const kword_t *map, unsigned int word)
{
        unsigned int slot;
        unsigned int shift;
        kword_t bits;

        bits = map[word / 18U];
        slot = word % 18U;
        shift = 34U - slot * 2U;
        return (unsigned int)((bits >> shift) & 03UL);
}

void
kcore_load(void)
{
        const kword_t *src;
        kword_t *dst;
        kword_t *init_end;
        kword_t *end;

#ifdef KINIT_DEBUG
        KINIT_TRACE(KCORE_LOAD);
#endif
        src = &__kcore_load_begin;
        dst = (kword_t *)(unsigned long)KINIT_KCORE_BASE;
        init_end = &__kcore_low_init_end;
        end = &__kcore_low_end;
        while (dst < init_end)
                *dst++ = *src++;
        while (dst < end)
                *dst++ = 0;
}

void
mres_init(void)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(MRES_INIT);
#endif
        mres_next_addr = (unsigned int)(unsigned long)&__kcore_low_end;
        mres_owner_next = 1U;
}

int
mres_install(const kword_t *package, unsigned int *basep)
{
        unsigned int init_words;
        unsigned int bss_words;
        unsigned int map_words;
        unsigned int export_count;
        unsigned int export_words;
        unsigned int base;
        unsigned int i;
        const kword_t *image;
        const kword_t *map;
        kword_t *dst;
        kword_t word;
        unsigned int code;
        unsigned int half;

#ifdef KINIT_DEBUG
        KINIT_TRACE(MRES_INSTALL);
#endif
        if (package == 0 || basep == 0 || package[0] != (kword_t)MRES_MAGIC)
                return -1;
        init_words = KINIT_LH(package[1]);
        bss_words = KINIT_RH(package[1]);
        map_words = KINIT_LH(package[2]);
        export_count = KINIT_RH(package[2]);
        if (map_words != (init_words + 17U) / 18U)
                return -1;
        export_words = (export_count + 1U) / 2U;
        image = package + MRES_HEADER_WORDS + export_words;
        map = image + init_words;
        {
                kword_t alloc_base;

                if ((kword_t)init_words + (kword_t)bss_words > KINIT_HALF_MASK ||
                    mm_alloc((kword_t)init_words + (kword_t)bss_words,
                    MM_TYPE_MODULE, mres_owner_next, MM_ALLOC_LOW,
                    &alloc_base) != MM_OK)
                        return -1;
                base = (unsigned int)alloc_base;
        }
        dst = (kword_t *)(unsigned long)base;
        for (i = 0U; i < init_words; ++i) {
                word = image[i];
                code = mres_reloc_code(map, i);
                if ((code & MRES_RELOC_LH18) != 0U) {
                        half = KINIT_LH(word);
                        if (half > KINIT_HALF_MASK - base)
                                goto fail;
                        word = ((kword_t)(half + base) << 18) |
                            (word & KINIT_HALF_MASK);
                }
                if ((code & MRES_RELOC_RH18) != 0U) {
                        half = KINIT_RH(word);
                        if (half > KINIT_HALF_MASK - base)
                                goto fail;
                        word = (word & ~((kword_t)KINIT_HALF_MASK)) |
                            (kword_t)(half + base);
                }
                dst[i] = word;
        }
        for (i = 0U; i < bss_words; ++i)
                dst[init_words + i] = 0;
        if (base + init_words + bss_words > mres_next_addr)
                mres_next_addr = base + init_words + bss_words;
        *basep = base;
        ++mres_owner_next;
        return 0;

fail:
        (void)mm_free((kword_t)base, MM_TYPE_MODULE, mres_owner_next);
        return -1;
}

unsigned int
mres_export(const kword_t *package, unsigned int base, unsigned int index)
{
        unsigned int count;
        unsigned int word;
        unsigned int offset;

        if (package == 0 || package[0] != (kword_t)MRES_MAGIC)
                return 0U;
        count = KINIT_RH(package[2]);
        if (index >= count)
                return 0U;
        word = 3U + index / 2U;
        if ((index & 1U) == 0U)
                offset = KINIT_LH(package[word]);
        else
                offset = KINIT_RH(package[word]);
        if (offset > KINIT_HALF_MASK - base)
                return 0U;
        return base + offset;
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
        kinit_boot_handoff[0] = *boot0;
        kinit_boot_handoff[1] = *boot1;
}

const kword_t *
module_current_mres(void)
{
        return module_mres_package;
}

void
module_service_set(unsigned int service, unsigned int address)
{
        if (service != 0U && service < MODULE_SERVICE_COUNT)
                module_services[service] = address;
}

unsigned int
module_service_get(unsigned int service)
{
        if (service == 0U || service >= MODULE_SERVICE_COUNT)
                return 0U;
        return module_services[service];
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
                module_mres_package = (const kword_t *)(unsigned long)KINIT_RH(*p++);
                if (entry != 0U)
                        kinit_call18(entry);
        }
        module_mres_package = 0;
}

void
kinit_enter(void)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_ENTER);
#endif
        unsigned int memory_kwords;

        kcore_load();
        memory_kwords = kinit_memory_kwords();
        {
                kword_t core_words;
                kword_t low_base;
                kword_t image_end;

                core_words = (kword_t)memory_kwords << 10U;
                low_base = (kword_t)(unsigned long)&__kcore_low_end;
                image_end = (kword_t)(unsigned long)&__kinit_image_end;
                mm_boot_init(core_words);
                if (low_base < KINIT_IMAGE_BASE && low_base < core_words) {
                        kword_t low_end = KINIT_IMAGE_BASE;
                        if (low_end > core_words)
                                low_end = core_words;
                        if (low_end > low_base &&
                            mm_add_free(low_base, low_end - low_base) != MM_OK)
                                kinit_halt();
                }
                /* The pushdown list begins at image_end and grows upward.
                 * Keep its bootstrap reserve outside MM until KINIT is gone. */
                if (image_end < core_words &&
                    KINIT_STACK_RESERVE_WORDS <= core_words - image_end) {
                        kword_t high_base = image_end + KINIT_STACK_RESERVE_WORDS;
                        if (high_base < core_words &&
                            mm_add_free(high_base, core_words - high_base) != MM_OK)
                                kinit_halt();
                }
        }
        kinit_diag_banner();
        kinit_save_boot_handoff();
        module_pi_init();
        kinit_diag_system(memory_kwords);
        mres_init();
        module_run_minits();
        sys_resident_words_immediate =
            (sys_resident_words_immediate & ~((kword_t)KINIT_HALF_MASK)) |
            (kword_t)mres_next_addr;
        if (kfs_boot_prepare() != 0)
                kinit_halt();
#ifdef KINIT_DEBUG
        kinit_diag_finished();
#endif
        kinit_boot();
        kinit_halt();
}
