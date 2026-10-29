# D6FS explicit-target internal remount v674

The historical `vfs_remount()` toggles the D6FS RO/RW state, which is not
safe as a retryable user-requested interface. The new internal
`vfs_remount_to(node, flags)` accepts only `VFS_MOUNT_RW` (0) and
`VFS_MOUNT_RDONLY` (1), rejects other providers and flag values, and checks
the current mount policy before deciding whether a transition is needed.
Repeating an already-satisfied request does not publish another superblock.

The existing transition mechanism and HALT semantics are unchanged; callers
of `vfs_remount()` are unaffected. This first increment adds a small KCORE
routine, so further RAM-size assessment is required before integrating the
userspace entry point. The wrapper is **not** an fsck authorization mechanism:
the future privileged syscall must check recovery completion, revalidate
on-disk media, and coordinate cache invalidation before invoking RW remount.

`daimos-d6fs-remount-target-v674.sh` source gate PASS. PDP-6 GCC/KCORE
assembly-object build (`vfs.dobj`) PASS. No end-to-end fsck/remount test yet.
