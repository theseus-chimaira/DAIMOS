# v1684: source-linked two-owner RAW PURE executable lifetime

Date: 2026-11-19. Status: **native PDP-6 C89 source-linked PASS**.
Development branch `kcore-pure-reclaim-pressure-v1673-20261118`.
Production KCORE, optional PURE MRES, MM assembly, user ABI, source
scanner, RAM budget and binary images are **unchanged**. This acceptance
is narrower than the outstanding nine-process physical source-lifetime
and actual native VFS unmount/rename failure matrix.

## Original implementation exercised

The new C89 source oracle belongs solely to daimos-testkit at
`tests/validation/pure-multiowner-source-v1684/`.
The full native link includes original KCC-compiled zero-copy RAW
`proc_pure_ram_drop_v1381`, bounded FULL `proc_pure_ram_restore_v1391`,
original PDP-6 MM PURE rebase and prefix assembly, original packed-mode
runtime and the original source-backed VFS `proc_swap_mount_busy` /
`proc_swap_backing_busy` instructions extracted without changes from
`proc_pure_backing_pdp6.s`. The existing v1649 original FULL-guard,
v1648 transaction rollback and v1395 source/target fault cases remain
passing prerequisites; no extra resident state or registry is added.

The C test fixture creates two independent resident process owners,
with different stable u-areas, executable vnode indices and public
mounts. The actual source-linked DROP transaction rebases both original
PDP-6 MM PROCESS extents into separate RAW PACKED suffix owners.
Neither original image is copied or synthesized by a fake DROP helper.
While the first process performs the actual bounded FULL restore,
the test sees three exact original live MM owners: the first PINNED
PACKED suffix, its PINNED PROCESS destination, and an independent
unpinned PACKED second suffix. Both mount and vnode guards protect the
original first and second executable sources. An unrelated mount or
vnode is not claimed busy. The complete second suffix word content,
MM descriptor span/meta, original VM mode and absent APR remain
unchanged while the original FULL source callback executes.

After successful FULL publication of the first process, **both sources
remain protected**, including the now-RESIDENT first owner: it still
owns an executable that may be required on future RAW reclaim. During
test development, treating the restored resident executable as
unprotected caused an assertion failure; this was a bad test
expectation, not a production defect. It was corrected without kernel
changes. The first original source descriptor is released and its full
APR is published, while the second original RAW suffix remains intact.

With the second mount root temporarily removed in the test fixture,
a source-linked FULL restore for the second process fails in original
`pure_admit_source_v1608` with `MM_ERR_INVAL`, before allocation and
without freeing, pinning or changing that unique suffix; its
TRANSITION and backing identity remain intact. Restoring the mount
root makes original source-guard lookup valid again. The v1684 fixture
does **not** attempt second-owner FULL after this return because the
older fixed native MM allocator fixture permits target allocation only
for slot 1; a claim of second FULL success would be unsupported.

## Executed original assembler negative controls

Both passing and unsafe variants execute the *same new two-owner C89*
oracle in the PDP-6 simulator. For negative runs, the separate
v1649 baseline main is disabled so an old single-owner assertion
cannot be mistaken for a v1684 result. The original assembler
scanner is patched only in disposable test link inputs:

| Source guard mutation | Required original guest exit (octal) |
| --- | --- |
| Invert original mount-owner scan condition | `000000004004` |
| Invert original vnode/backing identity comparison | `000000004200` |

Both executed mutations are rejected at the specified new-oracle
status, not at a link error or simulator timeout. The complete
unmodified source-linked native oracle returns `__test_exit=0`.

## Remaining physical acceptance

This v1684 test is C89 code executed by the PDP-6 simulator with the
**production assembler** MM/VFS code, not an assembly-authored fixture
for the new behavior. It is a native component transaction regression.
Mount roots and process slots are fixture-owned; this is not an actual
48K running TSFS/DTFS system making a user UNMOUNT/UNLINK/TRUNCATE,
not a real controller disappearance, and not a full PI6 interrupt
interleaving. Existing separate v1643/v1645 physical secondary-DTFS
read-failure and immutable-source tests cover different scopes.

Still open: all-nine-process source lifetime, source replacement and
unmount refusal under RAW memory pressure, actual secondary provider
loss/recovery and bounded FULL fallback, the exact physical MM write
high-water with faults, and root/controller/KCC FULL profiles. No
mainline integration or local/remote branch cleanup is authorized by
this scoped native result.

Testkit branch `test-pure-multiowner-source-v1684-20261119`, commit
`983c5fb`. No production DAIMOS source was changed.
