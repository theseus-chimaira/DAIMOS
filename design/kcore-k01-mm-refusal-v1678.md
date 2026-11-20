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
were not changed as part of this ownership fix. Read-only ownership
verification is covered by the subsequent v1700-v1701 stages. Full
guest PI6 injection and bounded administrative recovery remain open.

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
The u-area diagnostic snapshot is addressed by v1701 below. Full
guest PI6 testing and bounded recovery remain open.

### v1701: accurate GETTEARDOWN MM ownership diagnostics

`mm_uarea_probe_v887` searches the MM extent table by the stable
u-area physical base, then reports the matching record's type, owner
and pin count to `GETTEARDOWN(pid|0400)`. Previously the lookup and
metadata read ran with PI enabled. An interrupt-side MM release,
relocation or owner update could invalidate the slot between those
operations, producing a stale or misleading diagnostic error or pin
count during permanent-MM failure investigation.

The diagnostic now saves the caller's global PI enable state and
disables PI across the MM lookup and its type/owner/pin comparisons.
It preserves all existing return encodings: a valid pin count 0..255,
0400 for missing base, 0401 for wrong extent type and 0402 for wrong
owner. It restores the previous PI state on every return. No new
resident metadata, allocation or recovery policy is introduced.
This protects the MM snapshot only, not the entire process-control
query against subsequent state changes.

`daimos-k01-uarea-probe-pi-v1701.sh` exercises the source-linked
native PDP-6 diagnostic with an injected owner change after MM lookup
but before metadata read. The protected version leaves the descriptor
unchanged and returns the exact pin count; removing PI exclusion
rejects its negative mutant. All error classes and both initial PI
states pass, as do the existing v887/v886/v894 GETTEARDOWN, v888 MM
cause, v955 failure-matrix and v956 u-area lifetime tests. Both KCC
KCORE build modes pass. Full physical guest PI6 injection, bounded
recovery and other cross-subsystem ownership checks remain open.

### v1702: atomic free-gap allocation and descriptor publication

`mm_alloc_aligned_noreclaim` previously found a free physical gap in
the sorted MM table, then determined the insertion slot and called
`mm_extent_insert` with global PI enabled throughout. An interrupt-side
MM allocation/free could change the descriptor table in between: a
previously free gap could become occupied before its new PROCESS or
MODULE descriptor was published. This is a separate transaction from
the v1697-v1701 release/pin/verifier work.

After successful argument validation, the allocator now saves the
previous PI state, disables global PI before `mm_find_fit`, and
restores PI after every fit, no-space, fragmentation, descriptor-full
or successful publication path. The invalid-argument path never
acquires the PI lock. The existing frame gains **one transient stack
word**, not permanent RAM, for the saved PI state. AC10..AC16 still
occupy -010(17)..-2(17), so the PI snapshot is at -011(17); it must
never overwrite the saved AC16 image. The normal return value and
caller output base remain unchanged.

The first v1702 implementation incorrectly used -2(17) for that PI
snapshot. The existing v545 production compaction/VM regression caught
the resulting AC16 corruption. This was corrected before commit, and
the v1702 test now checks AC16 explicitly and rejects a second unsafe
mutant recreating the same frame overwrite. Its first mutant removes
PI exclusion and injects another allocation into the chosen gap
immediately before descriptor insertion. Both mutants fail; the
production native PDP-6 test passes exact type/owner publication,
descriptor capacity (-3), no-space (-1), invalid args (-4), preserved
caller PI on/off and unchanged AC16. v545/v548/v538 full MM/VM/source
integration and both KCC PURE modes pass. The old v749 boot-image
test cannot run without a separately prepared clean PDP-6 image.

### Existing bounded-retry and operator policy (reviewed v1702)

`proc_finish_mark_pending_v892`, `proc_finish_service_v881` and
`proc_finish_backoff_advance_v885` already keep the original VM/MM
owner quarantined while the slot-0 reaper retries allocation-free
cleanup no sooner than a shared 16/32/64/.../512 tick interval.
v1098 and v1102 source-linked native tests pass, including negative
mutants for an unarmed clock and a reset interval. This is bounded
**retry rate**, not a finite number of attempts or a promise that
permanently corrupt ownership can ever be reconciled automatically.
`pdp10-doc/KERNEL/OOM-FAIL-CLOSED-RECOVERY-V1185.MD` explicitly
requires offline repair and controlled restart for an unprovable MM
owner; it forbids forced unpin/free. No new administrative bypass or
per-owner recovery table is justified by this review.

Remaining K01 gates include full physical guest PI6 and MM fault
injection on final source, verification of owner quarantine and
bystander progress, and a residual cross-subsystem refusal audit.

### v1703: full-guest permanent MM owner-refusal matrix

Three existing 256K PDP-6 physical full-system fault-injection guests
were booted against DAIMOS f7e8450a (v1702) without editing the
production tree. The versioned testkit wrapper
`daimos-k01-physical-owner-gate-v1703.sh` selects each case and requires
its established guest-side PASS witness, rather than treating a built
image, partial boot or mere simulator exit as acceptance.

* v909: one genuine permanently pinned u-area, 7 checked MM releases,
  7 exact -5 refusals, 6 reaper attempts, retained owner and unreaped
  WAIT, independent runnable user, zero errors.
* v1101: one wrong-owner u-area MM descriptor, 7 checked releases,
  7 exact -4 refusals, 6 reaper attempts, correct 0402 ownership
  diagnostics, no false free, independent runnable user, zero errors.
* v1125: the same 7 wrong-owner -4 refusals, plus an independently
  serialized filesystem-provider lease still held after 80 observed
  provider waits, while the original quarantined MM owner and a
  separate runnable user remain intact; zero errors.

These are three individually successful runs; the combined selector
`K01_PHYSICAL_CASE=all` was not itself run. Native v1098/v1102 retry
clock and v887 ownership diagnostic suites were also re-run and pass.
This provides full-system evidence for permanent refusal quarantine,
bounded retry rate and bystander progress, but does not yet prove a
transient physical VM/u-area refusal later completes exactly once or
that every PI6 interrupt interleaving is safe. The existing full guest
v997/v1106/v1153 recovery fixtures need source-anchor review against
the v1695-v1702 MM/VM changes before they can be counted as current
acceptance. K01 remains open for those tests and a final integrated
ownership/refusal audit; no forced administrative MM release is added.

Verify permanent actual MM free/pin/unpin/rebase and process restore
refusals through all live owners, including ambiguous or missing extents,
bounded retry/reaper state, consistent typed GETTEARDOWN diagnostics, and
allocation-free administrative repair. Preserve K02-K04 and K05-K16 as
separate tasks: this stage does not implement general process teardown,
OOM victim choice or RAW PURE transaction acceptance. Complete physical
fault injection and mainline/feature-branch integration before closing
K01. No `pdp10-doc` completed-feature documentation is published while
K01 remains open.
