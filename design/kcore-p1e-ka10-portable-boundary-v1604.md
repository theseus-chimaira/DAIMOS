# P1-E swapless PURE: portable lifecycle and machine-owned geometry (v1604)

Date: 2026-11-15
Status: design boundary and staged source-linked preflight only.
P1-E production scheduler integration remains OPEN. No new KCORE/MRES
resident code, permanent RAM, process field, disk format or syscall ABI.

## Objective

Finish PDP-6 swapless PURE dropping and, only after its real lifecycle
works, evaluate lossless compression of the retained mutable state.
The result must later permit a KA10 private low segment to be reclaimed
without pretending that its independently owned read-only high segment
is a PDP-6 PURE prefix. No speculative page tables, multi-region
registries, runtime function tables, or extra per-process state.

## Shared process-lifecycle contract

Common source may verify a valid process slot, TRANSITION ownership,
stable u-area, stopped/sleeping state, absence of an outstanding
physical user mapping or fatal FILE continuation, saved executable
identity, and a valid storage-mode transition. It may preserve
executable immutability and provider/unmount guards; dispatch retry,
rollback, finalization, and parent WAIT semantics are also common.
The admission helper MUST NOT infer a physical APR base, alignment,
contiguity, TEXT/DATA boundary, or number of mapped segments.

The target VM/backend is responsible for describing the exact logical
private writable region(s) to be retained. Its geometry, alignment,
physical MM ownership and placement, and mapping activation are
machine-specific. The current implementation needs just one region;
do not introduce a general region vector before a real target needs it.
The logical retained start/count are uncompressed quantities; the MM
descriptor owns the actual physical stored allocation and its length.

PDP-6: the dedicated VM helper derives a sector-rounded omitted
prefix from the existing validated DXR2/TX2 PURE backing record and
retains [prefix, VM_SPACE_WORDS) from the one contiguous APR mapping.
The v1381 MM rebase changes the original pinned PROCESS descriptor
to the high-address PACKED_PROCESS suffix with no data copy. When the
old gap is free, v1384 can reconstruct the prefix in place; otherwise
the bounded full restore must allocate and initialize a complete APR
extent before publication. Compressed DXR MUST NOT write over the
only surviving raw suffix. Use exact loader destination bounds.

KA10 (future, NOT implemented): the high code segment is independently
mapped and potentially shared, with immutable read-only backing. An
inactive process retains only its private writable segment; no per-
process high-segment reload/free may invalidate another attachment.
High-segment physical ownership, protection and references are separate
from the process's mutable RAM representation. No hardware copy-on-
write or page-granular mapping is assumed.

## Storage-state and codec rules

Current two-bit per-slot map (18 slots per 36-bit word) holds RESIDENT
(0), DISK (1), and PACKED (2), where PACKED currently means inactive
RAW RAM, not actual compression. Value 3 is unused. A future separate
COMPRESSED representation may use 3 *only after* its ownership and
failure semantics are proven. Do not allocate a second state registry.
Raw disk/drum process swap remains unchanged and never writes an
encoded process record.

The bitmap v1451 encoder/decoder is independent of MMU geometry: it
operates only on trusted, bounded 36-bit word buffers. Encode into a
separate, exclusively owned MM extent; verify a strict whole-system
net RAM gain, including peak scratch and metadata, before publishing.
Retain the original until the new encoding and exact stored length
have passed admission. Decode while both source and destination remain
owned/pinned; preserve source and source guards on any failure.
Compression MUST be optional and MUST follow a real, accepted RAW
swapless PURE integration. A non-shrinking result falls back to RAW.

## Transaction requirements

1. Admit only an explicitly swapless boot configuration, a quiesced
   eligible process, and immutable executable backing retained for the
   entire inactive lifetime. Do not perform disk swap as a fallback.
2. Validate MM owner/type/extent/pins and all logical bounds before
   changing any descriptor or writing any destination.
3. Hold TRANSITION throughout candidate selection, MM mutation, VM
   deactivation, storage-mode publication and final unlock; keep PI
   disabled only over the short atomic publication region.
4. Publish the nonresident VM/storage owner together, never expose
   an incomplete image to scheduler, syscall, EXEC, BRK or compaction.
5. On restore, reserve exact executable and mutable spans, reconstruct
   immutable code without writing retained mutable words, restore
   private mutable words, publish a fully activatable mapping, then
   release the inactive representation only when ownership is known.
6. Recover under ENOMEM, fragmented APR, refused pin/unpin/free, bad
   executable source, aborted provider I/O, or interrupted EXEC/EXIT.
   No blind cleanup of an uncertain live owner. Keep retryable
   inactive state and immutable source references, or fail closed.
7. Test live PI6, source unmount/mutation guards, two inactive owners,
   32K/48K swapless pressure and exact KCORE+MRES/peak-RAM cost before
   claiming P1-E production completion. Existing strict 32K/
   optimization HOLD remains in force until separately authorized.

## Completed source increment v1604

Two private staging headers now separate the common
`pure_life_prepare_v1604()` process preflight from
`pdp6_pure_skip_v1604()` geometry. The v1381 zero-copy drop/full
restore and v1391 bounded full restore reuse those checks, keeping
their existing MM primitives, process descriptor, and 36-bit backing
record. This is source-linked staging, NOT production scheduler wiring.
The exact v1381/v1391 public symbols and lifecycle transactions are
unchanged.

DAIMOS-testkit native PDP-6 v1604 tests valid lifecycle admission,
missing/invalid owner, active user physical mapping, fatal close,
terminal-only release, positive and rejected PURE geometry. Current
source-linked v1381 DROP/RESTORE, v1384 prefix and v1395/v1397
rollback still pass with a repaired rootless test fixture.

## Next implementation gate

Resolve the production load/restore selection without duplicating
resident EXEC/DXR decoding (the old v1487 shared-parser candidate
cost +180 isolated TEXT words). Then implement safe RAW-only
scheduler service on a provably swapless profile, use the existing
mode map and MM owner, and prove that no activated process can
retain an inactive PURE representation. Only after physical guest
acceptance should the v1451 codec enter a size-gated experiment.

PDP10-DOC KERNEL/SWAPLESS-MEMORY.MD remains the architecture and
runtime acceptance reference. This design is not a handoff or
a substitute for native PDP-6 physical-guest results.
