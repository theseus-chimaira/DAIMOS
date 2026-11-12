# KCORE feature-completion source audit v1521

Date: 2026-11-12. DAIMOS committed baseline `65ed0b5b` (v1520);
separately observed local FSINFO v1492 changes are **uncommitted**.
Scope: fixed KCORE, necessary KINIT/installed-provider interfaces and
only the userland consumers needed to verify a KCORE ABI. Not MRES
unloading, general userland, online D6FS packing, unified TSFS,
compression/size optimization or strict 32K acceptance (on HOLD).
This is a source/documentation audit, not final release certification.

## Audit method and fresh verification

- Read `KCORE.MD`, `PROJECT-STATE.MD`, `TODO.MD`, the earlier
  v1489 source sweep, recent v1502-v1520 design/test records,
  pdp10-doc KCORE status, release audit and swapless documentation.
- Followed the live `system/kernel/Makefile` KCORE and MRES object lists
  rather than treating every versioned experiment as installed.
  Checked actual entry points, call sites, KINIT mount publication and
  original FSINFO/command loops; checked relevant daimos-testkit scripts.
- Fresh `make -C system/kernel kcore COMPILER=kcc` PASS (source-to-ASM
  generation, **not** an image link or physical guest). Log:
  `$TMPDIR/kcore-feature-source-compile-v1521.log`.
- With existing **uncommitted** FSINFO v1492 source in the worktree,
  `daimos-fsinfo-mount-limit-v1492.sh` PASS: source-linked native
  PDP-6 capacity query, rejection of its unsafe mutation, and execution
  of the actual MOUNTS/DF command bodies for 1/4/8/26 slots.
  This is **not** a committed or full-guest FSINFO acceptance.
- Neither this audit nor its documentation changes retests the
  historical physical v1466-v1520 acceptance; those results are
  reported only at the exact tested profiles in their original records.

## Accepted baseline (do not reopen)

| Area | Installed evidence / exact limitation |
| --- | --- |
| EXEC/RUN/WAIT, scheduler, ordinary backed swap and requester BRK OOM | `proc_run.s`, `exec_lowmem_pdp6.s`, `proc_swap_pdp6.s`, `syscall_dispatch_pdp6.s`; v1298 defined-profile 64/64 OOM and v1466 full 256K F07. |
| Grouped block codec | Actual `d6block_vfs_fixed_v1458.s`, Stage-1 and EXEC/PURE/TSFS call sites; accepted block-only v1466 guest. No further D6LZ36 migration is open. |
| Restricted MEMFS raw XIP and persisted snapshots | `exec_load.s`, `memfs_runtime.s`, `memfs_snapshot_pdp6.s`; v1469-v1485 restricted root-owned, nonwriting acceptance at 128/192/256K. MEMFS physical minimum is 96K. |
| Shared BCACHE and refused-release safety | `bcache_pdp6.s`, `d6fs_runtime.s`, `dtfs_runtime.s`, `backstore.s`; v1490/v1493/v1505 and v1518 tests. Do not add a second cache or allocator. |
| Failed BACKSTORE owner and independent progress | v1506 repaired owner TERM consumption; v1507 bounded owner-local retry; v1509 resident and v1510 swapped bystander; v1511 real DRM PI2 error and v1512 lost completion/PI6 timeout. Preserve DISK/u-area/pending TERM on irreparable refusal; do not repeat old open fairness/owner-return tasks. |
| Non-root D6FS return and mount failure ordering | v1513 actual source-linked transient secondary read failure and v1514 same-media unmount retry. These do not prove hot-swap/replacement. |
| Native DTFS mount and volume ID | v1518 invalidates directory/clean BCACHE on observed failed I/O; v1519 physically validates native RBN0144 on new mount; v1520 validates optional RBN0 `DTVOL1` and saves 72-bit ID in existing per-mount words. Legacy zero-ID media are admitted, not authenticated. |
| TSFS critical root membership and fatal leaf | v1516 exact physical member mask and v1515 no-return HALT leaf implemented. **No authoritative loss classifier or caller installed.** |
| Boot mount capacity | `boot/vfs_mount_capacity_v1.c`, `vfs_mount_slab_v4.c`, `kinit.c` publish dynamic `vfs_mount_limit` (up to the verified 26-slot planning limit). |
| Other accepted targeted profiles | v1486 actual non-XIP 48K boot; native 50/60-Hz clock software API. Neither is a 32K approval or physical calibrated 50-Hz device-timing result. |

## Outstanding required KCORE work, in dependency order

### P0-A: authoritative loss detection and critical-device dispatch

**Installed:** `proc/proc_storage_runtime_v30.s` defines
`kcore_critical_device_lost_v1515`; `fs/tsfs_runtime.s` defines
`tsfs_root_owns_dtc_unit_v1516`. Source-wide references show the
fatal leaf has **no production call site**. v1517 Type-551 SIMH
CONI/selection gives `000004` for unattached or detached DTC0, but
this is not authoritative hardware removal evidence. Type-136 I/O
errors and 300-tick PI6 watchdog expiration are equally insufficient.

**Open:** define a reliable positive permanent-device-loss signal or
validated independent identity/offline diagnosis for critical
boot-root and BACKSTORE, including selected multi-tape TSFS root
members (not every DTC unit). On confirmed loss alone call the
appropriate v1515 root or swap HALT reason. Prove native and physical
true-loss versus transient/illegal/unrelated-unit negative cases,
and exact HALT PC. The v1513 crash-only critical-owner policy
**cancels** older reconstruction, quarantine and forced-release plans.

### P0-B: mounted non-root removable-media identity and lifecycle

**Installed:** fresh DTFS mount format/ID verification and
observed-I/O-error invalidation, but `dtfs_runtime.s` only saves
the v1520 mount's two ID halves; there is no accepted post-error
mounted-ID comparison before cached data/open FILE reuse.

**Open:** validate RBN0 identity from real physical media after
observed error/return *before* re-enabling clean cached data or
old FILE/vnode operations; reject mismatched ID and handle legacy
zero-ID conservatively. Preserve mount ownership and other mounts,
define dirty write/partial update and outstanding FILE/vnode rules,
and prove same-media return versus changed/absent media plus
physical guest admission/rollback. Silent exchange without observed
I/O/event remains a separate detection limit; never promise that
the ID stamp alone solves it. Extend this policy to any relevant
non-root D6FS/other real media owners without inventing a duplicate
mount or cache system.

### P1-C: remaining mixed DEVICE/VOLUME and storage fault integration

**Installed:** direct DEVICE and mapped VOLUME routing
(`storage_router.s`, `fs_mres.s`, `blockset_boot.c`,
`auxstore_boot.c`), actual BACKSTORE/MM ownership, shared BCACHE,
and LOGSTORE/MEMFS snapshot bridge. Prior source/native tests cover
individual mapper paths, snapshot corruption, DTC cache errors,
and DRM failed swap reads, **not** the complete combined matrix.

**Open:** native and physical in-flight DEVICE/VOLUME provider loss,
multi-member mapping/return, cross-provider clean-cache invalidation,
write-through/dirty failure and descriptor-owner rollback; mixed
DSK/DRM/DTC boot root/auxiliary backing; and concurrent persistent
LOGSTORE/MEMFS snapshot and process swap without overlapping MM
descriptors, blocks, or stale mount identity. Cross-check v1484,
v1485, v1490 and v1511-v1514 first and do not retest already
accepted branches as new features. Classify true device loss
separately from transient controller failures.

### P1-D: dynamic FSINFO contract, consumer and real guest

**Installed committed:** dynamic KINIT slab and `vfs_mount_limit`;
existing FSINFO ordinary-slot path already checks this limit.
**Local only:** v1492 makes FSINFO AC2=-1 return capacity and
MOUNTS/DF iterate that capacity rather than `VFS_NMOUNT=4`.
The source-linked/native and host checks PASS this audit, but
the kernel/commands/header edits and three testkit files are
currently **uncommitted**, with no real linked multi-mount guest
acceptance. Do not overwrite those in-progress changes.

**Open:** reconcile/commit the existing ABI and testkit, validate
unpublished-capacity/error versus empty-slot distinctions,
noncontiguous/holes, small inventory, 1/4/8/26 mount capacities,
mount identities beyond slot four including limits 16/26, and
physical command scans with working provider failures. Only
the KCORE contract and necessary existing command consumers are
in scope; no unrelated userland work.

### P1-E: uncompressed swapless PURE inactive-owner integration

**Not installed:** `proc_pure_ram_v1381.c`,
`proc_pure_restore_v1384.c`, bounded full restoration
`proc_pure_ram_restore_v1391.c`, MM rebase helpers and
shared-DXR prototype do not occur in production KCORE link
membership. `proc_swap_reload_pure` is a real *backed* swap
reload and cannot be counted as swapless PURE.

**Open:** explicit opt-in; admit only safe inactive owners with
immutable original executable backing; zero-copy drop/rebase of
PURE text while pinning the mutable suffix; one full APR
reconstruction before scheduler dispatch with exact image/extent
bounds; atomic failures and pinned-owner cleanup under OOM,
EXEC/RUN/WAIT/EXIT, FS provider refusal/unmount and PI/retry.
Retain uncompressed raw DISK/DRM swap exactly; prove repeated
native and full physical guest pressure/owner/stack/MM tests.
Do not link the isolated v1487 shared-DXR +180-TEXT proposal
without a real need or a matched whole-system justification.

### Conditional after P1-E: optional mutable-only RAM compression

This is an explicit go/no-go decision, **not** a prerequisite for
uncompressed swapless PURE feature acceptance. v1451's small
bitmap and earlier RLE experiments are test-only. Adopt a codec
only with positive **total** RAM savings after including fixed
code, buffers, peak MM/APR restore reservation, fail-atomic
reconstruction and actual workload behavior. Otherwise record
NO COMPRESSION as complete. Compressed DISK-swap and the old
PACKED registry were cancelled by v1375; never reopen them.

### Final KCORE feature freeze / targeted acceptance

Re-run the affected native/real guest matrices on the **same**
committed sources and configurations: error/no-error, physical
mixed root/auxiliary provider recovery, FSINFO >4, swapless PURE
pressure and rollback, and no regressions in normal F07,
XIP, low-memory 48K and OOM. Independently qualify a real 50-Hz
hardware timing source only if that physical target is to be
claimed. This gate is evidence integration, not a new subsystem.

## Stale and duplicate entries resolved by this audit

- Two v1508 physical refusal summaries and two matching TODO
  checklists describe the same scoped test. Keep one precise
  historical record; later v1509-v1512 **close** its obsolete
  after-fault fairness and PI6 error/timeout tasks.
- v1502/v1503/v1505 unchecked physical TERM/owner-return and
  retry-bounded tasks have v1506/v1507-v1512 acceptance; do not
  repeat them in the active task list. Irrecoverable owner lifetime
  remains subject to P0-A/P1-C; no fake resource release.
- v1519's missing persistent DTFS ID was completed in v1520;
  **post-error reuse** is still open. v1488/v1502 claims about
  physical BACKSTORE acceptance must be read at their historical
  versions; later results supersede them.
- The old `P1 follow-up: restricted MEMFS XIP production
  integration` and bottom `P1: packed-process lifecycle and
  compression` checklist in KCORE.MD must not represent
  active KCORE tasks: v1469-v1485 accepted XIP, v1375 cancelled
  PACKED disk swap. D6LZ36 migration and 48K non-XIP boot also
  completed. Dated experimental research stays archived.
- v1492 FSINFO source remains uncommitted: do **not** preemptively
  check off physical F3 acceptance. Dynamic KINIT itself is
  already installed; do not restore `VFS_NMOUNT=4`.
- 32K strict gate remains **HOLD** and `VERSION` is still 0.5.
  0.9 online D6FS packer, unified TSFS, MRES unload/reload
  (post-1.0), unrelated userspace and all size optimization
  are outside this feature checklist.

**Current work order:** P0-A and P0-B (media correctness), P1-C
(genuine remaining mixed faults), P1-D (finish existing F3),
P1-E (uncompressed swapless PURE), optional compression decision,
then matched cross-feature regression and feature freeze.
