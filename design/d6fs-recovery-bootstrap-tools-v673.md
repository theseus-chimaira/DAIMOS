# D6FS recovery bootstrap tool accessibility gate v673

An explicitly requested RO D6FS bootstrap mount now checks the essential
recovery tools under `/SYSTEM`: `INIT`, `D6FSCK`, and `MOUNT`. For each it
requires a regular nonempty file and a successful first-word VFS read.
It uses the existing validated lookup/read routines, not raw on-disk
directory traversal. Failure rejects the RO mount and releases the mount.

This gate runs only on explicitly requested RO D6FS bootstrap mounts.
KINIT continues to request the normal writable path until the native
checker, secure repair I/O, INIT command sequence, and idempotent remount
authorization have passed full-system testing. No permanent KCORE storage
is added. The new functions reside in reclaimable KINIT.

Limitations: this first-word probe cannot prove the complete DXR image is
unmodified or that arbitrary DIRTY metadata is consistent. The executable
loader must continue validating the complete file, and the native checker
must not write through a mounted filesystem without exclusive raw I/O and
cache invalidation. No automatic repair is claimed.

Tests: `daimos-d6fs-recovery-tools-v673.sh` source check PASS, existing
v672/v671 and historical superblock regression PASS; PDP-6 GCC KINIT assembly
generation PASS. Native corrupted-media/boot acceptance remains outstanding.
