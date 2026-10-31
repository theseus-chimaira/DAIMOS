# v755: Swap-in resource pressure and scheduler liveness

Status: v851 full-system scheduler-pressure acceptance PASSES with the
010000-octal-sector test backstore; exchange transaction unproven.

## v878: Post-commit EXEC fatal path closes FILE before irreversible EXIT (2026-10-31)

The PDP-6 `native_sys_exec` branch handling `EXEC_REPLACE_FATAL` (-2)
used to jump straight to `proc_exit_current`, unlike ordinary EXIT,
fatal events and user APR faults. After low-core EXEC has destroyed the
old VM, returning to userspace is forbidden, but the surviving stable
u-area may still own up to 16 open FILE descriptors. The direct exit
skipped FILE cleanup. `proc_finish_slot` then correctly rejected the
live descriptors (v869), resulting in `proc_exit_release_failed` HALT
even when all FILE closes would have succeeded.

The fatal EXEC branch now invokes the existing `file_close_all` from
the process-private executive stack before switching to the permanent
slot-0 stack. Successful closes permit ordinary finalization; a close
failure preserves its descriptor for the finalizer's ownership guard,
not an orphaned provider reference. Because the old VM is already gone,
even a failed close cannot return an error to user EXEC. That condition
still HALTs until the nonreturning deferred-cleanup mechanism is safe;
this patch does **not** claim system-wide fatal-exit progress or OOM
recovery. No new RAM or backing state is introduced.

Native PDP-6 testkit v878 uses the actual `native_sys_exec` leaf and
injects post-commit fatal return, successful/failed FILE close,
recoverable pre-commit EXEC failure, successful EXEC and failed mapping.
It verifies finalizer handoff only for fatal EXEC, preserves a rejected
descriptor, and leaves all nonfatal paths unchanged: PASS. The same
fixture FAILs on pre-v878 v877 source at the missing close check.
Native v867/v868/v869/v875/v876 FILE/EXIT/teardown regressions PASS.
Lowmem KCORE link PASS, exclusive end `021227` octal, one word above
v877 `021226`. Full-system post-commit EXEC fault injection remains
open; automatic OOM victim killing stays disabled.

## v877: Post-v876 full-system session teardown acceptance (2026-10-31)

With the v875 controlling-TTY commit-order and v876 RT/TRANSITION
changes present, the unchanged session-leader exit/HUP guest
(`daimos-session-teardown-v1.sh`) PASSes end to end: running and
stopped members receive HUP, the controlling TTY detaches and later
reattachment succeeds. The simulator launches with Type 340 headless
and unrelated WCNSLS/OCNSLS SDL graphical devices disabled; the
previous unisolated execution crashed in SDL with exit 139 before INIT
could run, so that crash was not evidence about session behavior.

This is ordinary full-system session-lifecycle acceptance, separate
from native fault injection of failed FILE/VM/HUP destruction, and is
**not** full-system OOM or forced APR/EXEC cleanup acceptance. v877
changes documentation only; KCORE stays at `021226` octal.

## v876: Guard fatal teardown transition and preserve live RT ownership (2026-10-31)

The old `proc_finish_slot` revoked a target's real-time owner lease
*before* running `proc_session_teardown`, and marked the process
TRANSITION only *after* those HUPs succeeded. HUP may invoke provider
cleanup that sleeps. A fatal-other target could therefore become
runnable during its own partial teardown; failed HUP left it alive
with RT ownership already revoked. A later `vm_space_destroy` error
could also leave a still-live target without its RT lease.

The resident PDP-6 finalizer and its C fallback now reject a target
already in TRANSITION. After validating that its FILE table is empty,
they mark TRANSITION before session-HUP callbacks and keep RT ownership
intact. Failed HUP clears TRANSITION before returning an error; failed
VM destroy already clears TRANSITION and now also leaves RT ownership
unchanged. Only after the VM has been successfully removed is RT
ownership revoked. This does not allocate memory or add a static table.
Failure *after* VM deletion is still a distinct incomplete-teardown
case and may leave TRANSITION set; never resume its invalid user VM.

Native PDP-6 v876 runs the production finalizer, checks pre-existing
transition rejection, observes TRANSITION and unchanged RT ownership
during HUP and VM callbacks, injects both failures, verifies rollback,
then verifies normal finalization and RT lease release: PASS. Against
pre-v876 v875 code the same test FAILs, as expected. The v875 native
TTY/HUP regression and v869 final-FILE guard remain independent tests.
Lowmem KCORE link PASS: exclusive end `021226` octal, four words above
v875 `021222`. No automatic OOM victim kill is enabled.

The full-system session-leader exit/HUP test PASSes both after v875
and in a separate post-v876 run with unrelated SDL backends disabled.
Nonreturning APR/EXEC fatal teardown and bounded provider cleanup
remain the principal OOM blockers.

## v875: Delay controlling-TTY detach until session HUPs finish (2026-10-31)

Before v875, `proc_session_teardown` removed a session leader's TTY
association and kept only the output route *before* attempting HUP on
its other session members. A member could fail FILE close and retain
its u-area (as required by v867-v869), causing HUP to fail while the
leader and other live members had already lost their TTY attachment.
The leader's final cleanup then failed, leaving inconsistent session
ownership and an unsafe later retry.

The PDP-6 session-teardown path now retains the original TTY record
until each member HUP returns success. The TTY index lives in one saved
callee-saved AC across the bounded scan. After success it publishes the
same old output-route-only TTY record; on failure it leaves the original
association unchanged. No new static words, heap or backing are used.
This is *commit ordering*, not all-or-nothing HUP: earlier members may
have been terminated before a later HUP fails, and retry scans only
those with still-live u-areas.

Native PDP-6 v875 runs the real `proc_session_teardown`, injecting a
failed second HUP and clobbering caller-scratch ACs in the event stub.
It verifies unchanged TTY and retained state on failure, then successful
retry and route-preserving detach; nonleader/detached no-op also PASS.
The same fixture FAILs on pre-v875 code at the early-detach invariant.
Lowmem KCORE link PASS: exclusive end `021222` octal, four words above
v874 `021216`. Existing full-system session-leader exit/HUP acceptance
is tracked separately.

This reduces cleanup partial-commit damage but cannot safely resume
an APR-faulting or post-commit EXEC-failed process after an unclosable
descriptor. Kernel exit still HALTs on unrecoverable finalization;
provider-call blocking and an allocation-free deferred reaper remain
unresolved. Do not enable OOM victim termination.

## v874: Transactional Type 340 list replacement under MM failure (2026-10-31)

The old DPY replacement path wrote the new BLKO IOWD and published the
new extent before freeing the previous list. An unsuccessful old-list
`mm_free` was ignored, permanently losing the reference to its allocation.
This defeats resource accounting and makes memory-pressure recovery unsafe.

Replacement now allocates and copies a new list into an unpublished extent.
Its new IOWD stays in a saved register. With PI7 quiesced, the old extent
is released *before* publication of the new base/IOWD. An old-free error
preserves the original published base and IOWD, frees the unpublished
replacement, rearms clock refresh, and returns -1. A successful release
publishes the complete new list and advances the accepted-write counter
exactly once. The initial-install, allocation-failure and STOP paths
remain supported without persistent new RAM or backing storage.

If the new extent also cannot be freed while rolling back, there is no
safe owner for it in the one-base display state. The kernel therefore
HALTs as an explicit MM ownership invariant violation rather than
returning with a silently leaked live allocation. This double-failure
case, and live post-failure Type 340 replay, require separate acceptance.

The new native PDP-6 testkit v874 uses the real `dpy_write_words` assembly
with injected allocator/free results. It verifies old-list preservation,
new-list rollback, subsequent successful replacement, IOWD contents,
write-count publication, allocation failure, STOP and fresh installation:
PASS. The same test FAILs against pre-v870 source as the negative control.
v709/v871 DPY gateway and close tests, v870 PIPE close, and the Type 340
interrupt-policy test PASS. Direct DPY MRES assembly/link and the lowmem
KCORE link PASS: resident KCORE exclusive end stays `021216` octal,
because this additional code is confined to optional DPY MRES.

This completes the identified DPY replacement ownership fix but does
not turn on automatic OOM termination. Forced-fatal APR/EXEC cleanup,
provider-lock bounds, full-system named-FIFO startup and actual OOM
resource-progress acceptance remain open.

## v873: Named-FIFO full-system timeout predates v870 (2026-10-31)

The unchanged KCC `daimos-pipe-fifo-v2.sh` workload timed out with exit
124 after D6FS startup on current v871. The same test also timed out
after D6FS startup against an isolated pre-v870 v869 checkout
(`8b4e7d9`); the isolated build used explicit DAS/KCC source roots.
Both runs reached native simulator execution, but neither emitted the
final named-FIFO completion marker. Their final PCs differ and the
underlying stall has **not** been identified. No full-system named-FIFO
PASS is claimed; do not attribute its timeout specifically to v870.

As a separate negative control, the new native v870 failed-MM-close
fixture FAILs on v869 source at its original reference-count check,
and PASSes on v870/v871 source. That verifies the isolated rollback
test detects the intended bug, not that the complete FIFO workload
now passes. No production code changed for this diagnostic.

## v871: Preserve Type 340 display-list ownership on failed CLOSE (2026-10-31)

`file_close_dpy` formerly called the DPY raw-list stop service but then
discarded its result and invoked `vfs_sync`. Device SYNC returns zero, so
even a failed DPY extent release silently closed the descriptor. The DPY
MRES stop routine also cleared its list-base and IOWD before checking
`mm_free`, irreversibly losing the extent address on a release failure.

FILE now returns the actual DPY stop result, preserving the FD on failure.
The absent-MRES close target is `kret_zero` because no raw list can be
owned when DPY is absent; the absent-MRES write target remains `kret_neg1`.
The stop routine disables PI7 before freeing and keeps the original
immutable list IOWD until free succeeds. On failed `mm_free`, it restores
the original base and schedules the clock refresh again; on success, it
clears the IOWD and returns zero. No extra permanent words are allocated.

Native PDP-6 v871 injects failed and successful final frees through the
real FILE close and DPY stop paths, checking descriptor retention, list
base/IOWD preservation and retry: PASS. The v709 static and native
patched-gateway tests were updated for the intentional absent-MRES
contract change and PASS. Direct assembly of the full DPY MRES and the
lowmem KCORE link PASS; KCORE exclusive end `021216` octal, one word
smaller than v870 `021217`. An actual Type 340 replay after fault
recovery has not yet been exercised on native full-system hardware.

The DPY raw-list *replacement* path still ignores failure freeing the
old list after publishing its replacement; it requires separate
transactional correction. APR/EXEC forced exits, remaining provider and
descriptor cleanup failures, and system-wide OOM progress remain open.
No automatic OOM kill is enabled.

## v870: Final PIPE close is retry-safe after MM release failure (2026-10-31)

The existing `pipe_close_ref` decremented reader/writer counts and
signaled waiters *before* attempting to free the pipe object when the
last reference disappeared. If `mm_free` rejected the last free, the
FILE descriptor remained live (correct), but the pipe count was already
zero (incorrect). A second close could underflow or be rejected, leaving
the process impossible to terminate cleanly under memory pressure.

The final-reference path now invokes `mm_free` before publishing the
zero reference count or signaling events. For named FIFOs it first
unlinks the object, and if the free fails it restores the existing
object to the FIFO list head without allocating memory. The original
reader/writer counts and FD remain untouched on failure. Other list
members are preserved; successful last-close frees without reaccessing
the released memory. Non-final close retains the original count update
and wakeup semantics. No persistent storage or removable service added.

The native PDP-6 v870 test executes the real FILE entry close and PIPE
close/unlink routines. Injected final MM-free failure and retry PASS for
both anonymous and named pipes, including FIFO list restoration, plus
the normal non-final reference decrement. Native v417 mount-reference
regression PASS. The full-system `daimos-pipe-wordfifo-v2` regression
also PASSes (opaque words, EOF/EPIPE, duplication, RUN references and
blocking wakeups); named-FIFO acceptance is tracked separately.
The fresh lowmem KCORE link PASSes at exclusive end `021217` octal,
six words above v869 `021211`. This is another OOM-teardown
prerequisite, **not** activation of OOM killing or proof that all other
resource releases are infallible.

## v868-v869: Fail closed on current EXIT and before final process teardown (2026-10-31)

v868 extends the v867 aggregate FILE-close result to the ordinary PDP-6
EXIT syscall and the current-process fatal-event path. When VFS sync fails,
the affected descriptor remains live; EXIT or fatal-event delivery now
returns -1 instead of destroying its owning u-area. The C reference
fatal-event path also checks the bulk-close result. Native PDP-6 v868
injects a failed first close with a successful second close, proves both
current fatal-event and explicit EXIT avoid finalization, then checks
successful retry and unchanged exit status: PASS.

v869 adds a bounded, read-only final FILE guard to `proc_finish_slot`.
Before clearing RT ownership or changing session/VM state, it requires a
stable u-area and scans all 16 two-word FILE entries. Any live node
prevents finalization. The C reference implementation has the same guard.
Native PDP-6 v869 verifies that even the last descriptor blocks teardown
without helper calls, a missing u-area fails closed, and empty entries
permit normal zombie publication: PASS. Neither version introduces a
persistent table or dynamic allocation. A lowmem KCORE link PASSes at
exclusive end `021211` octal, compared with `021167` after v867, a net
increase of 18 words. The strict `020000` octal size goal is still open.

**Not yet safe for automatic OOM termination:** an APR user fault and a
post-commit EXEC fatal exit cannot return to the faulting process if the
close fails; the new guard prevents silent loss of descriptors but the
current fatal teardown can still reach the invariant HALT. Provider lock
waits, recursive session HUP, device-specific cleanup failures, swapped
victim release and a guaranteed resource-progress policy remain unproved.
Full-system fault-injection and actual OOM kills remain disabled.

## v867: Report failed victim closes before releasing the u-area (2026-10-31)

The previously allocation-free `file_close_all_at_v865` ignored individual
`file_close_entry_v865` failures. The latter deliberately retains an FD
when `vfs_sync` fails. Thus `proc_event_apply` could proceed to
`proc_finish_slot` and destroy the victim's u-area while live descriptors
still owned provider or pipe resources. This is unsafe for ordinary fatal
events and would be more dangerous in an OOM kill path.

Bulk close now retains a failure bit in one saved register, attempts all
16 fixed descriptors, and returns -1 if any close failed. The fatal-other
event path refuses to finalize a process with unclosed FDs, leaving its
remaining descriptors and u-area intact. Native PDP-6 testkit v867
injects a failed sync followed by a successful second FD close, verifies
that finalization does not happen, then retries after the sync recovers:
PASS. The existing v865 explicit-target close regression also PASSES.
The PDP-6 lowmem KCORE link PASSES. No persistent words, heap or backing
storage were added; the close frame temporarily saves one extra AC.

This is deliberately not a general atomic rollback of a partially closed
file table and not an OOM kill implementation. A blocked provider, a
persistently failing close, current-process exit, recursive HUP teardown,
or a device-specific cleanup failure still needs a bounded policy before
OOM termination can be activated. The failed target remains alive so its
descriptor storage cannot be accidentally reclaimed.

## v866: Encode physical-provider lease owner without new RAM (2026-10-31)

The KCORE serialized physical-provider lease formerly stored only
FREE (nonzero) or BUSY (zero). A victim could be selected for OOM while
it held this lease; fatal-other descriptor cleanup might then block
on the same lease, preventing the victim from ever finishing cleanup.

The existing `fs_provider_ready` word now uses RH nonzero for FREE
(unchanged at public wait/wakeup boundaries), and while BUSY records
`owner_slot+1,,0` in LH/RH. The KCORE provider gateway checks RH and
publishes the owner when entering the serialized call; unlock still
sets the word to -1 and wakes waiters. Bootstrap slot 0 is encoded as
LH 1. The OOM selector excludes this owner; if BUSY lacks a known owner
(all zero), selection fails closed rather than risking a deadlock.

Native PDP-6 provider tests v326/v327 and new v866 owner-encoding
test PASS; native OOM selector v863 and v866 provider-owner/unknown
owner tests PASS. Updated v325 static gateway check PASS. Fresh
lowmem KCORE link PASS: end 021313 octal versus v865 021301, a net
ten code words, no added permanent data or table words. The strict
020000 gate remains independently over budget.

This protects one known shared lock. It is not a proof for all victim
resources or a license to enable OOM termination: other lock classes,
close failures, recursive session/HUP cleanup and cleanup rollback
still require explicit analysis and fault injection.

## v865: Close another process's descriptors without global FILE switch (2026-10-31)

The previous `proc_event_apply` fatal-other path temporarily swapped
the global `file_table` pointer to the victim's table, called
`file_close_all`, and restored the pointer afterward. A filesystem
provider call may sleep, allowing other processes to execute with a
wrong global FILE table. This is a correctness defect independent of OOM.

`file_close_entry_v865` now performs normal close on an explicit
descriptor-entry address; both ordinary `file_close(fd)` and the new
`file_close_all_at_v865(table)` use the same close semantics.
`file_close_all` remains the current-process wrapper. Fatal event
delivery for another process now passes its stable u-area FILE table
directly, with no global pointer substitution.

Native PDP-6 testkit `daimos-file-close-target-v865.sh` executes the
real close routines and fatal-other dispatch, with instrumented VFS
sync and process finalization, and verifies that the caller's table
pointer is never changed: PASS. Existing v417 six-bit direct-mount
FILE regression PASS. Historical v19 fails with the same exit code
on both pre-change source and changed source; it uses obsolete FILE
mount-extension expectations and is not a v865 regression.
Fresh lowmem KCORE link PASS, exclusive end 021301 octal versus
v863 021306, saving five resident words. The independent 020000
KCORE size gate remains over its threshold.

OOM termination remains disabled. Explicit FILE pointer isolation
does not prove that a victim holding a provider lock can safely be
killed or that file-close errors and other finalization steps always
release resources. Continue with the lock/teardown audit before
enabling a kernel OOM kill transaction.

## v863: Allocation-free KCORE OOM victim selection (2026-10-31)

The fixed KCORE `proc_oom_select_v863(excluded_slot, resource_kind)`
scans existing process descriptors, swap records and pin state without
allocating memory, writing data, performing I/O or mutating processes.
Kind 0 chooses the largest eligible resident VM extent; kind 1 chooses
the largest eligible nonresident swap extent. Invalid kinds return -1.
Equal sizes choose the lowest slot. Slot 0, INIT (slot 1), current and
explicitly excluded processes, the RT owner, transition-locked and
user-mapped processes, and pinned resident extents are ineligible.
No BSS or kernel table was added. Native PDP-6 fixture
`daimos-oom-select-v863.sh` verifies both kinds and exclusions (PASS).

**Do not call the selector as a kill routine yet.** The existing
`proc_event_apply` fatal-other path closes the victim's FDs through
`file_close_all` before calling `proc_finish_slot`. `file_close` may
invoke `vfs_sync`; the KCORE `fs_provider_reg_call` serializer may wait
on `fs_provider_ready` via `proc_wait_event`, using a process-private
stack. This creates a potential OOM-time wait for a resource held by
the intended victim, even though D6FS and DTFS SYNC handlers currently
use allocation-free return paths. Termination needs a strictly bounded
no-new-RAM, no-new-backing, no-victim-owned-lock cleanup contract for
all providers and descriptors. Also test freeing swapped records and
correct zombie/WAIT reporting without paging the victim into RAM.
This is a safely testable selector, not an enabled OOM killer or a fix
for the v859 admission failure.

## v860: RUN admission needs a resource-progress protocol (2026-10-31)

Source inspection confirms the constrained v859 failure is structural.
`exec_load_process` calls `vm_space_create`, which must acquire an aligned
contiguous resident extent using `mm_alloc_aligned`. The allocator first
tries ordinary allocation, compaction and filesystem reclaim, then calls
`proc_swap_reclaim`. That path chooses a resident victim and attempts
`proc_swap_out`; it must reserve the victim's writable backstore span
*before* freeing its RAM. Once no span can be allocated, neither a
retry nor more compaction can produce the missing resident extent.

Existing equal-sized exchange replaces one resident with one already
swapped process, but does not create a free resident extent. Exchange
can enable progress only if the incoming process subsequently exits,
shrinks or releases resources. It runs from slot-0 idle scheduler
context, whereas `SYS_RUN` is currently synchronous in the caller.
Calling exchange directly from a failed `vm_space_create` would violate
the slot-0 serialization, live-process exclusion and scheduling rules.

A general fix therefore requires a bounded admission/scheduling
protocol, not more allocation retries or a special-case process count:
the pending RUN must retain its validated launch and resource state,
allow scheduler-visible work to free resources, and retry admission
only after a provable resource change; the child must not become RUN
until its VM and backing resources are committed. Define cancellation,
waiter wakeups, fairness and no-progress reporting, with bounded
resident metadata. Avoid deadlocking when all runnable processes are
themselves blocked on admission, and preserve process/accounting/FD
ownership on every cancellation path. No production patch has yet
implemented these semantics; v859 remains a release-blocking FAIL.

## v859: Default drum backstore RUN admission failure reproduced (2026-10-31)

Testkit `daimos-swap-default-pair-v859.sh` builds a fresh full-profile
PDP-6 boot with DRUM_BACKSTORE_BLOCKS=04000 and corrected v858 runtime
SWAP MRES breakpoint mapping. The native 16-child post-INIT pressure
regression FAILS: guest marker `pF!W` reports failure spawning the
last pressure child, before the writer case can finish. At
`proc_run_claimed_bad` AC1 is 777777777776 octal (-2), the
`EXEC_LOAD_NOMEM` status returned by executable loading. There are
17 RUN attempts, one `proc_run_claimed_bad` and `proc_run_bad`, no
`proc_run_mm_bad`, and nine ordinary swap-in restorations. No v832/v790
transaction was entered. The simulator reached HALT but the test's
guest success predicate correctly rejected it (runner exit 1).

This is an EXEC/RUN admission-capacity failure under the small
backstore, not proof of an exchange implementation defect. The v851
and v858 successes require the larger 010000-octal-sector fixture.
Do not declare swap-pressure recovery complete. Investigate whether
RUN can make resource progress under this backing capacity without
relaxing allocation invariants, or document a strict admission limit.

## v858: Runtime SWAP MRES breakpoint correction (2026-10-31)

The installed KCORE swap-service trampoline in the v857/v858 image was
at octal 020511, containing `JRST 0,32304`. Subtracting the SWAP MRES
service link-map offset 002423 establishes runtime base 027661, not
the stale 027632 assumed by earlier diagnostics. The old alleged v826
breakpoint therefore addressed unrelated code; its 255 hits were not
exchange pair calls. Testkit v858 recalculates breakpoints with the
correct base and verifies the resident trampoline instruction.

After fixing a diagnostic generator mistake that had accidentally
renamed the still-exported `proc_swap_pair_admitted_v857` symbol, the
native SIMH image was run with corrected pair and transaction probes.
The 16-child workload reached its `Q` and HALT success markers (SIMH
exit 0). Twelve swap-in service restorations and twelve resident
swap-service trampoline calls were observed. There were zero hits at
the corrected v826 pair entry, admission branch, or v832/v790 entry.
Thus ordinary swap restoration, not physical exchange, satisfies this
particular scheduler-pressure test. The successful full-system exchange
and subsequent process-resumption acceptance remain open. The v858
trace fix is testkit-only and adds no kernel RAM or executable code.

## v857: Pair-admission probe exposes invalid fixed MRES address (2026-10-31)

Added exported labels `proc_swap_pair_admitted_v857` and
`proc_swap_pair_done_v826` (zero added kernel words) and a full-system
testkit `daimos-swap-pair-admission-v857.sh`. The fresh full-profile
PDP-6 run with 010000-octal backstore sectors PASSED its 16-child
post-INIT pressure cycle (descriptor peak 34/136).

The trace script printed 255 hits at the presumed pair entry 027632,
zero at admission/return, and zero at v832/v790. **Those hits are not
valid pair-admission evidence**: the SIMH trace disassembled 027632 as
`CONO PAG,0`, although the pair source begins with `JUMPE 1,...`.
The static MRES base assumption in the v837/v851-derived breakpoint
script is therefore not verified for this image. Do not infer either
255 real pair calls or zero eligible pairs from these breakpoint counts.
Next resolve the installed SWAP MRES code address from a runtime-verified
entry and disassemble the expected instructions before placing branch
breakpoints. Preserve the successful guest workload acceptance as a
separate, valid result. End-to-end physical exchange remains unproven.

## v856: Mid-transfer write fault is fatal (2026-10-31)

The native PDP-6 testkit `daimos-swap-midwrite-fault-v856.sh`
injects an I/O failure on the second disk-sector write, after the
first destructive write succeeds. It executes the real exchange
transfer code with simulated sector I/O. The test verifies that two
writes were attempted and that the kernel reached
`proc_swap_storage_invariant_v30`; native test PASS.

The transaction must not resume or attempt ordinary rollback after
an irreversible partial write. This test confirms the fatal invariant
path, not recovery or full-system process resumption. No production
kernel logic was changed.

## v855: Incoming writable image before PURE reload (2026-10-31)

Testkit `daimos-swap-incoming-v855.sh` links the real transaction and
intercepts the entry to `proc_swap_exchange_pure_v785` before executable
reload. It verifies both incoming B writable sectors at the declared
RAM destination, then executes the real PURE reload and full v854
metadata/data-integrity assertions. Native PDP-6 test PASS.

This establishes the incoming writable contents at the destructive
transfer/reload boundary, in addition to v854's three-sector outgoing
disk verification. It does not establish recovery after a failed disk
write; a fault after irreversible writes must still reach the kernel
invariant halt, not resume with inconsistent process images.

## v854: Full outgoing writable-sector image verification (2026-10-31)

The new native testkit `daimos-swap-exchange-data-v854.sh` extends the
real v819 exchange integration with verification of all three outgoing
0200-word disk sectors (0600 words), rather than only the first two.
It exercises the real transfer, preflight, commit, accounting and
metadata publication, with simulated disk-sector I/O in native PDP-6
memory. The same fixture tests rejection before disk writes for invalid
backing, allocation bitmap, span and alignment. The test PASSES.

An initial assertion against the incoming process writable RAM after
commit was invalid, because executable reload replaces that region;
the assertion was replaced with checks of the persistent outgoing
backing sectors. A post-transfer pre-reload check of incoming writable
RAM and a fault-injected mid-transfer recovery test are still required.
This is native simulated-device data-path evidence, not full-system
hardware or simultaneous process-resumption acceptance.

## v853: Real v832-to-v790 control-flow native test (2026-10-31)

The new testkit `daimos-swap-orchestrator-v853.sh` links the actual
v832 orchestrator and the actual v790 transaction control path on a
native PDP-6. Its test fixture supplies preflight data, an injected
live-state validation success, and controlled stage/commit leaves.
It verifies failure with lock/pin abort followed by a successful
stage-to-commit call, including the prepared graph pointer and held
transaction locks. This native test PASSES. The existing v826, v819,
and repaired v790 tests also PASS.

This validates the *control-flow integration* but does not claim
sector-transfer correctness: the destructive stage and metadata commit
are test stubs. A real physical swap exchange, rollback of partial
disk writes, and resumption of both actual processes remain pending.
No production kernel change was made for v853.

## v852: Native exchange orchestration coverage (2026-10-31)

Existing native v826 pair-admission/lock/abort, v819 real v790 metadata
commit integration, and isolated v790 stage/commit tests all pass.
The isolated v790 harness had become stale: its source extraction
accidentally included live-state validation dependencies. The testkit
now extracts only the v790 orchestration leaf and substitutes explicit
preflight/live stubs; the *real* validation/commit integration remains
tested independently in v819.

The new `daimos-swap-orchestrator-v852.sh` runs a native PDP-6 fixture
covering both v832 abort and success-return branches. Its success
case substitutes a successful v790 result and verifies the passed graph
pointer and held transaction locks; it then invokes v830 abort to
release test-only locks. This does NOT represent a real committed disk
exchange. All three native suites passed after the test correction.

The untested acceptance requirement remains: construct a genuinely
eligible equal-sized resident/disk pair, run the real v832/v790 path,
verify data and metadata, exercise rollback under injected failure,
and resume both processes. No production code was modified in v852.

## v851 full-system native acceptance result (2026-10-31)

Executed testkit `daimos-swap-dispatch-acceptance-v851.sh` against a
fresh full-profile PDP-6 boot with DRUM_BACKSTORE_BLOCKS=010000.
The native v669 regression PASSed one post-INIT 16-child pressure cycle.
The descriptor peak was 34/136 from 138 insertion samples. All 17 RUN
entries succeeded with no RUN admission or claimed-child failures.

The instrumented execution recorded 12 service-success branch hits,
12 pressure/cooldown visits, and 255 v826 pair admission calls, but
zero entries into v832 exchange execution or v790 transaction.
The recorded `proc_swap_in_restore` hit count was zero despite the
service-success hits, so do not use it as a restoration-count measure
without checking label placement. The runner's fixed-address trampoline
confirmation also failed after layout changes; its PASS comes from
the actual guest workload completion check, not that auxiliary check.

This closes the *writer WAIT/EXIT liveness regression* under the enlarged
test backing configuration. The default 04000-sector configuration
previously failed RUN admission, and the unrelated full exchange
transaction still requires its own acceptance test. No permanent RAM
was added by the v851 dispatch hint.

## v849: Parent dispatch tracing after successful writer exit (2026-10-31)

The v848 native run observed the writer exiting with status 050 and
executing `proc_notify_parent_wake`, but the full WAIT/EXIT acceptance
still timed out. v849 exports the existing KCORE labels
`proc_sched_resched_choose` and `proc_sched_restore_selected` without
adding instructions or resident data. The testkit's clean-image
`daimos-swap-parent-dispatch-v849.sh` retains enlarged drum backing and
records these dispatch entries alongside the parent WAIT and wake paths.
The precise scheduler state after wakeup remains unverified until the
v849 native execution completes. No behavioral fix has been made.

## v851: One-shot dispatch after successful swap-in (2026-10-31)

The v849 trace proved all 16 pressure children were woken by gate
EOF. The v850 trace showed the selector continuing to request slot-0
swap service, without completing any pressure-child exit. On success,
SWAP MRES had written the newly restored slot to `proc_sched_cursor`,
but the selector treated that as a *previous* round-robin cursor and
could immediately choose the next swapped child instead of running the
restored one. This creates a repeatable overcommitment liveness hazard.

v851 carries a one-shot `01000` bit in the existing scheduler cursor
after successful swap-in or exchange (no additional persistent RAM).
The resident selector honors this hint after real-time-owner priority,
but only after checking the slot is in range, RUN, not transitioning,
and has a resident VM base. It clears the bit upon selecting that
slot, so later elections use the normal nice/CPU/cyclic policy. A
rejected hint falls through to regular election. This change targets
the dispatch boundary, not WAIT or pipe EOF, both already exercised
correctly in the native traces.

Full SWAP MRES and scheduler assembly pass. The isolated native v826
pair/lock/abort test passes. Full-system v851 acceptance is being
evaluated with the proven `010000`-sector test backstore rather than
the too-small default; the completed PASS is recorded above.

## v849: Gate EOF wakes all pressure children; dispatch still stalls (2026-10-31)

Using the `010000`-sector backstore fixture, the completed v847 trace
reached pipe reads by parent slot 1, two WAIT invocations, and a
successful writer exit/parent notification. v848 confirmed the parent
WAIT selectors were writer slot 2 followed by pressure child slot 3;
both regressions timed out. No RUN admission failures occurred.

The v849 native trace specifically observed the gate pipe's last
writer-reference release and EOF read-event signal (event address
044207 in this image). Exactly **16** `proc_wakeup_after_runq` hits
matched that gate read event, one for every pressure child. Thus
the gate's reference count, EOF notification and waiter wakeup are
working in the failing workload. `proc_wait_status` and
`proc_wait_child` were each entered twice, but only writer slot 2
reached `proc_exit_current`; no pressure child exited before timeout.
After the gate wake, the scheduler continued issuing swap-in requests
for pressure slots (beginning at 3) without executing a pressure-child
exit. The simulator timed out after 300 seconds (exit 124, PC 013604),
with eight successful swap-in restores and zero exchange-pressure hits.

The next target is **scheduler progress after successful swap-in**:
determine whether restored runnable children actually receive user
dispatch before the scheduler chooses further swapped candidates.
Do not alter pipe EOF or WAIT notification based on this trace.
Testkit `daimos-swap-gate-eof-v849.sh` contains the reproducible trace.

v850 exports the existing KCORE scheduler labels `proc_select_done`,
`proc_select_return_best`, and `proc_select_none`, with no additional
machine instructions or resident words. The testkit
`daimos-swap-dispatch-v850.sh` records the selected slot and scheduler
cursor at these branches using the same enlarged-backstore fixture.
The native v850 acceptance result is pending.

## v848: Writer exits and parent wakeup executes; dispatch unresolved (2026-10-31)

With the v847 larger backstore retained, the v848 fresh native full
system again timed out after 300 seconds (exit 124, PC 013604).
The testkit `daimos-swap-wait-dispatch-v848.sh` recorded two hits each
at `proc_wait_status` and `proc_wait_child`, plus one each at
`proc_exit_current`, `proc_finish_slot`, `proc_notify_parent`, and
`proc_notify_parent_wake`. The writer slot 2 called exit with AC1
octal 050 (the expected `WRITER_STATUS`), and parent slot 1 was the
target of notification and the wake branch. Eight swap-ins completed
normally; no RUN rejection or exchange-pressure branch was hit.

This disproves the hypothesis that a missing writer exit or missing
parent wakeup caused the remaining timeout. The next diagnostic needs
to inspect the parent descriptor and run-queue state *after* wakeup,
and determine whether PI6 dispatches the parent and which WAIT status
or pipe-drain step prevents final completion. No scheduler behavior
was changed and v758 WAIT/EXIT acceptance is still FAILED.

## v847: Increased backstore reaches WAIT; still no completion (2026-10-31)

The full clean v847 profile retained `DRUM_BACKSTORE_BLOCKS=010000`
and timed out at PC 013605 after the 300-second SIMH limit (exit 124).
All 17 RUN entries succeeded without rejection; seven swap-in
restorations succeeded, with no exchange-pressure branch hits.
Unlike the default-backstore fixture, the parent (slot 1) entered
`pipe_read_words` ten times and `proc_wait_child` twice; the kernel
entered `proc_notify_parent` once. Fifteen other pipe reads came from
pressure slots 3..022 (octal), and nine writes were made by writer
slot 2. Thus the v846 capacity change *did* permit post-RUN test
progress, including WAIT and child-notification activity, but did not
complete the writer assertions. The earlier prediction that WAIT was
not reached was only true for the default-capacity failed RUN case.
Next trace the precise writer exit report, WAIT result, and event
wakeup/dispatch for the restored parent. Do not claim acceptance.

## v845: RUN admission is the current blocker (2026-10-31)

The full v844 pipe/wakeup trace timed out at PC 013605 with eight
normal swap-in restorations. `pipe_write_words` was entered nine times,
all by writer slot 2; `pipe_read_words` was entered 15 times by pressure
slots 3 through 021 (octal). No parent slot-1 pipe read or WAIT was
observed. The test transcript contains `pF!W`. From
`daimos-mm-swap-postinit-v283.c`, `pF` uniquely identifies the RUN
failure while creating the last (sixteenth, index 15) pressure child,
not a failed parent wakeup. The test immediately returns failure from
`run_writer_case` and therefore never executes its later pipe-read or
WAIT assertions. The previous apparent WAIT deadlock diagnosis was
premature; this fixture currently fails its process-creation setup.

The PDP-6 full profile has a 64-slot process table, so the intended
18 live processes fit the configured descriptor limit; RUN resource
admission or a more specific construction failure remains to be
determined. v845 exports the existing RUN rejection and cleanup labels
from `proc_run.s` without adding code or persistent RAM. The testkit
`daimos-swap-run-admission-v845.sh` traces those resident points in a
fresh native build. Writer WAIT/EXIT acceptance remains FAILED.

The completed v845 trace observed 17 RUN entries, one hit at
`proc_run_claimed_bad`, and AC1 `-2` before child-slot rollback.
`exec_load_process` defines this exact result as `EXEC_LOAD_NOMEM` and
reaches it when `vm_space_create` cannot allocate the aligned user VM.
The immediate observed failure is insufficient allocatable process VM,
not slot-table exhaustion, u-area allocation, or WAIT notification.

The auxiliary drum's full-profile default is `DRUM_BACKSTORE_BLOCKS`
`04000` (octal, 2048 sectors). v846 is testing a targeted fixture
configuration of `010000` (4096 sectors) to determine whether
insufficient backing capacity prevents swap reclaim from satisfying
the final RUN. The experiment is not yet acceptance evidence; no
production backstore default or kernel logic is changed.

The completed v846 capacity experiment built a fresh full-profile
image with `DRUM_BACKSTORE_BLOCKS=010000`. It reached all 17 RUN
entries without any `proc_run_claimed_bad` or `proc_run_bad` hits,
and no `pF` marker was emitted. This confirms the enlarged backing
configuration removes the immediate final-child RUN failure. However,
the writer WAIT/EXIT workload still timed out after 300 seconds
(exit 124, PC 013604). Eight swap-ins completed normally and no
exchange-pressure entry was observed. The next investigation should
trace pipe progress, parent scheduling, and sleep-state transitions
using the v846-sized backing store; returning to default backstore
would reintroduce the known RUN setup blocker.

## v844: Trace the writer's pipe wakeup before WAIT (2026-10-31)

The v843b native regression timed out at PC 013605. Eight normal
swap-in restores were observed, together with 2297 run-queue insertions;
`proc_notify_parent` and `proc_wait_child` were not reached. The actual
test program `daimos-mm-swap-postinit-v283.c` first blocks its parent
in `dsys_read_words(pipe_r, ...)` and calls `dsys_wait` only after
reading the first 'A' written by the child. Consequently, absent WAIT
hits do not identify a defective child-notification path. The v843b
trace does not establish whether the writer ran or the pipe reader woke.

The v844 testkit trace targets KCORE `pipe_read_words`,
`pipe_write_words` and `proc_wakeup_event`, while retaining swap-in
completion instrumentation. This is a diagnostic-only change; the
writer WAIT/EXIT acceptance remains FAILED pending a successful run.

## v843: Trace scheduler handoff and child WAIT notification (2026-10-31)

The v842 full-system trace proved nine normal swap-in completions, but
the writer WAIT/EXIT workload still timed out. v843 exports the existing
`proc_idle_loop` KCORE label (one symbol only, zero additional words)
and adds testkit `daimos-swap-handoff-v843.sh`. This clean-image native
diagnostic observes resident `proc_idle_loop`, `proc_notify_parent`,
`proc_wait_child`, and `proc_runq_add`, together with the v842 swap-in
completion path. It records process slot and scheduler cursor at each
resident breakpoint and retains the exact current-run trace. The
initial testkit script accidentally renamed the pre-existing v842
symbol when versioning; this was corrected before the fresh rerun.
The original v758 WAIT/EXIT acceptance remains FAILED until the full
system completes the writer workload.

The first v843 attempt stopped before SIMH because the versioned
diagnostic mistakenly renamed the existing v842 symbol; the second
attempt used a breakpoint in the busy idle loop and was stopped after
excessive trace output. Both faults were corrected in the testkit,
including removing the idle-loop breakpoint and binding the report
to the current run's trace rather than a stale retained file.

The clean v843 rerun reached SIMH and timed out at 300 seconds (exit
124, PC 013605), again without writer completion. It recorded eight
normal `proc_swap_in_restore` / service-success hits; zero swap-in
pressure and zero exchange entry hits. KCORE breakpoint counts were
`proc_runq_add` 2297, `proc_wait_child` 0, `proc_notify_parent` 0.
These observations do not prove a parent wakeup failure: neither
WAIT-child entry nor parent notification was observed. Investigate
the test workload's user-level WAIT/EXIT phase, its syscall entry
and the child exit/finish path before changing notification semantics.
The v758 acceptance remains FAILED.

## v842: Swap restore/return branch observability (2026-10-31)

Added exported branch symbols for the normal swap-service restore
completion and the `proc_swap_in` return, pressure, and failure labels.
These are labels and symbol exports only: no new instructions or
persistent kernel RAM. The full SWAP MRES assembles and the isolated
native v826 pair/lock/abort regression passes. Testkit
`daimos-swap-return-trace-v842.sh` instruments these paths alongside
the v840 pressure and transaction entries in a clean full-system build.

The v841 trace saw ten swap-service calls and no hits on any pressure,
early-rejection, cooldown, or exchange entry point. The purpose of
v842 is to distinguish the normal swap-in return from the other
possible outcomes. The v758 writer WAIT/EXIT regression must not be
marked fixed without a completed full-system acceptance test.

The subsequent fresh full-system v842 run timed out after 300 seconds
(exit 124; PC 013605), preserving the failed writer acceptance. Its
runtime trace recorded nine hits at `proc_swap_in_restore` and nine at
`proc_swap_service_restore_done_v842`, with zero hits on swap-in
pressure, swap-in failure, service early rejection, pressure cooldown,
pair admission, or exchange execution. Thus the observed scheduler
requests can restore swapped processes normally, but writer WAIT/EXIT
still does not finish. Next investigate scheduler handoff, completion
notification, and writer wakeup rather than assuming swap-in allocation
pressure is the immediate blocker.

## v841: Clean native service-branch tracing (2026-10-31)

Executed testkit `daimos-swap-branches-v840.sh` against a fresh full
PDP-6 image. The full image built and booted, and the SWAP MRES
trampoline still targeted octal 032255. The native test again timed
out after 300 seconds (exit 124), with PC 013605 and no writer
WAIT/EXIT completion. Ten resident `proc_swap_service_one` breakpoint
hits were seen, but ZERO hits at each of the eight instrumented SWAP
MRES branch/entry points: pressure, pop-clear, failed, cooldown,
candidate selection, pair admission, exchange execute, and exchange
run. The trace is preserved under the v669 runner's versioned TMPDIR
directory; the last run's log is `$TMPDIR/daimos-v841.log`.

This evidence shows that the exchange-pressure branch is NOT observed
in the failing workload, despite repeated swap-service entry. It does
not by itself prove that every service invocation successfully swapped
in a process: a direct early return or another path is still possible.
The next instrumentation should explicitly capture the `proc_swap_in`
return value and the normal service-success path, then correlate with
the writer's wait state. The v758 acceptance remains FAILED; no
production control-flow change was made for v841.

## v840: Export native swap-service branch trace points (2026-10-31)

The clean v839 regression observed 10 resident swap-service calls but
no calls to the v826 pair-admission helper. To separate an early
request rejection from failed `proc_swap_in` and resource pressure,
the SWAP MRES now exports local branch symbols for `service_pressure`,
`service_pop_clear`, `service_failed`, `service_cooldown_v833`, and
`service_candidate_v834`. These are symbol visibility changes only:
no executable code, disk format or permanent RAM was added. The complete
SWAP MRES assembler passes.

Testkit `daimos-swap-branches-v840.sh` creates a fresh full-system
image with explicit breakpoints for these branch points and the v826,
v832, and v790 entries. Shell syntax validation passes. The full-system
v840 diagnostic has not yet completed; the last writer WAIT/EXIT
acceptance result remains the v839 timeout (FAIL).

## v839: Clean image with live MRES exchange breakpoints (2026-10-31)

The testkit `daimos-swap-clean-trace-v839.sh` generates a private v669
runner with SWAP MRES exchange-entry breakpoints installed during the
first, clean full-profile boot. This avoids the reused-image `?RT`
failure seen in v838 and preserves the build, link maps and SIMH trace.
For this build the installed service trampoline still points to octal
032255, yielding module base 027632 and exported v826/v832/v790 entry
addresses 027632/030315/031171 respectively. The script's source and
generated map match those offsets.

The native v839 run again timed out after 300 seconds (exit 124) with
the writer incomplete and simulator PC 013604. Resident scheduler
service breakpoint hits were present, but no break at the exchange
pair, execute or transaction entry was observed. Thus the observed
failure occurs before these entry points, or follows another service
branch: the next investigation should capture `proc_swap_in` result
and pressure-branch admission within the loaded MRES. Do not assume
victim scanning itself was exercised. v758 acceptance remains FAILED.

## v838: Runtime SWAP MRES address resolution and boot limitation (2026-10-31)

The preserved v837 trace showed `proc_swap_service_one` executing
`JRST 0,032255` (octal). Subtracting the v837 SWAP MRES service
link offset `002423` gives the installed SWAP module base `027632`.
Consequently, the candidate native breakpoints for that exact image
are v826 `027632`, v832 `030315` and v790 `031171` (all octal).

Testkit `daimos-swap-live-mres-v838.sh` derives these from the preserved
trampoline and matching link map, rather than assuming an overlay
address is fixed across builds. Dry-run address resolution PASSed.
An attempted actual diagnostic run against the preserved v837 boot
image HALTED at `?RT` before executing the writer workload. This is an
inconclusive runtime exchange trace, not a passed regression. The
runner now fails explicitly on `?RT` or missing workload markers.
For reliable exchange-path evidence, rebuild or restore a clean native
boot image before running with these breakpoints. v758 remains FAILED.

## v837: Preserved native breakpoint evidence (2026-10-31)

Executed the testkit v836 full-profile diagnostic with retained build
and simulator files. The native guest still timed out before the writer
WAIT/EXIT completion, so acceptance remains FAILED. Unlike v835, this
run retained its SWAP MRES link map and SIMH breakpoint transcript.

The current run's resident KCORE map identifies `proc_swap_service_one`
at 020462, `proc_swap_reclaim` at 020461 and `proc_swap_victim` at
014277 (octal). The trace observed **10 swap-service, 18 reclaim, and
18 victim-selection hits**. The first service hit had scheduler cursor
000000000402 (octal), identifying a pending swapped slot 2. Thus the
service entry IS reached; the earlier hypothesis that it might never
execute is disproven. The MRES map lists v826 at relative 000000,
v832 at 000463 and v790 at 001337; these cannot be used as permanent
physical breakpoints without accounting for the runtime MRES load.

The diagnostic's original report accidentally selected an older
retained map by lexical directory order. The testkit v837 correction
selects by map modification time and reports resident breakpoint hit
counts. The raw v837 transcript and live map were inspected directly,
not inferred from the erroneous original report. It is still unknown
whether the v832 wrapper or the v790 transaction reaches commit.
The next targeted trace should follow the live MRES load/entry and
the v826/v832/v790 paths rather than speculate about victim ordering.

## v836: Preserve native scheduler trace and distinguish MRES symbols (2026-10-31)

The v835 full-system runner removed its boot directory, including the
SWAP MRES symbol map and guest trace, on failure. The testkit now has
`daimos-swap-trace-map-v836.sh`, which enables the existing native v669
resident-breakpoint trace, preserves its build directory, writes a
versioned runner transcript and reports both KCORE and SWAP MRES symbol
addresses for follow-up investigation. Link-time MRES addresses are
explicitly NOT asserted to be permanently resident breakpoints, since
the module may load or unload. Shell syntax has been validated; a fresh
full-system execution of this new diagnostic has NOT yet completed.

This is diagnostic tooling only. The v835 300-second writer WAIT/EXIT
failure remains the acceptance result, and production scheduler code
is unchanged.

## v835: Full-system acceptance of v834 candidate scan FAILED (2026-10-31)

Ran the testkit v669 full-profile scheduler pressure regression with the
committed v834 candidate-scan source. The boot image built successfully,
D6FS consistency checks passed, and the PDP-6 guest loaded D6FS before
starting the workload. The SIMH run timed out after 300 seconds (exit
124), with terminal markers `<wpF!W` and no required writer WAIT/EXIT
completion. The simulator reported PC 013604 (SKIPN 0,0(1)) when
terminated. The test host log is `$TMPDIR/daimos-v835-full.log`.

This reproduces the v833 incomplete writer result despite scanning all
exchange-eligible slots. It does not establish that any pair reached
v832 or that v790 committed an exchange. The next investigation must
trace admission and the live transaction results, preferably with a
persisted full-system boot image and explicit breakpoints for SWAP
MRES symbols. Do not infer that candidate ordering alone explains the
remaining block. v758 writer WAIT/EXIT acceptance remains FAILED.

## v834: Bounded search for an exchange-eligible resident victim (2026-10-31)

The v833 pressure fallback selected exactly one `proc_swap_victim`
candidate, although the reclaim scoring policy does not incorporate
exchange eligibility. The v834 scheduler path now scans slots 1 through
`proc_high_slot - 1`, checks each candidate through the existing v826
read-only pair gate, and attempts v832 only for eligible pairs. It stops
on successful exchange, or retains the original cooldown after every
candidate is exhausted. The same 064-octal-word temporary graph is
reused for each attempt; no permanent RAM is added. The swap MRES
assembles, the isolated native pair regression passes, and testkit
v834 checks six bounded-selection cases (PASS). The latter is a host
policy model, not native scheduler acceptance.

The v833 full-system SIMH regression still stands as FAIL (`<wpF!W`,
timeout). A new full-profile v834 writer WAIT/EXIT run is required to
establish whether multiple-candidate admission solves the deadlock.

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

## v862: Static OOM teardown contract (2026-10-31)

OOM selection, victim termination, owned-resource release, and final
reaping must execute entirely from resident KCORE using an existing
fixed kernel stack and bounded register frames. The whole call graph
must be audited for **zero allocations**: no MM allocations, backstore
allocation, per-request buffers, dynamically allocated file or event
records, or loading removable services. In particular, inspect
`proc_event_apply`, `file_close_all`, `proc_finish_slot`,
`proc_session_teardown`, `vm_space_destroy`, `proc_swap_detach`,
`proc_uarea_release`, `proc_adopt_children_pdp6`, and
`proc_notify_parent`. A routine being statically linked alone does not
establish zero-allocation behavior.

Current `proc_finish_slot` uses an existing fixed stack/register frame,
releases VM and u-area before publishing ZOMB, and the regular WAIT
path reaps in place by clearing three existing descriptor words.
These are usable foundations, but the entire helper graph and swapped
victim cleanup must be verified before enabling OOM kills. Never
require a swapped victim to be paged into RAM to terminate it. Prefer
reuse of the normal parent-visible zombie/wait status semantics; no
allocation is permitted for notification. Protect slot 0, INIT,
current/transition-locked/critical processes; maintain deterministic
bounded victim selection. Perform packing first: enough *realizably*
reclaimable aligned RAM is an allocator defect, not an OOM condition.
No OOM implementation or full-path allocation-free proof exists yet.
