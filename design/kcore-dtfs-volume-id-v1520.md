# KCORE v1520: optional native DTFS persistent 72-bit volume identities

Date: 2026-11-12. Completed feature stage: native DTFS on-media
identity generation (host), compatible legacy detection, kernel
mount-admission validation and retention of the verified identity.
This is **not** automatic replacement-media recovery. No extra
fixed KCORE words, permanent data or dynamic mount-slab allocation.

## Format: existing reserved physical block 0, not the directory

Official native DTFS reserves physical DECtape block 0 and keeps it
zero on old V1 media. The 128-word native directory at block 0144
is fully occupied by its block-owner map, filename pairs and magic;
it has no spare unambiguous volume-id fields. We preserve that
original directory exactly, and use four words of reserved block 0:

| Word | Content (36-bit) |
| --- | --- |
| 0 | SIXBIT `DTVOL1`: octal `0446466575421` |
| 1 | High 36 bits of random 72-bit ID, nonzero |
| 2 | Low 36 bits of random 72-bit ID, nonzero |
| 3 | 36-bit complement of (word0 XOR word1 XOR word2) |
| 4..127 | Required zero padding |

The format constants live in `system/kernel/fs/dtfs_identity_v1520.h`,
included by `dtfs_media.h`. An **entirely zero** reserved block is
legacy DTFS; any nonempty malformed record is an error, never a
legacy fallback. Checksums detect accidental corruption, not forgery.
The 72-bit token is a probabilistically unique volume equality ID,
not a cryptographic credential or proof of media presence.

The official host `tools/host/mkdtfs.c` obtains nine bytes from
`/dev/urandom`, packs their 72 bits into two 36-bit words and
composes the header/checksum. Entropy read failure or a zero ID
half aborts image creation; images are still created exclusively,
without overwriting an existing source. No additional physical
block or host/target in-memory directory buffer is allocated.

Target `MKFS.DTFS` does not have a trusted source of nine random
bytes. It therefore intentionally produces **legacy** images,
zeroing all of physical block 0 BEFORE rewriting its native
directory. This also prevents inadvertently retaining the old
stamp of a previously formatted native volume. On a block0 write
error target MKFS aborts without writing the replacement directory.
We do not derive false unique IDs from the clock, PID or controller.
An identity-stamped writable volume must currently be prepared
by the host `mkdtfs` producer, not target `MKFS.DTFS`.

## Kernel and userspace validation

The transient `userland/dtfs/dtfs_probe.c` checks native magic
and allocation tags as before, then reads physical block 0
into its EXISTING `probe_block` buffer. It checks a complete
legacy zero block or all stamped record fields and all padding;
a failed physical read or malformed identity is rejected.

The optional PDP-6 `dtfs_mount_unit` independently validates
the directory as in v1519, then physically reads reserved block
0 directly through the MINIT-patched DTC jump (never BCACHE).
It refuses negative I/O and malformed nonzero identity records
before VFS publication. It temporarily saves the two 36-bit ID
halves on its own existing call stack across VFS mount creation.
After successful publication it records them by reusing the
already MM-allocated **TSFS shape and extent words for that DTFS
mount slot**. TSFS and DTFS never occupy the same VFS slot.
Legacy mounts publish two zero identity words; they cannot claim
identity-verified continuity after later media removal.

No new process/media quarantine state, permanent KCORE data,
KINIT mount-slab words, VFS mount capacity, I/O ABI or data-block
layout is introduced. The cold mount path needs an extra physical
read of reserved block 0. The common directory remains native V1;
the V1520 header is an optional native identity extension.

## Test results

Testkit `daimos-dtfs-volume-id-v1520.{sh,md}` exercises real
host `mkdtfs` on two independent full-size tape images, verifying
different IDs, exact packed header and unchanged native directory.
Host source-linked C89 tests run the actual userspace DTFSPROBE
against stamped, legacy and deliberately corrupt/partial/media-I/O
fixtures. The target `MKFS.DTFS` command implementation is
source-linked and tested to abort if clearing block0 fails, or
to perform clear-before-directory publication on success.

Native PDP-6 executes real `dtfs_mount_unit` and BCACHE
invalidation through the complete v1519 admission oracle, then
tests valid stamped identity retained in both MM slab entries,
invalid checksum, nonzero padding, partial header, unreadable
identity block and the same stamp admitted on a later checked
mount call. Five executed unsafe source mutations are rejected:
ignored checksum, ignored padding, arbitrary nonempty acceptance,
lost high ID, and lost low ID. The test does not emulate stale
open FILEs or a fully functional physical tape exchange.

All v1520 tests PASS. Existing native v1519 mount/6 negative
mutants, v1518 read/write failure/4 negative mutants, v1490
BCACHE ABI/MM-free refusal and DTFS create/unlink/rename/chain/
dirfield and TSFS slab/cross-member regressions PASS.

Both full isolated 256K KCC and 32K GCC tape-root KINIT with
DTFS/TSFS MRES LINK. KCC compiles target MKFS.DTFS and DTFSPROBE
objects. The separate strict fixed-KCORE 32K size gate **FAILS**
unchanged at `023241` octal (must be `<020000`). An independent
controlled old/new MRES relink, with the same KCORE map/other
objects and independently generated fold plans, measures:

| Symbol | Before (octal) | After (octal) | Difference |
| --- | --- | --- | ---: |
| `dtfs_mount_unit` | `001620` | `001620` | 0 |
| `dtfs_lookup` | `001734` | `002011` | +45 |
| `tsfs_mres_dispatch` | `002476` | `002554` | +46 |
| `tsfs_root_owns_dtc_unit_v1516` | `002510` | `002566` | +46 |

Net **+46 optional DTFS/TSFS MRES instructions, +0 permanent
data words, +0 fixed KCORE instructions**. Only the already
allocated two per-slot slab words are reused; no new RAM extent.

## Remaining feature boundaries

This stage does **not** compare the retained identity on a later
post-error read or silently replaced medium. The mounted filesystem
can still serve BCACHE/DTFS cached hits without detecting an
unreported physical swap; a later I/O error still needs a separate
checked identity transition before old FILE/vnode state is reused.
A legacy zero-id tape cannot be authenticated as the old volume.
The true physical Type-551 media-loss signal (v1517 investigation)
and TSFS root-member fatal dispatch (v1515 policy) remain P0 OPEN.

Next non-root DTFS feature: define an explicit media-suspect and
revalidation contract using the retained ID, with full open FILE,
dirty-data and mount-slot lifetime checks. No auto-recovery is
enabled here, and no root/BACKSTORE crash policy is changed.
