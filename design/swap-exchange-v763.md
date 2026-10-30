# v763: Variable-sized in-place process swap exchange

Status: source-level design/invariants, NOT implemented or native accepted.
The v758 writer WAIT/EXIT regression remains failing.

## v789: Ordered post-transfer commit coordinator

`proc_swap_exchange_commit_v789` calls the v785 PURE reconstruction,
v779 MM owner transfer, v786 paired records, v787 block accounting,
and v788 storage publication in that order. It is only valid after
v784 has completed destructive exchange under both transition locks.
Every unexpected return failure enters the storage invariant halt.
Native PDP-6 v789 checks successful dispatch sequencing with mock
dependencies (PASS). Full integration, request consistency checks,
actual media exchange, scheduler activation and the blocked v758 writer
case remain outstanding. This is not yet an end-to-end transaction.

## v788: Paired final storage-mode publication

`proc_swap_exchange_publish_v788` takes both process-entry pointers,
their stable slot IDs, and the common RAM base. It requires both
TRANSITION exclusions, unpins the shared MM extent, publishes A DISK
and B RESIDENT together via v778, and only then clears both transition
flags. A failure after reaching MM unpin is fatal rather than safely
rollbackable. The isolated native PDP-6 test verifies this ordering
and flag clearing (v788 PASS). This leaf does not itself validate the
entire exchange transaction; scheduler integration and the v758
writer regression remain outstanding.

## v787: Swap accounting and excess backing release

`proc_swap_exchange_account_v787` takes disk-first, old B saved-block
count, new A saved-block count and the v781 reservation flag. After MM
and process-record commit, it applies the process-only BACKSTORE usage
delta A-B and releases `[disk_first+A, disk_first+B)` if B is larger.
The extension when A is larger was already allocated by v781; no extra
allocation is made here. The isolated native PDP-6 test checks both
directions and the release block/length (v787 PASS). The caller must
ensure one-time use and exclusive lifecycle transitions; this helper
does not publish storage modes or activate the scheduler path.

## v786: Two process-record updates after exchange

`proc_swap_exchange_records_v786` accepts an eight-word request with
both process-table entry pointers, both swap-record pointers, shared
resident base, disk first block, A saved block count, and incoming B
executable backing state. After validation, it clears A's resident
base, installs B's resident base, stores A's disk record in the actual
`(first_block,,block_count)` layout, and replaces B's swap record with
its original executable backing. The native PDP-6 isolated test
verifies these changes and rejects a repeat with A no longer resident
(v786 PASS). The exchange caller must still synchronize both storage
modes, update accounting and PURE/u-area state, and clear transitions.
The helper is not called by the scheduler; full-system acceptance and
v758 writer recovery remain outstanding.

## v785: Incoming PURE prefix reconstruction leaf

`proc_swap_exchange_pure_v785` takes executable backing, common RAM
base, stable incoming u-area and a sector-aligned PURE prefix length.
After destructive sector transfer, it zeroes the prefix and calls the
existing `proc_swap_reload_pure` routine; a failed reload enters the
storage invariant halt. A zero-length PURE prefix needs no I/O.
Isolated native PDP-6 testkit v785 verifies the zero-then-reload call
order and the zero-prefix fast path (PASS). This does not test real
executable backing or complete process exchange, and the scheduler
still does not invoke the new stage.

## v784: Combined preparation and guarded transfer stage

`proc_swap_exchange_stage_v784` takes a two-pointer request containing
the v781 nine-word resource preparation record and the v783 seven-word
transfer record. Before reservation it verifies that both refer to the
same contiguous disk span and saved-state lengths and that writable
RAM starts correspond to sector offsets from the common base. It then
prepares backing/RAM, dispatches the guarded transfer, and invokes
v782 cancellation if v783 rejects the request before destructive I/O.
Once transfer begins, v783 halts on I/O failure rather than rolling
back data already overwritten. Native PDP-6 isolated v784 tests verify
success, cancellation, and early mismatch rejection (PASS).

No MM ownership or process record is published here. Caller must
validate physical extent ownership, full sector coverage, matching
PURE offsets, serialized operation and lifecycle exclusion before
entry. This does not resolve the native v758 writer regression.

## v783: Destructive-transfer guard

`proc_swap_exchange_checked_v783` checks positive saved-sector lengths,
nonnegative disk and sector-offset arguments, sector-aligned RAM starts,
and 18-bit RAM end bounds before invoking the v774 transfer. Invalid
requests fail without I/O. Any nonzero result after entering destructive
transfer halts through the storage invariant handler rather than
returning an unrecoverable half-exchanged image to the scheduler.
The isolated native PDP-6 test covers rejected zero-length and RAM
overflow cases without transfer calls, plus a valid transfer (PASS).
The helper does not yet validate actual disk allocation ownership or
guarantee full end-to-end publication. Scheduler invocation remains off.

## v782: Cancel prepared exchange before destructive I/O

`proc_swap_exchange_cancel_v782` reverses successful v781 preparation
before the first sector is overwritten. It restores the original A
extent size under A ownership, releases only the added BACKSTORE tail,
and clears the reservation flag. A failed MM shrink halts rather than
allowing an inconsistent process to resume. Native PDP-6 isolated test
v782 PASS checks both cleanup calls and flag clearing. It must not be
called after destructive transfer begins; scheduler integration and
end-to-end acceptance remain unfinished.

## v781: Coordinated disk and RAM preparation

`proc_swap_exchange_prepare_v781` accepts a nine-word request: the
five-word MM request, disk-first, existing incoming B block count,
outgoing A block count, and a reservation flag. It reserves the
contiguous disk extension first, then prepares RAM capacity using the
v780 helper. If RAM preparation fails, it releases only the newly
reserved disk suffix and clears the flag, preserving B's original
extent. Native PDP-6 isolated rollback regression PASS (v781).
No exchange sectors or process records are altered by preparation.

Remaining work includes failure injection for unsuccessful extension,
success-path capacity checks, cancellation of completed preparation
before destructive I/O, PURE reload, process-record publication and
native scheduler acceptance. The v758 writer remains blocked.

## v780: Reserve incoming RAM before transfer

The v779 commit helper could previously grow the outgoing MM extent
only *after* the incoming sector data had already been copied into
that larger range. That ordering risks writing outside a reserved
process allocation. Growth is now separate:
`proc_swap_exchange_prepare_ram_v780(request)` enlarges A's pinned
extent under A ownership before destructive transfer if B requires
additional RAM. The v779 commit helper now only retags ownership and
optionally shrinks afterward. Native PDP-6 v780 checks the sequence
growth -> transfer owner, and the no-growth -> owner -> shrink path.
The v779 standalone regression is superseded by v780 for this contract.

This is not a full preflight: backing-space reservation, transaction
failure handling, PURE reload, state publication, and full-system swap
acceptance remain outstanding. In particular, a successful growth
must be undone while A remains intact if subsequent preparation fails.

## v779: Native MM exchange transition ordering

`proc_swap_exchange_mm_v779` takes a five-word caller-owned request
(`base`, `old_slot`, `new_slot`, `old_words`, `new_words`). Following
completed data transfer and while both processes remain excluded, it
grows the pinned extent as its old owner when necessary, transfers the
MM owner, then shrinks under the new owner when necessary. Failures
before owner transfer return failure; failure after retagging halts.
Native PDP-6 isolated tests exercise both growth/retag and retag/shrink
ordering (v779 PASS). The caller must still establish preflight capacity,
PURE reconstruction, swap record updates and storage publication.
The scheduler does not invoke this helper; v758 remains unresolved.

## v778: Paired storage-mode publication primitive

`proc_storage_exchange_modes_v778(a_slot,b_slot)` validates both packed
storage modes (A RESIDENT, B DISK) before changing either. It then
publishes A as DISK and B as RESIDENT under the caller's two process
TRANSITION exclusions. Unexpected failures after the first publication
halt rather than allowing a half-updated pair to run. The isolated
native PDP-6 test checks success, stale-mode rejection without changes,
and same-slot rejection (testkit v778 PASS). The test reproduces the
real runtime header with a separate pointer to packed mode storage.

This primitive is not invoked by the scheduler. It does not transfer
physical MM ownership, modify swap records, reload PURE text, or
provide crash persistence. All those steps still require a complete
exchange transaction and native integration validation.

## v777: BACKSTORE release boundary correction

The release path in `backstore_free` had the same missing start-plus-count
bound as the v776 extension issue. It now rejects negative arguments
and requires `first <= backstore_blocks - count` before modifying the
bitmap or changing usage accounting. Native PDP-6 v777 verifies a
cross-boundary release leaves the bitmap/accounting untouched and that
the final legal block can still be released (PASS). This is a correctness
prerequisite, not process exchange completion.

## v776: BACKSTORE extension boundary correction

The transaction audit found that `backstore_extend` checked the new
length against pool capacity but failed to account for the starting
block. The native helper now rejects negative arguments and requires
`first <= backstore_blocks - new_blocks` before reading any bitmap
word. Native PDP-6 regression v776 tests the formerly unsafe
end-of-pool case alongside successful extension and occupied-tail
rejection (PASS). No process transaction is published by this change.

## v775: Unequal-length native coordinator regression and fix

The combined v774 coordinator incorrectly bypassed the outgoing A-only
tail whenever `SA > SB`: an inverted PDP-6 `CAMLE` branch. Replace it
with `CAMG`, so it writes the outgoing excess before overlapping sectors.
The testkit v775 native PDP-6 tests exercise both `SA=3, SB=2` and
`SA=2, SB=3`, checking the excess disk/restore sectors as well as the
overlapping sectors. Both PASS; prior equal-length v774 also PASS.
Testkit commit `5e69d4e`. This corrects a real production transfer
helper bug, but process-record publication and native writer recovery
are still not implemented or accepted.

## v774: Combined sector transfer coordinator (isolated)

`proc_swap_exchange_transfer_v774` accepts a seven-word, caller-owned
request describing the common disk span, two writable RAM starts,
SA/SB block counts, and two RAM sector offsets. It saves A-only disk
tail sectors before exchanging the overlapping region with the v772
ordering, then restores any B-only disk tail. The target assembler and
an isolated native PDP-6 two-sector combined transfer test pass (testkit
v774). This is **not** the complete process exchange transaction.

The coordinator assumes disk extension and sufficient aligned RAM have
already been reserved, both processes are excluded from scheduling,
and any failure after destructive writes is fail-closed by the caller.
It does not handle PURE reload, MM ownership and extent transitions,
bitmap release, per-process record publication or victim selection.
Unequal-length combined native tests remain necessary before integration.

## v773: Native saved-state excess-tail transfers

`proc_swap_exchange_tails_v773(request,phase)` transfers sectors not
present in both saved-state streams. Phase zero writes A-only sectors
at indices `[SB,SA)` into already-reserved contiguous BACKSTORE blocks
*before* the overlap exchange. Phase one restores B-only sectors at
indices `[SA,SB)` directly from BACKSTORE *after* overlap exchange.
No new data buffers are allocated. A native PDP-6 test exercises both
branches with independent disk contents (testkit v773 PASS).

The caller must reserve the needed disk extension, validate RAM extent
capacity and sector offsets, exclude both processes, and publish MM and
swap records only after all transfers and PURE reload complete. This
leaf does not implement those responsibilities. Native writer acceptance
remains FAIL.

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
