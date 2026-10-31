# v755: Swap-in resource pressure and scheduler liveness

Status: partially implemented, native pressure acceptance failed (v758).

## v833: Scheduler pressure invokes exchange; full-system FAIL (2026-10-31)

`proc_swap_mres_service_v481` now tries the resident candidate from
`proc_swap_victim` after a `+1` resource-pressure response from
`proc_swap_in`. It supplies a temporary 064-octal-word graph on the
slot-0 stack to `proc_swap_exchange_execute_v832`. Successful exchange
returns the requested incoming slot to the scheduler; an ineligible
candidate or pre-transfer failure retains the existing 0100-octal-tick
cooldown. No permanent RAM allocation was added.

The complete SWAP MRES assembler and isolated native v826 admission /
abort regression PASS. The full-profile testkit scheduler runner v669
was rebuilt with the v833 source and ran in PDP-6 SIMH, but TIMED OUT
after 300 seconds (exit 124), output ending `<wpF!W` rather than the
required writer WAIT/EXIT completion. The last reported simulator PC
was 013605. Log: `$TMPDIR/daimos-v833-native.log` on the test host.
This is a failing integration attempt, NOT a v758 fix. The next
diagnostic must distinguish whether a suitable resident pair is found,
whether v832 is entered/rejected, and where the writer remains blocked.
In particular, the single `proc_swap_victim` choice is not yet a search
over all exchange-admissible candidates. Do not mark the scheduler
acceptance resolved until the original workload completes.

## v832: Locked exchange orchestration, scheduler still disconnected (2026-10-31)

`proc_swap_exchange_execute_v832(a_slot,b_slot,graph)` now composes
v829 pair-lock acquisition, v831 graph construction, and v790 exchange
execution. The caller supplies 064 octal (52 decimal) temporary graph
words; there is no new permanent RAM allocation. On pre-destructive
failure it uses v830 to verify ownership, release the pin and clear
both transitions. A successful v790 publishes the two process states
and releases the pin and locks itself.

The native PDP-6 v826 fixture exercises the new wrapper with an
injected v790 failure and verifies that neither transition remains
held (PASS). The separate integrated v819 transaction fixture still
passes with the current production sources. Neither fixture proves
successful full orchestration with real MM/device callbacks, and the
scheduler has not yet been connected. The v758 writer WAIT/EXIT
full-system acceptance remains FAIL.

## v831: Native construction of locked raw exchange graph (2026-10-31)

`proc_swap_exchange_build_v831(a_slot,b_slot,base,graph)` now constructs
the v790 pointer graph for equal-sized, raw-image exchange in 52
caller-owned temporary words. It allocates no permanent kernel state.
The graph covers the run and stage pointers, preparation and transfer,
MM ownership, PURE metadata, swap records, backing accounting and
publication. In particular, the commit MM pointer must alias the
preparation request, as demanded by the existing v791 preflight.

The extended testkit native PDP-6 fixture checks graph pointers and
sector fields, invokes the production `proc_swap_exchange_preflight_v791`
on the generated graph, and then aborts the paired lock safely (PASS).
Real MM and disk callbacks are still stubbed in this fixture; the
graph builder is not wired to the scheduler or destructive v790
dispatch. The v758 writer WAIT/EXIT full-system regression remains FAIL.

## v830: Pre-transfer paired lock abort (2026-10-31)

Added `proc_swap_exchange_abort_v830(a_slot,b_slot,base)` to the SWAP
MRES. It is callable only before destructive transfer by the owner of
both v829 TRANSITION locks and the unique resident MM pin. It verifies
the process identities, state of both locks, resident/nonresident bases,
and exact MM owner/extent/single-pin contract before unpinning and
clearing both locks. Invariant violations halt rather than releasing an
unverified process. There is no new persistent RAM allocation. The
native PDP-6 v826 fixture now exercises the complete lock/abort cycle
with deterministic MM callbacks (PASS). The v790 request graph and
scheduler dispatch are not yet connected, and the v758 writer WAIT/EXIT
regression remains unpassed.

## v829: Paired transition lock and MM pin acquisition (2026-10-31)

`proc_swap_exchange_lock_v829` is a new SWAP MRES entry for execution
only from serialized slot-0 scheduler context. It first applies the
conservative pair gate, then sets both TRANSITION bits, pins A's
resident extent and verifies the precise MM owner/size/single-pin
contract. On failure, it releases any acquired pin and clears both
transition bits; on success it returns the physical base with both
locks and the single pin held for the v790 caller. It allocates no
persistent RAM. The testkit native PDP-6 fixture uses deterministic MM
callbacks to check pin failure rollback and successful lock acquisition
(PASS). Real MM pin behavior and integration with the transaction are
not yet covered by this fixture.

The function is not yet called by the production scheduler. The v758
writer WAIT/EXIT regression remains an active acceptance failure.

## v828: Live victim exclusion in native pair gate (2026-10-31)

The read-only exchange pair gate now rejects the current CPU process,
an outgoing user-map owner and a resident victim whose MM extent is
already pinned. The native PDP-6 fixture now checks these three negative
cases and verifies an eligible pair afterward (PASS). The gate preserves
the original process-entry pointer across the MM pin query. No persistent
RAM or disk state is modified. Scheduler transition locking and full
v790 transaction dispatch are still disconnected; the v758 writer
WAIT/EXIT acceptance remains unresolved.

## v827: Native scheduler-state admission gate (2026-10-31)

The conservative `proc_swap_exchange_pair_v826` now additionally
checks the actual packed scheduler states. The outgoing resident
candidate must be RUN, SLEEP or STOP, matching existing victim
eligibility. The incoming swapped process must be RUN, matching the
pending scheduler request. The extended native PDP-6 fixture rejects
an incoming SLEEP and an outgoing invalid/zero state, then admits
the restored SLEEP/RUN pair (PASS). No persistent RAM allocation.

The helper remains side-effect-free: this is neither a paired lock nor
a call from the scheduler. MM pin ownership, the full request graph,
and native v758 writer WAIT/EXIT acceptance are still open.

## v826: Native read-only equal-size exchange pair gate (2026-10-31)

Added `proc_swap_exchange_pair_v826(a_slot,b_slot)` in the existing SWAP
MRES assembly. It performs a side-effect-free, conservative shape check:
distinct valid slots, present stable u-areas, no TRANSITION, resident A,
nonresident B, equal nonzero 02000-aligned image sizes, a present A
executable-backing record and B disk record, no omitted PURE prefix in
either executable, and B's disk block count exactly covering the raw
image. A native PDP-6 testkit fixture exercises valid admission, unequal
image rejection and absent disk record rejection (PASS).

This does NOT lock the processes, check scheduler state or MM ownership,
choose a victim, construct the full v790 pointer graph, or trigger an
exchange. Those checks must occur under scheduler exclusion before the
transaction can be used. No new permanent data allocation is introduced;
the v758 writer WAIT/EXIT full-system regression remains unpassed.

## v822: Exchange resource-admission boundary (2026-10-31)

The testkit `daimos-swap-exchange-admission-v822.c` verifies ten
resource-preflight cases (PASS). Equal 064000-octal-word resident and
incoming images with 208 saved sectors each require no additional RAM
extent or BACKSTORE tail, unlike ordinary 208-sector swap-out when only
176 sectors are free. Larger incoming RAM or outgoing backing images
require the exact contiguous extension; aggregate free space is not
enough. Invalid alignment and zero-sized images are rejected.

This is a host model of capacity only, not a production scheduler gate.
An actual admission must additionally check process-state provenance,
RUN/SLEEP policy, MM pin and in-place ownership, transition exclusion,
executable PURE backing, disk bitmap and workspace lifetime. The v758
writer WAIT/EXIT regression remains FAIL; no production change is claimed.

## v821: Scheduler-to-exchange integration audit (2026-10-31)

The production `proc_swap_mres_service_v481` still calls only
`proc_swap_in`. On a resource-pressure result it installs a 0100-octal
tick cooldown, preserves the disk record, and returns to the scheduler.
`proc_swap_mres_reclaim_v481` still calls `proc_swap_victim`, then
`proc_swap_out`, before attempting `mm_compact`. A source search found
no production call site of `proc_swap_exchange_run_v790` outside the
exchange implementation itself. Thus the accepted isolated native
v819 exchange fixture does not yet provide resource progress to the
v758 writer workload. This is an integration finding, not a native
regression rerun or a claim that the deadlock is fixed.

Before connecting the scheduler, establish a locked resident-victim and
swapped-target pairing, verify both stable process identities and PURE
backing, and prove that the pinned victim can contain the incoming image
or can grow in place. Verify contiguous BACKSTORE extension or release,
and ensure no dependency on a third process or unavailable scratch RAM.
The scheduler must distinguish an exchange that is feasible from one
that cannot progress. Preserve the existing v758 failure as an active
release gate until the original writer WAIT/EXIT workload passes.

## v762: Native victim-footprint evidence (raw swap)

The testkit v762 native simulator diagnostic stopped at the same failed
reclaim transaction as v754 and captured the process table and per-slot
swap records. The resident process slots were **1 and 11 through 18**;
all nine retained an executable-backing record `037` (octal) with zero
PURE text length. Every resident image was `064000` octal words
(26624 decimal), and the raw swap-out requirement for each was 208
BACKSTORE sectors. Only 176 sectors were free. The other process slots
2 through 10 were already nonresident. The testkit checker verifies
this exact native signature and PASSed; testkit commit `5d4c700`.

Therefore selecting a different ordinary resident victim cannot fix the
**observed** full-image capacity stall; none has a smaller PURE-adjusted
payload. Generic reclaim policy may still benefit from capacity-aware
victim selection, but that is not a sufficient correction here. Any real
fix must prove a resource-progress guarantee for admitted processes or
transactionally exchange images without requiring another full swap span.
Do not solve this by enabling untested compressed swap or PACKED state.
The native writer WAIT/EXIT acceptance remains FAIL.

## v759: Deadlock capacity gate and workload dependency

The v283 native writer workload creates a writer and 16 pressure children.
The pressure children sleep on a gate that the parent opens **only after**
`wait_report(writer)` succeeds. At the captured v754 failure there are
just 907 free managed RAM words (largest gap 760), and 176 BACKSTORE
blocks free, against 26624 resident words/208 blocks needed for a full
process image. No additional full-sized resident process can be swapped
out into the free BACKSTORE space to restore the writer. The v757
cooldown does not release the blocked gate or produce either resource.

The testkit `daimos-mm-swap-capacity-deadlock-v759.py` verifies the
physical extent descriptors and backing-capacity inequality directly
from the preserved native v754 trace. It PASSed with RAM free=907,
largest gap=760, BACKSTORE free=176 versus 208 required; testkit commit
`ece0d8a`. This is a diagnostic PASS, not a scheduler recovery PASS.

For successful execution of this **admitted** workload, a scheduler-only
retry policy is inadequate. Design must either support a crash-safe
resident/swapped-image exchange without requiring a second full-sized
BACKSTORE span, or prevent over-admission and make the workload correctly
handle an explicit resource error. Simply failing RUN would change the
existing v283 acceptance contract, which assumes all 16 pressure children
start successfully. Preserve the v758 failure until this policy is
resolved and tested. No additional production kernel change in v759.

## v758: Fresh native pressure workload FAIL

Ran the complete testkit `daimos-mm-swap-scheduler-v669.sh` against a
fresh 256K PDP-6 full-profile image built from v757 sources, with
`DAIMOS_MM_TRACE_KEEP=1`. The D6FS root preflight passed: superblock
715 seq=1 CLEAN, 179196 blocks, 360 FCBs clean. Native boot reached
`D6FS LOADED` and the diagnostic INIT emitted `<w`, but no writer
completion or `Q HALT` followed. The simulator timed out after 300
seconds (exit 124; validation script exit 1). Its last PC was 014112.
This is a **native acceptance FAIL** and does not establish forward
progress or recovery.

Retained DevSpace2 artifacts: `$TMPDIR/daimos-mm-swap-scheduler-v669-4191`
and `$TMPDIR/daimos-scheduler-v758-native.log`. The native run changes
the test boot disk: do not reuse it as a CLEAN image. Next determine
whether the scheduler is repeatedly requesting swap, selecting resident
jobs, or idle without an eligible process. Capture the run queue,
current slot, cursor, cooldown, and backstore occupancy at the stall.
Do not claim a scheduler-policy fix without a passing native workload.

## v756: Typed transaction result implemented (partial)

The swap-in transaction now returns `PROC_SWAP_IN_PRESSURE` (+1) for
allocator resource errors `MM_ERR_NOMEM`, `MM_ERR_FRAGMENTED`, and
`MM_ERR_DESCRIPTORS`. It retains `PROC_SWAP_IN_FATAL` (-1) for invalid
records, failed storage/restore, and rollback failures; zero is success.
The classification is implemented in the native PDP-6 assembly and
the C reference path, with matching constants in `proc_swap.h`.

The versioned testkit `daimos-mm-swap-result-contract-v756.sh` passes
assembly and interface checks. **The scheduler still treats any nonzero
return as fatal and is not changed by v756.** Consequently v756 does not
resolve the native writer TERM or establish a recoverable scheduler.
Production commit `e720c3a`; testkit commit `fb6bbcf`.

Do not change `proc_swap_mres_service_v481` to ignore +1 in isolation:
the idle loop triggers a new reschedule immediately, and ordinary runq
selection can choose the same swapped process again. A complete atomic
scheduler retry/admission policy remains necessary.

## v757: Bounded pressure retry and resident scheduling (candidate)

The native PDP-6 swap service now distinguishes +1 (pressure) from
negative fatal results. On pressure it leaves the swap record intact,
clears the pending service request, and sets a shared 0100-octal-tick
cooldown. The 60 Hz scheduler clock decrements the cooldown. During the
delay, ordinary selection skips swapped runnable candidates and a
swapped RT owner yields to ordinary run-queue selection; resident jobs
remain runnable. Once the delay expires, swapped candidates become
eligible for a fresh restore attempt. No per-process retry table is
allocated. Existing fatal TERM delivery remains unchanged.

The isolated PDP-6 compiler/assembler run and the testkit v757 structural
contract PASSed. Production commit `7cc44e6`; testkit `1d50cb6`.
**Native full-system acceptance is still pending**, including writer WAIT,
actual recovery after memory becomes free, no-resident-runnable behavior,
and RT priority restoration. The 0100-octal-tick delay is a provisional
retry mechanism, not a substitute for the outstanding admission/headroom
invariant. Do not mark the swap feature complete on this evidence alone.

## Reproduced failure

The native v750-v754 trace in `design/kcore-feature-baseline-v735.md`
establishes an MM_ERR_NOMEM return while restoring slot 2, with only
907 physical words free. BACKSTORE has 176 free blocks; a full-size
resident victim requires 208 blocks. The SWAP MRES service converts
every nonzero swap-in result to a fatal TERM, including this case.

## Source-level invariants to preserve

* `proc_swap_in` preserves the swapped disk record when allocation fails
  and clears PROC_TRANSITION. No disk read or image publication occurs.
* The existing `proc_swap_in` return is binary (0/-1); it does not say
  whether the failure was capacity, malformed record, or disk I/O.
* The PDP-6 scheduler scans a single run queue and chooses the lowest
  effective priority. A swapped winner requests slot-0 swap service and
  becomes an idle-context job; RT ownership is selected separately.
* The idle loop always requests another reschedule after servicing a swap
  request. Suppressing TERM alone would immediately reselect the same
  swapped job and can starve resident runnable processes.
* Process extents require native APR 02000-octal alignment. Any solution
  must preserve existing pin, transition, and storage-publication rules.

## Required production fix

1. **Typed swap-in result:** retain zero for success, distinguish
   reversible `MM_ERR_NOMEM`/`MM_ERR_FRAGMENTED`/descriptor pressure from
   invalid swap metadata, irrecoverable read error, and failed MM cleanup.
   Check C and PDP-6 assembly transaction parity. Do not classify every
   negative result as retryable.
2. **Forward progress:** when pressure prevents a swap-in, allow an
   existing runnable resident process to run, without evicting or killing
   the valid swapped process. Prevent immediate repeated selection of the
   same unrestorable slot; consider RT owner behavior explicitly.
3. **No permanent deadlock:** define the no-resident-runnable case and
   an admission/headroom invariant, especially if BACKSTORE cannot hold
   one additional full process image. A deferred swap request must have a
   source of future resource progress or an explicit nonfatal failure
   surfaced to the admission caller; indefinite idle retries are invalid.
4. **Native acceptance:** cover both the 176-free/208-needed pressure
   fixture and a recovery fixture in which another process exits and frees
   memory. Verify writer normal EXIT/WAIT, preservation of swapped image,
   no spurious TERM, bounded scheduler retries, and corruption/I/O fatal
   paths. Keep tests in daimos-testkit.

Do not use a larger swap partition as a substitute for fixing scheduling
semantics. Do not change the backstore record format or introduce resident
telemetry merely to hide this capacity failure. The final code-size sweep
remains deferred until feature/correctness acceptance.
