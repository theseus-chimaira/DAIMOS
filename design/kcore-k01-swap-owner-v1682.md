# K01 swap MM owner and rollback safety, v1679-v1684

Date: 2026-11-19. Status: **implemented and native-tested partial K01
correctness repairs; K01 remains open**. Base: mainline KCORE after
v1678 typed compaction/reclaim refusals. No RAW PURE branch merge,
new storage mode, automatic OOM killing or general process-teardown
acceptance is implied by this report.

## Failure modes and changes

### v1679: refusal to release the PURE diagnostic pin

The conditional `PROC_SWAP_VERIFY_PURE=1` swap-out verification failure
path called physical `mm_unpin` but ignored its result. It then cleared
`PROC_TRANSITION_RH`, potentially allowing a process whose physical pin
was still held or whose MM ownership could not be proved to be resumed.
It now checks `mm_unpin` and goes to the existing KCORE MM invariant
fail-stop rather than clearing the transition on any refusal.

### v1681: failed initial swap-out pin

The preliminary `mm_is_pinned` query returns zero for both a valid
unpinned extent and a missing MM descriptor. When the process subsequently
entered TRANSITION, `mm_pin` could return `MM_ERR_INVAL` for the missing
descriptor. The old error branch cleared TRANSITION without proving the
VM owner. The v1681 branch now stops at the existing MM invariant gate
and never publishes that process as runnable. A saturated pin count is
similarly treated as an unsafe refusal if reached despite preflight.

### v1682: exact extent ownership after pin acquisition

`mm_pin` validates only the physical base, not the process slot, MM type
or expected size. A stale process VM base could therefore successfully
pin another owner's extent, write data to BACKSTORE and later encounter
an `MM_ERR_INVAL` when attempting a typed `mm_free`. The normal rollback
would clear TRANSITION despite the unproven resident MM owner.

Immediately after successful `mm_pin`, swap-out now invokes the existing
read-only `mm_process_verify_pinned_v792(base,slot,words)`. It requires
exactly one PROCESS extent, the current slot as owner, the precise VM
size and exactly one physical pin. Failure retains TRANSITION and the
acquired pin, entering the existing MM invariant gate. The owner/type/
size mismatch is not repaired by inventing an extent or unpinning
another owner's memory. A valid extent continues the original transfer.

These changes add no permanent kernel data, process fields, owner tables,
allocator workarounds or OOM policy. The v1682 additional verification
call is inside the swap-out service path, not the interrupt scheduler.

### v1683: physical release refused after successful swap write

The normal swap-out path first writes a backing-store image, unpins the
resident process extent and then calls `mm_free(base,PROCESS,slot)`.
Previously a nonzero `mm_free` result entered `proc_swap_out_record_fail`,
released the newly allocated backing run and ultimately cleared the
process TRANSITION flag. That is unsafe: a missing or wrong-owner MM
descriptor, or a newly pinned extent, has *not* been released and cannot
be treated as ordinary failed backing allocation. The caller may not
resume the process with unproven resident ownership.

The v1683 refusal now enters the existing `proc_swap_storage_invariant_v30`
fail-stop immediately. It retains the transition lock and the original
MM descriptor unchanged; it does not free the unpublished backing run
or invent a physical release. No additional persistent RAM is used.
The ordinary successful `mm_free` and swap publication remain unchanged.

### v1684: pinned growth corrupted swap preparation's request pointer

The production `mm_process_grow_pinned` leaf used AC10 as the extent
index and AC11 as the proposed limit without saving either register.
The caller `proc_swap_exchange_prepare_v781` holds the nine-word swap
request pointer in AC10 across `proc_swap_exchange_prepare_ram_v780`,
which calls that MM leaf when the incoming process needs more RAM.
The leaf therefore corrupted its caller's request pointer on success
and on several refusal paths. Subsequent disk-tail cleanup and request
publication could address unrelated words. The MM leaf now saves and
restores AC10/AC11 on all exits. No permanent RAM or new policy is used.

## Native validation

All tests reside in `daimos-testkit`, not in the DAIMOS source tree:

- `tests/validation/daimos-k01-verify-unpin-v1680.sh` runs the exact
  source-linked PURE verification rollback with production `mm_unpin`.
  One-pin release succeeds; missing pin and missing descriptor retain
  TRANSITION. Removing the v1679 guard fails the regression.
- `tests/validation/daimos-k01-swap-pin-v1681.sh` runs the source-linked
  swap-out initial pin transition and physical `mm_pin`. Missing MM
  descriptor and saturated pin count fail closed; ordinary pin succeeds.
  The historical unsafe transition-clear mutant fails.
- `tests/validation/daimos-k01-swap-owner-v1682.sh` executes the real
  MM pin plus exact `mm_process_verify_pinned_v792` and production branch.
  Wrong type, owner, size and pre-existing pin fail closed; an exact
  source is accepted. Removing the verifier error guard fails.
- `tests/validation/daimos-k01-swap-free-v1683.sh` runs the production
  post-write free branch with real PDP-6 `mm_free` and `mm_delete`.
  Wrong owner and a pinned extent must retain TRANSITION and physical
  descriptors; a valid owner is freed. Restoring the unsafe rollback
  branch fails the regression.
- `tests/validation/daimos-k01-grow-registers-v1684.sh` executes the
  real PDP-6 growth leaf and checks AC10/AC11 after growth, no-space
  refusal and owner refusal. Removing the register saves/restores
  fails the test; the earlier v769 native growth regression also passes.
- The earlier v1678 compactor and reclaim-pressure tests PASS, as do
  legacy v667 unpin invariant/native checks and v792 MM verification.
- KCORE KCC source generation builds with `PROC_SWAP_VERIFY_PURE=0` and
  `=1`; it is not a full boot or physical multitasking acceptance.

The old `tests/target/daimos-swap-pure-text-pdp6-v1/run.sh` currently
cannot start: it references the removed `core/d6lz_pdp6.s`. This is an
obsolete fixture dependency, not a PASS or an observed kernel failure;
it was not rewritten as part of K01. Real kernel boot, PI6 pressure,
full swap transaction and cross-target tests remain outstanding.

## Still open

Audit and test long-lived owner corruption during free/rebase/restore,
provider release and terminal teardown, then define bounded diagnostic
or administrative recovery rather than only fail-stop on lost MM proof.
K01 cannot be marked completed on the strength of these partial guards.
K02-K16 retain their separate scope. No `pdp10-doc` completed-feature
material is published until K01 itself is accepted.
