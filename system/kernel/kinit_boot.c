#include "kinit.h"
#include "kboot.h"
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

}
