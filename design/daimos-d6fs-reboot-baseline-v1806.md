# D6FS reused-media boot diagnosis v1806

Date: 2026-11-22. Status: baseline-confirmed, **not fixed**.

## Physical evidence

The same 256K PDP-6 DSK/disk-root simulator profile was
run twice without regenerating media, using separate copies
of the original CLEAN disk set for each kernel:

| Witness | v1803 main 495060f6 | v1804 integration e3e718e9 |
| --- | --- | --- |
| First boot | INIT V1, LOGIN | INIT V1, LOGIN |
| First exit | controlled timeout 124 | controlled timeout 124 |
| D6FS after first boot | DIRTY, seq 2, block 743 | DIRTY, seq 2, block 743 |
| Second boot | ?RT, HALT 074262 | ?RT, HALT 074301 |
| Second exit | HALT, simulator rc 0 | HALT, simulator rc 0 |

The v1803 evidence directory is
`$TMPDIR/daimos-v1806-baseline-reboot.gAjlLB`; the v1804
repeat used `$TMPDIR/daimos-v1805-smoke.P5sGbV`.
These are local transient logs, not committed binary media.
The second-boot halt addresses differ because the linked
images differ; both emit the same KINIT `?RT` code.

## Source diagnosis

`system/kernel/boot/kinit_boot.c` calls
`d6fs_boot_mount_root(0U)` in both revisions. In
`system/kernel/boot/d6fs_boot.c`, the default writable
mount uses `d6fs_super_select()` (CLEAN only). Only an
explicit `VFS_MOUNT_RDONLY` mount may use
`d6fs_super_select_recovery()` to inspect a structurally
valid DIRTY generation. The runtime publishes a new
DIRTY superblock when a writable mount begins; the first
boot does not cleanly unmount before the test timeout.
On restart, the normal writable boot therefore rejects
the DIRTY root and KINIT emits `?RT`.

This is a **pre-existing D6FS recovery gap**, not a new
RAW PURE integration regression and not a K01-K04 reopening.
The RO-first recovery/repair/remount chain remains staged
and is not production-enabled. The code explicitly forbids
accepting an arbitrary DIRTY filesystem for writable boot.
Do not bypass that invariant or rewrite the superblock to
CLEAN as a workaround. Repair must provide the missing
exclusive checker, safe metadata validation, explicit
remount authorization, and successful physical recovery
tests. Existing D6FS design documents v671-v678 cover
these prerequisites.

## Integration consequence

Clean-media v1805 physical boot smoke remains PASS.
K05-K09 may proceed once branch consolidation's separate
RAW PURE and owner/pressure gates are satisfied. A
DIRTY-root reboot is tracked under D6FS recovery rather
than being falsely attributed to the consolidation merge.
