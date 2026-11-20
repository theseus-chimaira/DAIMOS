# K01 permanent MM refusal propagation, v1678 (implementation stage)

Date: 2026-11-19. Scope: mainline PDP-6 KCORE process compaction and
allocation pressure. K01 remains OPEN; this stage does not claim completion
of all provider, free, pin, rebase or restore failure policy.

## Source-confirmed defect

In `system/kernel/mm/mm_pdp6.s`, `mm_compact` previously continued past
every nonzero `mm_move_extent` result. If a move was refused because its VM
owner could not safely stop or release a translated mapping,
`vm_extent_move` returned `MM_ERR_BUSY` (-5), but the final compaction
result was ordinary `MM_ERR_FRAGMENTED` (-2). The allocator could then
initiate process-swap reclamation despite that unresolved refusal.

In `system/kernel/proc/proc_swap_pdp6.s`, the MRES process-reclaim loop
previously retried victim selection after every nonzero `mm_compact`
result, including hard error codes, rather than returning the exact MM
refusal. The outer `mm_alloc_aligned` pressure loop previously ignored
the result of `proc_swap_reclaim` and retried up to its fixed pressure
limit. These paths conflated a hard refusal with normal space pressure.

## v1678 change

- The compactor remembers an `MM_ERR_BUSY` from a refused candidate,
  continues searching for another *eligible* movable extent, and returns
  `MM_ERR_BUSY` rather than `MM_ERR_FRAGMENTED` if none succeeds.
- An unexpected hard negative move result (below `MM_ERR_DESCRIPTORS`)
  stops compaction immediately; valid fragmented/no-memory results keep
  their existing meanings.
- The process-swap service returns a hard MM result immediately without
  selecting additional victims. The allocator recognizes that typed
  result and returns it to its caller rather than re-entering pressure.
- Existing MM descriptor/owner/pin records remain authoritative. No new
  KCORE BSS, per-process metadata, retry field, user ABI or automatic
  OOM killing is introduced. This stage does **not** unpin or forcibly
  release refused memory.

## Evidence and limits

`daimos-testkit` holds two new source-linked native PDP-6 fixtures:

- `tests/validation/daimos-k01-mm-refusal-v1678.sh`: `MM_ERR_BUSY`,
  `MM_ERR_INVAL`, alternative candidate success, ordinary fragmentation,
  no-memory and descriptor-identity assertions. PASS.
- `tests/validation/daimos-k01-pressure-chain-v1678.sh`: production
  `mm_alloc_aligned` and `proc_swap_mres_reclaim_v481` with deterministic
  MM/VM/FS leaves. Verifies exact -5/-4, no extra victim, bounded normal
  fragmentation, eventual allocation, and rejection before first swap.
  PASS.

Both fixtures correctly FAIL against the original mainline source at
`adcc042d` (the failing expected state is exit witness 1). Additional
source-linked native regressions passed: compactor dispatch v535,
full-path v538, pinned compaction v539, allocator/VM v545, and exact
u-area/MM release v1097 (including its unsafe-mutation controls). The v545
test harness required restoring its previously missing extraction of
the v1260 arena-capacity helper; this is a test-only change.

`make -C system/kernel kcore COMPILER=kcc` passed with isolated build
directory. This validates source generation, not complete guest boot or
full binary/physical acceptance. The optional 256K relocation-integrated
v537 test was deliberately stopped during its full guest/userland build;
it did **not** produce a pass/fail result for this change.

## Outstanding K01 work

### v1697: atomic MM owner release against interrupt-side pins

`mm_free` formerly called `mm_find_base`, verified type and owner,
checked that the pin count was zero, then deleted the descriptor with
global PI enabled. A pin acquired by an interrupt between the check
and `mm_delete` could leave that physical user holding memory whose MM
owner had already been deleted and made available for allocation.

`mm_free` now calls the existing `mach_pi_disable` before searching the
extent table and restores the exact prior global PI state on success,
wrong-type/owner/missing-base (-4) and pinned (-5) refusal. The return
code is preserved across restoration, and no permanent RAM or extra
MM descriptors are introduced. This does not force-release a refused
owner; the caller retains its existing typed recovery contract.

`daimos-k01-mm-free-pi-v1697.sh` source-links the production MM find,
free and delete leaves on a native PDP-6. It checks owner refusal,
existing pins, a pin arriving just as PI closes, a simulated pinning
interrupt at the delete boundary, balanced PI and normal release.
Removing the PI exclusion rejects the negative mutant. Existing MM
pin lifetime, swap release, TTY, u-area, FIFO, DPY and DTFS scratch
fixtures now source-link the real production PI helpers, rather than
introduce extra copies of their logic. Their source-extraction ranges
were updated where unrelated later helpers had made them stale.

The ordinary MM `mm_pin_adjust` update remains a separate PI audit
target. Physical full-guest interrupt injection and bounded K01
administrative recovery are still open.

### v1698: atomic MM pin and unpin publication

`mm_pin_adjust` previously searched the descriptor table, read the
physical pin field and wrote a new count with global PI enabled. If
an interrupt-side `mm_free` released the descriptor between lookup
and pin publication, the pending pin could write into an unowned MM
slot. If a separate pin arrived after an unpin read, the stale unpin
could overwrite and lose the second consumer's pin count. v1697
protected the release side, but not this reverse interrupt window.

The pin-adjust leaf now saves the exact prior PI state, disables global
PI before its descriptor search, and restores the prior enable state
after success, absent/underflow (-4) or saturation (-5). It uses the
same short inline `CONI/CONO PI` sequence as the BRK critical section;
no permanent RAM, helper call or new ownership state is introduced.

`daimos-k01-mm-pin-pi-v1698.sh` tests source-linked real MM pin/unpin
code on a native PDP-6, including two pins, two unpins, empty unpin,
missing base, an interrupt-side free at the pin store, an additional
pin at the unpin store, and restoration of both enabled and disabled
PI states. It rejects an unsafe mutation omitting PI exclusion while
using the simulator's real PI hardware state for the interrupt gate.
Related source-linked swap, TTY, u-area, DPY, FIFO and DTFS tests still
exercise the unchanged owner/error classes. K01 remains open for full
guest PI6 fault injection, bounded owner recovery and residual audits.

### v1699: serialize pinned process owner and extent-size publication

The three pinned PROCESS descriptor operations (`mm_process_transfer_pinned`,
`mm_process_shrink_pinned` and `mm_process_grow_pinned`) previously
validated exact owner and sole physical pin with PI enabled, then
updated owner or extent size in a separate instruction. An interrupt
could change the pin count, release an owner or take an adjacent gap
between validation and the final store. This is a distinct read/write
window from the `mm_free` and `mm_pin_adjust` fixes in v1697-v1698.

Each operation now saves the caller's global PI state, excludes PI
through descriptor lookup, type/owner/pin validation, arena and next
extent checks where applicable, and physical descriptor publication.
The prior PI enable state is then restored on success or the original
typed failure. `mm_process_grow_pinned` still preserves AC10/AC11;
the physical base is kept in scratch AC7 across the PI snapshot.
No new persistent data, MM owner representation or retry state exists.

The source-linked native PDP-6 test
`daimos-k01-pinned-mutate-pi-v1699.sh` uses real production mutation
leaves and injects a conflicting physical pin immediately before
owner transfer/shrink publication, or a successor extent intruding into
the gap just before a grow publication. Three distinct mutants omit PI
disable in one leaf each; all are rejected. Legacy v767/v768 scripts
now extract only their named production routines rather than pulling
unrelated MM code. The v769/v1684 grow and existing swap exchange
unit tests pass. Several older source-linked swap-exchange integration
fixtures (v805, v812, v854) currently fail to link because their
extractions omit later production dependencies such as
`PROC_UAREA_RH`, `PROC_SWAP_RETRY_LH` and `vfs_namespace_root`.
They are not counted as passing integrated acceptance for v1699 and
were not changed as part of this ownership fix. Full guest PI6 injection,
bounded administrative recovery and subsequent read-only ownership
verification audit remain open K01 requirements.

### v1700: atomic exact pinned-process descriptor verification

`mm_process_verify_pinned_v792` is a read-only MM transaction used
before swap-out writes, swap-in restores and process exchange. It
previously compared the extent length with the expected size, then
read the type/owner/pin word while global PI was enabled. If a PI-side
MM operation changed the size between those reads, the verifier could
return success based on a length from an older MM descriptor and
ownership/pins from the current one. An inconsistent snapshot is not
proof that the physical image being used still fits its descriptor.

The v1700 verifier now saves the previous PI state and excludes
interrupts across descriptor lookup and all four comparisons. It
restores that state on success or refusal, retains the original 0/-1
return ABI and preserves AC10/AC11 without persistent words. Invalid
arguments still refuse before touching PI. This guarantees only one
consistent point-in-time snapshot; the calling swap transaction must
still maintain its pin and transition ownership afterwards.

`daimos-k01-verify-pi-v1700.sh` runs the actual leaf in native PDP-6
and injects a changed extent length precisely between its size and
metadata reads if global PI is enabled. The protected verifier leaves
the descriptor unchanged, preserves AC10/AC11 and both enabled and
disabled PI states, and still rejects wrong size/owner/multiple pins.
Removing the PI exclusion makes the negative mutant fail. Source-linked
v792 and v1682 ownership tests and both KCC KCORE build modes pass.
Full guest PI6 testing, bounded recovery and the separate u-area
diagnostic-snapshot audit remain open.

Verify permanent actual MM free/pin/unpin/rebase and process restore
refusals through all live owners, including ambiguous or missing extents,
bounded retry/reaper state, consistent typed GETTEARDOWN diagnostics, and
allocation-free administrative repair. Preserve K02-K04 and K05-K16 as
separate tasks: this stage does not implement general process teardown,
OOM victim choice or RAW PURE transaction acceptance. Complete physical
fault injection and mainline/feature-branch integration before closing
K01. No `pdp10-doc` completed-feature documentation is published while
K01 remains open.
