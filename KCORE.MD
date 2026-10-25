## 2026-10-25 verified whole-core swap-out return snapshots v314

The PDP-6 SIMH debugger stops at the successor instruction when a
breakpoint is requested on an instruction. The v313 attempt stopped
at reclaim's `PUSHJ proc_swap_out` address 003266 and its 003267 return,
producing two identical 256K dumps because the call had already
returned at the first snapshot. Its zero-difference output is an
**invalid before/after transaction comparison**, not proof of no change.

v314 fixes the pre-call stop to 003265 (SIMH displays the following
`PUSHJ 17,2623`) while retaining 003267 for the post-return stop.
The DXR call, prior branch and return instructions are verified before
SIMH runs. Captures contained all 262144 PDP-6 physical words apiece,
identical address coverage, and **75 changed words**: 12 below KCORE,
9 in permanent KCORE code/data range, 2 MM descriptors, and 52 beyond
the descriptor table. `mm_extent_count` changed 031 -> 030 octal;
there were zero unmatched addresses. This is a genuine pre-call to
post-return memory difference; expected kernel stack, MM extent and
swap metadata updates are present. Not all 75 changes have been
semantically proven correct, so it is **not a global non-corruption
proof**. The 20-child native swap guest remains PASS under v311.
The testkit fixture is `daimos-mm-swap-snapshot-v314.sh`, using the
v312 splitter/comparator. No production modification or KCORE growth.
With swap correctness restored, return to EXEC/RUN permanent kernel
minimization; additional memory investigations are optional unless a
new regression appears.

## 2026-10-25 native full-core swap snapshots v312

The testkit captured both 256K-word (262144-word) PDP-6 physical
RAM images for a swap-out, from `proc_swap_out` entry to
`proc_storage_publish_v30` entry (the latter is **pre-publication**, not
post-commit). The two complete, equally addressed dumps differ in
82 words: 20 below KCORE, nine in permanent code/data below the MM
extent table, two MM descriptor words, and 51 words above it.
`mm_extent_count` moved from `031` to `030` octal, consistent with an
extent release, and the swap record/accounting changed as expected.
This is a reproducible memory comparison tool, **not a global
non-corruption proof**, an after-return snapshot, or another guest
acceptance run. In particular, some AC/low-memory words naturally
change and must not be called corruption. The fixed 20-child v311
acceptance run still passes, so no speculative kernel fix is needed.
The testkit scripts `daimos-mm-swap-snapshot-v312.sh`,
`daimos-mm-swap-snapshot-split-v312.py`, and
`daimos-mm-swap-snapshot-compare-v312.py` capture, extract, and
classify the images without consuming guest RAM. The comparator
requires identical full 256K address coverage. More granular
semantic invariants and optional post-return capture can be pursued
separately if another swap regression warrants them.

### Post-INIT swap pressure acceptance v311

The native 256K PDP-6 GCC 20-pressure-child single-cycle fixture
passed with 012000 octal (5120) drum backing blocks, guest completion
marker `Q` and normal HALT. The 16-child fixture already passed with
010000 octal (4096) backing blocks. This resolves the immediate swap
correctness investigation following the real failed-swap-in local-stack
pop repair `3b41e87`; it does not validate smaller backing or all
possible 64-slot pressure workloads. No KCORE or descriptor-size
increase. Return to resident EXEC/RUN minimization and low-RAM
correctness analysis; 32K strict acceptance remains deferred.

## 2026-10-25 v311 twenty-child native swap acceptance — RUNNING / NOT ACCEPTED

The production kernel remains unchanged after the successful v310
16-child 256K PDP-6 GCC full-profile one-cycle acceptance using
`DRUM_BACKSTORE_BLOCKS=010000` (4096 decimal blocks) and a 900-second
SIMH-only ceiling. v311 extends the existing instrumented guest from
16 to 20 pressure children, retaining writer pipe integrity, swapped
writer restore, child wait/reap, and guest completion checks. The
original requested `020000`-octal backstore could not fit the single
`020000`-sector drum with logstore and cachestore and failed at
`mkdsk` before SIMH. v311 was corrected to `012000` octal backing
(5120 decimal blocks); the drum image built and `d6fsck` passed.

The no-breakpoint simulator run has so far emitted
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFPGPHPIPJSR`:
all 20 children were admitted (`P0`..`PJ`), reached sleep (`S`),
and the parent reached its first writer-pipe read (`R`). At this
checkpoint the simulator process is **still running**; no `Q`, guest
HALT, or shell PASS result has been observed. This is *not* 20-child
acceptance, nor proof of a deadlock. The fixture has a 900-second
SIMH ceiling and retains its work directory for post-run inspection.
The two testkit commits are `f775ae4` (fixture) and `96d8711`
(drum geometry correction). Do not alter production swap policy,
KCORE size or descriptor capacity on this incomplete evidence.

Previous v307 snapshot analysis found expected live stack changes,
not a demonstrated memory overlap. v294 measured 28 of 64 idle-stack
words at the nested swap-out entry; v293 doubling the idle stack did
not remove the old UUO. Later `3b41e87` fixed a genuine swap-in
failure-path stack pop error, and v310 passed with sufficient backing.
Further tests belong only in daimos-testkit.

## 2026-10-25 extended 16-child swap acceptance v310 — PASS

The earlier 360-second high-capacity 16-child tests (v304/v305/v308/
v309) ended in host timeout, even though v309 reached marker `R`
just before the parent's first writer-pipe read. The unchanged v309
instrumented guest was rerun with `DRUM_BACKSTORE_BLOCKS=010000`
(4096 octal blocks), 256K PDP-6 full GCC image, no SIMH debugger
breakpoints and a **900-second simulator timeout**. Versioned testkit
fixture `daimos-mm-swap-progress-v310.sh` passed its one-cycle guest
completion checks (`<`, `w`, `Q`, guest HALT): writer pipe payload
integrity, sixteen pressure children, wait/reap and cleanup succeeded.
The full command including image build took roughly 569 seconds;
that duration is **not** the simulator-only elapsed time.

The v309 `proc_wait_event` snapshot was thus insufficient to conclude
a deadlock: under this configuration the program completed with a
longer execution budget. Original 2048-block backing remains a
separate capacity-exhaustion case and is not accepted at 16 children.
The 12-child current-kernel passing baseline remains valid.
No production code change, linked KCORE growth or descriptor-capacity
increase. Do not add debugger-heavy traces to this acceptance fixture.

## 2026-10-25 expanded swap writer-pipe blocking boundary v309

An additional one-byte progress marker `R` was emitted by the native
16-child 4096-block backing-store guest only after `dsys_close(gate_r)`
and `target_is_swapped()` both succeeded, immediately before the
first writer-pipe `dsys_read_words` call. The SIMH run printed
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFSR` then timed out at kernel
`proc_wait_event` PC `017440` after the configured 360 seconds.
All 16 pressure children were admitted and asleep. The parent reached
the first pipe read but never returned from it. This strongly
localizes the high-capacity failure to the writer process resuming,
pipe output availability/wakeup, or related scheduling, not to
pressure-child admission or the prior swap-state guard. The marker
does not itself prove whether the writer ran or attempted to write.
Next inspect the writer's PROC_SLEEP reason, SWAP residency and
run-queue state at the blocked pipe read without high-volume SIMH
breakpoints; compare with 12-child passing trace. No production code
change or KCORE growth.

## 2026-10-25 expanded backstore 16-child progress localization v308

The unmodified 256K PDP-6 kernel, with `DRUM_BACKSTORE_BLOCKS=010000`
(octal, 4096 blocks), ran a lightly marked v308 guest (no SIMH
breakpoints). Its guest output was `<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFS`.
Each `P` index proves a pressure child was admitted; `S` is printed
only after all sixteen reached PROC_SLEEP. The simulator nonetheless
timed out after 360 seconds at `proc_wait_event` PC `017440` without
`Q`, `a` or `s` error markers. Thus the long-run stall is **after**
all sixteen children sleep, in the `dsys_close(gate_r)` /
`target_is_swapped` check or first writer-pipe read before its
return. It is NOT an admission bottleneck. A v309 check should place
low-cost markers around this boundary, avoiding MM breakpoints.
No production code change or KCORE growth.

### Physical fault snapshots v307

The retained v297 snapshot compared 317 memory words, changing 27 of
64 sampled idle-stack words, 1 of 83 sampled process/MM metadata words,
and 0 of 4 sampled storage-map words. Both idle-stack base pointers
remain `040350` octal. Execution continued between captures, so these
changes cannot by themselves establish illegal writes or a stack
overflow. The v307 testkit report classifies physical changes without
assuming that a changed stack word is a corrupted word. No KCORE
growth or production change.

## 2026-10-25 current-kernel twelve-child regression v306 — PASS

After the production swap-in allocation-failure local-pop correction
(commit `3b41e87`), the original 256K PDP-6 GCC v281 twelve-child
post-INIT guest was rebuilt and rerun unchanged. Native SIMH reported
`PASS (one post-INIT 12-child pressure cycle)`, with allocator
insertion peak 31/136 across 71 samples. This reconfirms writer
restoration, pipe data integrity and child completion at 12 children
under the current code. It does **not** prove the 16-child case,
which still fails due to swap-space exhaustion and an observed writer
TERM event, or resolve the 4096-block v305 timeout. No production
source or KCORE-size change in this regression.

## 2026-10-25 expanded swap capacity experiment v305 — FAIL/timeout

A native 256K full-profile PDP-6 GCC 16-child acceptance attempt
with `DRUM_BACKSTORE_BLOCKS=010000` (octal; 4096 blocks), doubled
from the default `04000` (2048), used the v302 instrumented guest
without SIMH debugger breakpoints. The guest emitted `<w` but never
reached `Q`; the simulator stopped at its 360-second timeout
(`SIGTERM`, PC `017441` in the wait path). The run did **not** show
the previous unexpected UUO, but did not satisfy writer completion
or child reaping. The larger backing area therefore does not yet
establish 16-child acceptance or settle whether the failure is
capacity-only. A concurrent separate v305 run may also exist;
this result belongs to the run preserved as
`$TMPDIR/daimos-mm-swap-capacity-v305.log`.
Next distinguish slow forward progress from blocked writer / scheduler
wait with a lower-overhead trace, and retain correct behavior under
insufficient backing capacity. No production change or KCORE growth.

## 2026-10-25 writer termination event confirmation v303

A diagnostic SIMH run of the 16-child v302 guest placed one breakpoint
at `proc_event_apply` (linked address `015370`). At the breakpoint,
AC1=2, AC2=1 (`SYS_EVENT_TERM`), and `proc_sched_cursor=2`.
The guest emitted `<w!2101c!W` and halted, matching the earlier
v302 observation of writer PID low digit 2, EXITED kind 1, exit status
001 octal instead of 050. This establishes a kernel TERM event for
slot 2 in the failing run, rather than a normal writer exit.
The trace does not yet establish the event call's return address nor
that every exhausted swap-in should be terminated. Previously
captured swap backing space was insufficient for the tenth allocation;
the correct treatment of a temporarily nonresident process requires
investigation before changing scheduler semantics.
The event-only breakpoint took substantially longer than a plain
guest run. No production source or permanent memory change.

## 2026-10-25 writer WAIT exit status capture v302

The versioned 16-child guest v302 instrumented `wait_report()` with
`!` plus low PID octal digit, status kind, and 2-digit octal status.
The unmodified native PDP-6 GCC 256K kernel ran to guest HALT,
producing `<w!2101c!W`, **not** the former unexpected UUO.
The writer wait returned a PID with low three bits 2, EXITED kind 1,
and **exit status 001 octal** rather than the expected writer exit
status 050 octal. The writer source explicitly exits with 041, 042
or 050 only; this implicates kernel-side process termination.
`proc_swap_service_failed` on an unsuccessful restore invokes
`proc_event_apply(...,SYS_EVENT_TERM)`, a fatal event; termination
status 001 is consistent with this path, although no direct event
breakpoint has yet established the causal call for this writer.

Earlier v297 established 2048 swap blocks total, 1872 used, and a
208-block request with only 176 remaining. The 16-child case thus
probes real backing-store exhaustion, not simply MM descriptor or
storage map overlap. Further work must distinguish required graceful
capacity failure from incorrect scheduler termination and evaluate
whether the acceptance test should use an adequately sized swap tail.
Do not claim a 16-child PASS or change swap policy without an observed
failure path. Production code unchanged in v302.

## 2026-10-25 lightweight writer outcome isolation v300/v301

The 16-child 256K PDP-6 fixture used a versioned guest (v300) to
replace the compound `d` writer-failure marker with distinct markers:
`a` first pipe read, `b` first word mismatch, `c` writer WAIT exit
status mismatch, `d` pipe drain, `e` pipe close. A verbose v300 SIMH
breakpoint run timed out before any of these guest markers, so its
result was inconclusive. Version v301 removed the high-volume MM
breakpoints and ran the same guest with a 360-second SIMH timeout.
It completed with `<wc!W` and HALT, not the old unexpected UUO.
The first pipe read returned one word with the expected `A` value;
`wait_report(writer, SYS_WAIT_EXITED, WRITER_STATUS)` failed. The
actual wait PID, status kind and value remain unmeasured. It is not
established whether the writer was terminated on swap-in failure,
returned the wrong status, or produced some other wait result.
This run is still a FAIL, not 16-child swap acceptance. Next obtain
exact wait result and writer state, then repair the verified error
path, preserving correct backing-store exhaustion behavior.
No production kernel modifications in v300/v301.

## 2026-10-25 swap-in failed allocation return correction v299 test — partial

After commit `3b41e87` corrected the missing one-word stack-local
pop on the `proc_swap_in_alloc_fail` path, the existing native 256K
PDP-6 GCC 16-pressure-child v298 diagnostic was rerun. The original
`UUO 000000777777 340000000003` was **not** seen. Instead the
fixture emitted `d!W`: the writer output/read or wait verification
failed after all pressure children had been created. SIMH then ran
until its 300-second timeout (`SIGTERM`, PC `017441`, within
`proc_wait_event` boot wait), so the test is still **FAIL**, not an
acceptance run. `d` comes from the fixture's pipe/read/wait/drain
validation, not the swap allocator directly. The existing trace
shows recurrent `proc_swap_in` failure-return handling and the old
control-flow UUO is no longer reproduced under this workload.

The validated defect is the missing local pop. Do not interpret the
absence of a UUO in one run as complete repair: next identify which
writer outcome caused `d`, distinguish legitimate swap-capacity
failure from bad process termination semantics, and rerun the 12-child
PASS fixture to detect regressions. The old 16-child case cannot be
accepted until pipe integrity and complete child reaping succeed.
No additional production changes in v299.

## 2026-10-25 nested swap and backing-store exhaustion v294-v297

v294 captured the actual idle stack base `040350` and AC17
`000034040404` octal at the failing nested swap-out, i.e. 28
pushed words of the configured 0100-octal (64-word) idle stack.
At v295, execution reached the nested `proc_swap_out` call to
`proc_swap_skip_words` and the subsequent `mm_pin` check, with AC17
`000043040413` (35 pushed words). v296 traced the later
`backstore_alloc` return of -1 with AC17 `000045040415`
(37 pushed words). These snapshots do **not** demonstrate an idle
stack overflow at the sampled points; future deeper use is not
excluded. The separate 128-word experiment in v293 still failed.

v297 directly confirms backing-store exhaustion: total
`backstore_blocks=04000` octal (2048), in-use
`backstore_blocks_used=03520` octal (1872),
`backstore_enabled=1`, and new allocation AC1=`0320` octal
(208). Only `0260` octal (176) blocks remain. Nine prior 208-block
swap writes used 1872 blocks; the tenth allocation correctly returns
-1. This is a capacity refusal, not a detected overlapping MM
extent or a proven stack overflow. The subsequent unexpected UUO
`000000777777 340000000003` remains a **kernel error-path defect**
to diagnose. In particular, trace the propagation of the allocation
error from `proc_swap_out` through `proc_swap_reclaim`,
`mm_alloc_aligned` and `proc_swap_in` into `proc_swap_service_one`.
Do not increase KCORE, the idle stack or backing-store capacity as
a substitute for fixing error handling. The 12-child acceptance
fixture remains valid, the 16-child fixture still fails.

## 2026-10-25 v296/v297 physical pre/post swap-out snapshots

Native 256K PDP-6 full-image, sixteen-pressure-child SIMH breakpoints
captured 9 successful swap-outs as matching pre-entry and pre-storage-
publication snapshots (v296). A tenth swap-out entered but never
reached publication before `UUO 000000777777 340000000003`.
The v297 repeat added checkpoints at `mm_pin`, `backstore_alloc` and
a final dump after the UUO halt. On the failing tenth swap-out,
`mach_kernel_stack_base=040350` octal. AC17 at `proc_swap_out` entry
was `000034040404`, then `000044040414` at `mm_pin`, but at
`backstore_alloc` it was `000001040351`, nearly reset to the idle
stack base. The after-HALT dump again had AC17=`000001040351`.
Comparing the tenth swap-out entry with the post-HALT dump across
captured memory found 28 changed words in the inspected idle-stack
region, one process-table word and one MM extent-descriptor word.
The first packed storage-state word at `041141` remained
`000005252520` octal. These differences span continued execution
and an asynchronous fault, so they do not on their own prove illegal
memory writes. The abrupt stack-pointer change is strong evidence to
inspect PI6/PI7 handler and scheduler stack reset/return handling;
there is not yet proof of the specific mechanism.

Diagnostic files: daimos-testkit `daimos-mm-swap-snapshot-v295.sh`
(reuses boot, but a previously run disk image stopped at `?RT`),
`daimos-mm-swap-snapshot-v296.sh` and
`daimos-mm-swap-fault-snapshot-v297.sh` (fresh image), plus
`daimos-mm-swap-snapshot-diff-v297.py`. No production code change.

## 2026-10-25 nested allocator stack trace v293 — unresolved

The unchanged 16-pressure-child PDP-6 guest was traced through
`mm_alloc_aligned`, `mm_compact`, `proc_swap_reclaim`,
`proc_swap_victim`, `proc_swap_out`, and `proc_swap_in` with AC17.
At the first swap-in for slot 2, `mm_alloc_aligned` entered reclaim,
selected resident slot 1, and entered a nested `proc_swap_out` before
`UUO 000000777777 340000000003`. The trace captured AC17 as
`000034040404` octal at that swap-out entry. The configured idle stack
was 0100 octal (64 words), making stack overrun a hypothesis but not
proof: the full frame depth and actual allocated stack boundary were
not verified from that trace.

A controlled source experiment doubled the configured KINIT and
scheduler idle-stack size from 0100 to 0200 octal words. The same
16-child test still failed with the same UUO and the same captured
AC17. The two source changes were restored. This does not demonstrate
that expanding the idle stack fixed anything, and the unchanged SP
merits independent verification of stack selection/boot initialization.
Next check actual `mach_kernel_stack_base`, its owner-2 MM extent and
saved scheduler stack pointer under SIMH, and test stack bounds rather
than increasing permanent RAM or guessing at a larger stack.
No production code was retained.

### Swap metadata extent overlap inspection v292

At the 16-child failing guest's UUO aftermath, the two-bit storage
map at `041141` octal is within kernel-dynamic owner-4 extent
`[040750,041145)`. Captured neighboring MM extents do not overlap.
Slot 1 is RESIDENT, slot 2 DISK in its first map word
`000005252520` octal. Earlier slot-2 mode and block count validation
succeeded. Post-UUO checks that failed are not evidence of the
initial cause. Full descriptor-table overlap validation and nested
allocator/stack instrumentation remain open. No KCORE size change.

## 2026-10-25 first swap-read return probe v287 — inconclusive

Three iterations of the existing 16-child reproducer attempted
breakpoints around the linked `proc_swap_in` read return and rollback.
The base object-file disassembly exposes instructions after
`PUSHJ backstore_read`, but the inferred relocated breakpoints did
not trigger; early iterations also contained incorrect octal
address arithmetic. The native trace still reproduced the UUO.
A separate direct-image probe attempted to examine the linked code
but halted early at `?RT` and supplied no usable instruction evidence.
Therefore the read return code and the exact rollback cause remain
**unmeasured**. Do not conclude a read failure or a specific code
corruption mechanism from this probe. Next obtain authoritative final
linked disassembly or a simulator return-stack/trace snapshot and
verify each breakpoint address before launching a costly reproducer.
No production code change or KCORE growth.

## 2026-10-25 first swap-in rollback trace v286

The unmodified 16-pressure-child PDP-6 GCC reproducer again emitted
`UUO 000000777777 340000000003` and halted at PC 000001. A new
SIMH-only breakpoint fixture traced storage-state checks and
publication, backstore reads/writes, unpin, and MM-free operations.
The run recorded **one** `proc_swap_in` entry, **one**
`backstore_read` entry, **no** storage-publication entry associated
with that swap-in, and an `mm_free` entry before the subsequent
`proc_swap_out`. This is consistent with the first swap-in falling
into its rollback path; the exact failing return value was not
sampled. Over the entire run there were 10 swap-out entries,
9 backstore-write entries, 9 publication entries, 19 MM-free
entries and 10 unpin entries. Following the first swap-in rollback,
`proc_swap_out` was entered again, then the unexpected UUO appeared.
The 16-child workload remains a **FAIL**. No kernel source was
modified, no instruction-corruption root cause is claimed and
KCORE size/descriptor policy remain unchanged.
Next: sample the return from `backstore_read`, distinguish read
failure from unpin failure, and check saved process state after
failed restore.

## 2026-10-25 native swap-transition event trace v285

The unchanged 16-pressure-child test reproduced the unexpected UUO.
A new SIMH breakpoint fixture sampled `proc_current_slot`,
`proc_sched_cursor`, and AC1 on each `proc_swap_out`, `proc_swap_in`,
`proc_swap_service_one`, `backstore_read`, `backstore_write` entry,
without changing KCORE. The execution logged 10 `proc_swap_out`
entries, one `proc_swap_service_one`, one `proc_swap_in`, one
`backstore_read` and 9 `backstore_write` entries. The final sequence
was swap service request `proc_sched_cursor=0402` (octal),
`proc_swap_in` AC1=2 in slot-0 context, then `proc_swap_out`
AC1=1 in slot-0 context, followed by the same unexpected
`UUO 000000777777 340000000003` and PC 000001 HALT.
These are entry observations, not verified successful transitions.
The process-control corruption origin is still unresolved. Prioritize
return-path/instruction-state tracing of the first swap-in and the
following swap-out; do not increase process or descriptor limits.
No production code or permanent memory change.

## 2026-10-25 v284 unexpected UUO simulator breakpoint diagnostic

The unchanged v283 16-child guest was rerun under a new testkit
SIMH script with a memory-insert breakpoint and an additional PC=000001
breakpoint intended to examine `proc_current_slot`,
`proc_sched_cursor`, `mm_extent_count`, and selected ACs. The
same abnormal terminal output recurred:
`UUO 000000777777 340000000003` followed by
`HALT instruction, PC: 000001`. The new PC=1 breakpoint did **not**
fire, so no process/AC snapshot was captured. The fault remains
unresolved. In particular, PC 000001 in the terminal line must not
be interpreted as a successfully executed instruction at that PC.
No production change or swap high-water acceptance follows from v284.
Further debugging requires an emulator-side trap/event trace or an
earlier breakpoint on the actual UUO entry path, rather than relying
on post-fault PC=1 execution.

### Post-INIT stress escalation v282/v283: unexpected UUO unresolved

v282 (20 pressure children) fails creation at the 17th requested
child and exceeds simulator time. v283 (16 children) instead reaches
an unexpected UUO and halts at PC 000001 before normal guest markers.
SIMH descriptor insert observations for both runs were below 36/136;
neither run passed. A native trap/control-flow investigation now takes
precedence over increasing process counts or descriptor capacity.
No production changes or KCORE size growth.

### Post-INIT twelve-child swap/high-water acceptance v281

The 256K native PDP-6 test launches twelve 24K-word pressure children
after enabling swap and verifies a blocked writer resumes with intact
pipe data. One complete cycle PASS; 71 MM insert-entry samples give
peak descriptor occupancy 31/136. A three-cycle variant exceeded its
300-second simulator timeout during the second cycle and was not
counted as PASS. Full 63-slot concurrent swap remains unverified.
No KCORE source or size change.

## 2026-10-25 256K preload-pressure admission probe v280

Test-only PDP-6 GCC user image enlarged each guest with a private
`010000`-octal (4096-word) BSS array, checked at its first and last
word. The original 63-process preload and a reduced 48-process preload
both built valid boot images but halted without any guest start marker.
Consequently the test never reached runtime swap/reclaim; no high-water
value or allocator defect is established. This is a failed diagnostic,
not a passing or high-water acceptance fixture. The v265 63-process
small-image concurrency PASS and v278/v279 independently exercised
runtime swap remain the supported results.

The next high-concurrency swap fixture should create or enlarge user
memory after INIT has activated swap, not preload 48-63 large images
before the boot handoff. All simulator core settings remain in 16K-word
increments, the DAIMOS process policy in 8K increments.
No production changes or KCORE size increase.

### Three-cycle 256K swap descriptor high-water v279

Native PDP-6 SIMH `mm_extent_insert` breakpoints sampled 101
pre-insertion descriptor counts across three passing pressure
cycles. Maximum resulting descriptor occupancy was 27/136;
109 entries remained unused in that particular test. This is
distinct from the 63-process boot peak of 132/136; combined
63-process swapping has not been measured. No KCORE change.

### Repeated native swap/reclaim and process reuse v278

The 256K PDP-6 GCC full-profile test completed three successive
pressure rounds (blocked writer, eight pressure sleepers, swap
recovery, pipe data checks and child reaping). Testkit v278 PASS.
No permanent KCORE change. Descriptor high-water tracing during
these cycles and longer repeated evictions remain open; 32K GCC
lowmem disk-root is still the size measurement baseline.

### Correct AUXSTORE/root boot ordering and live swap recovery v277

`auxstore_post_minits()` now publishes dedicated backing-store mapping
after `kinit_boot()` mounts D6FS. Previously root mounting could
overwrite the shared direct-tail mapping, leaving backing allocation
capacity inconsistent with the active I/O span. With the corrected
order the 256K PDP-6 eight-child pressure fixture completes, including
blocked-writer restoration and pipe data checks. The standard 32K GCC
lowmem disk-root size run reports unchanged KCORE 10742 and
KCORE+MRES 13605 words; PERMANENT_LAST is 13652.

## 2026-10-25 live backing-tail measurements v276

Corrected test-only SIMH breakpoint examination at `backstore_write`
entry on the 256K PDP-6 pressure failure reads these **octal**
values: `backstore_blocks=04000` (2048),
`backstore_blocks_used=0320` (208), and
`blockset_direct_tail=02000` (1024). The allocator's advertised
backing extent exceeds the active direct-tail span. A first
experimental reading before boot incorrectly showed zero and was
replaced by readings within the execution breakpoint. The guest
still fails at child seven (`p7!W`) after swap-out rollback.
This is a verified configuration inconsistency; the test did not
capture the failing write's exact logical range or return code,
so the inconsistency is not yet proved to cause this particular
failure. Next trace configuration ownership and the write return
status; do not change production memory limits as a workaround.
32K GCC lowmem disk-root stays the KCORE size reference.

## 2026-10-25 backing-store rollback trace v275

On the fresh 256K PDP-6 eighth-pressure-child reproducer,
SIMH execution breakpoints recorded one call each to
`backstore_write`, `mm_unpin`, and `backstore_free`, with no
call to `proc_storage_publish_v30`. The guest still emitted
`<wp7!W`. This matches the `proc_swap_out_unpin` rollback
branch following a failed `backstore_write`, but the return
register after the write was not sampled, so the precise
failure remains to be proved. The absence of storage-state
publication confirms that this attempt did not complete the
normal swap-out commit sequence. The fixture remains a failing
diagnostic, not a swap acceptance test. Next inspect the native
backstore write return code and the blockset/direct-tail I/O
mapping under the tested 256K image. No production code or 32K
KCORE size-reference changes.

## 2026-10-25 256K pressure admission native traces v273/v274

SIMH breakpoints on the unchanged native PDP-6 guest captured one
`proc_swap_reclaim` entry, one `proc_swap_victim` entry, one
`proc_swap_out` entry, one `backstore_alloc` entry and one
`backstore_write` entry before the `p7` admission failure.
The deeper trace also observed 11 storage-state checks and
21 MM-free entries over the boot. Counts are entry observations,
not successful return results. In particular, whether the backing
write or subsequent transition fails remains unresolved.
The v272 eighth-child reproducer still fails; no new production
code or claimed swap acceptance. Continue at backing-store
return/rollback analysis, maintaining the 32K GCC KCORE baseline.

### Live 256K swap pressure: eighth child fails admission v272

The new test-only PDP-6 fixture holds a pipe writer blocked,
then starts sleeping 24K-word pressure children. Seven children
enter PROC_SLEEP; spawning the eighth reports failure (`<wp7!W`).
This reproduces pressure on the process creation path but does
not yet establish why admission failed or exercise successful
swap-out/resume. No KCORE modification or size change.

### 256K test BSS admission and SIMH granularity v271

SIMH physical-core selections use 16K-word increments; the
DAIMOS boot-slot and MM capacity formulas still use complete
8K-word banks. The 32K GCC lowmem disk-root profile remains
the KCORE size reference. The DXR loader rejects BSS values
above `077777` octal (32767 words). A 56K-word test array
therefore prevented INIT startup; a 24K-word test array
successfully reaches INIT on a fresh 256K image, then hits
the expected insufficient-swap-pressure assertion. This is
an admission/negative-control test, not swap acceptance.

## 2026-10-25 256K large-image swap-pressure probe v270

A disposable test-only derivative of the original 32K swap-stress
fixture enlarged each process's pressure array from 8K words to
56K words and built a fresh 256K full-profile GCC PDP-6 image.
The image linked and reached device initialization but halted
before the INIT guest's first `<` marker. The test thus yielded
**no swap-path result**. Simply enlarging each guest is not a
validated way to create pressure; startup / VM image sizing must
be examined before retrying. The failed experimental source was
not installed as an acceptance test or merged into production.
The correct next test should employ more moderate guest images,
additional sleeping pressure processes, verified physical process
residency, and explicit swapped-process resume checks. Existing
v267/v268 saturation and high-water results remain valid.
No production source or 32K KCORE-size reference changed.

## 2026-10-25 swap-pressure acceptance investigation v269

The older daimos-testkit daimos-swap-stress-v1 workload assumes
32K physical memory and forces a blocked pipe writer to swap
using two pressure children. Naively running that identical guest
at 256K does NOT reproduce the needed pressure: the guest emits
`<ws!W`, with `s` identifying failure of the explicit target-swapped
assertion in `run_writer_case`. This is not evidence of a broken
swap implementation: pressure requirements changed. Independently,
rerunning the original 32K profile fails at image construction:
`mkimage: KINIT image leaves less than the configured stack reserve`.
No 32K guest swap result can therefore be claimed from v269.

Next: construct a dedicated 256K test with enough concurrently held
process images to force real reclaim, explicit procinfo-backed
nonresidency checks, and resume/integrity verification. Preserve
32K GCC lowmem disk-root as the KCORE *size* reference even while
its runtime image does not fit. No production code changed in v269.

### Native descriptor saturation and reclaim v268 (2026-10-25)

The PDP-6 production mm_extent_insert/mm_delete test
fills the registry to 132 entries, adds four entries to
reach the 136-entry ceiling, verifies rejection of the
137th allocation without count mutation, and tests a middle
deletion followed by successful reinsertion at full capacity.
Testkit v268 PASS. This verifies the registry boundary and
slot reuse, not physical-memory pressure or process swap.
No KCORE source change; the 32K GCC lowmem disk-root size
baseline remains in force.

### 256K 63-process MM insertion high-water v267 (2026-10-25)

Fresh-image 256K GCC PDP-6 SIMH successfully completed all 63
process starts/finishes while an execution breakpoint sampled
`mm_extent_count` at each `mm_extent_insert` entry. There were
151 observations and a maximum **pre-insertion count of 131**
decimal. The PDP-6 production MM assembler increments this
count only once, at successful insertion. Therefore occupancy
cannot exceed 132 from these observed insertion entries. The
independent v266 point sample directly observed 132 allocated
descriptors, establishing the **132-descriptor peak** for this
specific first-boot workload (capacity 136, spare 4). The
breakpoint-action mechanism adds no persistent KCORE storage.
This does not prove adequacy for concurrent new allocations,
process replacement, repeated reuse or swapping/reclaim under
pressure. Do not change the eight-entry safety allowance before
testing those operations. No production code changed.

### MM native insertion trace v267 (2026-10-25)

A fresh 256K GCC PDP-6 first-boot workload exercising all
63 numbered process slots PASSed with a SIMH execution
breakpoint at `mm_extent_insert`. Breakpoint actions read
`mm_extent_count` at entry and resume. The test captured
151 pre-insert observations, peaking at 131 existing extents
(at most 132 immediately after that insertion); independent
v266 point sampling confirms 132. Capacity was 136 and no
descriptor-pressure failure was reported. This is a workload
trace, not a universal guarantee or validated swap/reclaim
peak; breakpoint actions do not establish whether each
insertion succeeded. No permanent kernel instrumentation
or new KCORE RAM. Size reference remains the 32K GCC lowmem
disk-root profile.

### 256K MM descriptor point observations v266 (2026-10-25)

The fresh-image 63-process 256K GCC PDP-6 SIMH test sampled MM
metadata at the last process's start and completion markers,
using test-only SIMH EXPECT and EXAMINE (no resident instrumentation).
The raw transcript shows `mm_extent_capacity=0210` octal
(136 decimal), `mm_extent_count=0204` octal (132) at
`[3F]`, and `mm_extent_count=0010` octal (8) at
`{3F}`. All 63 guest start/completion tokens were present.
There were only **four spare descriptors** at the last-start
sample; the eight-descriptor non-process/transient allowance
has not been demonstrated sufficient for further overlap.
These are instantaneous point observations, not a measured
peak, and do not prove descriptor safety during concurrent
allocation or swap/reclaim transitions. The simulator places
EXAMINE text on the same line as prior guest console markers;
the v266 fixture therefore extracts embedded address/value
pairs rather than anchoring them to the beginning of lines.
Production KCORE remains unchanged; use the established
32K-word GCC lowmem disk-root configuration for its size.

### Full 256K numbered process range v265 (2026-10-25)

The fresh-image 256K GCC PDP-6 concurrency fixture PASSed
both 48 and 63 guest process start/completion sets, including
guest private-state checks. Hexadecimal slot markers avoid
the earlier single-character marker collision. Slot zero is
reserved, so 63 is the largest numbered process count in the
64-entry table. This does not establish peak MM extent count
or sustained swap/descriptor reuse. KCORE code and the
required 32K-word measurement reference are unchanged.

### PDP-6 thirty-two-process 256K acceptance v264 (2026-10-24)

Fresh 256K full-profile GCC PDP-6 SIMH boot completed 32
process start and completion markers with private-state checks.
An initial false positive was an acceptance-script collision
between slot 24's valid 'X' completion and an error marker;
using '@' for errors resolved the fixture, and the clean
rerun PASSed. The test does not measure MM high-water,
simultaneous residency or swapping. Production KCORE was
unchanged; the 32K GCC lowmem disk-root reference remains.

### PDP-6 sixteen-process 256K acceptance v263 (2026-10-24)

First-boot 256K GCC full-profile native SIMH PASSed 16 process
start markers and 16 completion markers with per-process private
state checks. Test lives in daimos-testkit. The 64-slot policy
includes reserved slot zero, hence at most 63 numbered slots.
No MM peak occupancy was measured, and no KCORE change occurred.
KCORE size reference remains the 32K GCC lowmem disk-root build.

### 256K first-boot scheduler acceptance v262 (2026-10-24)

The separate daimos-testkit `daimos-mm-256k-scheduler-v262.sh`
builds a new GCC full-profile 256K PDP-6 image with five boot
users. Native SIMH PASSed preemption, process context, nice ABI
and disk sleep/wakeup with boot-selected 136-descriptor backing.
This checks real boot/scheduler functionality, but does not
establish 64-process stress or a peak extent occupancy value.
Production source remains unchanged. Continue using 32K-word
lowmem disk-root builds for KCORE size measurements.

### KINIT RAM-sized MM extent capacity v261 (2026-10-24)

Extended the v260 external backing to a boot-selected capacity:
`mm_extent_capacity = 2 * proc_slots_for_core(core_words) + 8`.
This yields 24 descriptors at 32K, 40 at 64K, 72 at 128K,
and 136 at 256K; capacity grows by four for each complete
8K bank. The fixed address of `mm_extents` and the direct
PDP-6 MM/VM memory operands remain unchanged. KINIT reserves
exactly two words per selected descriptor before publishing
its first free arena, and MRES starts afterward.
Native PDP-6 production-assembler tests at 24 and 136 extents
PASS, along with v249/v250 32-entry boundaries, the 32K
layout test and the **fresh-image five-process scheduler**
(preemption, context, nice ABI and disk sleep/wakeup). GCC
32K-profile KCORE is 024766 octal (10742 linked words), but
its 24-entry external registry occupies another 48 words:
actual fixed KCORE-plus-registry is 10790 words, 15 fewer
than the old 10805-word fixed-KCORE baseline. Likewise,
the linked KCORE+MRES report 13605 excludes these 48 words;
combined physical footprint is 13653 versus old 13668.
This is not a guarantee of 64 fully resident processes:
physical memory, pinned allocations and pressure still limit
simultaneous residency. High-load 256K boot acceptance is
still outstanding; only the maximum-capacity native allocator
boundary has been exercised at 136 extents.

### KINIT external MM backing restoration v260 (2026-10-24)

Restored the historical direct-address table boundary design as
an **intermediate 32-descriptor stage**: `mm_extents` aliases
`__kcore_low_end` without linked BSS storage. KINIT reserves and
zeroes the following 64 words before releasing any low arena;
MRES starts after that reserved metadata. The native PDP-6
insertion guard now reads `mm_extent_capacity`, currently reset
to 32 in `mm_boot_init` until dynamic higher capacities are
boot-validated. Native extent v249/v250 insertion/deletion tests
PASS; the 32K GCC disk-root image links and layout test v260
PASS. This change does **not** yet permit more than 32 extents.
32K-profile linked KCORE decreases from 10805 to 10742 words,
but the external 64-word reservation remains resident; with
one added capacity word the actual KCORE-plus-table footprint
is 10806 rather than 10805 words. Do not count moving the
table past the KCORE label as physical-RAM savings. Full
boot acceptance and memory-tier capacity selection remain open.

### KCORE measurement configuration v259 (2026-10-24)

For every KCORE size comparison, use a **32K-word** GCC
low-memory disk-root build, explicitly setting
`BOOT_PROFILE=lowmem ROOT=disk COMPILER=gcc
BOOT_CORE_WORDS=0100000 SIMH_CPU_KWORDS=32`.
The physical 32K boot may remain unsupported; linking and
measuring this profile is nonetheless the mandatory optimization
reference. Report KCORE, KCORE+MRES and PERMANENT_LAST separately.

### Process-slot 8K policy v258 (2026-10-24)

The boot process-slot count now grows from eight at 32K by two
for each complete extra 8K bank up to 64 slots at 256K.
The min-slot contract used by packed/storage metadata is eight.
Testkit checks 8K boundaries and incomplete banks; full GCC
lowmem disk link keeps KCORE+MRES at 032544 octal (13668).
This is process-slot sizing, not restoration of runtime MM
extent capacity; that remains a separate boot-layout task.

## 2026-10-23 MEMFS XIP alignment v203

Large dedicated MEMFS allocations now use `mm_alloc_aligned` with `02000`
alignment. Small packed chunks keep their existing path. Focused native
MEMFS pinning tests PASS on SIMH and AAP. EXEC/RUN remains unchanged.

## 2026-10-23 MEMFS XIP reservation foundation v202

MEMFS MRES has a four-word per-file XIP reference table and reuses the MM
extent pin count. The privileged reservation API currently admits only an
existing dedicated large MEMFS chunk with a `02000`-aligned physical base;
regular DXR files are not launched from this storage. Write, truncate,
reclaim and unmount paths now respect active reservations. No EXEC/VM
ownership changes have been made. The GCC lowmem image links, and focused
SIMH MEMFS and FIFO native target tests pass. AAP also passes the same
MEMFS pinning target fixture; actual AAP XIP process launch is pending.

# KCORE - Verified Open Work and Optimization Backlog

## 2026-10-23 native uncompressed EXEC physical OOM v205

daimos-testkit v205 PASSes uncompressed executable replacement on a
256K PDP-6 where old+new VM extents exceed physical memory. The
replacement executes as PID 1 by the v201-corrected low-memory EXEC
path. A false-negative DXR metadata assertion was fixed in the
testkit. No new KCORE text or static data; fresh link endpoint
`025657` octal. 32K acceptance remains deferred.

## 2026-10-23 resident EXEC -2 comparison correction v201

The EXEC staged-loader out-of-memory return must be tested with a
full 36-bit compare (`CAME 1,[-2]`), not `CAIE` with an 18-bit immediate.
The latter bypassed the already implemented low-memory replacement
retry. Native PDP-6 v201 pressured and unpressured compressed EXEC
replacement PASS after the fix; the v200 eight-case staged transaction
test also PASSes. No persistent RAM growth; the fresh full-profile
`__kcore_low_end=025657` octal. 32K acceptance remains deferred and
the full-profile permanent-memory limit remains open.

## 2026-10-23 EXEC late transaction boundary tests v200

daimos-testkit v200 executes the production `exec_replace_current`
assembler under PDP-6 instruction-level fault injection. Eight modes
PASS: load, startup and old-VM destruction failure; PURE backing-bit
commit variants; RT newly-claimed rollback, old-owner preservation and
non-RT successful release. Restored user mapping hold, swap backing
and prior credential mount bit are also checked. External call stubs
are injected, so these are not native MM allocation/destroy failures;
native full-image replacement/rollback tests are tracked separately.
No KCORE growth (`__kcore_low_end=025657`). Successful RT-to-RT EXEC
and direct EXEC ENOMEM under physical pressure remain to be verified.

## 2026-10-23 RUN loader VM creation resource exhaustion v199

Native 256K PDP-6 testkit v199 **PASS**: a separately installed valid
large child DXR RUNs before and after pressure, but four identical RUN
requests fail while the parent's live, unswappable VM consumes enough
physical core that `vm_space_create` cannot allocate another child.
Each rejection preserves caller break/data, live process slots and
process-word accounting. Shrink restores the old baseline, and v197
late descriptor-map rollback/reap/recovery tests also pass. Source
change is test-only: `__kcore_low_end=025657` octal, no added stack or
persistent RAM. EXEC replacement failure under deliberate pressure
remains unverified; 32K acceptance remains explicitly deferred.

## 2026-10-23 256K physical MM exhaustion regression v198

New daimos-testkit v198 native fixture PASSes four unallocatable but
well-formed `BRK(0776000)` growth requests with three live children.
After each failed VM/MM growth attempt, caller break, VM word count,
sentinel contents and process slot count remain intact. The combined
fixture subsequently passes v197's late RUN failure rollback and
healthy child recovery. Kernel unchanged: `__kcore_low_end=025657`
octal. Direct RUN/EXEC loader allocation failure injection and
fine-grained MM descriptor leakage tests remain unverified. The 32K
gate is deferred, not passed.

## 2026-10-23 RUN late descriptor-map rollback regression v197

Native 256K PDP-6 RUN V2 fixture PASSes sixteen late-stage failures
after child VM, startup and u-area setup, with three other children
live. Process slots and reported process words remain stable across
each failed RUN; after releasing/reaping the children, both return to
their baseline, and a subsequent valid RUN/WAIT succeeds. This is
not forced MM allocation exhaustion and does not individually audit
orphaned dynamic MM allocations. No kernel text or persistent data
changed; linked `__kcore_low_end` remains `025657` octal. The 32K
gate remains deferred by request.

## 2026-10-23 RUN native lifecycle/ID reuse pass v196

Existing 256K native PDP-6 process-lifecycle and process-ID-lifetime
testkit targets both PASS: RUN/WAIT/reap/orphan/descriptor cases and
creator-slot reuse protection for pgrp, session and domain IDs.
No resident code change; `__kcore_low_end=025657` octal. Continue
non-32K EXEC/RUN validation; the 32K gate is explicitly deferred.

## 2026-10-23 EXEC/RUN 32K acceptance deferred v196

Per current project direction, stop investigating 32K-only configuration,
KINIT packaging and oversized DSH EXEC errors for now. Keep the 8K/32K
targets documented as deferred, not passed; do not reduce memory reserves
or weaken size gates. Prioritize 256K native EXEC/RUN correctness,
rollback and process lifecycle tests, while preserving the resident RAM
footprint. The most recent 256K linked KCORE end remains `025657`
octal. The independent full-profile permanent-size overage remains open.

## 2026-10-23 native RT_REQUIRED RUN child follow-up v194

The existing compressed RT_REQUIRED child admission/rejection/release
native PDP-6 test passed after v193 post-VM EXEC rollback coverage.
This independent RUN path check introduced no KCORE modifications.
The linked `__kcore_low_end` remains `025657` octal. Functional-only
image-size settings are not strict permanent-memory acceptance.

## 2026-10-23 EXEC compressed decode failure RT cleanup v193

Native PDP-6 testkit v193 PASSes new RT owner cleanup and existing
owner preservation when an RT_REQUIRED compressed DXR2 has valid
header geometry but a truncated D6LZ control/literal stream. The
loader's normal path allocates the staged VM before invoking the
decoder; failure then destroys that staged VM and rolls back only a
new reservation. The earlier header-failure and successful EXEC
scenarios also PASS. No new kernel code or resident memory; current
post-p10fold KCORE end `025657` octal. Production timing, executable
image cap and 16K residency are still unaccepted.

## 2026-10-23 EXEC RT correction cross-regression v192

After correcting the live RT-claim marker/VFS stat overlap, the
versioned native fixture with 28 malformed EXEC/RUN blocks still PASSes
and the normal EXEC replacement completes successfully. Its full log
is `$TMPDIR/daimos-exec-v192-malformed-retest.log`. The v191 RT owner
transaction test independently PASSes. KCC/p10fold linked KCORE and
full permanent-memory measurements are unchanged; strict production
timing and memory acceptance are still open.

## 2026-10-23 EXEC loader RT claim scratch lifetime correction v191

The seven-word VFS stat output at `-011..-003(17)` was overwriting the
RT-claim marker previously stored at `-003(17)`. An existing RT owner
could therefore be released by a failed EXEC. Reuse `-012(17)` only
after its vnode output has been copied to AC12, initialize it before
the VFS stat call, and bypass marker-based cleanup on earlier failures.
The v191 native RT-owner transaction regression reproduces the prior
failure and PASSes after correction (new-claim rollback, old-owner
preservation, admission and normal-EXEC release). Both KCORE text/end
and transient stack usage are unchanged; `__kcore_low_end=025657`
octal after KCC/p10fold. Full memory still exceeds the strict limit.

## 2026-10-23 EXEC/RUN malformed launch regression v190

The PDP-6 native EXEC replacement fixture in daimos-testkit v190
now also PASSes 28 rejected malformed EXEC/RUN launch requests before
completing normal replacement. Invalid headers, count upper halves,
oversized counts, bad SIXBIT length fields and out-of-block records
were rejected without destroying the caller. Functional-only image
size and 35-second observation settings were retained. RT-owner
rollback, strict 15-second/`036000` gates and the full permanent
memory deficit remain open. This test-only checkpoint does not change
the `025657`-octal post-p10fold KCORE exclusive endpoint.

## 2026-10-23 EXEC/RUN native functional retest v189

Native PDP-6 replacement and child-session EXEC tests PASS on v188
(`6b9ce2a`), covering normal image replacement and child-process reap.
This follows the v188 linked `025657`-octal KCORE measurement.
The 35-second test observation and `037000` functional image cap do
not satisfy the strict timing/size gates. Malformed launch-block and
RT reservation failure cases remain open; this documentation-only
checkpoint does not alter the resident size.

## 2026-10-23 EXEC strict record start and low-memory RT cleanup v188

In `exec_replace_current` the argv/env record scanner tests that AC15
points strictly before the validated block end with `CAML/JRST`,
replacing the unused `MOVE/SUB/JUMPLE` difference. A nonzero record
count still requires a valid first word before calling the record
parser; record-end validation is unchanged. In low-memory EXEC the old
RT reservation is released only if AC14 equals `proc_rt_owner`, using
`CAMN/SETZM` directly instead of loading the owner in AC1 first.
The same-checkout KCC/p10fold exclusive KCORE endpoint changed from
`025661` to `025657` octal, two resident words saved. Native runtime
and malformed-block testing on v188 are still pending.

Full KCC `make permanent-size` accounting on the same profile reports:
KCORE 11135 words (`025577`), KCORE+MRES 18670 words (`044356`),
PERMANENT_LAST `044435`, MULTI_LAST `044476`. The strict `040000`
permanent-address limit still fails by 2333 addresses (single root)
or 2366 (multi root), as shown by the report's distance metric.
The KCORE exclusive-end address is not interchangeable with the
report's standalone KCORE size; keep both measurements distinct.

## 2026-10-23 EXEC/RUN native regression follow-up v187

Fresh-image PDP-6 simulator diagnostics for v186 PASS both EXEC current
image replacement and session-leader child EXEC/parent reap. The v185
EXEC replacement diagnostic also PASSed. Observation window was
explicitly extended to 35 seconds, with the functional-only `037000`
DSH image cap; original production timing and executable-size gates
remain open. Targeted RT-owner rollback and malformed launch-block
coverage also remain open. No new code or resident-word change here;
the current exclusive KCORE end remains `025661` octal.

## 2026-10-23 EXEC real-time owner cleanup v186

In the successful old-real-time reservation release and the failure
rollback, compare the saved process slot directly against
`proc_rt_owner` using `CAMN` and conditionally execute `SETZM`.
This replaces each `MOVE/CAME/JRST/SETZM` sequence with two words,
without clearing a reservation transferred to another owner. The
same-checkout post-p10fold KCORE exclusive end changed from `025665`
to `025661` octal (four resident words), without additional RAM or
stack. The v185 native EXEC replacement diagnostic PASSed using the
existing 35-second functional-only observation window; v186 native
runtime regression remains pending at this size measurement.

## 2026-10-23 EXEC direct validated scalar counts v185

`exec_replace_current` now loads the argc and envc launch words
directly into AC13 and AC14 for their existing upper-half-zero and
maximum-count checks. The formerly separate `HRRZ` copies are
redundant: both values are already validated pure 18-bit counts before
their use in the packed count pair or record total. Same-checkout
KCC/p10fold relink reduced exclusive `__kcore_low_end` from `025667`
to `025665` octal (two permanent words), with no additional RAM or
stack requirement. Native EXEC/RUN regression tests for v184-v185
remain pending at the time of this size measurement.

## 2026-10-23 EXEC old real-time ownership comparison v184

`exec_replace_current` now uses `CAMN 13,proc_rt_owner` followed by
`MOVEI 14,1` to set its existing old-RT flag only when the current
slot owns the real-time reservation. This avoids a temporary AC1 load
and a branch, preserving the original owner-equality behavior and
unchanged rollback logic. A same-checkout KCC/p10fold link moved
`__kcore_low_end` from `025671` to `025667` octal (two resident words).
No additional RAM or stack; v184 native EXEC regression pending.

## 2026-10-23 EXEC/RUN runtime verification v183

Native PDP-6 fresh-image EXEC replacement and child-session EXEC both
PASS after v181, using daimos-testkit's opt-in 35-second observation
window and functional-only `DSH_IMAGE_LIMIT=037000`. ABI inspection
PASSes using the live assembler rather than the retired C fallback.
Original 15-second timing acceptance and `036000` production executable
size remain outstanding, as do malformed-input and strict low-memory
regressions. No further code or resident-word change in this test pass.

## 2026-10-23 EXEC in-place path advancement v181

The first path record parse in `exec_replace_current` now advances AC15
directly to the first argv/environment record with `ADD 15,1` instead of
constructing the address in AC2 and copying it back. The existing
`CAMLE` end-of-block check still validates the computed address before
use. In a same-checkout KCC/p10fold KCORE relink, exclusive
`__kcore_low_end` moved from `025673` to `025671` octal, saving two
resident words. No extra stack or persistent RAM; syscall ABI unchanged.
Native EXEC/RUN launch and malformed-input regressions remain pending.

## 2026-10-23 EXEC argument-record end comparison v180

`exec_replace_current` now checks each argument/environment record end
directly (`CAMLE/JRST`) against the validated EXEC block endpoint rather
than subtracting the two addresses and testing the sign. This completes
the interrupted v180 edit without altering error handling or record
semantics. The same-checkout KCC/p10fold link reduced the exclusive
`__kcore_low_end` from `025674` to `025673` octal: one resident word.
Native EXEC/RUN malformed-input and process-launch tests are pending.

## 2026-10-22 EXEC path record endpoint check v179

`exec_replace_current` now checks the parsed path-record end directly
against the validated launch-block end (`CAMLE/JRST`) instead of using
`MOVE/SUB/JUMPL`. The `sixbit_record_words` parse and its nonzero check
remain in place. The same-checkout KCC/p10fold KCORE relink reduced the
exclusive `__kcore_low_end` from `025675` to `025674` octal, saving one
resident word without changing the syscall ABI, stack, or persistent state.
Native malformed-record and EXEC/RUN regression tests remain pending.

## 2026-10-22 EXEC launch size comparison v178

In `exec_replace_current`, replaced `MOVE/SUB/JUMPL` for the declared
launch-block length against available mapped words with the equivalent
`CAMLE/JRST` bound. The earlier minimum-length validations are preserved;
declared words greater than mapped words still fail before record parsing.
Same-checkout KCC/p10fold KCORE link reduced `__kcore_low_end` from
`025676` to `025675` octal, saving one resident word. A separate attempt
to change the RUN slot increment was reverted because it saved no
post-fold words. Native EXEC/RUN malformed-input testing remains pending.

## 2026-10-22 RUN record count combination v177

Following the existing validation of the RUN V2 argc and envc count
fields as pure 18-bit values, the record scanner now sums the two source
words directly (`MOVE` and `ADD`) instead of extracting both RH fields
into separate registers. This retains the exact nonnegative count bounds
and record-scanning semantics, and reduces the post-p10fold linked KCORE
exclusive end from `025677` to `025676` octal (one resident word).
KCC/p10fold link succeeded; native malformed-argument and startup tests
remain pending. No additional RAM or stack requirement.

## 2026-10-22 RUN initial path bound reduction v176

RUN V2 verifies `RUN_MIN_WORDS=010` before computing the path position
at `RUN_FIXED_WORDS=6`; consequently the path record starts strictly
before the validated block end. Removed the redundant `CAML/JRST`
pair in `proc_run_block`, retaining the `sixbit_record_words` validation
and the subsequent record-end bounds check. Same-checkout KCC/p10fold
link reduced exclusive `__kcore_low_end` from `025701` to `025677`
octal (two resident words). No additional stack or permanent storage.
Native malformed RUN block tests remain outstanding.

## 2026-10-22 RUN scalar count checks v175

RUN now checks the three scalar count fields (fd-map, argc, envc) with
`TLNE` on the full loaded value instead of using `HRRZ` followed by
`CAME`.  The left-half nonzero rejection and existing maximum-count
comparisons preserve the validation ABI. A KCC/p10fold relink in the
same checkout reduced `__kcore_low_end` from `025704` to `025701`
octal (three resident words). No extra permanent or stack state.
Native launch and malformed-block regression tests remain pending.

## 2026-10-22 RUN loader error cleanup v174

`proc_run_block` now routes all negative `exec_load_process` return values
directly to the existing claimed-slot discard path. The former comparison
against `EXEC_LOAD_NOMEM` and `MOVEI AC2,011` did not affect that cleanup
or its return value. A same-checkout KCC/p10fold relink moves the exclusive
KCORE endpoint from `025711` to `025704` octal: five resident words saved.
No permanent data or ABI changes; native process-launch regression remains
pending. A separate experimental merge of EXEC loader error rollback paths
linked successfully but yielded zero post-fold savings and was reverted.

## 2026-10-22 RUN validated pointer reuse v173

RUN previously called `sixbit_record_words` again after successful block
validation, to reconstruct the argv/environment record address for
`vm_space_startup`. `proc_run.s` now saves that address in one additional
temporary stack word before scanning the records and reuses it after
child image loading. This removes redundant parsing and its failure branch.
Same-checkout KCC/p10fold relink changed the `__kcore_low_end` exclusive
address from `025715` to `025711` octal (four resident words saved).
Peak RUN kernel stack increases by one word; permanent tables, EXEC/RUN
ABI and process descriptor size are unchanged. Native RUN/EXEC regression
and full image boot acceptance remain pending.

## 2026-10-22 EXEC validation pointer reuse v172

`exec_replace_current` in `proc/exec_lowmem_pdp6.s` now retains the
already-validated argv/environment record pointer in AC14 across its
record scan, rather than calling `sixbit_record_words` again on the path.
AC14 is available after argc/envc are merged and is repurposed for the
old RT-owner flag only after the scan. The next linked EXEC symbol moved
from `000430` to `000425` (octal), corresponding to five fewer words
in the leading EXEC section. The current linked `__kcore_low_end` is
`025715` octal; it is not compared against a freshly rebuilt matching
baseline, and no whole-image net saving or runtime acceptance is claimed.
Keep EXEC/RUN permanently resident; continue with RUN input-validation
and error-cleanup audit after native tests.

## 2026-10-22 additional unused C source sweep v171

The production MEMFS MRES builds `memfs_data_pdp6.s` and
`memfs_snapshot_pdp6.s` directly. Neither portable C source was
compiled by the active rule. Both superseded C files and their stale
source dependencies were removed; the assembly sources remain unchanged
except for a comment describing the replacement. Kernel `kcore` source
generation and boot integration dry run passed. This eliminates obsolete
source, with **no demonstrated resident KCORE or MRES word saving**.

Retain `d6fs_provider.c`: external daimos-testkit validation references
it even though the production D6FS provider uses handwritten assembly.
Also retain the live C portions of the mixed `proc`, `file`, module-runtime,
and swap implementations. No native machine execution is claimed.

## 2026-10-22 redundant C implementation sweep v170

The PDP-6 build now constructs EXEC exclusively from
`proc/exec_lowmem_pdp6.s` and `proc/exec_load.s`; the entirely disabled
`proc/exec.c` fallback was removed.  Likewise `proc/vm_pdp6.c` was removed:
all six functions were already compiled out by `VM_SPACE_*_PDP6_ASM`
defines, and the live routines remain in `proc/vm_pdp6.s`.  The PDP-6
`vm_backend.s` target is now an empty `.text` section; other backend
selections retain their C compilation rule.  These changes eliminate dead
source and compiler work, NOT linked KCORE instructions.  Do not count
the source-line reduction as resident RAM savings.

The same-basename kernel C/assembly inventory identified `vm_pdp6` as
the only exact pair; a build-rule examination additionally found EXEC's
differently named assembly replacement.  Other mixed units must remain:
`proc.c` and `file.c` still contribute active C functions after their
assembly-specific feature guards, and `module_runtime.c` contributes
descriptor/binding storage absent from its assembly implementation.
No runtime acceptance or new size measurement is claimed for this sweep.

## 2026-10-15 GCC 32K disk-root measurement and MM recovery v138

Isolated baseline at commit `37fba7e`, with `BOOT_PROFILE=lowmem`,
`COMPILER=gcc`, `ROOT=disk`, `BOOT=dsk`, `BOOT_CORE_WORDS=0100000`,
`SIMH_CPU_KWORDS=32`, and unchanged `02000` KINIT stack reserve:
complete image build PASS; KCORE exclusive endpoint `023503` octal
(10051 decimal); KINIT image end `072203` octal. The MM object is
520 text + 70 BSS words. The baseline is above the strict 8K target
by 1860 decimal words (exclusive endpoint must be below 020000).

Current committed MM assembler had lost optimizations from v094, v095,
v096, v106, v109, v110 and v111 compared with commit `9d52bb0`.
Restoring that exact v111 assembly in isolation builds a complete GCC
32K image: KCORE endpoint `023435` octal (10013 decimal), KINIT end
`072135` octal, MM object 482 text + 70 BSS words. Net whole-image
improvement is 38 decimal words after folding/layout;
the MM text contribution also falls by 38 words. The optimized image still
requires 1822 more words for a strictly below-8K exclusive endpoint.

This is a GCC link-and-package measurement, NOT native runtime
acceptance. `daimos-testkit` host `mm-core` cannot currently link due to
missing `proc_storage_publish_v30` and
`proc_swap_storage_invariant_v30`; fix the test fixture and rerun native
PDP-6 MM allocation, freeing, relocation and compaction regressions
before calling the recovered code runtime accepted. The older v136
external-descriptor branch additionally changes the insertion capacity
guard; it was not imported into the v138 image.

Date: 2026-10-24 (source reconciliation at v219, a405005)
Basis: checked DAIMOS source, PROJECT-STATE.MD, TODO.MD, and committed
native-test evidence. This is a documentation audit, not a new full link or
native simulator acceptance run. A staged, host-tested implementation is not
completed until linked, measured and accepted on the relevant PDP-6.

### Verified implementation baseline (v219)

- `proc/proc.h` defines three-word descriptors and 24/40/64/88/128/192/256
  boot-selected slot tiers; `proc/proc_boot.c` provides tier selection.
- `mm/mm.h` and `mm/mm_pdp6.s` still define 32 two-word MM extents.
  `ipc/pipe.h` still defines an eight-word FIFO buffer.
- `proc/exec_lowmem_pdp6.s` and `proc/proc_run.s` are the resident EXEC/RUN
  paths. Production `vm_space_startup` in `proc/vm_pdp6.s` writes user VM
  metadata even with zero arguments; it is not XIP-safe.
- The production VM destruction path in `proc/vm_pdp6.s` invokes the MEMFS
  provider's XIP release operation (026), with normal MM owner/type fallback.
  Private MEMFS operation 027 supplies restricted raw PURE XIP admission.
  Ordinary EXEC/RUN does not invoke that admission operation.
- The restricted linker XIP producer is experimental, with a limited
  instruction format. v215-v219 native fixtures validate selected admission,
  release, rollback and non-writing user-mode paths, **not** fully scheduled
  production EXEC/RUN, process exit, or swap interaction.
- The most recent documented **GCC lowmem** linked KCORE exclusive endpoint
  is `025134` octal (10844 decimal words, v213). Reaching a *strictly below*
  `020000` octal endpoint requires at least 2653 fewer words. The distinct
  EXEC/RUN **full-profile** checkpoint is `025657` octal (11183 words,
  v205); profiles must not be mixed. Neither is a newly measured v219 link.
- v190-v205 recorded native malformed EXEC/RUN, staged transaction, RUN
  failure rollback, and compressed/uncompressed low-memory EXEC regressions.
  This is positive targeted evidence, not 32K boot acceptance.
- The 32K strict gate is explicitly deferred by v196; do not report it as
  passing or relax it. The v138 GCC lowmem result (`023435` octal) and the
  older v41/v42 (`021541`) are historical, not current size baselines.

## P0: 32K resident limit and correctness

- [ ] When the deferred strict 32K work resumes, shrink the GCC lowmem KCORE
  exclusive endpoint below `020000` octal. The latest documented comparable
  lowmem link is `025134` (v213); first repeat a fresh v219 full link and
  full accounting, not just an object-level estimate.
- [ ] Measure full permanent KCORE plus committed MRES, irreducible scratch,
  KINIT allocations and process startup headroom; retain the separate general
  below-040000 permanent-address limit where applicable.
- [ ] [Deferred at v196] Pass the strict GCC 32K size gate and native 32K
  boot/INIT/LOGIN/DSH, process-pressure and filesystem tests with the same
  fresh image. Do not conflate 256K EXEC/RUN passes with this acceptance.
- [ ] Fix compiler, assembler and kernel correctness bugs at their source;
  add regression tests in daimos-testkit before marking the sweep complete.
- [ ] Audit source-level size hotspots, generated objects and real post-p10fold
  link maps; reject optimizations that increase the 32K resident footprint.

## P1: packed-process lifecycle and compression

The v33-v45 packed registry/layout and transaction sources are staged
experiments, not active production packed processes; retain ordinary swap.

- [ ] Choose a compact dense or bounded sparse packed registry using measured
  whole-KCORE code + dynamic data + MM extent cost, not isolated v33-v45
  prototype sizes; preserve MM-authoritative extent ownership.
- [ ] Connect KINIT registry allocation, boot abort, process exit, VM destroy,
  packed publication and teardown; validate exclusion and failure rollback.
- [ ] Complete packed restore against the selected registry, rather than its
  private descriptor, then integrate packed eviction/restoration with the
  scheduler; do not remove disk swap before full native acceptance.
- [ ] Integrate the production D6 codec with the resident KCORE encoder and
  streaming decoder, exact `000060` resumable ABI, bounded VFS refill,
  KPACK and EXEC/VFS/PURE consumers. Keep the detailed cross-subsystem plan
  in system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md.
- [ ] Measure combined resident codec text, stack, workspace and peak MM
  descriptors; test direct ABI, malformed input, resumptions, packed
  transaction failures and real PDP-6 execution before activating it.

## P2: kernel data structures and workload-specific sizing

These are profiling/selection tasks, not claims that the existing MM,
scheduler, pipe or slot-tier implementations are absent.

- [ ] Evaluate runtime-sized MM extent descriptors versus the current static
  `MM_MAX_EXTENTS=32` two-word table (mm/mm.h, mm/mm.c, mm/mm_pdp6.s);
  measure nested MAKE/KCC/KCPP load and relocation-sensitive assembly cost.
- [ ] Compare sorted linear MM scans with optional lookup indexes only for
  measured large-core workloads; avoid permanent small-core dispatch cost.
- [ ] Profile D6FS/DTFS/TSFS cache footprint, replacement and DECtape seek
  cost under native compilation; evaluate boot-selected budgets based on
  actual free memory after essential allocations.
- [ ] Profile VFS directory lookup under native MAKE/KCC and test compact
  entry caching or optional indexes, reusing D6FS hashing if advantageous.
- [ ] Benchmark dense versus sparse packed layout capacity and lookup cost
  under 24/40/64+ process slots, keeping the inactive experiments isolated
  until an accepted resident implementation is ready.
- [ ] Profile intrusive scheduler queue scans at larger process counts before
  adding priority indexing or extra branches to the common context-switch path.
- [ ] Benchmark the existing fixed eight-word, word-only pipe FIFO ring
  (ipc/pipe.h and pipe.s); change capacity only with demonstrated RAM or
  throughput wins. Do not revive obsolete character-width pipe formats.
- [ ] Confirm boot-selected process slot tiers (proc/proc.h and proc_boot.c)
  against real maximum-concurrency tests; do not reimplement already working
  boot-time selection or add runtime resizing without evidence.
- [ ] Audit remaining permanently resident device/accounting state and shared
  filesystem scratch for outright deletion or genuine buffer reuse; avoid
  moving always-required storage to MM merely to disguise its cost.

## P3: folding and compiler optimization

Priority decision (2026-10-24): further one-JRST folding is a **last-effort**
optimization after source and architectural shrinking opportunities are
exhausted. Preserve existing p10fold behavior, but do not spend current
engineering time expanding cross-domain JRST folding. Reconsider only after
a fresh final linked census and the existing >20-net-word/cold-path gate.

- [ ] Rerun p10fold read-only cross-domain census after final handwritten
  KCORE objects and fold plans stabilize; use explicit `-a` anchors, `-x`
  fold-plan exclusions and `-C` verified cold donors.
- [ ] Implement DLINK external-anchor folding only if nonoverlapping net
  savings exceed approximately 20 resident words with no KCORE growth,
  target-side metadata or hot-path JRST regressions.
- [ ] After self-hosting, use p10super results to improve KCC generator
  canonicalization (common tails, branches, AC allocation and equivalent
  forms), not by editing generated assembly.
- [ ] Defer native streaming low-memory p10fold and larger semantic/SMT
  searches until self-hosting and simpler optimizations justify their cost.

## P1 follow-up: restricted MEMFS XIP production integration

- [x] Provide aligned dedicated MEMFS XIP storage, pin lifetime, restricted
  PURE admission (private provider op 027) and VM release (op 026); native
  SIMH/AAP fixtures validate the individual mechanisms (v203-v219).
- [x] Provide an experimental raw PURE XIP linker mode limited to a verified
  non-writing instruction subset (v218); ordinary DXR/PURE is not XIP.
- [ ] Design a privileged argument-free, environment-free EXEC/RUN admission
  path, preserving existing ABI and no additional permanent process fields.
- [ ] Make XIP startup genuinely non-writing: zero argc/envc alone is not
  sufficient because `vm_space_startup` unconditionally stores VM metadata.
- [ ] Native-test *production* scheduler entry, failed EXEC replacement and
  rollback, process exit, swap exclusion, storage-map state, and memory
  pressure on both simulators before claiming XIP enabled.

## 2026-10-24 architectural shrink roadmap and proposal A start v221

### Proposal D first-boot MM checkpoint fixture v257 (2026-10-24)

Added `tests/validation/daimos-mm-firstboot-v257.sh` to
`daimos-testkit` (commit `32dfb7e`). The fixture builds its
own disposable five-process GCC-profile PDP-6 boot image,
checks the existing scheduler preemption/context/nice and
disk sleep/wakeup assertions, and examines the mapped
`mm_extent_count` during its **first** SIMH boot.
The first acceptance run PASSed the scheduler but deleted its
transcript before the observation was extracted. A second
independent fresh-build run PASSed and emitted one explicit
checkpoint sample of `000000000006` (six decimal) at
octal address 024537. The fixture now derives this address
from the fresh link map and requires an observation.
The test demonstrates successful non-production observation
without replaying modified media. **One point sample is not
the high-water value**: no MM registry reduction or permanent
kernel instrumentation is justified. KCORE stays 025065
octal (10805 words).

### Proposal D simulator replay/root diagnostic correction v256 (2026-10-24)

Revisited the v255 simulator failure. Manual artifact discovery had
used lexicographic sort of `daimos-scheduler-multiproc-v1-<pid>`
paths, selecting an October 20 image rather than the October 24
fixture. Corrected the selection to modification-time ordering and
replayed the newest saved October 24 image (MM count at 024537,
insert entry at 001145 octal), preserving its 8M SIMH throttle.
The subsequent replay also exited with `?RT` before the insertion
breakpoint. Kernel `KINIT_ERR_RT` documentation establishes that
`?RT` means root mount/validation failure or absence of a valid
`/SYSTEM/INIT`; it is **not a scheduler failure code**. The official
single-use scheduler build/run still PASSes, so a repeat execution
of saved mutable media is not a valid workload measurement.
Fresh-build breakpoint telemetry must be installed before the
first boot (with reproducible disposable media), not introduced
into a previously executed image. No MM high-water result or
production change is claimed.

### Proposal D fresh-image insertion tracing attempt v255 (2026-10-24)

Reran the unmodified official five-user scheduler regression with
`MAKEFLAGS=COMPILER=gcc` and retained its isolated build; the
native SIMH acceptance **PASSed**. Its freshly linked `kcore.map`
identified `mm_extent_count` at 023543 and
`mm_extent_insert` at 001167 octal. A separate manual SIMH
replay of that same build, with a one-shot breakpoint on
`mm_extent_insert`, instead emitted `?RT` and never reached
the breakpoint. Therefore the replay environment is not yet
equivalent to the official test invocation. No occupancy or
high-water data were accepted from that replay and there were
no kernel or testkit source changes. Resolve the launch-path
discrepancy before using debugger-based telemetry; keep all
32 extents.

### Proposal D official scheduler rerun v254 (2026-10-24)

After DevSpace2 became reachable again, reran the unmodified
`daimos-testkit` target `test-daimos-scheduler-multiproc`
with `MAKEFLAGS=COMPILER=gcc`. Its full isolated PDP-6 build
and SIMH regression **PASSed**, including preemption, process
context, nice ABI and disk sleep/wakeup. This supersedes the
earlier v253 implication of a persistent scheduler failure
from the ad hoc replay, without establishing a specific cause
for that `?RT` output. It does not turn v252's three extent
snapshots into a high-water measurement: the 32-entry MM
capacity remains unchanged. No kernel or testkit changes.

### Proposal D unstepped scheduler cross-check v253 (2026-10-24)

Replayed the preserved GCC five-process image without SIMH
EXPECT stepping, using the fixture's 8M instruction/second
throttle. SIMH booted the filesystem providers and halted
normally, but the guest emitted `?RT` instead of the expected
multichild scheduler progress markers. Therefore the earlier
v252 scheduler completion failure cannot be attributed solely
to intrusive diagnostic stepping. The only confirmed extent
data remain three point samples of 020 octal (16) from v252;
there is **no validated workload high-water result**. Do not
downsize the 32-entry MM table on these observations. No
production or testkit source changes were accepted.

### Proposal D GCC-profile MM occupancy checkpoints v252 (2026-10-24)

The earlier `036013 > 036000` DSH gate failure was caused by
building the five-process diagnostic with the default KCC
configuration. The previously documented lowmem `COMPILER=gcc`
configuration builds past the unchanged gate and reaches SIMH.
An isolated scheduler fixture inspection of mapped symbol
`mm_extent_count` at octal address 024537 yielded 020 octal
(16 decimal) at each of three scripted checkpoints. However the
diagnostic-stepped scheduler run then reported that timer
preemption did not complete all boot users; this is **not a
passing scheduler regression or an extent high-water measurement**.
No persistent instrumentation, production code or MM allocation
changes were made; the experimental fixture was removed.
Retain the 32-descriptor limit until representative full
workloads are measured with nonintrusive peak tracking.

### Proposal D real workload MM occupancy attempt v251 (2026-10-24)

Attempted a non-invasive PDP-6 runtime snapshot of `mm_extent_count`
at scheduler checkpoints by adapting the existing five-process SIMH
fixture in a separately versioned testkit-only script. The build
failed before simulator execution at the existing strict DSH user
image size gate: `036013` octal exceeds `036000`.
Consequently **no runtime extent occupancy samples or high-water
measurements exist from this attempt**. The DSH acceptance limit
was not relaxed, the experimental test script was discarded,
and unrelated testkit working-tree modifications were preserved.
The tested production MM registry remains 32 entries (64 words),
with no justified shrink or new permanent RAM saving.

### Proposal D native MM sorted insertion verification v250 (2026-10-24)

Added a separately versioned `daimos-mm-extents-v250` test to
daimos-testkit, extending v249's full 32-entry boundary and -3
rejection coverage with deletion followed by insertion into the
middle of the live descriptor array. The fixture checks both the
inserted two-word slot and the displaced old slot. It PASSes
using production PDP-6 `mm_extent_insert`/`mm_delete` leaves,
testkit commit `580e978`. This is deterministic correctness
coverage, not workload high-water telemetry; the 32-entry MM
limit stays unchanged and KCORE remains 025065 octal.

### Proposal D native extent registry boundary fixture v249 (2026-10-24)

Added a native PDP-6 fixture to daimos-testkit:
`tests/validation/daimos-mm-extents-v249.s` and
`daimos-mm-extents-v249.sh`. It extracts the production
`mm_delete` and `mm_extent_insert` leaves, fills all 040 octal
(32) slots, confirms that insertion 33 returns -3, deletes one
slot and successfully refills. It PASSes with p10run/PDP-6;
testkit commit `11dc124`. This tests the boundary, not real
workload high-water, fragmentation or VM compaction. No DAIMOS
production allocation or code change is justified. Preserve the
025065-octal (10805-word) KCORE baseline.

### Proposal D host MM fixture dependency investigation v248 (2026-10-24)

The `mm-core` failure described in v247 is not only an obsolete
Makefile path. A temporary testkit-only build experiment omitted
the missing `vm_pdp6.c` dependency. The host linker then
reported three unresolved VM functions: `vm_extent_move`,
`vm_space_can_swap`, and `vm_space_destroy`. These production
functions belong to the PDP-6 VM implementation, including
`vm_pdp6.s`, and host C tests cannot link the assembly directly.
The fixture asserts actual movement, pinning, live mapping and
swap-destruction behaviors; no dummy symbol implementations
would be valid acceptance evidence. The testkit Makefile was
restored byte-for-byte from a temporary backup, preserving
all unrelated pre-existing testkit edits. A proper test repair
requires an equivalent host implementation with verified semantics
or a PDP-6-native MM fixture. No production code or extent
registry limit changed; baseline remains 025065 octal KCORE.

### Proposal D extent registry acceptance gate v247 (2026-10-24)

The PDP-6 MM implementation has `MM_MAX_EXTENTS=040` (32)
two-word descriptors, occupying 0100 octal (64) resident words.
`mm_extent_insert` explicitly returns MM_ERR_DESCRIPTORS (-3)
when full; shrinking the registry would change real failure behavior.
No representative runtime high-water readings are available here,
so do not reduce this count merely to reclaim static words.

The daimos-testkit `mm-core` target currently fails to compile
before test execution: its Makefile references removed/missing
`system/kernel/proc/vm_pdp6.c`; the checkout contains
`vm_pdp6.s` and `vm_pdp6_swap.c` instead. The testkit checkout
also contains unrelated pre-existing working-tree changes; these
were preserved. Repairing the fixture requires identifying the
correct host-side VM implementation/link requirements, not simply
replacing the path with assembly. No source changes or new
permanent-memory savings were accepted.

### Proposal F static allocator and VM lifetime inventory v246 (2026-10-24)

Following the committed v245 filesystem scratch audit, inspected
resident BSS declarations in MM, MEMFS, IPC, PROC and VM. MM's
`mm_extents` is 0100 octal words (64 decimal) for its live
extent descriptors, not a disposable scratch buffer. MEMFS
`memfs_data_chunks` and `memfs_xip_refs` track allocator
and XIP ownership; the separate VM APR word and IPC/PROC
singletons hold live state. None can be aliased safely to the
128-word filesystem transfer workspace without first proving
non-overlapping use and ownership. No production change is
proposed; retain the v245 GCC lowmem KCORE baseline 025065
octal (10805 decimal). Measure peak extent occupancy before
any MM descriptor count reduction.

### Proposal F filesystem workspace ownership check v245 (2026-10-24)

Inspected resident filesystem static allocations and provider
serialization for possible buffer lifetime overlap. The only
128-word resident VFS transfer block is already `fs_block_workspace`.
DTFS, TSFS and D6FS share it, and `fs_provider_serialized` holds
the provider lock over potentially sleeping physical I/O. Arguments
are held on private process stacks during lock waits. MEMFS bypasses
that serialization because it does not sleep or use this block.
Reusing the transfer block for unprotected path/lookup scratch
would permit an unrelated process to corrupt an in-flight request.
The remaining assembly storage declarations inspected in this
area are small state variables or MEMFS-owned allocations rather
than obvious duplicate permanent transfer blocks. No safe
ownership-preserving static overlay was identified, and no source
change or new saving is claimed. Retain v244 KCORE baseline 025065
octal (10805 words); revisit only with a documented lifetime proof.

### Proposal B file-path request-frame inspection v244 (2026-10-24)

Audited the resident `file_symlink`, `file_rename`, `file_chdir`,
`file_make_node`, `file_remove`, `file_truncate` and
`file_stat_path` paths together with `vfs_create_common` and
`vfs_create_op`. MKDIR/MKFIFO already share their parent/access
frame, UNLINK/RMDIR share a typed removal frame, and the one-argument
TRUNCATE/STAT wrappers already share stack cleanup. The CREATE
provider request and returned-vnode publication are already common
for the relevant operations. A new cross-operation frame would have
to preserve distinct permissions, argument lifetimes and stack
offsets, with no identified positive linked-size candidate. No
production source changes were made.

`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS with GCC. Retain v243 KCORE baseline 025065 octal (10805
decimal). Continue looking for a concrete full-image reduction,
without touching deferred JRST folding or 32K acceptance.

### Proposal B descriptor and GETCWD clear follow-up v243 (2026-10-24)

After v242, the descriptor duplication paths both use bounded BLT
and the mount-extension replacement operation is shared. Examined
the variable-length output zeroing in `file_getpath_common` and
its `file_getcwd_clear` loop. A prospective BLT-based fill needs
additional pointer, destination and end-address preparation and
would add overlap/lifetime complexity. The exploratory source edit
was reverted immediately without accepting or benchmarking it.
No production code change and no new RAM reduction was claimed.
Do not force an additional dispatcher or BLT on this short loop
without a positive complete-link result and path regression.

### Proposal B DUP2 descriptor pair copy v242 (2026-10-24)

`file_dup2` now uses the same bounded two-word BLT as `file_dup`
to copy the packed vnode/metadata and offset. Its subsequent pipe
reference operation obtains the packed metadata from the completed
destination descriptor, rather than relying on the AC3 scratch
value from the former scalar-copy sequence. This retains the
descriptor-copy, mount-extension, pipe reference and old==new
semantics without extra resident state or stack.
Full linked GCC lowmem disk-root KCORE 025066 -> 025065 octal
(10806 -> 10805 decimal), KCORE+MRES 13669 -> 13668,
PERMANENT_LAST 13716 -> 13715. The existing native PDP-6
`daimos-file-mountext-v19` fixture PASSes 26-mount vnode, lock,
DUP/DUP2, close and unmount; file and file-walk tests both PASS.
Strict 32K acceptance and JRST folding remain deferred.

### Proposal B contiguous FILE descriptor duplication v241 (2026-10-24)

`file_dup` copies the packed descriptor node and offset from two
adjacent words. One bounded two-word BLT replaces four MOVE/MOVEM
instructions with a three-instruction pointer setup and BLT, without
changing destination extension replacement, lock-family semantics,
or pipe reference handling. No resident data or stack increase.
GCC lowmem disk-root full linked KCORE 025067 -> 025066 octal
(10807 -> 10806 decimal), KCORE+MRES 13670 -> 13669,
PERMANENT_LAST 13717 -> 13716. The existing daimos-testkit
`daimos-file-mountext-v19` PDP-6 test PASSes its 26-mount vnode,
lock, DUP/DUP2, close and unmount assertions; target-file-pdp6 and
target-file-walk-pdp6 PASS. No JRST folding or 32K acceptance work.

### Proposal B file mount-extension bitmap update v240 (2026-10-24)

`file_mount_ext_store` and `file_mount_ext_copy` previously duplicated
the final two-bit descriptor mount-extension clear/set sequence. Both
paths now enter `file_mount_ext_update` with the destination bit offset
in AC7 and already-positioned replacement bits in AC6. Clearing
outdated destination bits precedes ORing the replacement, as before.
The canonical vnode, descriptor layout, AC1/AC2 preservation and
zero-to-clear behavior remain unchanged. This adds no resident
state or stack frame.

Full linked GCC lowmem disk-root KCORE: 025075 -> 025067 octal
(10813 -> 10807 decimal). KCORE+MRES 13676 -> 13670;
PERMANENT_LAST 13723 -> 13717. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Specific full native descriptor mount
extension lifecycle verification remains open; the size result
does not imply that broader acceptance.

### Proposal B provider boundary and leaf-loop audit v239 (2026-10-24)

The v238 follow-up reviewed `vfs_lookup_provider`,
`vfs_parent_return`, `vfs_create_op`, `vfs_symlink` and MonitorFS
`mfsleaf_lookup`/`mfsleaf_readdir`. CREATE/MKDIR already share the
request formatter and SYMLINK already shares vnode publication.
Lookup must additionally resolve mounted targets, unlike PARENT.
MonitorFS leaf lookup must retain the requested count across the
entire loop; AC5 is reused for packed-name comparison, so naively
hoisting the length load would be incorrect. That source experiment
was immediately reverted without a misleading size claim.
No ABI-safe larger common routine was identified, and no production
code or tests were changed. Preserve the 025075-octal KCORE baseline;
further investigation should target a concrete duplicated data
lifetime or full provider request frame, not an extra dispatcher.

### Proposal B VFS mutation epilogue experiments v238 (2026-10-24)

From the clean v237 checkout (KCORE 025075 octal, 10813 decimal),
two cold error-path sharing candidates were compiled and measured
against the complete GCC lowmem disk-root image. Redirecting
`vfs_setattr` errors through the existing three-word unwind
`vfs_mutate3_ro` was size-neutral at 025075, so it was reverted.
Redirecting `vfs_symlink` and `vfs_rename` four-word failure
unwinds through `vfs_write_words_ro` **increased** the full linked
KCORE to 025077 (10815 decimal), KCORE+MRES to 13678 and
PERMANENT_LAST to 13725. It added an extra cold-path branch and was
also reverted. The linker/folder already removes or shares more than
the source-level instruction count suggests. No production changes
were accepted and no native runtime acceptance is claimed for these
discarded experiments. Continue proposal B with larger structural
opportunities rather than tiny branch/epilogue rewrites.

### Proposal B MonitorFS namespace BLT copies v235 (2026-10-24)

Two fixed three-word name-record copies in resident VFS, one in
`vfs_readdir_table` and one in `monitorfs_name_record`, now use
bounded PDP-10 BLT transfers instead of three MOVE/MOVEM pairs.
The source/destination record layout and length are unchanged.
Full GCC lowmem disk-root image: KCORE 025103 -> 025075 octal
(10819 -> 10813 decimal); KCORE+MRES 13682 -> 13676 and
PERMANENT_LAST 13729 -> 13723. No new static data or stack.
Both target-file-pdp6 and target-file-walk-pdp6 PASS on PDP-6.
The preceding one-branch provider fallthrough experiment was
size-neutral after linking and reverted. Further native directory
enumeration and getcwd acceptance remains useful. The native
test-daimos-monitorfs-devices-online on SIMH also PASSes with
COMPILER=gcc and SDL_VIDEODRIVER=dummy.

### Proposal B bulk device argument consolidation v233 (2026-10-24)

The resident file-runtime bulk READ/WRITE dispatch previously reloaded
the buffer pointer and word count separately in ten individual device
leaves (six WRITE, four READ). Both dispatchers now save their canonical
vnode in AC0 and establish the device-call arguments once before selecting
the device. The ordinary VFS provider fallback restores the vnode and
original word offset before calling VFS, preserving descriptor advancement,
append handling, provider ABI and the mapped-buffer contract. No data,
extra stack slots, or provider dispatch layers are added.

Full GCC lowmem disk-root linked measurement: KCORE 025115 -> 025103
(10829 -> 10819 decimal), KCORE+MRES 13692 -> 13682 and PERMANENT_LAST
13739 -> 13729: ten permanent words saved. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Native PTR/PTP test compilation completed but
SIMH terminated with status 139 after SDL2 window-event warnings. This
is **not** a native device I/O acceptance pass; separate device validation
remains open. A separate native card WORDTOKEN12/CR/CP round-trip test
PASSed with SDL_VIDEODRIVER=dummy. No changes to JRST folding or the
deferred 32K gate.

### Proposal B CHDIR stat reuse v232 (2026-10-24)

`file_chdir` already obtains a seven-word `vfs_stat` frame to check that
the target is a directory. Its subsequent `file_check_access` call repeated
`vfs_stat` on the same vnode. Use `file_access_stat` directly on that
validated frame for the same search-permission mask (1). This removes a
redundant provider STAT invocation from successful CHDIR, with no ABI,
permission-policy or persistent-state change. GCC lowmem disk-root full
link PASS: KCORE `025115` (10829 decimal), KCORE+MRES=13692; size-neutral.
`daimos-testkit target-file-walk-pdp6` PASSes on PDP-6 including CHDIR.
This is a runtime-path simplification rather than a KCORE shrink. Further
proposal B work must find actual text/data reduction.

### Proposal B MonitorFS namespace/classifier audit v231 (2026-10-24)

Reviewed MonitorFS device lookup, directory enumeration and STAT. They
already share `mfsdev_name_length`, `mfsdev_validate_id`, leaf table
lookup/readdir and (since v230) `mfsdev_io_type`; lookup and enumeration
retain distinct return layouts and device-name matching requirements.
Tested making character devices the default in `mfsdev_io_type` and
checking mount/block exceptions first. The lowmem full link saved only
**one word** (`025114`, 10828 decimal), while adding instructions to the
ordinary character-device path. Rejected the tradeoff and reverted it;
restored full link PASSes at `025115` (10829 decimal), KCORE+MRES=13692.
No production source change was retained. Next proposal B work should
concentrate on larger file/VFS semantics or duplicated provider operations,
not device-classification instruction rearrangements. Dedicated native
STAT/READDIR type-matrix coverage is still desirable.

### Proposal B MonitorFS shared device classification v230 (2026-10-24)

`mfsdev_stat` independently duplicated the device-type classification
already implemented by `mfsdev_io_type` for READDIR. It now passes the
device ID in AC0, calls that existing classifier, and uses returned AC7
as the STAT type. No new data or provider dispatch is introduced. GCC
lowmem disk-root permanent-size link PASS: KCORE `025115` octal (10829
decimal), KCORE+MRES=13692, PERMANENT_LAST=13739 decimal, **10 words**
below the v229 accepted baseline. Native MonitorFS device tests remain
required before calling the change runtime accepted. The daimos-testkit
`test-daimos-monitorfs-devices-online` native SIMH test subsequently
**PASSed** using the supported `COMPILER=gcc` configuration and the unchanged
DSH executable size gate. This confirms the online device view; a separate
STAT/READDIR mode/type matrix remains useful to directly exercise all
device-classification categories.

### Proposal A process-table scan census and next boundary v229 (2026-10-24)

Current `proc_pdp6.s` has distinct hand-written scans with different
contracts: `proc_has_live_user` checks only an active state, whereas
`proc_tty_session_has` excludes a slot and conditionally checks pgrp,
u-area presence and session. `proc_trim_high` traverses backward only over
FREE trailing descriptors; `proc_runq_add/remove` operate the intrusive
runnable queue. Sharing these through a general scan helper would add
calling convention, predicates and/or persistent state to short fast paths;
there is no measured benefit. **Do not implement a generic iterator.**
The existing boot-selected process capacity and three-word descriptors
remain the baseline; no new process metadata is proposed.

After the failed common teardown prototype (v228), proposal A has no
currently justified large source-level rewrite. Treat A's first pass as
measured but not exhausted forever; revisit only with a concrete duplicate
instruction block or demonstrable lifetime/state simplification. Turn to
proposal B (file/VFS/MonitorFS) next rather than spend time on one-word
instruction substitutions. No source change or additional saving at v229.

Initial B measurement from the current available compiled lowmem objects
(section totals **before** final link/folding; not additive potential):
`file_runtime` text=1175, `monitorfs_runtime` text=1565 data=39,
`vfs` text=778 data=27 BSS=128 words. Prioritize the MonitorFS dispatch
surface and file permissions/lookup interactions, but preserve provider
semantics and measure the post-fold whole-image result before editing.

### Proposal A teardown sharing investigation v225 (2026-10-24)

### Proposal A discard assembler prototype v226 (2026-10-24)

### Proposal A supported GCC native rollback verification v227 (2026-10-24)

### Proposal A shared post-VM cleanup experiment v228 (2026-10-24)

Tested one common `proc_cleanup_after_vm` routine to serve both normal EXIT
and failed RUN slot discard after successful `vm_space_destroy`. It shared
TTY session release, u-area release and runnable-queue removal while keeping
distinct VM transitions and process publication. The GCC lowmem disk-root
link **grew** from v226/v227 KCORE `025127` (10839 decimal) to `025136`
(10846 decimal), an increase of seven resident words; KCORE+MRES likewise
grew from 13702 to 13709 words. The additional calls and register saves
outweighed eliminated local operations. **Rejected and reverted**: no
production code change retained and no native test claimed for this
discarded experiment. Do not attempt the same wrapper without a new
measurable reason to expect savings. Further A candidates must eliminate
actual work or data, not merely factor assembly source.

The production DSH size gate of `036000` octal must remain unchanged.
Investigation found the default KCC userland fixture generated DSH `036013`,
but DAIMOS already documents `COMPILER=gcc` as the supported smaller-DSH
policy for low-memory operation (`design/dsh-compiler-size-comparison-20261012-v20.md`).
Without bypassing the gate, reran daimos-testkit's
`test-daimos-run-late-rollback` with inherited `MAKEFLAGS=COMPILER=gcc`.
Native target **PASS**: 16 post-VM RUN descriptor-map failures under three
live children, stable process accounting and successful recovery. This
validates an important failure path of the v226 assembler discard replacement,
not every termination, signal, hierarchy or RT-ownership case. The separate
full process-lifecycle regression is tracked independently.

The independent `test-daimos-process-lifecycle` was also rerun with the
supported GCC configuration and **PASSed** native SIMH execution: RUN,
GETPID, WAIT/nohang, zombie reap, orphan adoption and explicit descriptor
maps. Neither test required a DSH image size override. This lifts the
previous general lifecycle acceptance blocker for these covered paths,
but does not prove RT-owner failure injection or all VM teardown faults.

Replaced the production KCC-generated `proc_slot_discard` with a dedicated
PDP-6 assembler implementation, guarded by `PROC_SLOT_DISCARD_PDP6_ASM` in
the C fallback build. It preserves separate discard semantics: FREE early
return, conditional RT owner release, transition setup and failure rollback,
VM destruction, TTY/u-area release, run-queue removal, descriptor zeroing
and trailing slot trim. This is NOT yet a shared teardown middle with EXIT.
The lowmem disk-root permanent-size build PASSes: KCORE=`025127` octal
(10839 decimal), KCORE+MRES=13702, PERMANENT_LAST=13749 decimal. Net
post-link saving versus v223: **two words**. No persistent data added.

The targeted native RUN late-rollback fixture was attempted but stopped
before executing: DSH `036013` exceeded the unchanged production `036000`
EXEC image gate. Thus the prototype is linked and size-measured, but native
RUN rollback and resource ownership remain unaccepted. Fix the independent
DSH image-size problem, or use a genuinely independent targeted fixture,
before claiming native completion. Do not bypass the gate.

Source comparison identifies a real, but not yet safely merged, code overlap:
`proc_slot_discard` in `proc/proc.c` is a production KCC-compiled resident
function (`build/system/boot/pdp6/kcore-asm/proc.s`), whereas
`proc_finish_slot` is handwritten in `proc/proc_pdp6.s`. Both release RT
ownership, set process TRANSITION, call `vm_space_destroy`, release session
TTY ownership and u-area, remove the runnable slot and trim process slots.

The generated `proc_slot_discard` is about 50 assembly instructions in the
inspected build, including save/restore and local labels. This is *gross*
code to investigate, not 50 words of recoverable savings. Its implementation
differs from finish-slot materially: discard checks FREE, does not perform
session-leader HUP teardown or adopt children, and frees the slot instead
of publishing ZOMB. Preserve VM-failure transition clearing and the exact
MM/u-area release order. `proc_exit_finish` is a separate small generated
wrapper; fatal-event termination can enter `proc_finish_slot` directly.

Candidate A architecture: a single carefully delimited teardown *middle*
operating on a validated slot and explicit caller contract, with distinct
normal-exit and failed-creation publication tails. First measure whether
replacing the generated discard path with an ABI-compatible assembler
implementation can reduce the full post-fold KCORE without an extra hot-path
dispatch or more permanent state. Do not merge paths by inferring lifetime
semantics merely from similar instructions. Native lifecycle/RT/failed-RUN
injection acceptance is a prerequisite. No source change or saving is
claimed from this inventory.

### Proposal A lifecycle/RT audit v224 (2026-10-24)

### Proposal A cleanup ownership and rollback v225 (2026-10-24)

Inspected three distinct cleanup entries: `proc_finish_slot` (normal/fatal
termination), `proc_slot_discard` (failed creation) and
`proc_event_fatal_other` (closes the target's file table before finishing).
Although all involve VM/u-area/runq release, their ordering and ownership
requirements differ. No shared lifecycle state machine or MRES dispatch
has been introduced; a new wrapper is not justified without actual net
post-fold saving and proof of failure behavior.

Existing daimos-testkit host `proc-abort-transition-v40` PASSes against
current `proc.c`, including held TRANSITION during VM destroy, failed
destroy rollback, successful retry and slot reuse. The first invocation
could not find `pdp10-sixbit.h`; a rerun with `CPATH` pointing to the
existing DAIMOS `tools/host` headers PASSed. No test source changes.
This is focused C host coverage, not native PDP-6 assembler acceptance.
The DSH image gate continues to block the full lifecycle simulator fixture.


The RT owner release at the beginning of `proc_finish_slot` is intentional
in the existing C fallback (`proc.c`) as well as PDP-6 assembler. The same
early release is used by `proc_slot_discard` before VM destruction. This
ordering cannot be unilaterally changed in assembler as a size optimization:
failure-path policy must be investigated in both paths and with the event
fatal-process caller. No RT rollback bug is established by code inspection
alone, and no behavior change was made.

The v223 runtime verification attempt using daimos-testkit
`test-daimos-process-lifecycle` failed at image construction, before any
native process assertions: DSH `036013` exceeds the unchanged `036000`
EXEC gate. This repeats the independent v222 fixture blocker; it is not
a pass or evidence of a new process-management failure. A dedicated
failure-injection test of RT ownership during teardown remains open.

Further A work should target duplicated resource acquisition/release
mechanics across process exit, failed slot creation and event termination,
while preserving their distinct ownership and commit points. Do not add a
common service wrapper without proving lower post-fold memory consumption.

### Proposal A first source experiment v222 (2026-10-24)

### Proposal A process-finish RT ownership v223 (2026-10-24)

Replaced the four-instruction `MOVE/CAME/JRST/SETZM` real-time-owner
conditional in `proc_finish_slot` with `CAMN/SETZM`. This preserves the
ownership test and termination order without a new branch or storage.
The GCC lowmem disk-root `permanent-size` link passes with KCORE
`025131` octal (10841 decimal), KCORE+MRES=13704 and
PERMANENT_LAST=13751 decimal words; net reduction from v222 is two
resident words. A native process termination / RT-owner regression still
must be run; linker success is not runtime acceptance. Architectural
cleanup remains the priority over further micro-optimizations.


`proc_adopt_children_pdp6` now uses `CAIN` to branch directly for the
INIT-parent case, eliminating the former `CAIE/JRST/JRST` arrangement.
The production lowmem disk-root `make permanent-size` completed and reports
KCORE `025133` (10843 decimal words), one word below the previously
recorded `025134` baseline; KCORE+MRES is 13706 words in this build.
The lowmem map is profile-dependent, so do not compare it to the stale
full-profile map in another build directory.

Native process-hierarchy regression was attempted via daimos-testkit, but
its fixture build stopped at the *unchanged* strict DSH image size limit:
`036013` > `036000` octal. It did not execute the runtime assertion stage.
Do not call the behavior native-accepted; keep the production DSH size gate
unchanged. The next A step remains a measured call-site/lifetime audit of
process cleanup, signal/session and TTY responsibilities.

Measure total physical RAM (KCORE + MRES + dynamic allocations) first, and
the permanent KCORE endpoint separately. Execution cost is nearly as
important. None of these candidate savings is yet measured or additive.

1. **A: process-management decomposition.** Keep interrupt, scheduler,
   context-switch, runnable-queue, essential sleep/wakeup and exit-state
   paths in KCORE. Audit child hierarchy, terminal/session management,
   signal policy, wait/reap and resource teardown for duplicate cold logic.
   Eliminate code before considering MRES relocation; do not put another
   dispatch on the context-switch or tick path.
2. **B: file/VFS/MonitorFS consolidation.** Share repeated validation,
   exceptional dispatch and error handling where actual paths overlap;
   preserve existing provider ABI, direct fast paths and compatibility.
3. **C: EXEC/RUN consolidation.** Loader code is already shared. Fold only
   further proven duplicated validation and startup paths, while retaining
   distinct resident EXEC replacement and RUN publication transactions.
4. **D: MM architecture.** Preserve MM-authoritative ownership/pinning;
   profile real fragmentation and extent high-water before redesign. The
   current 32 two-word descriptors total only 64 words.
5. **E: packed inactive processes.** Long-term total-RAM candidate; assess
   registry, codec, stack and swap costs; retain disk swap until verified.
6. **F: scratch lifetime sharing.** Reuse nonoverlapping buffers only with
   proved reentrancy/ownership and a positive measured total-RAM benefit.
7. Revisit assembler/compiler cleanup and then additional JRST folding only
   after architecture opportunities are exhausted (subject to >20-word gate).

Proposal A initial source inventory (not an implementation):

- `system/kernel/proc/proc_pdp6.s` is one mixed assembly unit, with
  2164 text, 3 data and 3 BSS words in the available
  `build/system/boot/pdp6/proc_pdp6.dobj` (pre-fold, not a freshly
  rebuilt v221 object). Its size is NOT solely scheduler cost.
- Scheduler selection is near source lines 120-315, runnable queue near
  589-640, sleep/wakeup near 2345-2541 and PI/context-switch/timer work
  near 2597-2944. These are likely hot or latency-sensitive paths.
- Child adoption and finish/reap, u-area/session release and wait/status
  occur near lines 329-1024. Events/signal control and TTY/canonical
  input/output occupy substantial additional code in the same object.
- The current map exports `proc_finish_slot`, `proc_wait_status`,
  `proc_event_apply`, `proc_tty_canon_input`, `proc_sched_pi_resched` and
  `proc_swap_victim`; no cold-path move can yet be declared safe.
- Next: construct an entry/call-site graph including PI/indirect paths,
  then identify *one* duplicated internal routine or policy path to simplify.
  Compare full post-fold RAM and native scheduler/process/TTY tests; do not
  implement dispatch-only movement which merely hides memory in MRES.

## Acceptance and documentation

- [ ] For every candidate record post-p10fold KCORE text/data/BSS, total
  permanent/MRES RAM, boot dynamic words, MM descriptor high-water, peak
  process RAM, worst-case cost and actual maximum concurrency.
- [ ] Update PROJECT-STATE.MD and pdp10-doc's resident limits, GCC/KCC build
  policy and dated 32K acceptance evidence only after corresponding tests pass.

## Completed items removed from the active backlog

The current source already provides boot-selected process-slot tiers starting
at 24 (`proc/proc.h`, `proc_boot.c`), a compact three-word process descriptor,
an intrusive runnable queue, a word-only eight-word pipe, a p10fold build
path, and an explicit strict GCC 32K size gate. These are baselines to test
and optimize, not absent features to implement again. The v33-v45 packed
registry/transaction components are staged prototypes, not production KCORE.

### Proposal B native PTR/PTP acceptance v237 (2026-10-24)

Re-ran `daimos-testkit test-daimos-pt-wordio` using the supported GCC
userland configuration and `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
Native SIMH **PASS**: PTR/PTP WORDTOKEN8 full and partial physical
round trip. The earlier status-139 window-event failure is not reproduced
with the headless SDL configuration. This strengthens native acceptance
of the v233 bulk-device argument consolidation without changing any kernel
source, persistent data, or the 036000 DSH image limit. KCORE remains
025075 octal (10813 decimal), KCORE+MRES=13676, based on the prior
full lowmem link. Continue proposal B on file/VFS structural savings.
