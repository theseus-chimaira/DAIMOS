/**
 * @file syscall_info.s
 * @brief Small permanent KCORE ABI for pinned SYSINFO MRES.
 *
 * MINIT patches the first two one-word entries after installing the pinned
 * package; the services are not callable before installation. MEMFS uses a
 * separate patchable jump so its absence still reports unavailable stats.
 * The resident size value is data (not a patched instruction immediate),
 * written once by KINIT before any userspace process can call MEMINFO.
 */
        .text
        .globl  kret_neg1
        .globl  sys_procinfo
        .globl  sys_meminfo
        .globl  sys_memfs_usage_call
sys_procinfo:
        jrst    kret_neg1
sys_meminfo:
        jrst    kret_neg1
sys_memfs_usage_call:
        jrst    kret_neg1

        .data
        .globl  sys_resident_words_immediate
sys_resident_words_immediate:
        .word   0
