/**
 * @file exec.c
 * @brief Resident EXEC image-replacement policy and transactional handoff.
 *
 * The compact target loader lives in exec_load.s; this C file implements the
 * higher-level EXEC V1 replacement transaction.  A replacement image is fully
 * validated, loaded into a staged VM, and given startup records before the old
 * VM is destroyed, so ordinary validation/load failures leave the calling
 * process executable.  Process identity, stable u-area state, swap backing,
 * and real-time ownership are preserved or transferred explicitly.
 *
 * DAIMOS kernel sources target the PDP-6/PDP-10 family only; there is no host
 * fallback implementation in this file.
 */
#include "exec.h"
#include "vm.h"
#include "proc_swap.h"
#include "syscall.h"

/* Target exec_load_process() and sixbit_record_words() are implemented in
 * exec_load.s so KCORE does not carry the larger compiler-generated bodies. */
extern unsigned int sixbit_record_words(const kword_t *record, int nonempty);




/**
 * @brief Replace the current user image while preserving process identity.
 * @param block Mapped EXEC V1 launch block in the current user image.
 * @param available_words Words accessible from block to the end of its mapping.
 * @param entry_startup Receives entry PC/SP and startup argument registers.
 * @return Zero on success, -1 without committing a replacement on failure.
 *
 * EXEC V1 uses the same bounded counted-SIXBIT startup records as RUN V2.  The
 * new VM and startup data are completed before the old VM is destroyed.  Swap
 * backing and RT ownership are staged/rolled back so validation and load errors
 * leave the caller's old image runnable.
 */
int
exec_replace_current(const kword_t *block,
    unsigned int available_words, kword_t *entry_startup)
{
    const struct sys_exec_v1 *args =
        (const struct sys_exec_v1 *)block;
        struct proc staged;
        struct proc *current;
        const kword_t *path;
        const kword_t *records;
        const kword_t *end;
        const kword_t *scan;
        kword_t startup[4];
        kword_t old_swap;
        kword_t new_swap;
        kword_t counts;
        unsigned int slot;
        unsigned int words;
        unsigned int argc;
        unsigned int envc;
        unsigned int i;
        int load_result;
        int old_rt_owner;

        if (available_words < SYS_EXEC_V1_MIN_WORDS)
                goto invalid;
        if ((unsigned int)((args->version_words >> 18U) & PROC_HALF_MASK) !=
            SYS_EXEC_VERSION_1)
                goto invalid;
        words = (unsigned int)(args->version_words & PROC_HALF_MASK);
        if (words < SYS_EXEC_V1_MIN_WORDS || words > available_words)
                goto invalid;
        if ((args->argc & ~PROC_HALF_MASK) != 0UL ||
            (args->envc & ~PROC_HALF_MASK) != 0UL)
                goto invalid;
        argc = (unsigned int)args->argc;
        envc = (unsigned int)args->envc;
        if (argc > SYS_RUN_ARG_MAX || envc > SYS_RUN_ENV_MAX)
                goto invalid;

        end = (const kword_t *)args + words;
        path = &args->path[0];
        i = sixbit_record_words(path, 1);
        if (i == 0U || path + i > end)
                goto invalid;
        records = path + i;
        scan = records;
        for (i = 0U; i < argc + envc; ++i) {
                unsigned int record_words;

                if (scan >= end)
                        goto invalid;
                record_words = sixbit_record_words(scan, 0);
                if (record_words == 0U || scan + record_words > end)
                        goto invalid;
                scan += record_words;
        }
        if (scan != end)
                goto invalid;

        slot = (unsigned int)proc_current_slot;
        current = &proc_table[slot];

        old_swap = proc_swap_records[slot].state;
        old_rt_owner = ((unsigned int)proc_rt_owner == slot);
        staged.meta = current->meta;
        load_result = exec_load_process(&staged, slot, path);
        if (load_result < 0)
                goto restore_swap_fail;
        counts = ((kword_t)argc << 18U) | (kword_t)envc;
        if (vm_space_startup(&staged, records, counts, startup) != 0) {
                (void)vm_space_destroy(&staged, slot);
                if (!old_rt_owner && load_result == EXEC_LOAD_RT_REQUIRED &&
                    (unsigned int)proc_rt_owner == slot)
                        proc_rt_owner = 0UL;
                goto restore_swap_fail;
        }
        new_swap = proc_swap_records[slot].state;
        proc_swap_records[slot].state = old_swap;

        /* The mapped launch block is no longer referenced.  Clear the hold
         * before freeing its containing VM; the failure path restores it. */
        PROC_CTL_WORD(current) &= ~PROC_USER_MAP_BIT;
        if (vm_space_destroy(current, slot) != 0) {
                PROC_CTL_WORD(current) |= PROC_USER_MAP_BIT;
                (void)vm_space_destroy(&staged, slot);
                goto restore_swap_fail;
        }

        current->vm_state = staged.vm_state;
        proc_swap_records[slot].state = new_swap;
        PROC_SWAP_BACKING_WORD(current) = 0UL;
        entry_startup[0] = PROC_ENTRY(&staged);
        entry_startup[1] = startup[3];
        entry_startup[2] = startup[0];
        entry_startup[3] = startup[1];
        entry_startup[4] = startup[2];
        if (old_rt_owner && load_result == EXEC_LOAD_OK &&
            (unsigned int)proc_rt_owner == slot)
                proc_rt_owner = 0UL;
        return 0;

restore_swap_fail:
        if (!old_rt_owner && load_result == EXEC_LOAD_RT_REQUIRED &&
            (unsigned int)proc_rt_owner == slot)
                proc_rt_owner = 0UL;
        proc_swap_records[slot].state = old_swap;
invalid:
        return -1;
}
