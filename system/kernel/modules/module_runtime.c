/**
 * @file module_runtime.c
 * @brief Shared state for deferred runtime movable-module relocation.
 *
 * This file defines the descriptor/binding state consumed by the compact
 * PDP-6 relocation primitive in module_runtime_pdp6.s. The facility is not
 * currently wired into MM compaction; it remains groundwork for the deferred
 * post-overlay movable-module stage.
 */
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
extern kword_t sys_dtc_read_block_jump;
extern kword_t sys_dtc_write_block_jump;
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
extern kword_t sys_dtfs_mount_jump;
extern kword_t fs_d6fs_service_jump;
extern kword_t fs_tsfs_service_jump;
extern kword_t d6fs_cache_reclaim_jump;
extern kword_t memfs_reclaim_jump;
extern kword_t memfs_shutdown_jump;
extern kword_t blockset_runtime_service_jump;

static kword_t *const module_fixed_bindings[] = {
        &pdp10_pi_level1_dispatch_jump,
        &pdp10_pi_level2_dispatch_jump,
        &pdp10_pi_level3_dispatch_jump,
        &pdp10_pi_level4_dispatch_jump,
        &pdp10_pi_level5_dispatch_jump,
        &pdp10_pi_level6_dispatch_jump,
        &native_sys_putchar_call,
        &native_sys_getchar_call,
        &sys_dtc_read_block_jump,
        &sys_dtc_write_block_jump,
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
        &sys_dtfs_mount_jump,
        &fs_d6fs_service_jump,
        &fs_tsfs_service_jump,
        &d6fs_cache_reclaim_jump,
        &memfs_reclaim_jump,
        &memfs_shutdown_jump,
        &blockset_runtime_service_jump
};

#define MODULE_FIXED_BIND_COUNT \
        (sizeof(module_fixed_bindings) / sizeof(module_fixed_bindings[0]))
