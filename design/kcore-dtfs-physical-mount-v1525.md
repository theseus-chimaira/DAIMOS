# KCORE v1525: source-correct native DTFS mount, TSFS path walk, physical guest

Date: 2026-11-13. This is **KCORE feature completion**, not an
optimization, module-policy expansion or a new native filesystem
format. VERSION stays 0.5 and the strict 32K gate stays HOLD.

## Why the older physical test could not accept a real mount

The corrected v1524 SIMH first-RBN driver read/write test removed the
physical RBN0 transport defect. The v1525 full-guest investigation
then found two independent production source bugs and one obsolete
test prerequisite, using an actual 256K tape-selected KCORE boot
with optional DTFS MRES **LOADED**.

1. **Native directory index.** The native DTFS allocation table
   reserves physical block `0144` in C `dir[14]` (decimal).
   The old `dtfs_mount_unit` checked assembly
   `fs_block_workspace+014` (octal offset 12 decimal), which
   was zero on genuine media. Correct code checks `+016`
   (14 decimal). Host `mkdtfs`, target `MKFS.DTFS`, and the
   C `dtfs_probe` agree on C index 14; the old v1519 source-linked
   test had reproduced the same erroneous assembly index in its
   mock directory. The v1519 test is corrected and rejects a
   new executable old-index mutation.
2. **TSFS parent return polarity.** `tsfs_parent_common`
   returns the nonzero parent vnode in AC1 on success and a
   negative value on error. Both `tsfs_parent` and
   `tsfs_parent_name` incorrectly used `JUMPN` to branch
   to the error exit, rejecting all actual nonzero parents.
   Both now use `JUMPL`. This is required for the
   real `vfs_parent_name` path walk to reconstruct
   `/MOUNT/DT0` for `GETCWD` and `FSINFO`. A new
   source-linked native PDP-6 test rejects each original
   polarity as an executable unsafe mutation.
3. **Stale installed host tool.** The previously installed
   `mkdtfs` binary still emitted the legacy zero RBN0,
   despite current `tools/host/mkdtfs.c` already writing
   a fresh `DTVOL1` identity. Rebuilding that unchanged
   source as `mkdtfs-v1525`, packaging in versioned
   `local.tar.gz`, and installing under `PDP10_PREFIX`
   produced a verified stamped record with both nonzero
   independent 36-bit identity halves and complement.
   The old binary is retained as `mkdtfs-v1524-prestamp`.
   The physical guest test rejects legacy or malformed
   host media before starting the simulator.

None of these changes adds a KCORE or optional-MRES instruction
or an allocated persistent word. The new optional DTFS/TSFS
MRES image is still octal `003746` text / `000002` BSS,
matching v1524, with the same folding and other objects.

## Physical test design

The original DTC-selected root DSH/external-`MOUNT.DTFS`
acceptance was repeatedly dominated by full TSFS root-media
executable loading, with hundreds of Type-551 DTC0 commands
and no secondary DTC3 call before its timeout.
For a controlled physical KCORE test, daimos-testkit now builds
a **disposable minimal TSFS root** from actual manifest records,
retaining KINIT and the normal physical root module selection,
but launching one short KCC user executable as real PID 1.
This executable invokes the **production** `dsys_dtfs_mount`,
`dsys_fsinfo`, `dsys_stat`, `dsys_chdir`, `dsys_getcwd`,
`dsys_unmount` and FILE syscalls, without mocking any
resident FS, VFS, controller or physical DTC transfer.
It does not change the installed production INIT or modules;
only a disposable test-only TSFS root manifest points
`/SYSTEM/INIT` to the gate executable.

Required evidence:

- KINIT actually prints `DTFS LOADED`, not merely a
  linker-created optional package.
- A genuinely stamped DTC3 RBN0 and native directory
  are physically validated at mount admission.
- `DTFSGATE V1525 MOUNTED` follows the real syscall
  return rather than a STAT of a pre-existing TSFS directory.
- FSINFO enumerates provider 5 at `/MOUNT/DT0`, then
  GETCWD reconstructs the same path.
- An active cwd denies unmount without retiring the
  published mount; subsequent checked owner release
  permits final unmount and removes the FSINFO entry.
- A real directory FILE descriptor is opened on the
  mounted DTFS, then cwd is moved to the root TSFS.
  Unmount still refuses solely because the live FILE
  owns the DTFS vnode. After closing that descriptor,
  unmount succeeds and FSINFO reports the slot empty.
- The separate native v1519 and v1525 negative mutations
  remain rejected, including the original broken
  assembly index and parent polarity.

The installed v1524 SIMH Type-551 RBN0 fix is retained;
this increment changes only the two source-correctness
sites above, in optional DTFS/TSFS MRES.

## Remaining KCORE feature gates

The real-guest test is a *single-process, physical mount,
cwd and FILE lifetime* acceptance. Cross-process FILE
ownership, physical DTC3 detach/reattach, exact 72-bit
same- and different-tape media-return, READ_SUSPECT
and WRITE_UNCERTAIN failure handling, and dirty/partial
write administrative recovery remain separate P0-B gates.
No false permanent-loss HALT is permitted from ordinary
Type-551 I/O or status faults; authoritative P0-A
critical root/BACKSTORE loss classification and v1515
HALT dispatch remain open.

P1 mixed-provider faults, capacity-aware FSINFO under
the existing v1492 worktree changes, swapless PURE/MM/APR
integration and final full feature regression also remain.
Mutable-only compression is conditional on measured net
peak RAM reduction, and is not part of v1525.
