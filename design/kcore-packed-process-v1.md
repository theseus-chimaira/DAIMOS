# KCORE compressed-process split: staged implementation v1

Status: provisional codec interface only; process eviction is NOT enabled.

Target: PDP-6, CTY and four DECtapes, three TSFS system tapes and one
native DTFS writable home tape, no external swap. Both providers must
remain mounted together; executable backing must be provider-neutral.
Eviction must discard PURE text and compress remaining valid writable state.
Packed state need not be APR-aligned; restoration must reconstruct a complete
aligned extent before returning to user mode. Regions may be omitted only if
VM/ABI can prove no valid process state remains there.

The kpack codec v1 is a lossless identity transform. It consumes no dynamic
scratch RAM, but saves no writable-state words, so it must not be mistaken for
the final compressor or used to enable compressed eviction.

Implementation phases:
1. Separate executable backing and PURE reload from disk-backed swap.
2. Implement packed-process MM descriptors and transactional relocation.
3. Implement bounded-space packing, rollback and restore; preserve current
   full-swap behavior until the no-swap configuration is verified.
4. Replace identity codec following real process-state compression benchmarks
   with instruction counts and PDP-6 timing-table estimates.
5. Place all behavioral and failure-injection tests in daimos-testkit.

Acceptance requires no swap I/O, no hot-path context-switch overhead, correct
backing pinning, exact restoration, and safe operation under fragmentation.

## Packed descriptor contract (v7, inactive)

An isolated storage-mode map (v28) now uses two bits per configured process
slot instead of repurposing the overloaded swap record. Its zero encoding
denotes RESIDENT; DISK and PACKED are distinct and encoding three is invalid.
As of v29 its storage is boot-allocated with swap records, with a resident
pointer/slot-count descriptor; it is not yet wired to scheduler decisions.
State updates
must be serialized with scheduler/lifecycle transitions, and must publish
only after executable PURE restoration and MM ownership transfer succeed.

The initial two-word proc_packed_v1 descriptor carries physical base and
encoded length together with the decoded writable suffix offset and length.
It is not a per-process permanent allocation yet; the lifecycle must first
provide an explicit RESIDENT/DISK/PACKED discriminator outside the overloaded
legacy swap record. Packed MM extents belong to their slot and are excluded
from the existing aligned runnable-process compactor. A later sparse codec
may need a different layout record; the first descriptor deliberately handles
only a contiguous retained writable suffix.

## PCLK and civil time (deferred)

The mandatory CLK MRES owns 50/60 Hz scheduler ticks. It must be the
timekeeping source in the minimum profile. PCLK hardware (0730) is optional
and its current KCORE routine should eventually move to a separate package.
Do not substitute CLK tick counts directly for civil timestamps: the system
needs an initialized TIME36 epoch, set by the operator during startup BEFORE
login, plus tick-to-seconds conversion with proper 50/60 Hz selection.
Date entry and wall-clock migration are deliberately deferred. Preserve
existing PCLK behavior until an initialized software wall clock is tested;
all direct pclk_time36 callers (time syscall, MEMFS, D6FS) must be migrated
together. The DTFS/TSFS-only profile must not depend on PCLK hardware.

## KCORE split invariants

Disk-backed swap cannot simply be omitted from the link: exec attaches PURE
backing via proc_swap_attach; VM destruction detaches it; the scheduler and
allocation path reference swap helpers. First separate the executable backing
record and text reload into a provider-neutral mechanism. Then supply packed
resident-process reclaim rather than a disk-swap stub. The dummy codec must
not be enabled for eviction since it does not reduce writable-state words.
