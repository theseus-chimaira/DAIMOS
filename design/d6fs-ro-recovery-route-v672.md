# D6FS explicitly read-only recovery mount routing v672

The existing transient `d6fs_boot_mount_root(flags)` now selects the latest
structurally valid generation via `d6fs_super_select_recovery()` **only**
when `VFS_MOUNT_RDONLY` is set. In all other cases it retains the existing
CLEAN-only selector. The mount runtime already uses RDONLY to suppress
publication of a new DIRTY superblock. No new resident state is introduced.

This is a staged facility, **not** permission to launch production INIT from
an arbitrary DIRTY filesystem. A structurally valid superblock does not prove
that `/SYSTEM/INIT`, `/SYSTEM/D6FSCK`, `/SYSTEM/MOUNT`, directory indexes,
FCBs, or their file block mappings are safe to traverse. Recovery bootstrap
must validate those paths and fail closed on unsafe references.

KINIT still passes zero flags to `d6fs_boot_mount_root` until the complete
native checker, exclusive repair path, explicit-target remount and early-INIT
success gate are present. Do not change that call in isolation: production
startup currently expects a writable root.

Verification: testkit `daimos-d6fs-ro-recovery-route-v672.sh` PASS; the v671
recovery selection and original CLEAN-only regression PASS. PDP-6 GCC full
profile KINIT assembly generation PASS with no new compile error. Full-system
DIRTY-root read/repair/remount acceptance is still outstanding.
