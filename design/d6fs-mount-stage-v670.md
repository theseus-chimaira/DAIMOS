# Native KINIT D6FS mount-stage diagnosis v670

The v669 full-system scheduler-pressure image stops with `?RT` before
the workload starts. The fault is emitted by `kinit_boot()` because
`d6fs_boot_mount_root()` returns failure.

An isolated SIMH breakpoint sequence from the **linked KINIT map**
confirms entry to `blockset_boot_discover`, repeated
`blockset_boot_read`, and `d6fs_super_select`, followed by
`kinit_error18`. Neither `vfs_mount` nor `vfs_stat` is reached.
The versioned testkit v670 diagnostic reproduces that sequence and
reports classification without claiming a successful mount or swap.

This narrows the failure to superblock selection or the subsequent
pre-VFS checks. The precise failing condition is **not yet known**.
Do not change production D6FS policy until the return value and media
fields have been captured. The v664-v668 isolated BACKING regressions
remain valid, but scheduler fault-atomicity acceptance is blocked.
