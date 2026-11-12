# KCORE v1518: invalidate DTFS directory and clean cache after physical I/O errors

Date: 2026-11-12. Scoped optional-DTFS-MRES correctness fix for
non-root DECtape transient I/O errors. The change is not automatic
media-disappearance detection, on-media replacement identity checking
or a claim of complete non-root recovery.

## Problem: a failed physical I/O left stale directory identity

DTFS keeps a single shared directory image (`dtfs_dir`) and a cached
mount-ID discriminator (`dtfs_cache_mount`). The common BCACHE has two
clean block identities for physical DTC UNIT,,BLOCK. The real
`dtfs_load` skips physical directory I/O when its cached mount ID
matches the requested one, and `dtfs_dtc_read` can satisfy clean
blocks from BCACHE without reaching the tape.

Before v1518, when a real tape read failed, `dtfs_dtc_read` returned
the negative I/O result but did not clear `dtfs_cache_mount` or the
two clean cache tags. A later operation could therefore reuse its
previously trusted in-memory directory after the device had already
demonstrated an I/O failure. Failed writes invalidated BCACHE on
entry but left the directory identity trusted even after a failed
or partial metadata commit.

The corrected physical read veneer branches on a *negative physical
result* and clears `dtfs_cache_mount`, invokes the existing
`bcache_invalidate_all` (without MM reclaim), and preserves the
original negative AC1 result. Its successful cache hit, physical
read and clean insertion paths remain unchanged. The write veneer
still invalidates the clean cache before hardware I/O, preserving
the raw DTC AC1=unit, AC2=block, AC3=buffer ABI. It now returns
through the original hardware trampoline with a `PUSHJ`, tests
its result and clears `dtfs_cache_mount` only on failure. A
successful write retains the existing directory mount identity.

Neither path retires a VFS mount, forces a physical storage release,
allocates new memory nor introduces quarantine or a media status.
The shared cache invalidation can temporarily reduce hit rate for
unrelated mounts, but ensures no cache entry retains a false clean
identity after a reported physical error. The TSFS root uses the
same DTC read veneer; it gets fail-closed cache invalidation rather
than a false automatic fatal stop on a transient controller error.

## Source-linked PDP-6 tests

`daimos-testkit/tests/validation/daimos-dtfs-io-error-cache-v1518.{s,sh}`
links the production DTC read/write veneers, native `dtfs_load`
directory loader, and complete KCORE BCACHE assembler into a
PDP-6 instruction test. Only the boot-patched physical DTC jump
targets are replaced with deterministic hardware callbacks; the
preallocated BCACHE slab is a real cache buffer under the actual
BCACHE implementation, not a mock cache-hit return.

The baseline PASS checks an initial successful directory and clean
cache entry on non-root DTC3 mount 2, a different uncached read
failure, negative result preservation, both clean tags invalidated,
and directory mount identity cleared. A second directory access
while the device is unavailable must **actually retry physical
I/O** and return error instead of using stale memory. The same
media returning then permits an actual checked clean read. A
failed physical write likewise clears both clean tags and the
directory identity; after restored read access, a successful
write invalidates tags but retains the directory identity.

Four executed negative source mutations are rejected: no read-side
directory invalidation, no read-side BCACHE invalidation, no
write-side directory invalidation, and no write-side BCACHE
invalidation. The original v1490 source-linked BCACHE MM free
refusal/raw-DTC-ABI regression PASSes with a test-only updated
veneer mutation anchor and a test-only `dtfs_cache_mount` symbol.
BCACHE multisource, DTFS chain-walk, create/unlink/rename/dirfield
and TSFS stored/cross-member suites all PASS.

Both isolated **256K KCC full tape-root** and **32K GCC low-memory
tape-root** KINIT + DTFS/TSFS MRES builds link successfully.
The existing strict fixed-KCORE 32K limit still **FAILS** at
`__kcore_low_end=023241` octal against required `<020000` octal;
this is not an accepted 32K profile. The optional MRES change
does not touch fixed KCORE. A controlled old/new optional DTFS/TSFS
MRES link with identical other objects and independent folding
plans moves the following symbols:

| Symbol | Before (octal) | After (octal) | Difference |
| --- | --- | --- | ---: |
| `dtfs_dtc_read` | `001522` | `001522` | 0 |
| `dtfs_dtc_write` | `001546` | `001551` | +3 |
| `dtfs_load` | `002226` | `002234` | +6 |
| `tsfs_mres_dispatch` | `002435` | `002443` | +6 |

Thus the change costs **six additional optional MRES instruction
words**, **zero permanent data words**, **zero fixed-KCORE words**
under the standard p10fold plan. No on-media format or public ABI
was altered. The emitted code remains readable assembly at the
physical I/O boundary, not additional cross-layer retry state.

## What remains open

No Type-551 hardware-compatible media-removal event has been
established in v1517. If all operations return cache hits without
attempting physical I/O, no failure can yet be observed. The
present fix acts only **after an actual negative physical request**.
Also, a returned tape must not be assumed to be the same volume
just because its directory can be read or has valid structure.
The test models return of the **same** medium; it does not prove
automatic safe reuse of a *different* tape with old file handles.

P0 remains reliable confirmed TSFS root-member removal/transport
loss distinct from transient Type-136 errors, followed by v1515
root fatal dispatch. P1 remains non-root DTFS identity, safe
explicit revalidation/remount, dirty data/open FILE ownership,
BCACHE replacement discipline and full physical detach/return
testing. Optimization and compression remain deferred until
functional gates are accepted.
