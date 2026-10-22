# KCORE legacy/compatibility audit v14 (2026-10-16)

## Confirmed and removed

- The old `vfs_mount_target[4]` and `vfs_mount_root[4]` permanent BSS arrays
  had no runtime readers after the v12 KINIT MM-table cutover. KINIT publishes
  both MM-backed bases before the first root mount; no MINIT code mounts VFS
  volumes. Both pointer words now start at zero and the eight obsolete BSS
  words and their exported symbols are gone.
- D6FS's early reader was stored twice: once in `d6fs_active_reader` (the
  `mm_alloc` destination) and once in `d6fs_reader_slots[0]`. KINIT now takes
  the boot reader directly from `d6fs_active_reader`; the redundant resident
  slot and its MINIT store instruction are gone. The MM-backed reader base
  remains initialized by KINIT before D6FS runtime dispatch.

Verified by a native disk-root SIMH ROOT login and ECHO diagnostic test (using
the repository's existing *diagnostic only* DSH size override). Disk and tape
permanent-size links pass. Net permanent KCORE+MRES saving from v13:
**nine 36-bit words**, not an estimated gross byte count. Disk-root
permanent last: `036723` -> `036712`; tape-root: `036071` -> `036060`.
GCC lowmem 32K KCORE end: `023540` -> `023527`. The separate 32K gate
`<020000` still fails by **1879 decimal words**; shell/DSH is irrelevant to
that gate. No 32K boot success is claimed.

## Not dead code (retain)

- `pdp10_pi_handler_return` is described as a compatibility name, but
  CTY/CLK/DCS/DPY/GE/SLV drivers still branch to it. It aliases an existing
  PI return entry, occupying **zero additional instruction words**. Deleting
  the name saves no KCORE memory and breaks live references.
- `/MONITOR` and the `/PROC` namespace alias remain part of the live VFS
  built-in namespace. Their names are not automatically removable as stale
  code; removing an alias changes observable filesystem behavior.
- The PDP-6 EXEC low-memory retry is a current out-of-memory recovery path.
  As of the 2026-10-22 v170 sweep, the unused C replacement source was
  deleted; `exec_lowmem_pdp6.s` and `exec_load.s` supply resident EXEC.
- DECtape TENEX/ITS branches in `dtfs_runtime.s` are gated or reside in the
  DTFS MRES package, **not** the always resident KCORE. Compatibility with
  non-native tape formats should be preserved unless deliberately removed
  as a feature; deleting inactive guarded source alone does not save RAM.
- Staged `vfs_mount_access_v5.s`, `vfs_parent_mount_v6.s` and the alternative
  policy C implementation are not linked into active KCORE. Their source
  files cost no resident words.

## Follow-up candidates, not yet safe to remove

- The remaining fixed four-entry provider tables (`dtfs_media`, TSFS shape
  and extent state) must be converted to the MM slab, then their duplicates
  can be removed. Do not shrink or remove them before all provider readers
  and writers are migrated, including MINIT handling.
- As of v16, the former four-slot `vfs_mount_ro` has been replaced with one
  36-bit `vfs_mount_policy` word (16 RO bits and two five-bit reservation
  owners). The staged v2 three-bitmap prototype remains unlinked. The
  resident policy word cannot be discarded, because it tracks live ownership.
- The pointer-based VFS hot paths currently cost more resident instructions
  than direct fixed indexed addressing. Profile and optimize those paths
  after the full slab cutover rather than reintroducing fixed arrays or
  assuming compatibility labels explain the 32K KCORE overrun.

This is a source-reference and linked-image audit, not a claim that every
assembly symbol has been proven reachable or dead through dynamic dispatch.
