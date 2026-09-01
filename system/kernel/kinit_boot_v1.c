#include "kinit.h"
#include "kboot_v1.h"
#include "exec_v1.h"
#include "mach_user_v1.h"
#include "proc_v1.h"
#include "d6fs_disk_v2.h"
#include "d6fs_boot_v2.h"
#include "dsk270.h"
#include "d6log_v2.h"
#include "d6fs_provider_v2.h"

#define KBOOT_LOG_SEVERITY_INFO  6U
#define KBOOT_LOG_SOURCE_BOOT    1U

void
kinit_boot_v1(void)
{
        struct proc_v1 *p;
        struct d6log_v2 boot_log;
        vnode_v1_t d6fs_root;
        int d6fs_rc;

        if (dsk270_read_addr_v1 != 0U) {
                struct d6fs_dsk_v2 *boot_disk;
                kword_t *scratch;
                kword_t payload[1];

                d6fs_rc = d6fs_boot_v2_mount_root(0U, &d6fs_root);
                if (d6fs_rc < 0)
                        return;
                boot_disk = d6fs_boot_v2_disk();
                scratch = d6fs_boot_v2_block_buffer();
                if (dsk270_write_addr_v1 != 0U &&
                    boot_disk->logstore_blocks >= 2UL) {
                        if (d6log_v2_recover(&boot_log, boot_disk,
                            scratch) == 0) {
                                payload[0] =
                                    VFS_V1_SIX6('B','O','O','T','/','R');
                                (void)d6log_v2_append(&boot_log,
                                    KBOOT_LOG_SEVERITY_INFO,
                                    KBOOT_LOG_SOURCE_BOOT, 0UL, payload, 1U,
                                    scratch);
                        }
                        /* LOGSTORE deliberately borrows the one-block D6FS
                         * cache as scratch.  Invalidate after every attempted
                         * log operation, including failed recovery. */
                        d6fs_boot_v2_cache_invalidate();
                }
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
