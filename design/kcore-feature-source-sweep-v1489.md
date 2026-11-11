# KCORE feature-only source and documentation sweep v1489

Date: 2026-11-11. Baseline: DAIMOS main after v1488 (`af2c4640`), with
the v1485 kernel fixes. This is a KCORE feature-completion inventory,
not a size-optimization proposal or an assertion of fresh simulator tests.

## Scope and interpretation

Production authority is `system/kernel/Makefile` plus reachable KCORE,
KINIT and directly invoked provider interfaces. A versioned source file
outside the production object lists is not an installed feature. Dated
TODO entries are historical *at the revision in their heading*; unchecked
boxes in older sections do not override subsequent accepted tests.
This sweep excludes independent MRES module development and runtime
unloading, userland features, online D6FS packing, unified TSFS, and
all size-only optimization. Necessary KCORE-to-KINIT and KCORE-to-userland
interface correctness is in scope. The historical 32K effort is on hold.

## Source and documentation observations

| Area | Current production authority | Finding |
| --- | --- | --- |
| KCORE assembly membership | `system/kernel/Makefile`, `KCORE_FIXED_ASM` | `exec.s`, `proc_run.s`, `proc_swap.s`, `vm_backend.s`, `backstore.s`, `bcache_pdp6.s`, `storage_router.s`, `d6block_fixed_v1458.s`, `d6block_v1458.s` belong to the KCORE build. |
| Process, scheduler, BRK/OOM | `proc/proc_pdp6.s`, `proc/proc_run.s`, `proc/exec_lowmem_pdp6.s`, `proc/vm_pdp6.s`, `proc/syscall_dispatch_pdp6.s` | Actual requester-only BRK OOM switch is `PROC_OOM_BRK_ENABLED_V1000,1`, superseding v1286's OFF claim. v1298 accepts 64/64 only for its exact 256K profile. |
| Compressed executable format | `core/d6block_vfs_fixed_v1458.s`, `proc/exec_load.s`, `proc/proc_pure_backing_pdp6.s`, KINIT Stage-1 inclusion | Canonical block decoder is in production, following v1464, rather than obsolete D6LZ36 staging. Keep versioned historical decoder files referenced by daimos-testkit. |
| XIP | `proc/exec_load.s`, `proc/exec_lowmem_pdp6.s`, `proc/proc_run.s`, `proc/vm_pdp6.s`, `fs/memfs_runtime.s`, `fs/memfs_data_pdp6.s` | Privileged root-owned nonwriting MEMFS raw XIP is live and accepted by v1469-v1485 128K/192K/256K evidence. Minimum MEMFS physical RAM remains 96K. |
| Physical swapping | `proc/proc_swap_pdp6.s`, `proc/proc_pure_backing_pdp6.s`, `storage/backstore.s`, `storage/backstore_entry_v484.s` | Ordinary disk/drum swap and checked BACKSTORE release are live; v1405/v1466 have distinct native/full-guest evidence. Swapless PURE drop is not linked. |
| Shared cache | `storage/bcache_pdp6.s`, `fs/d6fs_runtime.s`, `fs/dtfs_runtime.s` | D6FS and DTFS call the *same* clean two-entry BCACHE, using shared filesystem scratch plus a reclaimable 0201-octal-word private slab. Do not reimplement an old D6FS-specific cache. |
| Persistent snapshots/log I/O | `storage/logstore_runtime.s`, `boot/kinit.c`, `storage/logstore_boot.c`, `fs/memfs_snapshot_pdp6.s` | v1484 corrected KINIT's LOGSTORE direct physical-bridge ABI; v1485 fixed corrupt-snapshot admission rollback. Already passed the specified two-boot test matrix. |
| Mount capacity | `boot/root_select.c`, `boot/vfs_mount_capacity_v1.c`, `boot/vfs_mount_slab_v4.c`, `boot/kinit.c`, `fs/vfs.s` | KINIT validates the inventory, allocates six MM-backed mount arrays and publishes **dynamic** `vfs_mount_limit` last. The 26-slot representational/planner limit does not imply a live 26-volume test. |
| Mount enumeration ABI consumer | `fs/vfs.h`, `proc/syscall_dispatch_pdp6.s` (`native_sys_fsinfo`), `userland/exec/commands.c` | `VFS_NMOUNT=4` is a *legacy command scan limit*, not KCORE's bound. `MOUNT` and `DF` stop at four slots although FSINFO itself checks `vfs_mount_limit`. Out-of-range returns -1, the same result as errors, so blindly scanning 26 and stopping on error loses failure distinction. |
| Clock | `drivers/wall_clock.c`, `drivers/pclk.s`, `drivers/clk_io.s`, build `WALL_CLOCK_HZ` | Configured 50/60-Hz native SETTIME/TIME36 and races are tested (v608/v612). Actual physical 50-Hz rate calibration and complete device/scheduler timing agreement remain unaccepted, not an excuse to rewrite the working software clock. |

## Precisely outstanding KCORE feature and correctness work

### F1. Swapless inactive-process PURE reclamation (unimplemented integration)

`proc_pure_ram_v1381.c`, `mm_pure_rebase_v1381.s`,
`mm_pure_expand_v1384.s`, `proc_pure_restore_v1384.c`,
`proc_pure_ram_restore_v1391.c`, and `exec_image_common_v1386.s`
are isolated prototypes. They do not occur in the production KCORE
object lists. The live `proc_swap_pdp6.s` reloads executable text via
`proc_swap_reload_pure`, but does not schedule the v1381 drop/restore
transaction. Isolated success/rollback/strict-capacity checks up to
v1397 are valuable evidence, NOT full process-lifecycle acceptance.

Required implementation: opt into swapless mode explicitly; select only
inactive safe process states; preserve stable immutable backing identity;
rebase the pinned MM descriptor while retaining all mutable suffix data;
make every refusal ownership-atomic; reconstitute the full APR mapping,
entry/initialized image and saved mutable state before dispatch; integrate
process death, wait, EXEC replacement, provider unmount and MM pressure
cleanup. Preserve normal backed swap unchanged. Verify negative failure
injection and repeated scheduler/PI cycles on real target configurations,
with matched total RAM accounting. **Do not adopt v1487's +180-word
common-DXR pilot as if it were a saving.**

### F2. Shared storage fault and mixed-media gates (partial acceptance)

The allocator, cache and LOGSTORE direct bridge already exist. Close only
the missing distinct behavior: checked BACKSTORE bitmap/free refusal
through actual scheduled swap-in and termination/retry (v1405's source-
linked rollback is not this guest path); in-flight DEVICE/VOLUME mapper
loss and recovery without dispatching to the wrong raw device; BCACHE
scratch invalidation / media replacement / write-through failure with
both filesystem consumers; mixed root and auxiliary DSK/DRM/DTC layouts;
and simultaneous MEMFS persistence, LOGSTORE and process-swap ownership
without descriptor or backing-region overlap. Classify each test against
v1393/v1405/v1466/v1484/v1485 before adding it: accepted cases must not
be counted as missing or silently reimplemented. No second allocator,
cache, log ring or raw device API is warranted.

### F3. Mount capacity visible through the KCORE ABI (real interface gap)

`native_sys_fsinfo` indexes the dynamically published limit correctly,
but `commands.c`'s two legacy consumers only enumerate four mounts.
Define an unambiguous capacity/end-of-list mechanism, preserving error
reporting, or extend the existing FSINFO contract compatibly. Then update
only necessary command consumers and test mount identity/enumeration above
slot 4, with explicit boundary tests for identifiers 16 and 26 and a
small-inventory root. This is an interface feature, NOT a request to
replace the active MM-backed mount slab or to canonicalize FILE offsets.

### F4. Optional in-RAM mutable process-state compression (not production)

After F1 passes as uncompressed PURE-only reclamation, decide whether
zero-presence bitmap v1451 (30 isolated TEXT words), RLE v1376, or no
compression is preferable in actual target memory. Require exact source
and destination capacity checks, no RAM-expanding admission, bounded
encoding time, fault-safe rollback and FULL APR restoration headroom.
Measure *net* peak resident physical RAM, including code, temporary
buffers, alignment and pinned owner state, under actual simultaneous
process pressure. Never compress disk-swap records; that proposal was
cancelled in v1375. The obsolete `kpack_codec_v1` identity-copy stub
had no production or test consumers and is removed by this sweep.

### F5. Targeted KCORE-platform qualification (test gaps, not missing APIs)

Run the exact current-source grouped GCC/KCC native and real-guest
correctness gates for physical KINIT failure/abort, device timeout and
multi-source storage mixes as needed by F2/F3. The v1460 DTC/MTC Stage-1
and v1486 non-XIP 48K cold boots are accepted and should not be marked
open again. Native configurable 50/60-Hz clock tests are accepted;
physical 50-Hz scheduling/device timing remains conditional on access
to a real, independently qualified 50-Hz source. Preserve any future
`?RT` failure image before rebuilding; this is failure triage policy,
not a standing missing feature.

## Stale/duplicate cleanup and exclusions

1. Remove `core/kpack_codec_v1.[ch]`, an unlinked identity-copy
   placeholder (not a compressor, and no test/build consumer). Git
   history preserves the original experiment.
2. Correct `fs/vfs.h`'s misleading comment: KINIT no longer publishes
   exactly four mounts. Retain `VFS_NMOUNT` for its *live* legacy
   command/test consumers pending F3, rather than breaking builds by
   deleting the macro without its ABI/consumer migration.
3. Retain old versioned PURE and D6CODEC experiments while daimos-testkit
   names their exact files. None are production duplicates or occupy
   KCORE RAM; do not conflate source inventory with linked duplication.
4. Supersede, do not erase, historical v512, v1029, v1269, v1286,
   v1375 and v1395 unchecked items referring to missing D6 codec cutover,
   missing XIP, OOM OFF, compressed disk swap or old four-mount KINIT.
   The current v1488 reconciliation closes those claims. Perpetually
   nonreturning providers cannot be forcibly recovered with a safe
   general mechanism; retain the fail-closed policy rather than invent
   a generic forced-owner-free workaround.
5. Do **not** open MRES unload/reload (post-1.0), online D6FS packer,
   unified TSFS, new userland features, 32K execution, or any KCORE
   shrinking in this feature-only audit. Separate release planning
   may track them, but none should inflate this KCORE checklist.

## Verification / evidence limits

- Inspected the current production kernel Makefile, C and assembly paths,
  KCORE.MD, PROJECT-STATE.MD, TODO.MD, relevant DAIMOS design records,
  and the corresponding pdp10-doc KCORE, swapless, STORE, CLOCK, MM,
  BLOCKSET and LOGSTORE documents.
- Cross-checked the named staged sources against production Makefile
  dependencies and daimos-testkit references; checked unused identity
  symbols across DAIMOS and testkit source. No identical hashed kernel
  source files were found; distinct versioned revisions are not duplicates.
- Fresh isolated `make -C system/kernel kcore COMPILER=kcc` generated 69
  files and completed without a compiler error, using a versioned
  `$TMPDIR` build path. This is a *compile check*, not a final KCORE
  link or boot acceptance, and not a fresh rerun of the v1298/v1466/
  v1484/v1485/v1486 guest matrices.
- Source removals and comment-only header correction introduce no new
  production object dependency; linked runtime sizes are not claimed.

**Feature completion order:** F2 correctness/fault closures and F3
runtime mount interface, then F1 uncompressed scheduler restoration,
then the conditional F4 compression decision, followed by F5 cross-
regressions. Only *after* these does any KCORE size optimization begin.
