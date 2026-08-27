#include "kboot_v1.h"
#include "devicefs_v1.h"
#include "exec_v1.h"
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
static struct memfs_v1_node kboot_nodes_v1[KBOOT_V1_NODE_COUNT];
const struct memfs_v1_node *kboot_nodes_src_v2;
const kword_t *kboot_image_data_v2;
vnode_v1_t kboot_ramfs0_dir_v2;
kword_t kboot_fs_ready_v2;

static const kword_t kboot_init_path_v1[] = {
        12UL,
        VFS_V1_SIX6('/','S','Y','S','T','E'),
        VFS_V1_SIX6('M','/','I','N','I','T')
};

void
kcore_boot_v1(void)
{
        struct proc_v1 *initp;
        struct vfs_v1_name temp_name;
        unsigned int owner;

        unsigned int i;

        if (kboot_fs_ready_v2 == 0 || kboot_nodes_src_v2 == 0 ||
            kboot_image_data_v2 == 0)
                return;
        for (i = 0U; i < KBOOT_V1_NODE_COUNT; ++i) {
                unsigned int j;
                for (j = 0U; j < VFS_V1_NAME_WORDS; ++j)
                        kboot_nodes_v1[i].name.words[j] =
                            kboot_nodes_src_v2[i].name.words[j];
                kboot_nodes_v1[i].name.chars = kboot_nodes_src_v2[i].name.chars;
                kboot_nodes_v1[i].meta = kboot_nodes_src_v2[i].meta;
                kboot_nodes_v1[i].size_chars = kboot_nodes_src_v2[i].size_chars;
                kboot_nodes_v1[i].data = kboot_nodes_src_v2[i].data;
        }
        kboot_fs_v1.nodes = kboot_nodes_v1;
        kboot_fs_v1.node_count = KBOOT_V1_NODE_COUNT;
        kboot_fs_v1.pool = (kword_t *)(unsigned long)KBOOT_V1_RAMFS0_BASE;
        kboot_fs_v1.pool_words = KBOOT_V1_RAMFS0_WORDS;
        kboot_fs_v1.used_words = 0U;
        kboot_fs_v1.writable = 1;
        kboot_fs_v1.image_data = kboot_image_data_v2;
        file_v1_init(&kboot_fs_v1);
        vfs_v1_name_set6(&temp_name,
            VFS_V1_SIX6('T','E','M','P',' ',' '), 4U);
        if (file_v1_alias_root(&temp_name, kboot_ramfs0_dir_v2) != 0)
                return;
        proc_v1_init();
        devicefs_v1_init(kcore_cty_putchar_v1 != 0 &&
            kcore_cty_getchar_v1 != 0 ?
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0) : 0);
        initp = proc_v1_alloc_init();
        if (initp == 0)
                return;
        owner = proc_v1_slot(initp);
        proc_v1_current = initp;
        if (exec_v1_load_init(initp, owner, kboot_init_path_v1,
            KBOOT_V1_USER_BASE, KBOOT_V1_USER_LIMIT) != 0)
                return;
        sys_v1_set_memory_bounds(KBOOT_V1_TOTAL_WORDS,
            kcore_resident_end_v1);
        (void)sched_v1_run_once(mach_enter_user_v1);
}
