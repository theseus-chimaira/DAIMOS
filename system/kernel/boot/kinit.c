/**
 * @file kinit.c
 * @brief Transient kernel initialization and permanent MRES installation.
 */

#include "kinit.h"
#include "kcore.h"
#include "module.h"
#include "mres.h"
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

/*
 * State used while permanent MRES packages are installed.
 *
 * mres_next_addr is the first free permanent word above KCORE/MRES.
 * mres_owner_next supplies temporary MM ownership while an MRES is copied.
 * The last-owner fields let MINIT consumers identify the package just placed.
 * module_mres_package names the package belonging to the MINIT currently
 * running, and module_services is the boot-time service-address directory.
 */
static unsigned int mres_next_addr;
static unsigned int mres_owner_next;
kword_t mres_source_end;
unsigned int mres_last_owner;
unsigned int mres_last_image_words;
static const kword_t *module_mres_package;
static unsigned int module_services[MODULE_SERVICE_COUNT];
kword_t kinit_boot_handoff[2];

#if KINIT_STACK_WATERMARK
/*
 * Record the largest observed KINIT pushdown-list usage.
 *
 * Watermark builds use this at selected boot boundaries so stack sizing can be
 * verified without keeping any measurement machinery in the resident kernel.
 */
static void
kinit_stack_watermark_record(void)
{
        unsigned int used_words;

        used_words = kinit_stack_watermark_measure();
        if ((kword_t)used_words > kinit_stack_highwater)
                kinit_stack_highwater = (kword_t)used_words;
}
#endif

/**
 * @brief Copy fixed KCORE from its embedded KINIT image to low memory.
 *
 * Initialized words are copied from the embedded image and KCORE BSS is
 * cleared explicitly.  This establishes the permanent resident core before
 * any MRES packages are placed immediately above it.
 */
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
        /*
         * KCORE is linked as initialized words followed by BSS.  Only copy
         * the initialized prefix from KINIT; explicitly zero the remainder
         * so KCORE does not depend on the contents of physical low memory.
         */
        src = &__kcore_load_begin;
        dst = (kword_t *)(unsigned long)KINIT_KCORE_BASE;
        init_end = &__kcore_low_init_end;
        end = &__kcore_low_end;
        while (dst < init_end)
                *dst++ = *src++;
        while (dst < end)
                *dst++ = 0;
}

/**
 * @brief Initialize the permanent MRES placement state.
 *
 * MRES packages are laid out contiguously above KCORE so the permanent kernel
 * occupies one gapless low-memory range and leaves maximum contiguous core for
 * later dynamic allocations and user processes.
 */
void
mres_init(void)
{
#ifdef KINIT_DEBUG
        KINIT_TRACE(MRES_INIT);
#endif
        /* MRES packages are packed immediately above fixed KCORE. */
        mres_next_addr = (unsigned int)(unsigned long)&__kcore_low_end;
        mres_owner_next = 1U;
        mres_last_owner = 0U;
        mres_last_image_words = 0U;
}

/**
 * @brief Validate, relocate, and install one MRES package into permanent low memory.
 *
 * The package contains an initialized image, BSS size, packed relocation map,
 * and exported entry offsets.  The image is allocated at the next required
 * permanent address, relocated by adding its final base to marked PDP-10
 * halfwords, zero-filled through BSS, then converted from a temporary MM
 * allocation into a boot reservation.
 *
 * @param package MRES package image to validate and install.
 * @param basep Receives the installed absolute base address.
 * @return 0 on success, -1 on validation or installation failure.
 */
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
        unsigned int reloc_slot;
        unsigned int reloc_shift;
        const kword_t *image;
        const kword_t *map;
        const kword_t *reloc_map;
        kword_t *dst;
        kword_t reloc_bits;
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
        /*
         * Each relocation-map word contains 18 two-bit entries, from bits
         * 35..34 downward.  Walk that packed stream sequentially instead of
         * calculating i/18 and i%18 for every image word.  Division is
         * particularly expensive on the PDP-6, and KCC does not combine the
         * quotient/remainder operations here.
         */
        reloc_map = map;
        reloc_slot = 18U;
        reloc_shift = 0U;
        reloc_bits = 0UL;
        for (i = 0U; i < init_words; ++i) {
                if (reloc_slot == 18U) {
                        reloc_bits = *reloc_map++;
                        reloc_slot = 0U;
                        reloc_shift = 34U;
                }
                word = image[i];
                code = (unsigned int)((reloc_bits >> reloc_shift) & 03UL);
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
                ++reloc_slot;
                if (reloc_shift != 0U)
                        reloc_shift -= 2U;
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

/**
 * @brief Resolve one exported MRES entry to its installed absolute address.
 *
 * Export offsets are packed two per PDP-10 word in the package header.  A zero
 * result denotes an invalid package, export index, or address overflow.
 *
 * @param package Source MRES package containing the export table.
 * @param base Installed package base address.
 * @param index Zero-based export index.
 * @return Absolute exported address, or zero if the request is invalid.
 */
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

/**
 * @brief Preserve the two-word Stage1-to-KINIT boot handoff.
 *
 * Stage1 leaves controller/root-selection information in fixed low memory.
 * KINIT copies it into private storage before low memory is reused by KCORE
 * and permanent packages.
 */
void
kinit_save_boot_handoff(void)
{
        volatile kword_t *boot0;
        volatile kword_t *boot1;

#ifdef KINIT_DEBUG
        KINIT_TRACE(KINIT_SAVE_BOOT_HANDOFF);
#endif
        /*
         * Stage1 leaves its two-word handoff in fixed low memory which KINIT
         * will shortly reuse.  Preserve it before further initialization can
         * overwrite those locations.
         */
        boot0 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD0;
        boot1 = (volatile kword_t *)(unsigned long)KINIT_BOOT_WORD1;
        kinit_boot_handoff[0] = *boot0;
        kinit_boot_handoff[1] = *boot1;
}

/**
 * @brief Return the MRES package associated with the MINIT currently executing.
 *
 * MINIT routines use this to install or export their own resident package
 * without embedding package addresses in each individual initializer.
 *
 * @return Current package address, or NULL when no MINIT package is active.
 */
const kword_t *
module_current_mres(void)
{
        return module_mres_package;
}

/**
 * @brief Publish an installed module service address for later MINIT consumers.
 *
 * Service zero is reserved as "not available"; out-of-range service numbers
 * are ignored so callers cannot overwrite unrelated boot state.
 *
 * @param service Service-table index.
 * @param address Installed 18-bit service entry address.
 */
void
module_service_set(unsigned int service, unsigned int address)
{
        if (service != 0U && service < MODULE_SERVICE_COUNT)
                module_services[service] = address;
}

/**
 * @brief Look up a service address published by an earlier MINIT.
 *
 * Zero means the service is absent or the requested service number is invalid.
 *
 * @param service Service-table index.
 * @return Installed service address, or zero if absent/invalid.
 */
unsigned int
module_service_get(unsigned int service)
{
        if (service == 0U || service >= MODULE_SERVICE_COUNT)
                return 0U;
        return module_services[service];
}

/**
 * @brief Execute the linker-generated module initialization table in order.
 *
 * The table is first copied to the KINIT stack because its linked source area
 * is released to the memory manager before MINIT processing completes.  Each
 * MINIT may install an MRES and publish services.  MRES source packages are
 * linked in last-use order, allowing their transient source images to be
 * returned to free memory incrementally; this is required to keep boot viable
 * on the 32K PDP-6 profile.
 */
void
module_run_minits(void)
{
        /*
         * The MINIT table is deliberately bounded at 040 words (32 decimal).
         * Keeping the copy fixed-size avoids a dynamic allocation while the
         * permanent MRES block is still being assembled.
         */
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

/**
 * @brief Perform the complete transient kernel initialization sequence.
 *
 * This entry establishes KCORE and the boot memory map, preserves the Stage1
 * handoff, installs interrupt and module services, packs all permanent MRES
 * packages, allocates the permanent idle stack, mounts and validates the root
 * filesystem, initializes process state, and finally transfers to the late
 * handoff that starts normal operation and reclaims KINIT.
 *
 * Ordering is significant: permanent MRES placement must finish before any
 * movable dynamic kernel allocation is allowed into the low-memory gap, and
 * the KINIT image/bootstrap stack cannot be reclaimed until all boot-only code
 * has finished using them.
 */
void
kinit_enter(void)
{
        unsigned int memory_kwords;
        kword_t kernel_stack_base;
        kword_t reclaim_end;

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

                /*
                 * kinit_memory_kwords() reports 1024-word units.  The PDP-6
                 * allocator and all linker addresses are expressed in words.
                 */
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
#if KINIT_FULL
        dtfs_post_minits();
        auxstore_post_minits();
#endif
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

                /*
                 * Fill unused idle-stack words with their own addresses.
                 * The process-stack watermark code later detects the first
                 * overwritten sentinel without needing a separate bitmap.
                 */
                idle_stack = (kword_t *)(unsigned long)kernel_stack_base;
                for (i = 1U; i < (unsigned int)KERNEL_IDLE_STACK_WORDS; ++i)
                        idle_stack[i] = kernel_stack_base + (kword_t)i;
        }
#endif
        /*
         * SYS_MEMINFO uses a patched immediate for permanent resident words.
         * The permanent range starts at KINIT_KCORE_BASE and ends at the first
         * word after the packed MRES block.
         */
        sys_resident_words_immediate =
            (sys_resident_words_immediate & ~((kword_t)KINIT_HALF_MASK)) |
            (kword_t)(mres_next_addr - KINIT_KCORE_BASE);
        /*
         * KINIT's linked image and bootstrap stack are reclaimed together.
         * Their physical end cannot change after this point, so compute and
         * validate that boundary once and carry it to the late handoff.
         */
        reclaim_end = (kword_t)(unsigned long)&__kinit_image_end +
            KINIT_STACK_RESERVE_WORDS;
        if (reclaim_end > mm_core_words)
                kinit_halt();
#ifdef KINIT_DEBUG
        kinit_diag_finished();
#endif
        kinit_boot();
#if KINIT_STACK_WATERMARK
        kinit_stack_watermark_record();
#endif
        if (proc_boot_init() != 0)
                kinit_halt();
#if KINIT_STACK_WATERMARK
        kinit_stack_watermark_record();
#endif
        kinit_late_handoff(kernel_stack_base, reclaim_end);
        kinit_halt();
}
