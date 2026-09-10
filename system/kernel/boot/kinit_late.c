#include "exec.h"
#include "kinit.h"
#include "mach_user.h"
#include "mm.h"
#include "proc.h"
#include "vfs.h"

extern kword_t __kinit_late_begin;
extern kword_t __kinit_late_end;
extern kword_t __kinit_image_end;

/*
 * Finish boot from the only KINIT text which remains reserved after the main
 * bootstrap image is published to MM.  The final mm_add_free() deliberately
 * publishes the instructions which are still executing.  That is safe on the
 * PDP-6 because mm_add_free() changes only MM descriptors; PI is still off and
 * no allocator is called before mach_enter_user() transfers control to INIT.
 */
void
kinit_late_start(kword_t idle_stack_base, kword_t reclaim_end)
{
        struct proc *p;
        int init_slot;
        unsigned int slot;
        kword_t entry;
        kword_t stack;
        kword_t first_entry;
        kword_t first_stack;
        kword_t late_base;
        kword_t late_end;
        kword_t image_end;
        kword_t init_path[3];

        late_base = (kword_t)(unsigned long)&__kinit_late_begin;
        late_end = (kword_t)(unsigned long)&__kinit_late_end;
        image_end = (kword_t)(unsigned long)&__kinit_image_end;
        if (late_base <= KINIT_IMAGE_BASE || late_end <= late_base ||
            image_end < late_end || reclaim_end <= image_end)
                return;
        if (mm_add_free(KINIT_IMAGE_BASE, late_base - KINIT_IMAGE_BASE) !=
            MM_OK ||
            (image_end > late_end &&
            mm_add_free(late_end, image_end - late_end) != MM_OK))
                return;

        init_path[0] = 12UL;
        init_path[1] = VFS_SIX6('/', 'S', 'Y', 'S', 'T', 'E');
        init_path[2] = VFS_SIX6('M', '/', 'I', 'N', 'I', 'T');

        proc_table[0].meta = 0UL;
        proc_table[0].mem_layout = 0UL;
        proc_table[0].sched = PROC_SCHED_DEFAULT;
        PROC_SET_STATE(&proc_table[0], PROC_SRUN);

        if (PROC_BOOT_USERS < 1 || PROC_BOOT_USERS >= PROC_MAX_SLOTS)
                return;
        first_entry = 0UL;
        first_stack = 0UL;
        for (slot = 1U; slot <= (unsigned int)PROC_BOOT_USERS; ++slot) {
                init_slot = proc_slot_claim(0U);
                if (init_slot != (int)slot)
                        return;
                p = &proc_table[slot];
                if (exec_load_init(p, slot, init_path) != 0)
                        return;
                PROC_SET_STATE(p, PROC_SRUN);
                entry = PROC_ENTRY(p);
                stack = PROC_MEM_WORDS(p) -
                    (kword_t)EXEC_DXR_STACK_WORDS - 1U;
                if (slot == 1U) {
                        first_entry = entry;
                        first_stack = stack;
                }
                if (proc_user_context_init(slot, entry, stack,
                    (kword_t)slot, 0UL, 0UL) != 0)
                        return;
        }

        p = &proc_table[1];
        proc_current_slot = 1UL;
        proc_sched_cursor = 1UL;

        /* Publish the permanent idle/exit stack only after late KINIT has
         * finished every operation which can allocate or enter VFS.  The
         * current KINIT reserve stack may then be returned to MM together with
         * this final code range: no allocator runs before the no-return user
         * transition, so the physically unchanged instructions and stack stay
         * safe until mach_enter_user() leaves them forever. */
        mach_kernel_stack_base = idle_stack_base;
        if (mm_add_free(late_base, late_end - late_base) != MM_OK ||
            mm_add_free(image_end, reclaim_end - image_end) != MM_OK)
                return;
        mach_enter_user(PROC_MEM_BASE(p), first_entry, first_stack,
            1UL, 0UL, 0UL);
}
