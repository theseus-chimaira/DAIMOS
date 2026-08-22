#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "mres.h"

static unsigned int mres_next_addr;
static const kword_t *module_mres_package;
static unsigned int module_services[MODULE_SERVICE_COUNT];

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
        base = mres_next_addr;
        if (base > KINIT_HALF_MASK || init_words > KINIT_HALF_MASK - base ||
            bss_words > KINIT_HALF_MASK - base - init_words)
                return -1;
        dst = (kword_t *)(unsigned long)base;
        for (i = 0U; i < init_words; ++i) {
                word = image[i];
                code = mres_reloc_code(map, i);
                if ((code & MRES_RELOC_LH18) != 0U) {
                        half = KINIT_LH(word);
                        if (half > KINIT_HALF_MASK - base)
                                return -1;
                        word = ((kword_t)(half + base) << 18) |
                            (word & KINIT_HALF_MASK);
                }
                if ((code & MRES_RELOC_RH18) != 0U) {
                        half = KINIT_RH(word);
                        if (half > KINIT_HALF_MASK - base)
                                return -1;
                        word = (word & ~((kword_t)KINIT_HALF_MASK)) |
                            (kword_t)(half + base);
                }
                dst[i] = word;
        }
        for (i = 0U; i < bss_words; ++i)
                dst[init_words + i] = 0;
        mres_next_addr = base + init_words + bss_words;
        *basep = base;
        return 0;
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
        kcore_boot_handoff[0] = *boot0;
        kcore_boot_handoff[1] = *boot1;
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
        kinit_diag_banner();
        kcore_load();
        kinit_save_boot_handoff();
        kcore_init();
        kinit_diag_system();
        mres_init();
        module_run_minits();
#ifdef KINIT_DEBUG
        kinit_diag_finished();
#endif
        kinit_call18((unsigned int)KINIT_KCORE_BASE);
        kinit_halt();
}
