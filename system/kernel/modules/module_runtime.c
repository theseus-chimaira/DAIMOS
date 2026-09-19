#include "module_runtime.h"
#include "fs_mres.h"
#include "kcore_pi.h"
#include "mres_reloc.h"

#define MODULE_BIND_OWNER_SHIFT 18U

kword_t module_runtime_descs[MODULE_RUNTIME_MAX + 1U];
kword_t module_dynamic_bindings[MODULE_DYNAMIC_BIND_MAX];

extern kword_t pdp10_pi_handlers[PDP10_PI_HANDLER_CAPACITY];
/* Fixed KCORE words whose RH is patched by MINIT to a module entry point. */
extern kword_t pdp10_pi_level1_dispatch_jump;
extern kword_t pdp10_pi_level2_dispatch_jump;
extern kword_t pdp10_pi_level3_dispatch_jump;
extern kword_t pdp10_pi_level4_dispatch_jump;
extern kword_t pdp10_pi_level5_dispatch_jump;
extern kword_t pdp10_pi_level6_dispatch_jump;
extern kword_t native_sys_putchar_call;
extern kword_t native_sys_getchar_call;
extern kword_t storage_pi_dsk_jump;
extern kword_t storage_dct_dsk_jump;
extern kword_t storage_pi_tape_jump;
extern kword_t storage_dct_tape_jump;
extern kword_t storage_clock_dsk_jump;
extern kword_t dsk270_read_jump;
extern kword_t dsk270_write_jump;
extern kword_t drm236_read_jump;
extern kword_t drm236_write_jump;
extern kword_t fs_memfs_service_jump;
extern kword_t sys_memfs_usage_call;
extern kword_t fs_dtfs_service_jump;
extern kword_t sys_dtfs_format_jump;
extern kword_t sys_dtfs_mount_jump;
extern kword_t fs_d6fs_service_jump;
extern kword_t d6fs_cache_reclaim_jump;
extern kword_t diskset_runtime_service_jump;

static kword_t *const module_fixed_bindings[] = {
        &pdp10_pi_level1_dispatch_jump,
        &pdp10_pi_level2_dispatch_jump,
        &pdp10_pi_level3_dispatch_jump,
        &pdp10_pi_level4_dispatch_jump,
        &pdp10_pi_level5_dispatch_jump,
        &pdp10_pi_level6_dispatch_jump,
        &native_sys_putchar_call,
        &native_sys_getchar_call,
        &storage_pi_dsk_jump,
        &storage_dct_dsk_jump,
        &storage_pi_tape_jump,
        &storage_dct_tape_jump,
        &storage_clock_dsk_jump,
        &dsk270_read_jump,
        &dsk270_write_jump,
        &drm236_read_jump,
        &drm236_write_jump,
        &fs_memfs_service_jump,
        &sys_memfs_usage_call,
        &fs_dtfs_service_jump,
        &sys_dtfs_format_jump,
        &sys_dtfs_mount_jump,
        &fs_d6fs_service_jump,
        &d6fs_cache_reclaim_jump,
        &diskset_runtime_service_jump
};

#define MODULE_FIXED_BIND_COUNT \
        (sizeof(module_fixed_bindings) / sizeof(module_fixed_bindings[0]))

static void
module_retarget(kword_t *slot, int old_base, int new_base, int image_words)
{
        int address;

        address = (int)(*slot & MODULE_HALF_MASK);
        if (address >= old_base && address - old_base < image_words)
                *slot = (*slot & ~MODULE_HALF_MASK) |
                    (kword_t)(new_base + address - old_base);
}

int
module_runtime_move(unsigned int owner, unsigned int new_base,
    unsigned int total_words)
{
        kword_t d;
        int old_base;
        int image_words;
        int init_words;
        kword_t *src;
        kword_t *dst;
        const kword_t *map;
        int i;

        /* The MM direct-move path owns owner/state/range validation. */
        d = module_runtime_descs[owner];
        old_base = (int)MODULE_RUNTIME_BASE(d);
        init_words = (int)MODULE_RUNTIME_INIT_WORDS(d);
        image_words = (int)total_words -
            (int)MODULE_RUNTIME_MAP_WORDS(d);
        src = (kword_t *)(unsigned long)old_base;
        dst = (kword_t *)(unsigned long)new_base;
        /* MM holds PI disabled across the copy, relocation, publication,
         * and extent-descriptor rebase. */
        fs_move_words(src, dst, (unsigned int)total_words);
        /* Walk the two-bit map sequentially; division by 18 in the inner loop
         * is unnecessarily expensive on PDP-6. */
        map = dst + image_words;
        {
                int delta;
                kword_t bits;
                unsigned int slot;
                unsigned int map_index;

                delta = old_base - new_base;
                bits = map[0];
                slot = 0U;
                map_index = 0U;
                for (i = 0; i < init_words; ++i) {
                        unsigned int code;
                        kword_t word;
                        int half;

                        code = (unsigned int)((bits >> 34U) & 03UL);
                        bits <<= 2U;
                        if (++slot == 18U) {
                                slot = 0U;
                                ++map_index;
                                if (i + 1 < init_words)
                                        bits = map[map_index];
                        }
                        if (code == MRES_RELOC_NONE)
                                continue;
                        word = dst[i];
                        if ((code & MRES_RELOC_LH18) != 0U) {
                                half = (int)((word >> 18U) & MODULE_HALF_MASK);
                                word = (word & MODULE_HALF_MASK) |
                                    ((kword_t)(half - delta) << 18U);
                        }
                        if ((code & MRES_RELOC_RH18) != 0U) {
                                half = (int)(word & MODULE_HALF_MASK);
                                word = (word & ~MODULE_HALF_MASK) |
                                    (kword_t)(half - delta);
                        }
                        dst[i] = word;
                }
        }

        for (i = 0; i < (int)MODULE_FIXED_BIND_COUNT; ++i)
                module_retarget(module_fixed_bindings[i], old_base, new_base,
                    image_words);
        for (i = 0; i < (int)MODULE_DYNAMIC_BIND_MAX; ++i) {
                kword_t source;
                unsigned int source_owner;
                kword_t offset;
                int source_base;

                source = module_dynamic_bindings[i];
                if (source == 0UL)
                        break;
                source_owner = (unsigned int)((source >> 18U) & MODULE_HALF_MASK);
                offset = source & MODULE_HALF_MASK;
                if (source_owner == owner)
                        source_base = new_base;
                else
                        source_base = (int)MODULE_RUNTIME_BASE(
                            module_runtime_descs[source_owner]);
                module_retarget((kword_t *)(unsigned long)(source_base + offset),
                    old_base, new_base, image_words);
        }
        for (i = 0; i < (int)PDP10_PI_HANDLER_CAPACITY; ++i)
                module_retarget(&pdp10_pi_handlers[i], old_base, new_base,
                    image_words);
        module_runtime_descs[owner] =
            (d & ~MODULE_HALF_MASK) |
            ((kword_t)new_base & MODULE_HALF_MASK);
        return 0;
}
