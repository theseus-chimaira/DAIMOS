#include "syscall_v1.h"
#include "proc_v1.h"
#include "procfs.h"
#include "kboot_v1.h"

int
sys_v1_procinfo(unsigned int slot, struct sys_v1_procinfo *info)
{
        struct proc_v1 *p;

        if (info == 0 || slot >= PROC_V1_NPROC)
                return -1;
        p = &proc_v1_table[slot];
        info->pid = (kword_t)PROC_V1_PID(p);
        info->ppid = 0;
        info->state = (kword_t)PROC_V1_STATE(p);
        info->words = PROC_V1_MEM_WORDS(p);
        info->comm = slot == 0U ? VFS_V1_SIX6('S','W','A','P','P','E') :
            VFS_V1_SIX6('I','N','I','T',' ',' ');
        return 0;
}



int
sys_v1_meminfo(struct sys_v1_meminfo *info)
{
        if (info == 0)
                return -1;
        info->total_words = KBOOT_V1_TOTAL_WORDS;
        info->resident_words = kcore_resident_end_v1;
        info->process_words = PROC_V1_MEM_WORDS(&proc_v1_table[1]);
        info->ramfs_used_words = file_root != 0 ?
            (kword_t)file_root->used_words : 0UL;
        info->ramfs_capacity_words = file_root != 0 ?
            (kword_t)file_root->pool_words : 0UL;
        info->process_slots_used = PROC_V1_NPROC;
        info->process_slots_total = PROC_V1_NPROC;
        info->file_slots_used = file_used_slots();
        info->file_slots_total = FILE_V1_NFILE;
        return 0;
}
