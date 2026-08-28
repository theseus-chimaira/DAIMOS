#include "syscall_v1.h"
#include "proc_v1.h"
#include "procfs_v1.h"
#include "kboot_v1.h"

int
sys_v1_procinfo(unsigned int slot, struct sys_v1_procinfo *info)
{
        struct proc_v1 *p;

        if (info == 0 || (p = proc_v1_get(slot)) == 0)
                return -1;
        info->pid = (kword_t)PROC_V1_PID(p);
        info->ppid = (kword_t)proc_v1_ppid(p);
        info->state = (kword_t)PROC_V1_STATE(p);
        info->words = PROC_V1_MEM_WORDS(p);
        info->comm = proc_v1_comm(p);
        return 0;
}



int
sys_v1_meminfo(struct sys_v1_meminfo *info)
{
        struct memfs_v1 *fs;
        unsigned int i;
        kword_t proc_words;
        kword_t proc_slots;

        if (info == 0)
                return -1;
        proc_words = 0;
        proc_slots = 0;
        for (i = 0U; i < PROC_V1_NPROC; ++i) {
                if (PROC_V1_STATE(&proc_v1_table[i]) == PROC_V1_FREE)
                        continue;
                ++proc_slots;
                proc_words += PROC_V1_MEM_WORDS(&proc_v1_table[i]);
        }
        fs = file_v1_root;
        info->total_words = KBOOT_V1_TOTAL_WORDS;
        info->resident_words = kcore_resident_end_v1;
        info->process_words = proc_words;
        info->ramfs_used_words = fs == 0 ? 0 : (kword_t)fs->used_words;
        info->ramfs_capacity_words = fs == 0 ? 0 : (kword_t)fs->pool_words;
        info->process_slots_used = proc_slots;
        info->process_slots_total = PROC_V1_NPROC;
        info->file_slots_used = file_v1_used_slots();
        info->file_slots_total = FILE_V1_NFILE;
        return 0;
}
