# MonitorFS extraction into resident MRES: implementation boundary v13

Status: design analysis; no runtime ABI changed.

## Measured starting point

The GCC 32K fixed KCORE link ends at 023360 octal. The existing
monitorfs_runtime.dobj is 991 text + 39 data = 1030 decimal words before
folding. Its entire size is NOT removable: VFS has direct entry points and
runtime MonitorFS data is shared with the filesystem namespace.

## Current call and initialization paths

`fs/vfs.s` calls `mfsdev_lookup`, `mfsproc_lookup`, `mfsdev_readdir`,
`mfsproc_readdir`, `mfsdev_stat`, `mfsproc_stat`, `mfsproc_read_words`,
`mfsdev_readchar`, and `mfsproc_format_slot`. Domain operations are reached
through the process view. `module_table.s` currently records
`mfsdev_minit,,0` (no MRES package); `mfsdev_minit` in `module_minit.c`
publishes present device names after driver registration.

## Required design before moving code

1. Preserve the VFS ABI and stable MonitorFS vnode encoding.
2. Introduce a compact resident interface consisting of the existing VFS
   entry symbols forwarding to a boot-installed service table, or their
   equivalent direct patchable calls. Never dereference an absent service.
3. Put cold namespace enumeration, formatted statistics and domain reports
   in a nonremovable MonitorFS MRES package. The package must remain pinned
   while any VFS operation or open vnode can reach its code. Removable
   modules are not appropriate for a filesystem with persistent open handles.
4. Retain immutable/small shared MonitorFS data where lifetime requires it.
   Do not move process scheduler, exit/wait/fork or core MM into the package.
5. Extend the MINIT table entry from `mfsdev_minit,,0` to a real MRES package
   only after the package loader, jump patching and all early VFS consumers
   can be verified for the ordering used by both 32K and full profiles.
6. Test initial boot, the MonitorFS /DEV /PROC /MONITOR views, open handles,
   missing devices, and memory-pressure behavior in daimos-testkit.

## Acceptance

Compare fully linked GCC `__kcore_low_end` before and after extraction; the
object byte count alone is not a saving. Also report added MRES footprint,
fixed dispatch words and total permanently occupied core. No kernel overlays,
no hot-path process lifecycle indirection, and no silent feature removal.

The next patch should establish the dispatch/lifetime contract before any
MonitorFS functions are moved. Keeping the implementation unchanged until
that contract is tested avoids unsafe VFS calls into unloaded code.
