# KCORE compressed-process split: staged implementation v1

V35 (inactive) makes MM authoritative for packed physical base and encoded
extent words. Its two-word sparse entries contain only slot ID and layout;
lookup reconstructs a temporary two-word descriptor from MM plus layout,
rejecting duplicate owners or duplicate entries. Sparse v35 saves one data
word per reserved entry against v34 but adds compiled code. At 24 slots,
a future dense MM-authoritative layout table would need 24 data words, versus
16 for an eight-entry sparse v35 table. A KINIT-selected dense/sparse backend
remains an open design choice; do not link two full separate implementations
into the 32K KCORE. Existing codec, scheduler, process teardown and slot
allocation remain unchanged.

V34 adds a sparse, bounded three-word-per-entry descriptor registry as an
alternative to the v33 dense two-word-per-slot table. Its capacity is a
caller-provided limit on simultaneously packed processes; full tables must
reject another eviction, not silently discard ownership information.
For 24 slots an eight-entry sparse table uses 24 dynamic words rather than
48, at the cost of 85 more standalone pre-fold text words and linear scans.
Choose a strategy only after total permanent RAM and actual packed-process
concurrency have been measured. Both registries remain inactive, and the
production compressor/decompressor is deliberately deferred.

V33 introduces an isolated, checked per-slot descriptor registry for the
future packed lifecycle. Registration/teardown validate exact MM ownership;
release of a pinned packed extent fails without clearing the descriptor or
storage mode. A successful teardown frees the extent and resets the slot to
RESIDENT mode before reuse. It is deliberately not boot-allocated or wired
into live process destruction until packed eviction and a production codec
are enabled. The eventual runtime table would use 2 words per configured
slot (48 words at the current 24-slot 32K policy), not 512 words for the
maximum architectural process count. Before activation, consider whether
the same ownership contract can be satisfied more compactly to limit RAM.

As of v32 an isolated packed eviction path complements restore v31.
Eviction insists on true shrinkage of the retained writable suffix and
an encoder that fills its exact allocated capacity. The rollback boundary
is before freeing the source MM allocation. A synthetic pair encoder in
daimos-testkit demonstrates byte-for-byte round-trip integrity, but it is
not a suitable production codec. This stage does not enable scheduler
eviction or remove any legacy swap infrastructure.

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

V31 adds an unlinked whole-restore transaction, requiring caller-held process
lifecycle exclusion and PROC_F_TRANSITION. The strict ordering is: validate
PACKED mode/source MM ownership and the saved executable backing; pin source;
allocate and pin aligned destination without recursive reclaim; zero omitted
prefix and reload PURE; decode retained writable suffix **after** executable
reload (compressed DXR may fill the entire image); free source; publish VM
base and RESIDENT map mode; release pin and transition. Reload/decode failure
must free destination without modifying source or the stored descriptor.
Testkit host mock tests cover those paths, not live scheduler integration.

Disk-backed swap transactions now maintain the separately allocated mode
map (v30): out/in publish their transitions only after committing MM and
backstore records, while detach returns discarded disk-swapped slots to
RESIDENT mode. No PACKED state or scheduler selection is enabled yet.
The live disk transition helpers add 70 folded KCORE words; their behavioral
swap/failure tests still need to run in daimos-testkit.

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
