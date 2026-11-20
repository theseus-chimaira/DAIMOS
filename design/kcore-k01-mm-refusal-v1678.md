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

Verify permanent actual MM free/pin/unpin/rebase and process restore
refusals through all live owners, including ambiguous or missing extents,
bounded retry/reaper state, consistent typed GETTEARDOWN diagnostics, and
allocation-free administrative repair. Preserve K02-K04 and K05-K16 as
separate tasks: this stage does not implement general process teardown,
OOM victim choice or RAW PURE transaction acceptance. Complete physical
fault injection and mainline/feature-branch integration before closing
K01. No `pdp10-doc` completed-feature documentation is published while
K01 remains open.
