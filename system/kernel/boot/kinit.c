#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "mres.h"
#include "kboot.h"
#include "mm.h"
#include "mm_internal.h"
#include "proc.h"


extern kword_t sys_resident_words_immediate;
extern kword_t __kcore_load_end;
extern kword_t __kinit_image_end;
#if KINIT_STACK_WATERMARK
extern kword_t kinit_stack_highwater;
#endif
void kinit_late_handoff(kword_t stack_base, kword_t reclaim_end);

static unsigned int mres_next_addr;
static unsigned int mres_owner_next;
kword_t mres_source_end;
unsigned int mres_last_owner;
unsigned int mres_last_image_words;
static const kword_t *module_mres_package;
static unsigned int module_services[MODULE_SERVICE_COUNT];
kword_t kinit_boot_handoff[2];

#if KINIT_STACK_WATERMARK
static void
kinit_stack_watermark_record(void)
{
        unsigned int used_words;

        used_words = kinit_stack_watermark_measure();
        if ((kword_t)used_words > kinit_stack_highwater)
                kinit_stack_highwater = (kword_t)used_words;
}
#endif

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
        mres_last_owner = 0U;
        mres_last_image_words = 0U;
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
                kword_t image_words;

                image_words = (kword_t)init_words + (kword_t)bss_words;
                if (image_words > KINIT_HALF_MASK ||
                    mres_owner_next > MM_OWNER_MASK ||
                    mm_alloc(image_words, MM_TYPE_MODULE, mres_owner_next,
                    MM_ALLOC_LOW, &alloc_base) != MM_OK)
                        return -1;
                base = (unsigned int)alloc_base;
                /* Boot MRES is permanent and must form one packed block
                 * immediately above KCORE.  No dynamic allocation is allowed
                 * before module installation completes. */
                if (base != mres_next_addr)
                        goto fail;
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
        if (mres_owner_next == 0U || mres_owner_next > MM_OWNER_MASK)
                goto fail;
        /* Packed boot MRES is permanent.  Commit it out of the general MM
         * table immediately; MM descriptors are reserved for memory that can
         * later move or be reclaimed. */
        if (mm_boot_reserve((kword_t)base, MM_TYPE_MODULE, mres_owner_next) !=
            MM_OK)
                goto fail;
        mres_next_addr = base + init_words + bss_words;
        *basep = base;
        mres_last_owner = mres_owner_next;
        mres_last_image_words = init_words + bss_words;
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
        kword_t minit_table[040];
        const kword_t *p;
        const kword_t *end;
        const kword_t *previous_package;
        const kword_t *package;
        unsigned int entry;
        unsigned int i;
        unsigned int table_words;

#ifdef KINIT_DEBUG
        KINIT_TRACE(MODULE_RUN_MINITS);
#endif
        table_words = (unsigned int)(&__minit_table_end -
            &__minit_table_begin);
        if (table_words > 040U)
                kinit_halt();
        for (i = 0U; i < table_words; ++i)
                minit_table[i] = (&__minit_table_begin)[i];

        /* KCORE has already been copied to its permanent low-memory address.
         * Preserve the small MINIT table on the disposable KINIT stack, then
         * publish the complete KINIT prefix containing the table and embedded
         * KCORE source.  Permanent MRES may therefore grow through 030000
         * without colliding with dead bootstrap data. */
        if (mm_add_free(KINIT_IMAGE_BASE,
            (kword_t)(unsigned long)&__kcore_load_end - KINIT_IMAGE_BASE) !=
            MM_OK)
                kinit_halt();
        p = minit_table;
        end = minit_table + table_words;
        previous_package = 0;
        while (p < end) {
                entry = KINIT_LH(*p);
                package = (const kword_t *)(unsigned long)KINIT_RH(*p++);
                /* MRES package sources are linked in last-use MINIT order.
                 * Once MINIT advances to the next package, the preceding
                 * contiguous source image is dead and may immediately back
                 * later MRES allocations.  This is essential on 32K systems
                 * and avoids retaining disposal data until late KINIT. */
                if (previous_package != 0 && package != 0) {
                        kword_t previous_addr;
                        kword_t package_addr;

                        previous_addr =
                            (kword_t)(unsigned long)previous_package;
                        package_addr = (kword_t)(unsigned long)package;
                        if (package_addr < previous_addr)
                                kinit_halt();
                        if (package_addr > previous_addr &&
                            mm_add_free(previous_addr,
                            package_addr - previous_addr) != MM_OK)
                                kinit_halt();
                }
                module_mres_package = package;
                if (entry != 0U)
                        kinit_call18(entry);
                if (package != 0)
                        previous_package = package;
        }
        if (previous_package != 0) {
                unsigned int init_words;
                unsigned int map_words;
                unsigned int export_words;
                kword_t words;

                init_words = KINIT_LH(previous_package[1]);
                map_words = KINIT_LH(previous_package[2]);
                export_words = (KINIT_RH(previous_package[2]) + 1U) / 2U;
                words = (kword_t)MRES_HEADER_WORDS + export_words +
                    init_words + map_words;
                mres_source_end = (kword_t)(unsigned long)previous_package +
                    words;
                if (mm_add_free((kword_t)(unsigned long)previous_package,
                    words) != MM_OK)
                        kinit_halt();
        }
        module_mres_package = 0;
}

void
kinit_enter(void)
{
        unsigned int memory_kwords;
        kword_t kernel_stack_base;

#if KINIT_STACK_WATERMARK
        /* kinit_enter() has already allocated its fixed frame.  Mark only
         * words above the live pushdown pointer; the unmarked prefix is
         * therefore counted as used by the final high-water scan. */
        kinit_stack_watermark_begin();
#endif
#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_ENTER);
#endif
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
        /* Private PDP-6 filesystem UUOs are used by resident VFS leaves during
         * the remainder of boot, so install 041 immediately after preserving
         * the Stage1 handoff rather than waiting for first-user setup. */
        kinit_user_trap_init();
        module_pi_init();
        kinit_diag_system(memory_kwords);
        mres_init();
        module_run_minits();
#if KINIT_BADMAP
        badmap_post_minits();
#endif
#if KINIT_STACK_WATERMARK
        kinit_stack_watermark_record();
#endif
        /* Dynamic kernel objects begin only after the packed permanent MRES
         * block is complete.  This keeps KCORE+MRES gapless in low memory. */
        if (mm_alloc(KERNEL_IDLE_STACK_WORDS, MM_TYPE_KERNEL_DYNAMIC, 2U,
            MM_ALLOC_LOW,
            &kernel_stack_base) != MM_OK ||
            mm_pin(kernel_stack_base) != MM_OK)
                kinit_halt();
#if PROC_STACK_WATERMARK
        {
                kword_t *idle_stack;
                unsigned int i;

                idle_stack = (kword_t *)(unsigned long)kernel_stack_base;
                for (i = 1U; i < (unsigned int)KERNEL_IDLE_STACK_WORDS; ++i)
                        idle_stack[i] = kernel_stack_base + (kword_t)i;
        }
#endif
        sys_resident_words_immediate =
            (sys_resident_words_immediate & ~((kword_t)KINIT_HALF_MASK)) |
            (kword_t)(mres_next_addr - KINIT_KCORE_BASE);
        {
                kword_t image_end;
                kword_t reclaim_end;
                kword_t future_free_words;

                image_end = (kword_t)(unsigned long)&__kinit_image_end;
                reclaim_end = image_end + KINIT_STACK_RESERVE_WORDS;
                if (reclaim_end > mm_core_words)
                        kinit_halt();
                future_free_words = reclaim_end - KINIT_IMAGE_BASE;
#ifdef KINIT_DEBUG
                kinit_diag_finished();
#endif
                kinit_boot(future_free_words);
#if KINIT_STACK_WATERMARK
                kinit_stack_watermark_record();
#endif
        }
        if (proc_boot_init() != 0)
                kinit_halt();
#if KINIT_STACK_WATERMARK
        kinit_stack_watermark_record();
#endif
        {
                kword_t reclaim_end;
                kword_t image_end;

                image_end = (kword_t)(unsigned long)&__kinit_image_end;
                reclaim_end = image_end + KINIT_STACK_RESERVE_WORDS;
                if (reclaim_end > mm_core_words)
                        kinit_halt();
                kinit_late_handoff(kernel_stack_base, reclaim_end);
        }
        kinit_halt();
}
