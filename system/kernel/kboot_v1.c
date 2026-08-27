#include "kboot_v1.h"
#include "devicefs_v1.h"
#include "file_v1.h"
#include "memfs_v1.h"
#include "mach_user_v1.h"
#include "proc_v1.h"
#include "sched_v1.h"
#include "syscall_v1.h"

kword_t kcore_resident_end_v1;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;

static struct memfs_v1 kboot_fs_v1;
struct memfs_v1_node kboot_nodes_v1[KBOOT_V1_NODE_COUNT];
const kword_t *kboot_image_data_v2;
vnode_v1_t kboot_ramfs0_dir_v2;
kword_t kboot_fs_ready_v2;
kword_t kboot_init_ready_v3;

void
kcore_boot_v1(void)
{
        if (kboot_fs_ready_v2 == 0 || kboot_image_data_v2 == 0 ||
            kboot_init_ready_v3 == 0)
                return;
        kboot_fs_v1.nodes = kboot_nodes_v1;
        kboot_fs_v1.node_count = KBOOT_V1_NODE_COUNT;
        kboot_fs_v1.pool = (kword_t *)(unsigned long)KBOOT_V1_RAMFS0_BASE;
        kboot_fs_v1.pool_words = KBOOT_V1_RAMFS0_WORDS;
        kboot_fs_v1.used_words = 0U;
        kboot_fs_v1.writable = 1;
        kboot_fs_v1.image_data = kboot_image_data_v2;
        file_v1_init(&kboot_fs_v1);
        if (file_v1_alias_root(0, kboot_ramfs0_dir_v2) != 0)
                return;
        devicefs_v1_init(kcore_cty_putchar_v1 != 0 &&
            kcore_cty_getchar_v1 != 0 ?
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0) : 0);
        sys_v1_set_memory_bounds(KBOOT_V1_TOTAL_WORDS,
            kcore_resident_end_v1);
        (void)sched_v1_run_once(mach_enter_user_v1);
}
