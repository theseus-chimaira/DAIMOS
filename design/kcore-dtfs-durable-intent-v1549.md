# KCORE P0-B v1549: durable DTFS intent protocol, design acceptance

Date: 2026-11-13. Feature-completion design and fail-closed test model.
P0-B implementation and physical acceptance are **NOT complete**.

## Observed production state

Native DTFS accepts either an all-zero legacy reserved RBN0 or a
valid `DTVOL1` 72-bit identity record in RBN0. The stamped record
contains four checked words and 124 zero words. `dtfs_mount_unit`
reads that record physically, validates the directory at RBN 0144,
and publishes the VFS mount. `dtfs_dtc_write` invalidates BCACHE
and marks the live mount's media word `WRITE_UNCERTAIN` when a
physical WRITE reports failure. The checked v1531 root-only
`DROP_FD`/`REPLACE_SLOT` administrative procedure preserves live
FILE/cwd/PIPE/swap owners and requires different stamped replacement
media. Those per-mount uncertainty bits are **volatile**.

The v1529 full PDP-6 / Type-551 fault demonstrates exactly one
new data word and one old word after a failed two-word write.
After v1531 discards its VFS mount, or after cold boot, an otherwise
legible old RBN0 `DTVOL1` stamp and directory cannot distinguish
that partly damaged tape from a healthy former volume. Merely
writing a poison record **after** the failure is not reliable:
the unavailable/partially writable tape may refuse that write.
A RAM blacklist cannot survive cold boot, and matching the same
72-bit stamp is not proof of unchanged data.

## Chosen durability rule: write-ahead, on the tape

Version `DTVOL2` will use the *already reserved* RBN0 and the
existing 72-bit identity plus a checked two-word state seal. The
state will be `CLEAN` or `WRITE_INTENT`; unknown, torn, nonzero
padding or invalid checksums fail closed. A write-intent seal is
not a claim that any actual data block was damaged; it means the
volume cannot be admitted as healthy without authorized off-line
inspection and a new format/identity. A second data recovery
operation must not simply erase the seal.

Proposed six-word 36-bit record, rest of RBN0 zero:

| Offset | Meaning |
| --- | --- |
| 0 | SIXBIT `DTVOL2` |
| 1, 2 | Independent nonzero 36-bit volume ID halves |
| 3 | Complement of (magic XOR high XOR low) |
| 4 | 0 = CLEAN, `0552255225522` octal = WRITE_INTENT |
| 5 | Complement of (magic XOR high XOR low XOR state XOR seal salt `0246134572610`) |

The checksum detects accidental torn records; this is not
cryptographic tamper resistance. No new physical block, allocation
map entry or separate persistent registry is required.

### Required operation ordering

1. New media from an explicit reformat starts with DTVOL2/CLEAN.
   The kernel **must not automatically upgrade** an old DTVOL1
   or all-zero legacy tape. Their previous dirty history is
   unknowable; under the new integrity-enforcing mount policy they
   are rejected as healthy DTFS until separately salvaged and
   reformatted by an operator. This compatibility break is a
   safety requirement, not something to silently paper over.
2. Before the *first* filesystem data/directory WRITE of a mounted
   volume, publish WRITE_INTENT to RBN0 by **raw physical I/O** and
   read it back from physical media. Verify exact 72-bit identity,
   seal and zero padding. Only then may the data write start.
   A failed/torn marker or uncertain readback aborts the user write,
   retains all live owners, and poisons the mount. No later data
   write is allowed until a successful new, checked transition.
3. All further writes during that mounted transaction retain the
   intent flag. A failed/partial Type-551 data or directory write
   leaves the on-media intent intact, in addition to the existing
   sticky volatile WRITE_UNCERTAIN bit.
4. Only after all filesystem changes have been written and checked,
   no process owns the mount, and a final provider-serialized
   unmount barrier has succeeded may CLEAN be written and physically
   verified. If the clean-seal WRITE/readback fails, UNMOUNT fails;
   the mount and its owners are retained or its on-media record
   remains fail-closed. A new write after a clean seal must re-arm
   WRITE_INTENT *before* the next data transfer. The cleanup must
   not run as an ordinary `dtfs_sync` from arbitrary FILE owners.
5. On fresh mount after **cold boot** or reinsertion, a physical
   RBN0 read must find exactly DTVOL2/CLEAN, not DTVOL2/WRITE_INTENT
   or a DTVOL1 identity/directory that merely looks intact. A
   different, independently formatted DTVOL2/CLEAN replacement can
   mount normally. No stale FILE, vnode, cwd or backing owner may
   be copied from the discarded original mount.

## Integration constraints found during sweep

The existing `dtfs_dtc_write` receives a caller-owned 128-word
buffer, frequently `fs_block_workspace`. An RBN0 read into that
workspace would silently destroy the pending data write. The
separate `dtfs_dir` is also live and cannot be overwritten safely.
The normal process kernel stack has `0315` octal words, so putting
another 128-word scratch block there is not safe. Allocate one
optional DTFS-only temporary 128-word MM buffer, or provide an
explicitly reserved, provider-serialized BCACHE scratch lease;
no extra permanent fixed KCORE/low-memory slab is justified.
Ensure that allocation and release failures preserve their exact
MM owners; do not trade correctness for an apparently free buffer.

VFS `vfs_unmount` performs SYNC, an optional provider PREPARE,
then checks owners again before retiring the slot. Marking CLEAN
at the first SYNC is **incorrect**: another owner may appear while
that I/O sleeps. The future unmount seal must be integrated into
the final exclusive, owner-checked provider transition, with
no unsealed user data write allowed to race its publication.

`DTVOL1` plus legacy read/write/format probes exist in the host
formatter, target `dtfs_probe`, native DTFS provider and testkit.
Any DTVOL2 rollout must update all format readers together and
include a deliberate migration/reformat path. Do not emit media
which current production KCORE cannot safely mount. No on-media
behavior or production DTFS code has been changed by v1549.

## v1549 executed model and remaining real acceptance

The testkit's `daimos-dtfs-durable-intent-v1549.py` PASSes 16
protocol invariants and 256 simulated prefix-partial Type-551
writes (128 possible prefixes for each seal transition). It
rejects legacy and DTVOL1 promotion, torn seals, mismatched
IDs, unexpected padding, dirty reinsertion/cold boot and false
cleanup. It verifies that every failed pre-write marker
aborts the data WRITE and that a full verified intent permits it.
**These are model tests, not executed PDP-6 production code.**

A second testkit suite operates on real host `mkdtfs` DECtape image
bytes, encodes/decodes two 18-bit lanes, checks experimental RBN0
record roundtrips, an RBN0145 prefix-partial data write, and a
new process opening the still-dirty media image. Five independent
offline assertions PASS. This is still NOT a native or full-guest
Type-551 controller test, and is no evidence of installed KCORE
durability. The physical owner/cwd/PIPE/swap lifetime cases remain
to be tested after the actual provider implementation.

Next implementation milestones before claiming P0-B acceptance:

- a single shared native/host DTVOL2 record parser and **real**
  Type-551 RBN0 mark/readback operations under the provider lease;
- pre-write arming for every mutating native DTFS path, exact
  failure containment and MM scratch/owner lifetime tests;
- checked CLEAN seal at final owner-safe unmount, no stale cache;
- physical full-guest v1529 data-partial fault **after** DTVOL2
  intent, discarded-slot replacement, old/changed/new media,
  cold reboot with the old tape, and healthy multi-process,
  cwd, PIPE and BACKSTORE owners left untouched;
- native unsafe mutation tests for skipped arm, wrong identity,
  early clean, lost marker, returned I/O error and rejected
  partial RBN0 update.

P0-A remains complete. P0-B **OPEN**. P1-C unaffected.
VERSION 0.5, strict 32K and optimization HOLD.
