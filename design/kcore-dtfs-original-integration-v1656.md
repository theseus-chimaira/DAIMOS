# KCORE v1656: restored original-source DTFS raw/cache/identity integration

Date: 2026-11-17. Original source-linked PDP-6 native V1 DTFS/BCACHE
read/write and mounted-identity integration tests v1518/v1522 now PASS
on current source. The combined 82-case v1655 matrix also PASSes.
No production KCORE, MM, VFS or MRES changes.

## Original testkit staleness

v1518 was written before the v1561 physical-unit DTFS write-intent
gate, and its native MM/VFS mount table was not published. The current
original dtfs_dtc_write enters that v1561 gate BEFORE cache
invalidation or raw DTC writes; a fixture with no mounted owner
cannot pass a valid call. Its old bad-source anchor was likewise
a comment removed in v1561. This was TESTKIT STALENESS, not a
demonstrated production defect.

v1656 now links the untouched original v1561 assembly gate with
a real V1 mount2 on DTC3 and two-slot published VFS roots.
The fixture V2 first-write service is explicitly fail-closed
if erroneously called; original V1 owner logic must permit the
write. Original raw DTC read/write wrappers, native directory
loader and full real BCACHE remain in the executable. The
write-cache negative edit now targets the original actual
invalidation call. All four original unsafe mutations are
compiled, executed and rejected on native PDP-6.

v1522 extracted the original FSINFO space function up to an
obsolete v1531 comment. In current source the next independent
operation is v1575 poisoned-owner drain. The updated extractor
preserves the exact original space function and exits until
that current boundary. It links original v1561 gate with V1
published DTC3 sources, original native V1 identity parser,
V1/V2 mounted recovery, full raw wrappers and real BCACHE.
The stale negative instruction manipulating original mount
slot was aligned with actual original v1556 saved-slot restore.
All eight source mutations execute and fail: read/write marker,
both 36-bit ID halves, legacy, sticky WRITE_UNCERTAIN,
original mount slot and cache workspace invalidation.

## Current-source PDP-6 acceptance

Standalone original v1518 (1 integrated execution, 4 mutants)
and original v1522 (1 integrated execution, 8 mutants)
each PASS independently with current production source.
The new combined v1656 runner additionally executes current
v1655 82-case native V1/V2 identity/DIRTY/intent/owner matrix,
including 39 independently rejected mutants.

**Combined PASS: 2 original V1 integration tests,
82 enumerated current V1/V2 scenarios and 51 independently
executed/rejected bad-source mutations.** This is not a
new full physical Type-551 multi-process KCORE guest.
No hardware reel-retention interlock is invented;
the original v1575 operator-trust media contract applies.

Testkit source:
tests/validation/dtfs-integration-v1656/
and repaired tests/validation/daimos-dtfs-io-error-cache-v1518.*
and daimos-dtfs-mounted-id-v1522.*.
Testkit branch test-kcore-dtfs-integration-v1656-20261117,
commit a507c17. Checked original native logs and full
source/mutant assembly evidence:
$TMPDIR/daimos-kcore-v1656-evidence-20261117.tar.gz,
SHA256 7393d83105e281b66bf24852dfbb08e864b9b198978d64002a26f7b0896b1ec5.

## Remaining PDP-6 KCORE release work

The stale combined v1518/v1522 source integration gap noted
at v1655 is CLOSED for native V1. Original v1554 and v1561
still cover physical V2 DIRTY and first-write MM protection.
Next gates: full continuous physical memory peak including
KCORE/MRES/BCACHE/provider scratch, fail-closed permanent
uncertain MM ownership, optional mutable-suffix compression
measured YES/NO, and final current-source system test/freeze.
Strict 32K boot/footprint acceptance remains DEFERRED.
