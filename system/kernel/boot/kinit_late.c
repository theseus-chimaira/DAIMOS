/**
 * @file kinit_late.c
 * @brief Final KINIT reclamation and transition to the initial user process.
 *
 * This file is linked into the protected tail of the transient KINIT image.
 * It runs after ordinary KINIT code and most embedded MRES source images have
 * become reclaimable.  Its job is to publish those remaining transient ranges
 * to MM, create the configured initial user processes, establish their minimal
 * console file state, switch the kernel to its permanent idle stack, release
 * the final KINIT code/stack reserve, and enter the first INIT process.
 *
 * The final release is intentionally unusual: kinit_late_start() publishes the
 * memory containing its own executing instructions.  This is safe because PI
 * remains disabled and no allocator runs between that publication and the
 * non-returning vm_enter_initial_user() transition.
 */

#include "exec.h"
#include "kinit.h"
#include "vm.h"
#include "mm.h"
#include "proc.h"
#include "vfs.h"
#include "file.h"
#include "monitorfs.h"

extern kword_t __kcore_load_end;
extern kword_t __kinit_late_begin;
extern kword_t __kinit_late_end;
extern kword_t __kinit_image_end;
extern kword_t cty_mres_package;
extern kword_t mres_source_end;
#if KINIT_STACK_WATERMARK
extern kword_t kinit_stack_highwater;
#endif

/**
 * @brief Finish bootstrap and enter the first INIT process.
 *
 * The linker divides transient KINIT storage into reclaimable prefix/source
 * ranges and this protected late tail.  The function validates those linker
 * boundaries before publishing any range to MM.  It then creates
 * PROC_BOOT_USERS copies of /SYSTEM/INIT, initializes their saved user
 * contexts and CTY descriptors, queues them runnable, and remembers the first
 * process's entry state for the final machine transition.
 *
 * Only after all operations which can allocate memory or enter VFS are done is
 * the permanent idle stack published as the kernel stack.  The late text and
 * bootstrap-stack reserve are then returned to MM.  No allocator may run after
 * that point: execution must proceed directly to vm_enter_initial_user().
 *
 * @param idle_stack_base Base of the permanent idle/exit kernel stack.
 * @param reclaim_end First word after the transient KINIT stack reserve.
 */
void
kinit_late_start(kword_t idle_stack_base, kword_t reclaim_end)
{
        struct proc *p;
        kword_t *uarea;
        unsigned int slot;
        kword_t entry;
        kword_t stack;
        kword_t first_entry;
        kword_t first_stack;
        kword_t first_argc;
        kword_t first_argv;
        kword_t first_envp;
        kword_t late_base;
        kword_t late_end;
        kword_t image_end;
        kword_t source_begin;
        kword_t source_end;
        kword_t init_path[3];
        kword_t startup[4];

        late_base = (kword_t)(unsigned long)&__kinit_late_begin;
        late_end = (kword_t)(unsigned long)&__kinit_late_end;
        image_end = (kword_t)(unsigned long)&__kinit_image_end;
        source_begin = (kword_t)(unsigned long)&cty_mres_package;
        source_end = mres_source_end;
        if (late_base <= KINIT_IMAGE_BASE || late_end <= late_base ||
            source_begin < late_end || source_end < source_begin ||
            image_end < source_end || reclaim_end <= image_end)
                return;
        if (mm_add_free((kword_t)(unsigned long)&__kcore_load_end,
            late_base - (kword_t)(unsigned long)&__kcore_load_end) != MM_OK ||
            (source_begin > late_end && mm_add_free(late_end,
            source_begin - late_end) != MM_OK) ||
            (image_end > source_end && mm_add_free(source_end,
            image_end - source_end) != MM_OK))
                return;

        init_path[0] = 12UL;
        init_path[1] = PDP10_SIX6('/', 'S', 'Y', 'S', 'T', 'E');
        init_path[2] = PDP10_SIX6('M', '/', 'I', 'N', 'I', 'T');

        proc_table[0].meta = 0UL;
        VM_SPACE_RESET(&proc_table[0]);
        proc_table[0].sched = PROC_SCHED_DEFAULT;
        PROC_SET_STATE(&proc_table[0], PROC_SRUN);

        if (PROC_BOOT_USERS < 1 || PROC_BOOT_USERS >= PROC_MAX_SLOTS)
                return;
        for (slot = 1U; slot <= (unsigned int)PROC_BOOT_USERS; ++slot) {
                if (proc_slot_claim(0U) != (int)slot)
                        return;
                p = &proc_table[slot];
                if (exec_load_process(p, slot, init_path) < 0)
                        return;
                PROC_SET_STATE(p, PROC_SRUN);
                entry = PROC_ENTRY(p);
                if (vm_space_startup(p, init_path,
                    (kword_t)1U << 18U, startup) != 0)
                        return;
                stack = startup[3];
                if (slot == 1U) {
                        first_entry = entry;
                        first_stack = stack;
                        first_argc = startup[0];
                        first_argv = startup[1];
                        first_envp = startup[2];
                }
                if (proc_user_context_init(slot, entry, stack,
                    startup[0], startup[1], startup[2]) != 0)
                        return;
                proc_runq_add(slot);
                PROC_SET_PGRP(p, 1U);
                uarea = (kword_t *)(unsigned long)PROC_UAREA_BASE(p);
                uarea[PROC_FDCTL_OFFSET] =
                    ((kword_t)1U << PROC_SESSION_SHIFT) |
                    ((kword_t)1U << PROC_DOMAIN_SHIFT);
                uarea[PROC_FILE_TABLE_OFFSET] =
                    VFS_NODE_PACKED(MONITORFS_DEVICE_PROVIDER, MONITORFS_KIND_DEVICE,
                    MONITORFS_DEV_CTY0) | FILE_META_READ;
                uarea[PROC_FILE_TABLE_OFFSET + 1U] = 0UL;
                uarea[PROC_FILE_TABLE_OFFSET + 2U] =
                    VFS_NODE_PACKED(MONITORFS_DEVICE_PROVIDER, MONITORFS_KIND_DEVICE,
                    MONITORFS_DEV_CTY0) | FILE_META_WRITE;
                uarea[PROC_FILE_TABLE_OFFSET + 3U] = 0UL;
                uarea[PROC_FILE_TABLE_OFFSET + 4U] =
                    VFS_NODE_PACKED(MONITORFS_DEVICE_PROVIDER, MONITORFS_KIND_DEVICE,
                    MONITORFS_DEV_CTY0) | FILE_META_WRITE;
                uarea[PROC_FILE_TABLE_OFFSET + 5U] = 0UL;
        }

        p = &proc_table[1];
        proc_current_slot = 1UL;
        proc_current_ptr = p;
        proc_sched_cursor = 1UL;

        /* Publish the permanent idle/exit stack only after late KINIT has
         * finished every operation which can allocate or enter VFS.  The
         * current KINIT reserve stack may then be returned to MM together with
         * this final code range: no allocator runs before the no-return user
         * transition, so the physically unchanged instructions and stack stay
         * safe until vm_enter_initial_user() leaves them forever. */
#if KINIT_STACK_WATERMARK
        {
                unsigned int used_words;

                used_words = kinit_stack_watermark_measure();
                if ((kword_t)used_words > kinit_stack_highwater)
                        kinit_stack_highwater = (kword_t)used_words;
        }
#endif
        mach_kernel_stack_base = idle_stack_base;
        if (mm_add_free(late_base, late_end - late_base) != MM_OK ||
            mm_add_free(image_end, reclaim_end - image_end) != MM_OK)
                return;
        vm_enter_initial_user(p, first_entry, first_stack,
            first_argc, first_argv, first_envp);
}
