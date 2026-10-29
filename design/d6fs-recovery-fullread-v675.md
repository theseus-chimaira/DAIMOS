# D6FS recovery executable full-read probe v675

An explicit RO recovery mount now streams every word of the three required
regular files (`/SYSTEM/INIT`, `/SYSTEM/D6FSCK`, `/SYSTEM/MOUNT`) through
the normal VFS reader before authorizing bootstrap continuation. This
replaces the previous one-word accessibility probe and catches unreadable
later extents. The stream uses the already allocated 128-word KINIT block
scratch and has a bounded file-size/offset limit; no new data buffer or
resident storage is introduced.

This is a read-access check, **not** a checksum, DXR format validator, full
filesystem structural proof, or an executable recovery path on its own.
Native D6FSCK, exclusive repair access and validated remount still gate
production RO-first boot enablement.

Testkit v675/v673/v672/v671 and historic superblock checks PASS. PDP-6 GCC
KINIT source assembly generation PASS. Full-system DIRTY-root integration
remains untested; normal boot startup still requests its previous mount mode.
