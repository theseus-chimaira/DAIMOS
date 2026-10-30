# v763: Variable-sized in-place process swap exchange

Status: source-level design/invariants, NOT implemented or native accepted.
The v758 writer WAIT/EXIT regression remains failing.

## v772: Native multi-sector overlapping exchange loop

`proc_swap_exchange_overlap_v772` consumes a six-word request with disk
start, A writable RAM start, B writable RAM start, overlap sector count,
and their respective saved-sector offsets. The loop chooses ascending or
descending indices (memmove-style) to avoid destroying A sectors not yet
written, and invokes the tested v771 sector transfer for each block.
An isolated PDP-6 reverse-overlap test verifies both disk-sector and
RAM-destination contents (testkit v772 PASS). The helper is not connected
to the swap scheduler, does not handle non-overlapping tails, PURE reload,
MM ownership, or storage-record publication. No full-system recovery claim.

## v771: One-sector exchange transfer leaf

`proc_swap_exchange_sector_v771(disk_block,a_ram,b_ram)` now executes
the bounded per-sector operation: invalidate cache workspace tag, read
the incoming sector to the shared 0200-word buffer, write the outgoing
resident sector into that disk block, and copy buffered incoming data
into its resident destination. An I/O failure returns -1 and a complete
transfer returns 0. The native PDP-6 isolated test with mock backing
I/O verifies both the first and last sector words (testkit commit
`70a323e`, PASS). No new persistent RAM data structures are used.

This leaf is deliberately NOT invoked by the scheduler or process swap
service yet. It assumes serialized access to the VFS scratch workspace,
does not schedule sectors or manage unequal excess lengths, does not
reserve backing extensions or publish process states, and requires
additional I/O-failure injection and full native-system validation.

## v770: Contiguous BACKSTORE extent extension

`backstore_extend(first,old_blocks,new_blocks)` now validates an existing
allocated run and an immediately adjacent free tail, then marks only
the additional blocks and updates `backstore_blocks_used`. Validation
precedes all bitmap mutations, so rejected requests do not partially
reserve blocks. This preserves the single-span swap record and does not
support fragmented allocation. The isolated native PDP-6 regression
tests successful extension and occupied-tail rejection (v770 PASS).
No process swap code calls the primitive yet; end-to-end exchange and
writer WAIT/EXIT remain unfinished.

## v769: Adjacent-gap MM process growth

`mm_process_grow_pinned(base,owner,new_words)` is implemented in the
PDP-6 assembly MM and matching C reference. It changes no process
location and allocates no new descriptor; it permits growth only for
an exactly-once pinned, owned process extent, with nonzero APR-aligned
new size, within the containing arena and strictly before the next
allocated extent. Otherwise it preserves the old extent unchanged.
The isolated PDP-6 test validates successful growth and rejection of
overlap and wrong-owner requests (testkit v769 PASS). The native swap
exchange does not yet call this primitive. This does not resolve v758.

## v768: Pinned resident extent shrink

The native MM implementation and C reference now provide
`mm_process_shrink_pinned(base,owner,new_words)`. It validates exact
`MM_TYPE_PROCESS` ownership, a single pin, nonzero APR-aligned size,
and `new_words <= old_words`, then changes only the size half of the
existing span descriptor. The base and descriptor order stay intact;
the released high tail becomes an implicit free gap. The operation is
not used by the swap scheduler yet. The native PDP-6 leaf simulator
test verifies a successful shrink and wrong-owner/growth rejection;
daimos-testkit commit `799a1e3` (PASS). This covers smaller incoming
images only: growth requires separate adjacent-RAM capacity validation.

## v767: Native owner-transfer execution verified

The testkit `daimos-mm-owner-transfer-native-v767.sh` executes the
production `mm_process_transfer_pinned` instruction sequence on PDP-6,
with a one-entry MM descriptor table and the real packed pin/type/owner
metadata. A valid 1->2 and 2->3 transfer preserves type and pin count;
wrong-owner and same-owner transfers are rejected. Testkit PASS,
commit `8ac38f6`. The portable C MM reference now defines the same
transfer contract. The native test is limited to the transfer leaf;
it does not validate scheduler exclusion, variable-size extent changes,
or the complete exchange transaction. The original writer stall still
requires the latter.

## v766: Native MM owner-retag primitive (not yet used by swap)

`mm_process_transfer_pinned(base,old_owner,new_owner)` now changes only
the RH owner field of an existing `MM_TYPE_PROCESS` extent. It requires
the current owner to match, distinct valid process-slot IDs (1..0377
octal), and exactly one physical pin. It preserves type, pin and span;
it does not allocate, free, resize or move RAM. Caller-held process
lifecycle exclusion remains mandatory. No swap code calls it yet.

The target assembler and testkit `daimos-mm-owner-transfer-v766.sh`
assembly/source-contract checks pass; they do NOT yet demonstrate
native runtime behavior. Before exchange integration, add native
failure-injection/ownership tests and an explicitly validated size
transition for unequal resident images. The existing no-progress
writer regression remains FAIL.

## v765: Preflight and native MM ownership blocker

The testkit `daimos-swap-exchange-capacity-v765.py` checks 110090
synthetic BACKSTORE bitmap/contiguous-extension cases and explicitly
rejects incoming images exceeding the available resident image extent.
Host model PASS, testkit commit `4b0c014`. The model never modifies the
production allocator or its bitmap. Contiguity is mandatory under the
current one-word disk swap record; even enough aggregate free blocks
cannot make an unavailable adjacent extension safe.

The current native `mm.h` and `mm_pdp6.s` provide allocation, pinning,
unpinning, freeing, and compaction, but **no verified atomic ownership
transfer** of a pinned `MM_TYPE_PROCESS` extent from victim A to incoming
B. Changing the process VM base and disk swap records without changing
MM extent ownership would leave teardown, compaction and future MM frees
using the wrong owner. A native exchange must have an MM-validated retag
operation (and a carefully bounded resize operation for differing RAM
sizes) performed under lifecycle exclusion. Do not silently emulate it
by freeing A's descriptor before B's image is safely installed: that
exposes the occupied RAM span to allocator reuse.

Before implementation verify the exact MM extent representation,
pin-count transition, descriptor capacity, APR-aligned boundaries and
whether larger B can be accommodated without relocating A while its
source is still needed. The full native writer gate is still FAIL.

## v764: Bounded transfer-order feasibility (host model)

The testkit `daimos-swap-exchange-order-v764.py` models 256564 synthetic
layouts with unequal complete resident sizes, writable suffix offsets,
and saved-sector counts. All simulated *capacity-feasible* layouts pass.
Testkit commit `592e8de`. This is NOT native kernel acceptance.

For a single contiguous resident A span, an incoming B span fitting
inside that RAM allocation, and a disk extent extended contiguously
before exchange when required, the following ordering preserves unread
saved sectors with one 0200-word transfer buffer:

1. Write A's saved tail beyond B's existing disk span into the newly
   reserved contiguous disk extension **before** modifying resident RAM.
   These blocks do not contain B data.
2. For the overlapping disk-sector indices `0..min(SA,SB)-1`, read B
   sector into scratch, write A's corresponding saved sector directly
   from resident RAM to the B disk sector, then place the buffered B
   sector into its resident destination. Transfer in descending index
   order if B's writable suffix begins later in RAM than A's; otherwise
   use ascending order. This is the same dependency direction as memmove.
3. If SB > SA, copy the remaining B-only disk sectors to RAM **after**
   the overlap: every A source sector has already been saved to disk.
4. Reconstruct B's omitted PURE portion only after the saved-state copy
   is complete, so executable loading cannot overwrite still-needed A.

The model validates byte-for-byte final RAM and disk state, but does not
model device calls, bitmap fragmentation, process locks, or nested VFS
workspace usage. It cannot establish that the exchange works when B's
complete image is larger than A's resident allocation, that an adjacent
contiguous disk extension is available, or that a sector buffer survives
device calls. Those are explicit preconditions to validate before kernel
integration. There is no justification yet for scatter/gather swap records.

## Scope and failure model

Exchange one resident process A with one disk-swapped runnable process B
when ordinary swap-out followed by swap-in cannot progress. Process swap
is volatile: do not add a persistent journal, post-reboot reconstruction,
or power-failure recovery. In an unrecoverable write/device failure, the
kernel may halt rather than resume a partially modified process. A
recoverable I/O failure must not resume either process until its image is
complete. The shared BACKSTORE allocator also serves MEMFS: a sector
owned by MEMFS must never be overwritten by exchange.

This is NOT a requirement that A and B have equal process-image sizes,
equal swap lengths, or the same PURE backing properties. Compressed disk
swap and in-RAM PACKED state are separate, deferred features.

## Independent capacity dimensions

Let RA/RB be complete APR-aligned RAM image sizes, and SA/SB be the
corresponding *saved writable-state* block counts after PURE omission.
SA and SB need not equal RA/0200 or RB/0200. Let FA and FD denote truly
available aligned RAM and freely allocatable BACKSTORE blocks respectively.

1. Incoming B needs RB contiguous, APR-aligned RAM words. If RB > RA,
   the excess RB-RA must come from demonstrably free/compacted RAM (or a
   separately justified relocation of other processes). Copying sectors
   does not create RAM. If RB < RA, the unused RAM tail may be freed only
   after B is complete and its MM ownership is published correctly.
2. Outgoing A needs SA writable BACKSTORE blocks. The reusable B extent
   provides SB blocks; if SA > SB, at least SA-SB additional free blocks
   are needed. Existing records support a SINGLE contiguous span only.
   Extending that span in place is valid only when adjacent blocks are
   free, owned exclusively by the transaction, and the resulting span
   fits the record's half-word length. Scattered additional blocks are
   *not* representable without an explicit future layout change.
3. If SA < SB, the surplus disk tail may be freed after the exchange has
   completed; freeing early would expose sectors still holding B data.
4. A two-sector difference is not the same as two free contiguous blocks:
   actual bitmap placement and the free contiguous run must be checked.
5. The one-word record carries disk start/length; the full original RAM
   extent size remains in the three-word process descriptor, and the
   executable backing remains in each process's stable u-area.

## Transfer and publication

Keep A and B under lifecycle exclusion / TRANSITION, exclude both from
the scheduler and victim selection, and pin/validate the resident A
extent. Preserve the original B disk extent reservation for the entire
copy, including any still-unread B sectors. Never release or overwrite
a block while its original B payload is still needed. Transfer saved
sectors through a bounded workspace; for PURE B, reconstruct omitted
text from the saved executable backing before publishing B runnable.

The source shows `fs_block_workspace` is used by PURE reload and backing
I/O. The exchange buffer lifetime and possible nested VFS calls MUST be
audited before reusing that workspace; do not assume it survives a call.
The existing BACKSTORE bitmap tracks one bit per 0200-word sector and
must be updated exactly once for any allocated/retired blocks. Existing
proc_swap_blocks_used must change by SA-SB after the transaction.

On complete exchange, publish A's disk start/length and preserved PURE
backing, B's new resident MM base, both storage modes, and both process
descriptors under the same exclusion; clear TRANSITION only after all
state is consistent. If I/O cannot continue, halt/fail closed rather
than returning either process to the scheduler partially exchanged.

## Major unsolved constraint: staging without overwrite

Reading B's disk sector into a buffer and replacing that disk sector
with A's sector is safe only if A's source sector remains available
and the buffered B sector can reach its final RAM destination. With
different PURE prefixes, A's saved-sector offset and B's restored RAM
offset are NOT generally the same. Their ranges may overlap future
unread A sectors. A single 0200-word buffer does not by itself prove
that an arbitrary overlapping exchange is lossless. The implementation
must establish a safe transfer order or use a bounded additional staging
strategy, and prove it for unequal RA/RB and SA/SB before native use.

## Acceptance gates

* Synthetic unequal-sized/raw/PURE layouts, zero/positive free tails,
  contiguity failures, and correct bitmap/accounting updates.
* Prove no disk block still holding unread B data is overwritten and no
  RAM word still holding unread A data is replaced; check both transfer
  directions and PURE text offsets.
* Native failure injection for retryable read/write errors and fail-closed
  unrecoverable I/O; no recovery after a system halt is required.
* Native writer WAIT/EXIT/pipe integrity, no spurious TERM, no scheduler
  busy-loop, and preservation of resident and disk image integrity.
* Test code belongs in daimos-testkit. Do not change production records
  or add a scatter/gather map without an explicit measured justification.

The immediate next task is a bounded transfer-order feasibility analysis
for unequal images and a read-only footprint/capacity preflight. Do not
implement an equal-size-only shortcut or falsely mark v758 as fixed.
