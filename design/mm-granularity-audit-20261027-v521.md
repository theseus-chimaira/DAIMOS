# PDP-6 memory granularity P0 source audit (v521, 2026-10-27)

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
