#include "kboot_v1.h"
#include "memfs_v1.h"
#include "mach_user_v1.h"
#include "proc_v1.h"
#include "sched_v1.h"

kword_t kcore_resident_end_v1;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;

struct memfs_v1 kboot_fs_v1;
void
kcore_boot_v1(void)
{
        (void)sched_v1_run_once(mach_enter_user_v1);
}
