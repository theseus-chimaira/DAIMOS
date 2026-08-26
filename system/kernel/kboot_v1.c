#include "kboot_v1.h"
#include "devicefs_v1.h"
#include "exec_v1.h"
#include "file_v1.h"
#include "initfs_v1.h"
#include "memfs_v1.h"
#include "mach_user_v1.h"
#include "proc_v1.h"
#include "sched_v1.h"
#include "syscall_v1.h"

const kword_t *kcore_initfs_image_v1;
kword_t kcore_initfs_words_v1;
kword_t kcore_resident_end_v1;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;

static struct memfs_v1 kboot_fs_v1;
static struct memfs_v1_node kboot_nodes_v1[KBOOT_V1_NODE_COUNT];

static const kword_t kboot_init_path_v1[] = {
        12UL,
        VFS_V1_SIX6('/','S','Y','S','T','E'),
        VFS_V1_SIX6('M','/','I','N','I','T')
};

void
kcore_boot_v1(void)
{
        struct proc_v1 *initp;
        struct vfs_v1_name mount_name;
        struct vfs_v1_name ramfs0_name;
        struct vfs_v1_name temp_name;
        vnode_v1_t mount_dir;
        vnode_v1_t ramfs0_dir;
        unsigned int owner;

        if (kcore_initfs_image_v1 == 0 || kcore_initfs_words_v1 == 0)
                return;
        if (initfs_v1_mount(&kboot_fs_v1, kboot_nodes_v1,
            KBOOT_V1_NODE_COUNT, kcore_initfs_image_v1,
            (unsigned int)kcore_initfs_words_v1) != 0)
                return;
        vfs_v1_name_set6(&mount_name,
            VFS_V1_SIX6('M','O','U','N','T',' '), 5U);
        if (memfs_v1_import_dir(&kboot_fs_v1, memfs_v1_root(&kboot_fs_v1),
            &mount_name, 0555U, 0, &mount_dir) != 0)
                return;
        vfs_v1_name_set6(&ramfs0_name,
            VFS_V1_SIX6('R','A','M','F','S','0'), 6U);
        if (memfs_v1_import_dir(&kboot_fs_v1, mount_dir, &ramfs0_name,
            0777U, 1, &ramfs0_dir) != 0 ||
            memfs_v1_attach_pool(&kboot_fs_v1,
            (kword_t *)(unsigned long)KBOOT_V1_RAMFS0_BASE,
            KBOOT_V1_RAMFS0_WORDS) != 0)
                return;
        file_v1_init(&kboot_fs_v1);
        vfs_v1_name_set6(&temp_name,
            VFS_V1_SIX6('T','E','M','P',' ',' '), 4U);
        if (file_v1_alias_root(&temp_name, ramfs0_dir) != 0)
                return;
        proc_v1_init();
        devicefs_v1_init(kcore_cty_putchar_v1 != 0 &&
            kcore_cty_getchar_v1 != 0 ?
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0) : 0);
        initp = proc_v1_alloc_init();
        if (initp == 0)
                return;
        owner = proc_v1_slot(initp);
        if (owner >= PROC_V1_NPROC)
                return;
        proc_v1_current = initp;
        if (exec_v1_load_init(initp, owner, kboot_init_path_v1,
            KBOOT_V1_USER_BASE, KBOOT_V1_USER_LIMIT) != 0)
                return;
        sys_v1_set_memory_bounds(KBOOT_V1_TOTAL_WORDS,
            kcore_resident_end_v1);
        sched_v1_init();
        (void)sched_v1_run_once(mach_enter_user_v1);
}
