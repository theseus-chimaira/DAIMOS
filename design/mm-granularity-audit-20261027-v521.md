# PDP-6 memory granularity P0 source audit (v521, 2026-10-27)

## V561 acceptance: full-system scheduler continuation after compaction (2026-10-28)

**Accepted.** The production 256K full-profile PDP-6 SIMH kernel now has
deterministic testkit coverage for physical relocation and resumed process
execution: `tests/validation/daimos-mm-compaction-scheduler-v561.{c,sh}`.
The workload starts sixteen `020000`-word-data child processes (resident
images of `024000` octal words), blocks them, terminates and reaps the
alternating children, then grows INIT's logical BRK from `024000` to
`0300000` octal words. This allocation exceeds the largest free gap of
the earlier measured fragment layout but is within aggregate free RAM.

A fresh end-to-end run recorded **two `mm_compact` entries and two
production `vm_extent_move` entries**; the BRK succeeded, and the guest
continued through `CQ` and halted normally. After the move, all surviving
children resumed from the shared-pipe wait, verified sentinel data at
both ends of their private BSS, exited, and were reaped with expected
exit status. The harness checks each status and requires both real
physical relocation and guest completion. SIMH breakpoint/examine lines
may interrupt individual guest serial characters; the harness therefore
matches the ordered completion markers across those lines instead of
requiring an uninterrupted console string. It does not infer relocation
from successful BRK alone. The earlier native v548 public-allocator to
relocated user fetch/MOVEM test also passed after v561.

**The specific P0 physical-compaction plus scheduler-resumption gate is
closed.** This does not mark the other P0 correctness/release work in
`TODO.MD` complete and does not replace post-freeze RAM-placement and
high-water measurements. No v561 production source change or additional
permanent RAM was required beyond the accepted v560 BRK retry fix.

Status: source inspection completed; implementation and native acceptance open.

## Actual units and mechanisms

- `02000` octal is 1024 decimal words. `01000` octal is 512 decimal
  words. Neither number implies a generic 2048-decimal-word allocator.
- `system/kernel/mm/mm.c`: `mm_alloc()` calls `mm_alloc_aligned()` with
  alignment **1**. Extents record exact lengths. Gaps, not per-quantum
  bitmaps, represent free space. `mm_alloc_aligned_noreclaim()` also honors
  caller-selected power-of-two alignment and exact requested length.
- `system/kernel/proc/vm.h`: `VM_EXTENT_ALIGN_WORDS` is `02000` octal;
  `system/kernel/proc/vm_pdp6.s` uses APR-dependent size rounding and
  relocation at that boundary. The backend cannot grant hardware-protected
  process mappings at a 512-decimal-word boundary on the existing PDP-6.
- `system/kernel/mm/mm.c` compacts movable process extents using
  `VM_EXTENT_ALIGN_WORDS`; changing just the compactor to 512 would violate
  process relocation invariants.
- Existing software allocations include variable-size mount slabs,
  process tables, u-areas, and 0200-octal-word cache allocations using
  unaligned `mm_alloc()`. Do not rewrite those as fixed 512-word slots.

## What remains to be measured before editing MM

1. Trace exact process logical sizes versus rounded APR physical extents
   across RUN/EXEC/BRK, resize, swap and XIP; compute aggregate rounding
   waste and actual allocation failures under 32K, 48K and 256K profiles.
2. Audit each provider, backing, MEMFS and startup MM request for avoidable
   alignment or size rounding; distinguish required DMA/device geometry.
3. Confirm any additional useful 512-word *software-only* suballocation
   within larger APR extents can preserve ownership, protection, pinning,
   isolation, relocation, rollback and descriptor RAM without new hot paths.
4. Implement only a proven safe reduction, add regression cases to
   daimos-testkit, measure permanent RAM, peak RAM and timing, and publish
   implementation documentation in pdp10-doc after acceptance.

Decision: the P0 optimization target remains active, but changing the generic
MM allocation quantum is **not** an implementation task: no such global
quantum currently exists. Do not misrepresent a software suballocator as a
512-word PDP-6 APR capability. No source code changes or savings are claimed.

## V523 native APR contract guard (2026-10-27)

The accepted policy is now native PDP-6 APR mapping at 1024 decimal words,
not smaller emulated user maps. `daimos-testkit` adds
`tests/validation/daimos-mm-apr-granularity-v523.py`: a source-contract
regression for exact-size unaligned MM allocation, VM and swap alignment,
BRK rounding, and process compaction. It passes on the current source.
This is not a physical fragmentation/pressure benchmark; no RAM saving or
new runtime implementation is claimed. V524: postpone representative
fragmentation profiling and optimization until BSTORE, production XIP and
eligible-image compressed swap have stabilized. Use current measurements
only to diagnose correctness failures. Preserve native 1024-word APR maps;
verify descriptor/pin/compaction/rollback invariants first, then profile
realistic 32K/48K/256K workloads and assess net RAM and CPU benefits.

## V525 native descriptor checks (2026-10-27)

Under daimos-testkit using the current DAIMOS checkout, both existing
PDP-6 step-execution tests pass:

- `daimos-mm-extents-v250.sh`: 32 descriptor entries, capacity rejection,
  insertion and shifted contents.
- `daimos-mm-pressure-v268.sh`: 132-to-136 descriptor pressure, rejection,
  middle removal and refill.

These establish descriptor-table arithmetic only. APR hardware isolation,
physical compaction, pin/release and transactional swap/BRK rollback still
require independent native runtime acceptance; no fragmentation optimization
or RAM-saving source change is warranted on this evidence alone.

## V526 native pin lifetime regression (2026-10-27)

New daimos-testkit `daimos-mm-pin-lifetime-v526.{s,sh}` extracts the actual
PDP-6 MM assembly and verifies missing-pin underflow, first pin, pin query,
refusal to free while pinned, saturated 255-pin rejection, last unpin,
wrong-owner free rejection and final valid free. The initial diagnostic
fixture mistakenly cleared the extent's type bit when setting a one-pin
value; corrected packed metadata to `[011,,07]` (one pin, type one, owner
seven) and `[03771,,07]` (255 pins, type one, owner seven). Native v526,
v250, v268 and v523 source-guard tests now pass. No production MM change
or RAM saving follows from these contract tests. Physical compaction and
APR process isolation remain separate native acceptance tasks.

## V527 APR user-mode regression build blocker (2026-10-27)

Attempted existing daimos-testkit
`tests/validation/daimos-apr-user-protection-v1.sh` against the current
checkout with fresh 256K/full image. Compilation stopped before SIMH:
`DSH image 036157 exceeds EXEC limit 036000` (111 decimal words over).
The DAIMOS checkout has a separate uncommitted DSH AST-allocation change
in `userland/dsh/dsh_exec.c`; its causality has **not** been established.
Preserve that parallel work. This is a build-size blocker, **not** an APR
test failure. Do not mark hardware APR isolation or physical-compaction
rollback accepted on this run. The pin/descriptor stand-alone PDP-6
regressions remain passing. Fix the DSH image-size source issue in its
own workstream, then rerun the full-image APR and rollback tests.

## V527 compaction audit and APR test blocker (2026-10-27)

Production `mm_move_extent` excludes the source descriptor temporarily
while interrupts are disabled to find an overlapping destination, then
restores the descriptor before invoking `vm_extent_move`. A failed move
keeps the old descriptor; successful relocation is published afterward.
`mm_compact` skips pinned/non-process extents and retains the `02000`
octal APR alignment. This is source review, not native rollback acceptance.

Attempted `daimos-apr-user-protection-v1.sh`: the full image build failed
before SIMH with `DSH image 036157 exceeds EXEC limit 036000`. Thus APR
hardware protection was *not* exercised in this run. The concurrent DSH
working-tree edit was preserved. Correct the image-size/build blocker or
use a valid reduced APR fixture, then rerun protection and relocation tests.

## V530 update after KCC DSH size fix (2026-10-27)

The full PDP-6 APR user-protection regression now PASSES after DSH v528
folding. Repeated native EXEC commit/rollback v200: eight cases PASS;
MM pin lifetime v526 and MM descriptor v250 also PASS. These tests do not
directly exercise successful physical `mm_move_extent` relocation or its
VM-rejected rollback with real process payload/runqueue preservation.
That dedicated native acceptance remains open. Do not claim reduced RAM
consumption or completed fragmentation optimization from these checks.

## V531 isolated native MM relocation transaction (2026-10-27)

The new daimos-testkit native step test
`daimos-mm-relocation-v531.{s,sh}` runs the unmodified production
`mm_move_extent` and descriptor insert/delete assembler routines under
PDP-6 execution. A controlled fit and VM service verifies the original
descriptor is restored **before** VM invocation, then rejects the move:
the old extent, owner and count are preserved. A second run returns
success: the new physical base is published, preserving size/type/owner
and descriptor count. Both cases PASS, and PI disable/restore is balanced.
This is a deliberately *isolated* transaction test, not a claim that
real `vm_extent_move` process payload copying, scheduler/runqueue state,
or the hardware APR mapping were exercised. Integrated relocation native
acceptance remains open and RAM savings are not claimed.

## V532 native VM physical move (2026-10-27)

The new daimos-testkit `daimos-vm-extent-move-v532.{s,sh}` executes the
actual `vm_extent_move` and `fs_move_words` PDP-6 assembly using a minimal
non-runnable SIDL process without u-area. It verifies current-process
relocation rejection without mutation, then copies four physical words to
an APR-aligned destination, checks payload, published process base,
cleared transition bit and unchanged runqueue. Native test PASS.
The original fixture mistakenly stored the process records directly in
`proc_table`, which actually holds their pointer; fixing the fixture fixed
the failure without touching production code. Full stable-u-area/runqueue
resumption and actual user APR continuation still need integrated tests.

## V533 resident u-area stop/resume relocation (2026-10-27)

The native PDP-6 testkit `daimos-vm-uarea-relocate-v533.{s,sh}` executes
production `vm_extent_move` and `fs_move_words`, with an SRUN process
descriptor and stable u-area. A held translated user mapping correctly
rejects relocation before runqueue modification. Once released, four
physical data words relocate, the VM physical base updates, `PROC_STOP_MM`
is cleared, the process returns to SRUN, and the controlled runqueue hooks
observe one removal and one addition. PASS. No DAIMOS kernel source
changed. The runqueue service is stubbed and the test does not resume
real user instruction execution; fully integrated scheduler and APR
continuation remain open.

## V534 full 256K scheduler baseline (2026-10-27)

Ran `daimos-testkit/tests/validation/daimos-mm-256k-scheduler-v262.sh`
against current DAIMOS in fresh SIMH/full 256K configuration. PASS:
preemption, process context, nice ABI, sleep and wakeup. This executes
actual scheduler service and user processes, independent of the v533
isolated relocation fixture. It does **not** deliberately provoke a
physical relocation while a runnable process is parked and subsequently
resumed. Therefore the combined scheduler+compaction acceptance remains
open, even though the component regressions pass. No kernel source change
or RAM reduction is attributed to this baseline test.

## V536 production runqueue integration with VM relocation (2026-10-27)

New `daimos-testkit/tests/validation/daimos-vm-runq-relocate-v536.{s,sh}`
runs real PDP-6 assembly for `vm_extent_move`, `fs_move_words`,
`proc_runq_add` and `proc_runq_remove` on a stable-u-area SRUN fixture
with actual intrusive queue state. A held mapping rejects relocation;
after release the VM physical base/payload are updated, stop state is
cleared, and the process is queued again. Native PDP-6 step test PASS.
This removes the controlled runqueue stubs used by v533, but does not
cause a preempted user process to resume and execute through its newly
programmed hardware APR. That final full-system acceptance remains open.
No production source or resident RAM changed.

## V535 native compactor dispatch coverage (2026-10-27)

`daimos-testkit/tests/validation/daimos-mm-compact-dispatch-v535.{s,sh}`
executes the unchanged production `mm_compact` PDP-6 assembly. Its
controlled free-gap/move boundaries demonstrate that zero-size requests
are rejected without probing memory, non-process and pinned process
extents are skipped, the eligible process extent is selected with
`02000`-octal alignment, and a subsequent fit ends the transaction with
success. Native PDP-6 step test PASS. As free-gap search and relocation
are stubbed, this does not establish actual process relocation under the
full scheduler. That integrated resumption test remains open. No kernel
RAM growth or savings are asserted.

## V537 full-SIMH relocation pressure probe (2026-10-27)

An experimental testkit script `daimos-mm-relocation-integrated-v537.sh`
reuses the v280 private-data overcommit workload, observing the real
`vm_extent_move` entry via SIMH breakpoints. No kernel instrumentation is
added. The initial 48-user configuration failed before its first guest
start marker (admission not verified). A preserved 16-user diagnostic
completed all users and sampled 59 extent-insert entries (maximum
pre-insert descriptor count 37, hence at most 38/136), but observed **zero**
VM relocation entries. Neither configuration establishes an integrated
relocation success. The script is an **unaccepted diagnostic** and must
not be added to a passing test aggregate until it reliably triggers
real relocation and verifies the resumed guest. Next construct a
deterministic, legitimate fragmentation/compaction trigger instead of
increasing concurrency blindly or introducing a production debug hook.

## V538 deterministic native compaction-to-VM transaction (2026-10-27)

`daimos-testkit/tests/validation/daimos-mm-full-path-v538.{s,sh}`
builds a deliberate fragmented arena in an isolated PDP-6 step fixture.
It executes the actual `mm_compact`, `mm_find_fit`, `mm_move_extent`,
extent table mutation, `vm_extent_move`, `fs_move_words`, and intrusive
`proc_runq_add/remove` routines. Moving the resident stable-u-area SRUN
process from `014000` to `020000` (octal) joins two smaller gaps into
the requested contiguous `04000`-word region. The test verifies physical
payload at both ends of the 02000-word image, published VM base, extent
ordering, restored SRUN state and real runqueue membership. PASS.
The fixture stubs PI enable/restore only. This removes v537's uncertain
memory-pressure trigger but is still an isolated PDP-6 step test, not
continued user-mode instruction execution through APR under full SIMH.
No permanent kernel instrumentation or RAM growth introduced.

## V539 pinned-compaction rejection then unpin recovery (2026-10-27)

New native PDP-6 testkit `daimos-mm-pin-compact-v539.{s,sh}` extends
the real `mm_compact`/fit/extent/VM/FS/runqueue v538 transaction: with
the process extent pinned, compaction must return `MM_ERR_FRAGMENTED`
without changing the VM base or extent descriptor. Releasing that pin
and retrying the same fragmented arena then succeeds, preserves payload,
and restores runnable state and queue membership. PASS. The fixture
stubs only PI control; it does not demonstrate resumed user instructions
after APR reprogramming under a full multiprocess boot. No production
code or resident RAM changed.

## V540 native relocation-to-APR activation handoff (2026-10-27)

New testkit `daimos-mm-apr-handoff-v540.{s,sh}` extends deterministic
production assembly compaction (MM, VM, FS copy and real runqueue) by
executing production `vm_activate_current`. With the process VM word
correctly initialized to the `size,,base` ABI, after physical relocation
the APR shadow equals `0,,020000`: the expected relocated base and
limit for a `02000`-word process. Native PDP-6 step test PASS. The
separately rerun production APR fault-path test also PASSED on the AAP
PDP-6 emulator (`daimos-apr-aap-v1.sh`). These are distinct component
tests; resumed user instruction fetch after relocation under full SIMH
remains **unverified**. No production code or resident RAM was added.

## V541 user-fetch integration attempt (2026-10-27)

Attempted a testkit-only extension of the v540 native compaction fixture:
write a short PDP-6 instruction sequence inside the process image, move it
with actual `mm_compact`/`vm_extent_move`, program the APR through
`vm_activate_current`, and enter user mode at the relocated logical PC.
The first `p10run` attempt entered its low-core-break check at logical
`020`, not an MM fault. Moving the user entry to `0100` instead caused
the runner to time out without a readable result. Therefore this
experiment is **not accepted**; no user-execution-after-compaction PASS is
claimed. The underlying direct-fetch idea remains valid, but requires a
user-mode-aware SIMH/AAP fixture that can observe an exit/marker after
APR entry, rather than the existing executive-step harness's expectations.
No production source changes were made. Preserve the existing v540 PASS
without extrapolating it to scheduler/APR continuation acceptance.

## V542 native user instruction fetch after relocation (2026-10-27)

The testkit `daimos-mm-user-fetch-v542.{s,sh}` corrects the experimental
v541 fixture: its third instruction must be stored as an assembled word,
not executed as a host-test JRST during setup. With this correction, real
PDP-6 `mm_compact`/VM/FS/runqueue assembly moves an image containing
actual user instructions, production `vm_activate_current` programs APR,
and a user-mode JRST to logical `0100` executes a MOVEI/MOVEM pair.
The SIMH breakpoint confirms reaching logical `0102`; an executive
examination confirms the mapped MOVEM wrote `012345` at physical
`020200` under the relocated `020000`-word base. Native step test PASS.
This closes direct APR instruction-fetch and data-access verification
after relocation, but is not the full-system scenario of a preempted
user process restored by the real scheduler. No production changes.

## V543 full-kernel trigger feasibility audit (2026-10-27)

Re-ran native PDP-6 compaction v538, pin/retry v539 and relocated user
fetch v542 against the current checkout: all PASS. The remaining
full-system gate is more specific than merely observing an overcommitted
boot or BRK call. `mm_alloc_aligned` calls `mm_compact` only after an
initial fragmented allocation and the pressure/reclaim stage; the
request must have sufficient total free words but no aligned fitting
gap. A `BRK` request normally grows in adjacent space or first uses
reclaim; even its full-image fallback is not guaranteed to compact.
The existing v537 workload is consequently not an adequate acceptance
trigger: at 16 users it completed without any VM move, and at 48 users
it failed admission before guest startup. A decisive test must arrange
allocation/free lifetimes that cause this specific allocator transition,
observe entry and successful return from real `vm_extent_move`, and then
verify the relocated scheduled process's data and continuation. Merely
counting allocations or completing the scheduler test cannot substitute.
No kernel source or resident instrumentation was changed in this audit.

## V544 production 256K swap-cycle relocation observation (2026-10-27)

Prepared an external SIMH-only observer based on the existing v278
256K full-system swap/reclaim test, using the same test guest and a
breakpoint/examine at the production `vm_extent_move` symbol. A fresh
image completed all three guest swap/reclaim and process-reuse cycles,
but there were **zero** observed relocation entries. Accordingly v544
is an **inconclusive relocation diagnostic**, not a relocation acceptance
PASS; the observer remains experimental in daimos-testkit. This confirms
that forcing process swap does not by itself create an affordable yet
fragmented allocation demand that reaches `mm_compact`. Follow-up needs
a deterministically ordered guest extent lifetime and explicit request
with total free space sufficient but largest aligned gap insufficient.
No production changes, added resident counters, or claimed RAM savings.

## V545 allocator-entry integration probe (2026-10-27)

An experimental daimos-testkit native fixture extends the deterministic
v538 physical layout to call production `mm_alloc_aligned` instead of
calling `mm_compact` directly. The initial public allocation request
returned `MM_ERR_FRAGMENTED` (-2), with no descriptor movement or VM
base change. Temporary **test-fixture-only** counters subsequently showed
two `fs_memory_reclaim` calls but zero `mach_pi_disable` calls from the
move path. Thus this attempt has **not** demonstrated that the public
allocator reached the compactor; it must not be reported as a PASS.
Next inspect the native 6-argument call frame and the exact return values
and control flow through `mm_alloc_aligned_noreclaim` and the retry stage.
Do not change kernel MM policy or fabricate pressure until the root cause
is established. All temporary counters are local to the unaccepted test
fixture, not permanent kernel instrumentation. No source code changes.

## V546 public allocator signed-error comparison fix (2026-10-27)

Root cause of the v545 allocator-to-compactor failure: PDP-6 assembly
`mm_alloc_aligned` compared the signed 36-bit `MM_ERR_FRAGMENTED` (-2)
using `CAIE`, whose 18-bit effective-address immediate did not match the
full-width negative error result. The production test incorrectly skipped
`mm_compact` after reclaim. Changed the comparison to full-word
`CAME 1,[MM_ERR_FRAGMENTED]` in `mm_pdp6.s`.

The v545 testkit native fixture now uses the real allocator, free-gap
search, compactor, MM extent movement, VM physical relocation, FS copying,
and intrusive runqueue routines; its reclaim and PI functions are test
boundaries. It asserts that compaction actually moved a process, returned
the requested `04000`-octal contiguous module allocation, and preserved
process data and runnable state. PASS: v545, v538, v539, v540 and v542.
The complete preemptive-SIMH process-resumption gate remains outstanding.
No persistent kernel state added; measure linked-code size in subsequent
size accounting instead of presuming unchanged code size.

## V547 corrected full-system swap-cycle regression (2026-10-28)

Re-ran the existing v544 SIMH relocation observer after the production
v546 allocator error-comparison fix: the 256K guest again completed all
three swap/reclaim cycles without any observed `vm_extent_move` entry.
That is a valid negative relocation result, not a failed swap test.

Separately, source inspection uncovered a pre-existing C99 call-site bug
in daimos-testkit `daimos-mm-swap-cycles-v278.c`: the stopped-job case
passed two integer process IDs to `target_is_swapped`, whose second
argument requires `const int *` and third argument requires a count.
Replaced these arguments with a local two-element ID array and count 2.
The corrected guest cross-compiles without that warning, and the full
256K SIMH v278 regression PASSES all three swap/reclaim, wait, and
process-slot reuse cycles. No DAIMOS production code changes in v547.
The complete scheduled-relocation gate remains open: neither running
swap pressure scenario generated the necessary fragmented allocation.

## V548 public allocator to relocated user instruction execution (2026-10-28)

New testkit `daimos-mm-alloc-user-fetch-v548.{s,sh}` integrates the
production PDP-6 `mm_alloc_aligned` allocation/reclaim/compaction entry
with the deterministic fragmented extent fixture and v542 real APR user
instruction fetch. A `04000`-octal MODULE allocation forces the fixed
allocator path to move a resident `02000`-word process. The test checks
the new `012000` allocation, updated extent table and process mapping,
copied process payload, restored runqueue, APR programming, and mapped
user-mode MOVEI/MOVEM execution ending at logical `0102` with the expected
physical `020200` write. Native SIMH step test PASS. Only reclaim and PI
services are controlled boundaries; no permanent resident data added.
The remaining full-system gate is actual scheduler-restored continuation
following relocation during a complete kernel boot, not native APR fetch.

## V549 24-process full SIMH relocation probe (2026-10-28)

Re-ran the experimental v537 256K SIMH multiprocess observer with
`DAIMOS_MM_CONCURRENCY_COUNT=24` against the v546 signed-status fix.
The guest completed its expected markers, and the test recorded 75
extent-insertion samples with a maximum pre-insertion descriptor count
of 53 (at most 54 used of 136 capacity). It observed zero entries
to `vm_extent_move` and therefore returned INCONCLUSIVE as designed.
This is useful evidence that the admission/pressure workload is not a
deterministic fragmented-but-affordable allocator test; high descriptor
occupancy is not the problem. Do not interpret missing relocations as
evidence of a kernel relocation failure. Preserve the already accepted
v548 native public-allocator-through-APR validation; full-scheduler
post-relocation continuation remains unverified. No source code or
resident kernel state changed.

## V550–V551 controlled process-lifetime SIMH probes (2026-10-28)

Created experimental testkit-only full-boot guests which create sleeping
child processes gated by distinct pipes, release alternating children,
and check continued execution. Both load complete 256K PDP-6 SIMH
images. The v550 six-child allocation/release/replacement run reached
all its guest completion markers but produced **zero** breakpoint hits
on the production `vm_extent_move` entry: INCONCLUSIVE for relocation.

The initial v551 fourteen-child variant stopped at guest marker `!P`
before growth: it exceeded the INIT process's per-process file descriptor
capacity by creating too many private gate pipes. The corrected seven-
child variant reached `AB` (all children created and alternating children
released), but the requested parent `BRK` growth of `030000` octal words
failed (`!G`) and no VM relocation was observed. It was not an accepted
fragmented-but-affordable allocation, and no kernel bug is inferred.

Both experiments remain uncommitted in daimos-testkit; the successful
v548 deterministic native public-allocator-to-user-fetch test is
unchanged. A viable final full-system acceptance workload must measure
available/maximum contiguous space or otherwise constrain allocation
lifetimes and sizes so an MM_ERR_FRAGMENTED request has enough **total**
free memory to be resolved by moving a runnable process. A fixed oversized
BRK request or generic swap pressure does not establish that condition.
No production kernel code or permanent runtime instrumentation changed.

## V552 shared-gate SIMH fragmentation probe (2026-10-28)

Created a testkit-only experimental full-boot guest with one shared pipe
and 16 sleeping process images. It releases alternating child PIDs through
`SYS_PROCCTL_EVENT_PID` / `SYS_EVENT_TERM` and waits for their termination,
avoiding the per-process file-descriptor limit found in v551. The full
256K PDP-6 SIMH run reached `AB`, confirming both creation and selective
release. Parent `BRK` growth by `010000` octal words still returned
failure (`!G`); the production `vm_extent_move` breakpoint was not hit.
Therefore this is **not** a relocation acceptance test or evidence of a
kernel relocation bug. It confirms the gate-count limitation can be
removed without changing production code. Further arbitrary BRK sizing
is not warranted: first instrument the actual physical extents/free gaps
via the external simulator at the growth boundary to determine whether
there is enough total free memory and insufficient contiguous free space.
No kernel changes or permanent resident counters. The unaccepted v552
fixture remains in daimos-testkit working tree for further diagnosis.

## V553 measured MM free-space geometry at failed BRK (2026-10-28)

A fresh, isolated 256K SIMH boot of the v552 shared-gate guest used an
external breakpoint on exported `vm_space_brk_current` and examined the
production MM arena/count/descriptor addresses. This avoided the invalid
post-boot writable-media replay used in the first attempt. The guest
reached `AB` before `!G` (growth failure); two BRK-entry snapshots were
observed, with 25 sorted occupied extents in the first snapshot.

Decoded managed arena: physical `037771` through `1000000` (octal,
exclusive end). The free gaps between those 25 extents total **150,685
DECIMAL words**, and the largest individual gap is **`0224746` OCTAL
words** (76,262 decimal). This is much greater than the parent's
requested extra `010000` octal (4,096 decimal) words. Consequently,
a simple global RAM exhaustion / no sufficiently large physical hole
explanation is inconsistent with the snapshot. However, the `BRK` path
may require a larger **full replacement image** than the requested delta,
or reject a logical break constraint; the snapshot alone does not isolate
the exact return cause. No compaction event was observed. A versioned
host-side snapshot decoder is in the testkit as
`tests/validation/daimos-mm-extent-gaps-v553.py`; it decodes the
specific kernel-map locations of this v553 image, not a universal ABI.
Do not infer a production MM defect without observing the BRK input,
rounded size, allocator result, and current process image extent.
No production code, persistent counters, or RAM overhead added.

## V554 exact logical BRK request and fallback analysis (2026-10-28)

The experimental v554 guest extended the shared-gate v552 test with
six-digit octal serial reporting of the parent `dsys_brk` arguments and
result. Fresh 256K SIMH run reached `AB` and reported
`[024000+034000=777777]!G`: current logical break `024000`, requested
break `034000` (delta `010000` octal words), failure result `-1`.
The new break is a valid 18-bit logical address already aligned to the
native `02000` APR quantum; logical overflow or excessive virtual break
cannot explain failure. The previously recorded v553 extent snapshot
contains a gap substantially larger than the *entire* requested `034000`
word replacement image, not merely the delta. This rules out simple lack
of any physical space for the replacement as a sufficient explanation.

Source inspection of `vm_pdp6.s` shows BRK tries same-base upward growth,
then in-gap downward relocation, retries after swap reclaim, and finally
allocates a complete new process extent via production `mm_alloc_aligned`
with `MM_ALLOC_HIGH`. The old image at the high end is adjacent to a
preceding occupied extent and has only a small gap above it. Thus a
full-image fallback is plausible. The snapshot and syscall result alone
still do not show whether the allocator was reached, returned failure,
or succeeded before later copy/free/commit rejection. Next instrument
these exact boundaries *externally* (without persistent kernel hooks)
prior to modifying kernel logic. v554 remains an unaccepted diagnostic
fixture in daimos-testkit. No production changes or permanent RAM cost.

## V555 full-system allocator-entry breakpoint (2026-10-28)

A fresh isolated 256K SIMH image of the v554 guest was run with an
external breakpoint on exported production `mm_alloc_aligned` (map
address `001665`). The guest again printed `AB[024000+034000=777777]!G`.
The breakpoint occurred 62 times during startup/guest execution, proving
that production allocator entry is reachable in this boot. The captured
pre-call descriptor counts alone do not identify which specific entry,
if any, belongs to the failed parent `BRK` transaction. No claim is made
that the failed BRK entered or returned from the allocator; the necessary
next diagnostic is to correlate caller PC, arguments, and return value
with the parent BRK syscall boundary, without adding persistent KCORE
state. The v555 observer is still experimental in daimos-testkit.
No production code or permanent RAM change.

## V556 correlated parent BRK and allocator entry trace (2026-10-28)

Fresh isolated full-profile 256K SIMH boot, using the experimental v554
parent/16-child guest and external breakpoints on `vm_space_brk_current`
(`016614`) and `mm_alloc_aligned` (`001665`). At guest phase `AB`, two
successive BRK entries have current process slot 1 and MM descriptor
count 25 (`031` octal): the first is `dsys_brk(0)`, returning `024000`;
the second is `dsys_brk(034000)`, which returns `777777` (`-1`). There
is **no intervening `mm_alloc_aligned` breakpoint** between that second
BRK entry and the guest `!G` failure. The overall boot recorded 62
allocator entries, but the only entry after the failed parent BRK belongs
to process slot 0 during exit/cleanup (extent count `026` octal).

Thus this particular BRK failure happens before the full-image public
allocator fallback. The allocator's ability to find a large gap is not
relevant until the early return is explained. Investigate BRK's preflight,
process descriptor/pin/owner checks, arena checks, and pre-fallback
path using the exact linked image and source. Avoid attributing this
failure to compaction or changing the kernel without further proof.
No production code or permanent RAM modification. The v556 breakpoint
harness remains experimental in daimos-testkit.

## V557 BRK failure-exit breakpoint confirmation (2026-10-28)

A fresh 256K PDP-6 SIMH boot of the v554 shared-gate guest used external
breakpoints calculated from the linked `vm_backend_pdp6.dobj` text offset
and map position. At the parent `dsys_brk(034000)` request, production
`vm_space_brk_current` transferred to `vm_brk_fail_pi` at `017134`, then
to `vm_brk_fail` at `017137`, returning `777777` (`-1`). `proc_current_slot`
was 1, `proc_current_ptr` was `040156`, and the descriptor count at the
failure was `030` octal. The public allocator was not entered during this
failing call (consistent with v556). This establishes a pre-allocator
PI-disabled failure exit; it does **not** yet distinguish invalid process
extent type, owner, length, pin state, or arena validation, since these
checks share `vm_brk_fail_pi`. Next sample the parent record and matched
extent at this breakpoint, then identify the precise failing conditional.
No kernel changes, no permanently resident diagnostics, no claimed
scheduler-relocation acceptance. The v557 trace harness remains
experimental in daimos-testkit.

## V558–V560 actual BRK reclaim retry defect and accepted fix (2026-10-28)

The v558 full-SIMH failure snapshot showed that INIT's process record
(`024000,,752000`) and corresponding MM process extent match in size,
base, owner 1 and pin count 0; the managed arena encompasses the image.
V559 external breakpoints established the exact sequence: initial process
extent lookup succeeded; the BRK growth path entered reclaim; MM extent
count dropped from `031` to `030` octal; the subsequent lookup failed
before the arena and full-image allocator stages. The defect was in
`vm_space_brk_current`: its reclaim retry jumped directly to
`vm_brk_find_extent` without resetting AC16, which still held the
previous descriptor index, even though reclaim may delete and shift
descriptors. This causes the retry to skip the current process extent.

An initial hypothesis that swap reclaim needed additional AC13–AC16
preservation was tested with a full 256K SIMH boot; the guest still
failed (`[024000+034000=777777]!G`), so that experiment was reverted.
The actual production fix is one `setz 16,` before the reclaim retry's
`jrst vm_brk_find_extent` in `system/kernel/proc/vm_pdp6.s`, with a
comment explaining descriptor-table reordering. No resident storage.

After the fix, the previously failing guest produced
`<AB[024000+034000=034000]CQ` under full 256K SIMH. The standalone,
versioned testkit `daimos-mm-brk-reclaim-retry-v560.{c,sh}` independently
rebuilt and passed the same complete 16-child/reclaim/BRK/continuation
scenario. The existing native public allocator/relocated user fetch v548
regression also still passed. This closes the production BRK retry bug,
**not** the remaining full-system scheduler-after-physical-relocation
acceptance requirement; no `vm_extent_move` was required in v560.
