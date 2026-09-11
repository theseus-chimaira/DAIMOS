#include "kinit.h"
#include "kboot.h"
#include "d6fs_boot.h"
#include "module.h"

void
kinit_boot(kword_t future_free_words)
{
        int d6fs_rc;

        if (module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) != 0U) {
                d6fs_rc = d6fs_boot_mount_root(0U);
                if (d6fs_rc < 0) {
                        kinit_put6((kword_t)SIXBIT("MNTERR"));
                        kinit_newline();
                        return;
                }
                if (kfs_boot_mount_ramfs(future_free_words) != 0) {
                        kinit_put6((kword_t)SIXBIT("MNTERR"));
                        kinit_newline();
                        return;
                }
        }
}
