# KCORE v1514: non-root failed-sync ownership and clean unmount after return

Date: 2026-11-12. Source-linked native PDP-6 VFS feature check.
Production KCORE is unchanged; no resident code/data, ABI or
process status growth.

The selected v1513 policy treats confirmed loss of root or
BACKSTORE as terminal, but permits non-root filesystem recovery.
An ordinary non-root I/O failure must **not** clear a mount or
its cache/FILE ownership. In particular, a failed sync during
unmount cannot be treated as successful teardown.

Testkit v1514 source-links actual `vfs_unmount` and
`vfs_mount_retire_slot` from `system/kernel/fs/vfs.s` into a
PDP-6 instruction-execution oracle. Root D6FS mount ID 1
and secondary D6FS mount ID 2 are represented independently.
The secondary's sync callback returns -1 while media is
unavailable. The VFS unmount returns -1 and retains the
secondary root, target and RO policy; no provider preparation,
FILE unlock or mount retirement occurs. The root namespace
and original root mount remain intact.

When the *same* media becomes available, the next call through
the same real VFS code succeeds: sync is called again, D6FS
preparation is performed exactly once, FILE mount unlock runs
exactly once, and the secondary root/target/RO policy are
retired. The root namespace and mount ID 1 remain unchanged.
Three executed source mutations are rejected (ignored negative
sync, omitted slot retirement, omitted FILE unlock).

Run:

```
export DAIMOS_REPO=$HOME/git/DAIMOS
export PDP10_PREFIX=${PDP10_PREFIX:-$HOME/cross}
export TMPDIR=${TMPDIR:-$HOME/tmp}
sh $HOME/git/daimos-testkit/tests/validation/daimos-nonroot-unmount-retry-v1514.sh
```

All four native cases PASS. Testkit commit `34fab36`. This
test uses controlled callbacks, not actual media hot-unplug,
and its provider-preparation callback does not physically
revalidate superblocks or reclaim real BCACHE. It does not
prove removal of dirty data/open FILEs, invalidation of stale
cached replacement media, or safe remount of a *different*
filesystem. Those remain P1 feature gates. A missing medium
continues to be mounted until a real successful checked cleanup;
no forced unplug-time unmount is introduced.
