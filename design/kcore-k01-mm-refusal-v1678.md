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

### v1704: real PI6 and physical VM/u-area retry-to-completion

The v997/v1106/v1153 copied-kernel recovery injector still expected an
old `mm_free` success sequence ending in `POPJ 17,`. Since v1697,
successful physical release must branch to `mm_free_restore_pi_v1697`
before returning, or the caller's interrupt state is lost. The test
injector was brought up to date at both VM and mixed-DPY MM release
sites; the production kernel required **no change**.

New `daimos-k01-recovery-injection-v1704.sh` exercises all four
production-source injection modes (base, physical VM, integrated and
mixed PIPE/DPY/VM/u-area) and rejects a negative mutant with the old
MM-free `POPJ` tail. All four modes and that mutant pass on native
source, before attempting the expensive full-guest build.

The combined v1153 disposable 256K PDP-6 guest then booted and
**passed all independent v997/v1103/v1106/v1117/v1144/v1153 guest
counter assertions**, with its explicit HALT and zero error counters:

* Process VM: one real MM pin, one real unpin, two checked MM frees,
  one genuine MM_BUSY refusal, one accepted free and one backing detach.
* PIPE, DPY and u-area: each had one live pin, one matching unpin,
  one MM_BUSY refusal and one exactly-once physical free. All four
  physical allocation bases were distinct.
* The FILE->VM->u-area->ZOMB publication order was checked after
  the final release; refused provider/FILE/HUP steps did not fabricate
  completion. The foreign filesystem lease persisted.
* Qualified PI6 clock advanced by 55 ticks, a separate user advanced
  3 times during the measured span, and the full test registered
  48 independent stale WAIT/PID/event refusals without process-MM
  or slot growth. Bystander counter reached 36; all error counts zero.

These results close K01's transient physical MM recovery and mixed
PI6-progress acceptance gap. They do not establish that every possible
interrupt interleaving was executed. K01 stays open for the remaining
physical swap/restore refusal matrix, final source-wide MM ownership
and failure-propagation review, and an integrated acceptance sign-off.

Verify permanent actual MM free/pin/unpin/rebase and process restore
refusals through all live owners, including ambiguous or missing extents,
bounded retry/reaper state, consistent typed GETTEARDOWN diagnostics, and
allocation-free administrative repair. Preserve K02-K04 and K05-K16 as
separate tasks: this stage does not implement general process teardown,
OOM victim choice or RAW PURE transaction acceptance. Complete physical
fault injection and mainline/feature-branch integration before closing
K01. No `pdp10-doc` completed-feature documentation is published while
K01 remains open.

### v1705: strict F07 swap-in fault gate blocked before injection

The physical v1494 BACKSTORE bitmap-refusal injector expected the
pre-v1507 `proc_swap_service_failed` event path. Current production
first saves `PROC_SWAP_RETRY_LH` in the owner's existing u-area word,
then delivers `SYS_EVENT_TERM` at
`proc_swap_service_failed_event_v1507`. The test-only injection point
was moved after that event, retaining the production retry deadline.
`daimos-k01-swap-restore-injection-v1705.sh` tests both v1494 and
v1495 source-backed instruments and rejects a missing-event mutation.
The v1494 verifier rejects 12 synthetic negative controls, and seven
native swap owner/restore regression suites passed. No production
kernel source was altered in v1705.

The subsequent **real 256K PDP-6 F07 run did NOT pass**. The guest
booted, but its strict original v1207 admission test observed only
6 of 18 requested RUN child acknowledgments and halted on user-side
`fail('n')`. Physical swap-out count advanced to 10 and swap-in to 3
in the live sampler, so the swap engine had operated; the v1494
instrumentation counters remained `armed=0`, `faults=0`, `errors=0`.
The final backing usage was 1264 blocks, not the required clean zero,
because the guest stopped at that earlier workload check. These
counters do NOT support a swap-in bitmap-fault recovery claim. The
F07 RUN admission cause requires separate investigation; do not weaken
its original 18-child check or move the injected fault to fabricate
success. The local pre-existing change to the v1494 TERM witness's
stack offset was preserved, not committed as v1705.

### v1706: no orphaned destructive-EXEC launch copy on MM refusal

`exec_replace_current_lowmem` keeps a kernel-dynamic MM allocation
(owner 013) containing the validated launch block while it destroys
the old user image and stages a new one. At
`exec_lowmem_free`, both successful and fatal-after-commit paths
previously ignored `mm_free` and returned the EXEC result. If MM
refused release with -4/-5, the stack frame then disappeared, losing
the only pointer to still-owned physical RAM without an administrative
or process-owned retry record. That would falsely report completion
after an unproven permanent MM release.

The v1706 code checks the actual `mm_free` result and branches to the
existing distinct `proc_swap_storage_invariant_v30` HALT on refusal.
The accepted normal free still returns the exact original EXEC status;
it does not fabricate a release, force-unpin an extent or allocate a
new persistent recovery table. The fail-stop is deliberate because
the existing transient launch-copy owner has no safe surviving retry
record. Future process-lifetime recovery design remains K02/K04.

`daimos-k01-exec-launch-free-v1706.sh` source-links the real EXEC
cleanup leaf to a native PDP-6 fixture with a validated owner 013,
testing normal release and independent -4/-5 failures. A mutant that
returns to normal EXEC completion after a refused MM free fails. The
existing v200 low-memory EXEC rollback, v1 ABI and v20 PURE tests pass,
as do both KCC KCORE builds with PURE verification on/off. No new
permanent KCORE RAM was introduced.

### v1707: unpublished provider scratch cannot survive MM refusal

An audit of all production `pushj 17,mm_free` callers revealed three
more owner-only-in-transient-state hazards after the v1706 EXEC fix.
`pipe_create_free` released a newly allocated PIPE object after FILE
descriptor creation failed. If MM refused release, `pipe_create_fail`
returned -1 and discarded AC10, the object's only owner. Similarly,
`d6fs_mount_free` discarded the private, not-yet-installed D6FS reader
on failed `vfs_mount_prevalidated`; neither the mount table nor the
stack frame would retain its physical base after return.

TSFS `tsfs_read_words` allocates a decoder buffer only for a compressed
restart block and keeps its address on the transient stack. Both
successful decode cleanup and the error branch to
`tsfs_read_block_free` formerly dropped this pointer after `mm_free`
returned -4/-5. Since the caller has no durable reference to that
buffer, retry or graceful error return would silently orphan a still
MM-owned extent.

All four physical-free call sites now branch on a nonzero `mm_free`
return to the existing `proc_swap_storage_invariant_v30` KCORE HALT.
An ordinary accepted free still follows the original return, mount
cleanup and decoder result. This is a strict ownership-integrity stop,
not an automatic recovery algorithm. It neither unpins another owner
nor allocates resident state; no permanent RAM or new owner encoding.

`daimos-k01-temp-owner-free-v1707.sh` source-links the four actual
PDP-6 production cleanup leaves, using only test-side MM and result
stubs to inject exact -4 and -5 refusals. It verifies release arguments,
one accepted free on the normal path, exactly one invariant stop on
each hard refusal, and rejects four mutants with the refusal branches
removed. Both KCC KCORE PURE variants build. The legacy v870 pipe-close
fixture currently fails to link because it extracts code without the
later `vfs_dtfs_poisoned_slot_v1531` dependency; it is not credited as
a passing v1707 regression. The F07 admission failure remains an
independent unclosed physical-guest gate for K01.

The v1707a follow-up uses the real TSFS decoder owner value 011
(octal), with a source assertion preventing fixture drift. It gives
the older v1538 TSFS source-linked I/O test an unreachable test-side
invariant-HALT symbol for the newly imported KCORE entry. All v1538
root and secondary-mount cases and four unsafe mutants pass. The
independent v899 real PDP-6 physical PIPE-close test also passed:
one pin, one -5 refusal, one unpin, successful eventual release and
three checked close attempts, without errors.

### v1710: never relocate the executing PROCESS in MM compaction

`vm_extent_move` correctly refuses to relocate the currently running
process because its active APR continues to reference that physical
image. The compactor previously tried ALL unpinned PROCESS entries,
including that active owner, collected the resulting expected -5 and
treated it as a hard MM error after scanning the remaining candidates.
That turns ordinary fragmentation into a false invariant refusal and
prevents further swap reclamation by `mm_alloc_aligned`.

`mm_compact` now skips a nonzero `proc_current_slot` owner before
calling `mm_move_extent`. Other owners still undergo exactly the
existing physical/VM checks; their hard -4/-5 errors are still
preserved. No owner record, persistent RAM or retry state is added.
`daimos-k01-compact-current-v1710.sh` source-links the production
compactor and proves: active-owner immobility yields ordinary -2 (not
spurious -5), a different owner genuinely refusing move still yields
-5, and a movable noncurrent owner can resolve the fit. The mutant
without the active-owner skip is rejected. Historical v1678 compactor
and pressure, v535 dispatch and v545 full VM/FS compaction regressions
also pass. The subsequent physical F07 diagnostic admitted and reaped
18 of 18 children, up from 6/18. The v1494 bitmap fault was armed,
its backing bitmap repaired, and final backing usage reached zero, but
strict F07 acceptance still FAILED: TERM events were 2 instead of 1
and witness errors were 5. That separate failure needs investigation.

### v1711: atomic swap-victim advisory pin snapshot

`mm_is_pinned` was the remaining lookup-then-read MM pin observer
running with global PI enabled. A PI-side free/pin/descriptor movement
could change the extent between matching the physical base and reading
its pin field. Swap victim selection uses this advisory answer, so a
mixed-time result could select the wrong candidate or bypass a live
pin preflight (the actual swap still has independent checked pinning).

The v1711 leaf excludes PI from the base lookup through the pin read,
then restores the original caller PI state while retaining exactly
the existing boolean 0/1 result, including missing-base behavior.
No permanent metadata or new retry state is introduced. The source-
linked native PDP-6 `daimos-k01-is-pinned-pi-v1711.sh` verifies stable
unpinned/pinned/missing-base outcomes and PI initially on/off. It
injects an interrupt-side pin exactly after lookup, and the mutant
without PI exclusion fails. The v1698 atomic pin regression and
v1710 active-owner compaction test pass alongside it. The result is
not a lasting ownership lease; the swap-out transaction must still
acquire and verify its own physical pin before committing a swap.
