# KCORE BCACHE/DTC write and MM-refusal correction v1490

Date: 2026-11-11. Production commit: `3835c5ae`. Regression testkit
commit: `d81b039`. Feature correctness only; no size optimization.

## Source defect and reproduction

The production DTC write veneer in `system/kernel/fs/dtfs_runtime.s`
called `bcache_reclaim()` immediately before jumping to the raw DTC
service. The service ABI requires AC1=physical unit, AC2=physical block,
AC3=buffer. But `bcache_reclaim()` calls MM using AC1..AC3 (and returns
its own status in AC1). Even with no MM allocation, it can alter the
physical unit; with an allocated slab, it can overwrite all three. The
physical write could be routed with invalid unit/block/buffer arguments.

The second failure was independent: a refused `mm_free()` in
`bcache_reclaim()` left both clean cache identities valid. Callers
using the routine to invalidate for write-through or removable-media
handoff could then obtain a stale cache hit even if the authoritative
device had been updated. MM ownership and cache-data validity are
different contracts; inability to free RAM cannot justify retaining
a cache identity across media mutation.

The new `daimos-testkit/tests/validation/daimos-bcache-reclaim-io-v1490.sh`
links the actual production BCACHE instructions and extracts the
real DTC read/write veneers, substituting only physical DTC I/O and MM
callbacks in a PDP-6 instruction emulator. The old source fails the
`__test_exit=0` assertion before this fix.

## Implemented correction

`bcache_invalidate_all()` clears the shared-workspace and private-slab
tags, without freeing or forgetting the MM slab owner. Its only scratch
register is AC4, leaving the three public DTC I/O argument registers
untouched. `dtfs_dtc_write` calls this leaf and dispatches the unchanged
physical request. Reclamation invokes the same leaf *before* trying
`mm_free`; a failed free retains `bcache_state` for a later retry, while
both tag identities remain invalid. A subsequent cache store may reuse
the still-owned private slab with a newly valid identity.

No new dynamic allocation, owner record, process state, cache slab,
media encoding, second cache or device I/O API is introduced. Ordinary
BACKSTORE disk swap and persistent MEMFS/LOGSTORE layouts are untouched.
These changes affect fixed KCORE's BCACHE and its existing DTFS MRES
consumer, only as required to maintain the KCORE cache contract.

## Focused acceptance

- **PASS:** real PDP-6 DTC source veneers, with nonzero physical unit 2,
  exact block 5 and exact buffer pointer, write and read-after-write.
- **PASS:** all previously cached identities are invalidated before the
  physical write; a later read obtains new device data, not stale cache.
- **PASS:** first MM slab release refuses; both tags are invalidated,
  `bcache_state` retains the owner; next MM release succeeds and clears it.
- **PASS:** executable negative control reinstating reclaim-before-write
  is rejected by the same test's `__test_exit` assertion.
- **PASS:** separate executable negative control omitting reclaim-time
  invalidation is rejected by the same assertion.
- **PASS:** preexisting v1 BCACHE allocation/multisource and v688 shared
  mount-cache contract tests.
- **PASS:** fresh KCC `make -C system/kernel kcore` assembly generation;
  fresh full KCC tape-root KCORE and DTFS MRES links resolve the new ABI.

The `permanent-size ROOT=tape BOOT=ptr` check returns nonzero because
its *all-package* `KCORE+MRES` inventory exceeds the 16K permanent
address limit (observed 19578 words and permanent last 19625).
This is not a runtime loaded-module occupancy measurement or evidence
that v1490 caused the historical package limit exceedance. Optimization
and constrained profile acceptance are separate later gates.

## Remaining acceptance

The historical v572 disk-root two-boot DTC check reached ROOT/DSH,
native-format write/FSCK, subsequent FSCK, and both requested mount
attempts, but failed its final predicate because it still expects a
disk-root DTFS mount to succeed. Production `dtfs_minit()` installs
DTFS only for a DTC-selected root, so both disk-root mount attempts
must refuse. This is verified in the later `daimos-dtc-online-v1.sh`
oracle, not evidence of a kernel regression or a successful DTFS mount.
The independent v578 *tape-root* secondary DTFS guest is the correct
installed-provider end-to-end qualification; its obsolete host-tools
`make build` dependency was replaced with the installed production
`mktsfs` (test assertions unchanged). Its result is tracked separately.
Other F2 storage failures (BACKSTORE scheduled free refusal,
DEVICE/VOLUME in-flight failure, simultaneous backing/log/swap
ownership) remain open. Do not mark the whole F2 feature group closed.
