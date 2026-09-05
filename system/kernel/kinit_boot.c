#include "kinit.h"
#include "kboot.h"
#include "exec.h"
#include "mach_user.h"
#include "proc.h"
#include "d6fs_boot.h"
#include "dsk270.h"
#include "module.h"
#include "logstore.h"
#include "d6fs_provider.h"
#include "diskset_boot.h"

#define KBOOT_LOG_SEVERITY_INFO  6U
#define KBOOT_LOG_SOURCE_BOOT    1U

void
kinit_boot(void)
{
        struct proc *p;
        struct logstore boot_log;
        vnode_t d6fs_root;
        int d6fs_rc;

        if (module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) != 0U) {
                kword_t *scratch;
                kword_t payload[1];

                d6fs_rc = d6fs_boot_mount_root(0U, &d6fs_root);
                if (d6fs_rc < 0) {
                        kinit_put6((kword_t)SIXBIT("MNTERR"));
                        kinit_newline();
                        return;
                }
                if (kfs_boot_rebind_root() != 0) {
                        kinit_put6((kword_t)SIXBIT("MNTERR"));
                        kinit_newline();
                        return;
                }
                scratch = d6fs_boot_block_buffer();
                if (module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) != 0U &&
                    diskset_boot_log_blocks() >= 3UL) {
                        if (logstore_recover(&boot_log, scratch) == 0) {
                                payload[0] =
                                    VFS_SIX6('B','O','O','T','/','R');
                                (void)logstore_append(&boot_log,
                                    KBOOT_LOG_SEVERITY_INFO,
                                    KBOOT_LOG_SOURCE_BOOT, 0UL, payload, 1U,
                                    scratch);
                        }
                }
        }
        proc_table[0].meta =
            (kword_t)PROC_SRUN << PROC_STATE_SHIFT;
        p = &proc_table[1];
        {
                static const kword_t init_path[] = {
                        12UL,
                        VFS_SIX6('/','S','Y','S','T','E'),
                        VFS_SIX6('M','/','I','N','I','T')
                };

                if (exec_load_init(p, 1U, init_path, KBOOT_USER_BASE,
                    KBOOT_USER_LIMIT) != 0) {
                        return;
                }
        }
        mach_enter_user(PROC_MEM_BASE(p), PROC_ENTRY(p),
            PROC_MEM_WORDS(p) -
            (kword_t)EXEC_DXR_STACK_WORDS - 1U, 0, 0, 0);
}
