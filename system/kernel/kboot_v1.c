#include "kboot_v1.h"
#include "exec_v1.h"
#include "memfs_v1.h"
#include "mach_user_v1.h"
#include "proc_v1.h"
#include "d6fs_disk_v2.h"

kword_t kcore_resident_end_v1;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;
kword_t kcore_dsk_read_sector_v1;
kword_t kcore_dsk_write_sector_v1;

struct memfs_v1 kboot_fs_v1;
void
kcore_boot_v1(void)
{
        struct proc_v1 *p;

        vnode_v1_t d6fs_root;
        int d6fs_rc;

        if (kcore_dsk_read_sector_v1 != 0UL) {
                d6fs_rc = d6fs_dsk_v2_mount_boot_root(
                    (unsigned int)kcore_dsk_read_sector_v1,
                    (unsigned int)kcore_dsk_write_sector_v1, 0U, &d6fs_root);
                if (d6fs_rc < 0)
                        return;
        }
        proc_v1_table[0].meta =
            (kword_t)PROC_V1_SRUN << PROC_V1_STATE_SHIFT;
        p = &proc_v1_table[1];
        {
                static const kword_t init_path[] = {
                        12UL,
                        VFS_V1_SIX6('/','S','Y','S','T','E'),
                        VFS_V1_SIX6('M','/','I','N','I','T')
                };

                if (exec_v1_load_init(p, 1U, init_path, KBOOT_V1_USER_BASE,
                    KBOOT_V1_USER_LIMIT) != 0)
                        return;
        }
        mach_enter_user_v1(PROC_V1_MEM_BASE(p), PROC_V1_ENTRY(p),
            PROC_V1_MEM_WORDS(p) -
            (kword_t)EXEC_V1_DXR_STACK_WORDS - 1U, 0, 0, 0);
}
