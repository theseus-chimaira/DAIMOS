# P1-E v1644: physical immutable PURE backing guards on writable DTFS

Date: 2026-11-17. Base: v1643 physical non-root read failure and
same-reel FULL recovery PASS. No production KCORE, PURE MRES, MM,
VFS, native DTFS, TSFS, ABI or media-format changes. The entire
v1644 C89 guest and physical/source-linked test belongs to
daimos-testkit, not kernel production files.

## Safety question and source contract

The v1643 secondary executable was mounted read-only to test
reconstruction after genuine DTC3 loss. That setup cannot prove
the source immutability guard: VFS refuses modifications on any
RO filesystem even without a sleeping process. v1644 instead
copies the same original compressed PURE DXR2 to real native
DTC3 DTFS (provider5/public mount2), makes it mode 0777,
performs a clean unmount, and remounts the same verified stamped
medium RW. The three-member selected TSFS namespace ROOT remains
provider7/mount1, DTC0..2 physically online throughout.

Source inspection establishes the existing intended distinctions:

- `vfs_write_guard_v449` checks `proc_swap_backing_busy` for
  WRITE_WORDS and TRUNCATE, including `O_TRUNC` during OPEN.
- `file_remove` checks the actual target vnode against
  `proc_swap_backing_busy` before UNLINK.
- `vfs_unmount_owner_check_v1523` consults
  `proc_swap_mount_busy` before a mount can be retired.
- Native DTFS rename changes the name of the SAME directory
  file slot, without changing its vnode/index or contents.
  Changing spelling while the file data remain immutable is
  therefore safe; the source is pinned by identity, not name.

The v1644 original guest checks all these interfaces through
public syscalls while six genuinely sleeping PACKED PURE children
retain their immutable executable dependency, before the parent
releases the inherited PIPE writer. The guard test refuses
unmount, unlink, truncate, O_TRUNC and actual FILE WRITE, even
though `O_RDWR` open is independently allowed on the RW mount.
Rename `COMP` to `SAFE` and back succeeds; UNLINK of the renamed
vnode is still refused. All six children later recover and pass
original PIPE/private data sentinel/EXIT-057 and exact PID/
WAIT/status-057 checks. Once they have exited, the same parent
unmounts mount2 successfully, then prints `U` and `Z` and HALTs.

## Source-linked physical PDP-6 evidence

The unchanged source-built 48K/no-AUX TSFS/PURE kernel boots a
separate source-derived v1644 C89 guest, where initial source
setup and all normal child checks are preserved from v1643.
Console at normal HALT:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]P{013470-013470}LwwwwwwUZ`

The larger test guest's actual compiled DXR2 TX2 is `001420`
octal text words. The original PDP-6 `vm_pdp6_pure_range_v1604.h`
sector-floor rule gives omitted prefix
`(001420+020)&~0177=001400` octal words, leaving a retained
suffix `010000-001400=006400` octal words PER child.
Do not reuse the different v1643 (`006600`) or v1638 (`007000`)
suffix metrics for this compiled executable.

Original physical MM B, A, mount-guard-entry, vnode-guard-entry
and post-protection P have IDENTICAL child extents, all six
type PACKED_PROCESS, unpinned and nonoverlapping:

| Owner | Octal physical words | Pins |
| --- | --- | ---: |
| 7 | 051400..060000 | 0 |
| 6 | 061400..070000 | 0 |
| 5 | 071400..100000 | 0 |
| 4 | 101400..110000 | 0 |
| 3 | 111400..120000 | 0 |
| 2 | 121400..130000 | 0 |

The actual original KCORE breakpoints confirm the guards are
really invoked on the expected owner identity:

- `proc_swap_mount_busy` physical PC **003431**, AC1=`2`
  (native secondary public mount2), while all six are PACKED.
- `proc_swap_backing_busy` physical PC **003322**,
  AC1=`050202000000` (provider5, public mount2, file slot0),
  while the SAME six child MM owners and pins remain intact.

In all five snapshots `vfs_mount_policy` is exactly
`000000000001`: bit0 selected ROOT is RO; bit1 secondary mount2
is **RW**, proving mutation refusal was not due to read-only
mount policy. The referenced source file was mode0777 and
the guest successfully opened it O_RDWR before the write
operation itself was refused. A returned negative value from
file access alone is insufficient evidence; the test also
records the actual original vnode busy guard PC and identity.

The P marker is printed only after ALL guest syscall checks
including both renames and the renamed-vnode unlink denial.
The subsequent U marker is printed only after six exact WAITs,
a successful checked native DTFS unmount, and FSINFO confirming
the secondary mount is no longer published. MM snapshots at U
and Z each have eight remaining system/parent extents, **zero
PROCESS or PACKED child owners and zero leaked child pins**.
There is no selected-root HALT or physical device detachment.

The complete source-linked auditor PASSes and rejects three
independent forged traces: an injected RW->RO secondary policy
bit, a retained PACKED owner pin after protection, and a wrong
mount argument at the original guard entry. Original native
PURE backing tests v1382 and staging/owner test v1563 PASS.

## Scope and remaining P1-E acceptance

This v1644 acceptance covers **sequential** existing content-
mutation, unlink and mount-retirement guards while six PACKED
children run on a fully writable secondary device, safe
same-file-slot rename, and cleanup that permits unmount AFTER
the executable dependency ends. It does NOT assert general
PI6 interleaving/race freedom; physical MM uncertain rollback,
32K/64K layout regressions, or matched time-resolved peak RAM.

The older native DTFS v1522 source-extraction test is still
stale after v1575/v1561 and is not counted PASS (see v1643).
No new permanent RAM or resident instruction word was added.
PURE remains OFF by default and strict 32K whole-KCORE size
optimization HOLD pending the remaining P1-E gates.
