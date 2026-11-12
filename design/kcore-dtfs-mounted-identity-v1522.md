# KCORE v1522: checked DTFS mounted-media return after observed I/O errors

Date: 2026-11-12. This implements the first production P0-B
mounted-volume recovery boundary following v1520 stamped identity.
It does **not** close P0-A authoritative loss notification, physical
open-FILE/dirty-state acceptance, or all of P0-B. No source change is
made to the root/BACKSTORE crash-only policy.

## Why the v1520 admission check was insufficient

v1520 checked physical RBN0 before publishing a *new* native DTFS
mount, then saved its two 36-bit volume-ID halves in existing MM
mount SHAPE/EXTENT words. A previously opened mount could still
reuse its old vnode and BCACHE after another I/O failed. v1518
correctly invalidated shared clean block and directory identities
on physical error, but did not remember which mounted volume required
subsequent identity proof. The global directory cache can switch
among mounts; it is not an authoritative per-mount failure record.

## Installed v1522 protocol

The existing MM-backed `dtfs_media_base[slot]` word stores the physical
unit/personality in its RH and UID9/GID9 in its LH. Two unused RH
bits now denote transient failure state, **without another data word**:

| RH bit | Value | Meaning |
| --- | --- | --- |
| 16 | octal `0200000` | READ_SUSPECT: verify old volume before reuse |
| 17 | octal `0400000` | WRITE_UNCERTAIN: possible partial physical write, no implicit reuse |

On negative physical `dtfs_dtc_read`, the existing v1518
directory/BCACHE invalidation remains in place. The runtime now scans
the boot-published MM mount roots, flags **only mounted DTFS slots
matching the actually failing physical DTC unit**, and leaves all
other providers and units alone. A physical write failure performs
the analogous scan, but retains WRITE_UNCERTAIN rather than assuming
that equality of volume ID repairs a possibly damaged directory or
data block. No DTC status, timeout or generic negative return is
promoted into root/BACKSTORE loss or a fatal HALT.

Every `dtfs_load` checks those per-mount bits *before* accepting
an already-cached directory. A read-suspect mount uses a raw
`dtfs_dtc_read_jump` for RBN0, bypassing BCACHE, then calls the
**same native v1520 full-block identity parser** used by fresh
mount admission. It refuses failed I/O, invalid checksum, nonzero
padding, partial ID, legacy zero-ID, and mismatch of **either**
persisted 36-bit half. It clears only READ_SUSPECT after full
structural validation and both half-comparisons, then refills the
directory through the existing read path. Failure preserves the
suspect bit, mount identity, old FILE/vnode ownership and cache
invalidation. The loader does not invent a second ID parser, VFS
mount registry or independent buffer.

`dtfs_space` (FSINFO) and `dtfs_sync` now use the same checked
`dtfs_load` boundary. They cannot report old cached allocation
figures or tell VFS unmount that a possibly missing/replaced tape
has been synchronized. Normal legacy volumes are still accepted
and usable before errors; a legacy zero-ID *return after an error*
is unprovable and remains fail-closed. After a failed write, even
a positive same-ID read is not sufficient to resume automatic
operations: WRITE_UNCERTAIN is intentionally not cleared.

The tested source is in `fs/dtfs_runtime.s`; the two C-visible
flag constants in `fs/dtfs_media.h` are shorter than the old
KCC identifier limit. The original Type-551 physical-loss
status issue is unchanged; refer to v1517. Confirmed root or
BACKSTORE device loss still requires independent authoritative
event evidence and routing to v1515.

## Native checks and exact controlled RAM accounting

New `daimos-testkit`
`tests/validation/daimos-dtfs-mounted-id-v1522.sh` executes
real production read/write error veneers, shared BCACHE, mounted
flag scan, identity parser, loader, FSINFO and SYNC on PDP-6,
with only physical-media replies and the mount slab populated
by the fixture. Accepted sequence:

- Initial legacy and stamped directory loads.
- Physical DTC3 read failure flags exactly two live DTFS mounts
  on that unit, not unrelated DTC0 or a D6FS root slot.
- Offline FSINFO/SYNC refuse old directory state.
- Authenticated *same* stamped ID returns; all needed new
  physical reads occur and read-suspect bit clears.
- Independently replacing high or low 36-bit half of an otherwise
  valid stamped identity is refused without clearing original owner.
- Retrying the exactly matched medium later succeeds.
- A failed physical WRITE sets a different sticky bit; even after
  medium return and correct ID, read, FSINFO and SYNC refuse reuse.
- An unrelated DTC7 error does not poison other mount units.
- Legacy zero-ID initial access succeeds but post-error return is
  conservatively refused without a false identity assertion.

Seven **executed** source mutations fail the native oracle: skip
read marker, skip write marker, accept mismatched high half,
accept mismatched low half, accept unauthenticated legacy
return, retry after uncertain write, and lose the mount-slot
register across a raw DTC transfer. The raw test fixture deliberately
clobbers AC6, catching a real ABI/lifetime error in the initial
version; production saves and restores the slot on its own stack
around physical I/O and identity parsing. No mutation is accepted.

Earlier exact-source native tests also pass after updating
their *fixture* mount-slab stubs: v1518 read/write invalidation
with four mutations; v1490 BCACHE/MINIT call ABI, refused MM
free and two negative controls; v1519 physical native admission
with six negatives; v1520 full stamp/mount with five negatives;
DTFS chain/create/unlink/rename/resize/dirfields and TSFS
root-member v1516; BCACHE allocation/multisource. The v1518
fixture intentionally substitutes its original pre-v1522
identity-neutral boundary; **only v1522** claims tested
mounted identity and error flag semantics.

A clean isolated **256K KCC tape-root KINIT + fixed KCORE +
DTFS/TSFS MRES** build assembled and LINKED. As before, the
separate strict single-root placement target rejects the current
too-large resident image; this is not a compiler/linker regression,
and neither strict 32K nor full booted guest is accepted. A KCC
note about unused `minit_diag_nodrv` is pre-existing;
the initially overlong v1522 header macro was shortened and no
long-identifier warning remains.

The controlled MRES relink used **exactly the same KCORE map,
DTFS/TSFS companions and independent fold plans**, swapping
only the v1521 `dtfs_runtime.s` object for the v1522 object:

| Metric | v1521 | v1522 | Change |
| --- | ---: | ---: | ---: |
| DTFS+TSFS package DXR image | `003620` | `003746` | +86 decimal words |
| Package BSS | `000002` | `000002` | 0 |
| Fixed KCORE | same map | same map | 0 |
| MM mount slab data | same | same | 0 |

The additional 86 words are **optional resident MRES instruction
RAM when tape FS is installed**, not zero system RAM. No extra
static or dynamic data words are allocated.

## Still OPEN, explicitly not accepted by v1522

1. Full booted guest fault/recovery using genuine open FILE/vnode
   ownership, concurrent dirty writes and VFS mount retirement;
   exercise repeated same-media return and mismatched replacement
   while an independent root remains live.
2. Safe explicit administrative recovery/FSCK/unmount protocol for
   WRITE_UNCERTAIN and legacy zero-ID volumes. Do not clear the bit
   merely because the same tape can be read again.
3. Silent physical exchange before any observed error, and
   authoritative Type-551 media-removal diagnosis (v1517/v1515 P0-A).
   A header read cannot detect a swap that occurs without a read.
4. Mixed-device error and provider ownership acceptance (P1-C),
   independent of this narrow DTFS checkpoint.

v1522 closes the specific **source-level, error-triggered revalidation
and cache/FSINFO/SYNC admission mechanism**; it is not completion of
all KCORE media-loss or removable-volume work.
