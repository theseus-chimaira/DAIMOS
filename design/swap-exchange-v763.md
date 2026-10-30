# v763: Variable-sized in-place process swap exchange

Status: source-level design/invariants, NOT implemented or native accepted.
The v758 writer WAIT/EXIT regression remains failing.

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
