## 2026-11-08 v1343: proven disposable provider-release dependency repaired

The v1342 256K PDP-6 source-linked live sample confirms genuine
requester OOM=1 and **12** real fatal provider-busy attempts, but
the test-only holder release and holder return were still zero.
The linked process table shows INIT in CHILD wait, requester in
timer SLEEP, gateway waiting for `fs_provider_wait_event_v917`,
and holder waiting for `daimos_test_hold_event_v1150`. The original
v1150 test-only release hook required a subsequent INIT sleep,
which could no longer occur after INIT entered `proc_wait_child`.
This proves a **disposable test-fixture scheduling dependency**;
it does not prove a production kernel MM/PI6/provider defect.

Evidence (12 live samples, interrupted deliberately after proof):
`$TMPDIR/daimos-swap-provider-dependency-v1342-evidence-r1.tar.gz`,
SHA256 `9a286c2a48e635d19411fab3fa2cbc453c99763561608200fcf704130bbb875b`.

Testkit v1343 `28bd504` now allows the independent resident spinner
to invoke the *same original* test-only guarded provider release
after actual OOM and at least two busy failures. The original
holder must still wake and call production `fs_provider_unlock`;
no direct unlock, synthetic MM result, process-state override or
new production resident code is introduced. Preflight and three
unsafe fixture mutation tests PASS. Full booted guest HALT,
18-child TERM/WAIT and final zero-backing acceptance still OPEN.
Docs: `pdp10-doc/KERNEL/SWAP-TEST-PROVIDER-RELEASE-V1343.MD`
(`d8cdb87`). No production KCORE/MM/FS/PI6/SWAP/MRES changes.

## 2026-11-08 v1342: post-release wait dependencies isolated

The v1340 booted 256K PDP-6 guest did not reach final HALT after
successful independent physical MM owner release and subsequent
swap-in progress (13 to 18). At the last of 24 source-linked
read-only SIMH samples, PI6 clock and timer frontier still advanced
but swap successes remained 34 out / 18 in. The retained genuine
guest output included `@%&*` after the v1340 Ctrl-E correction.

New testkit `daimos-swap-stall-waits-v1341.py` decodes the real
linked process descriptors and clock from that raw SIMH trace.
INIT slot 1 was waiting for a CHILD, requester slot 3 was in a
timed sleep, gateway slot 4 waited for `fs_provider_wait_event_v917`,
spinner slot 5 slept for 2 ticks, and holder slot 6 waited for the
**disposable-only** `daimos_test_hold_event_v1150`. The original
v1150 fixture signals that held provider only from an INIT sleep
after real OOM and at least two provider-busy failures. INIT was
instead in a child wait. This is a possible **test fixture circular
dependency**, not established production kernel deadlock.

Testkit `24dd9bd` stages a v1342 fresh booted diagnostic that
samples exact `real_oom`, `fatal_busy`, holder, release and provider
state from linked addresses; sampler/PTY/early-HALT selftests PASS.
The full guest must determine whether the original v1150 release
preconditions were satisfied before modifying any fixture. Docs:
`pdp10-doc/KERNEL/SWAP-POST-RELEASE-WAIT-DIAGNOSIS-V1342.MD`
(`9d96543`). Production KCORE/MM/FS/PI6/SWAP/MRES remain unchanged.

## 2026-11-07 v1340: independent release and post-release swap progress

The booted 256K KCC PDP-6 v1338 test reached real swap-in
MM_ERR_NOMEM/no-eligible-exchange-partner cooldown. A genuinely
scheduled spinner (slot 5) observed cooldown=1 via a read-only
diagnostic syscall and invoked exactly one original typed-owner
release. Production `mm_free` released all four 8K-word extents;
production `mm_unpin` released the real spinner VM pin. Source-
linked counters: queries=273, release calls=1, released=1,
prepared=4, freed=4, count=0, error=0, spinner pin base=0.
Immediately after release, successful swap-ins increased from
13 to 16 and then 18 (swap-outs 31 to 34), with no fatal swap-in
or exchange. This closes the previously missing **booted real-MM
independent release plus immediate forward-progress subcase**.

**Overall no-partner full-system liveness still OPEN.** The guest
then remained after the INIT `^` milestone without completing its
remaining process lifecycle. It was interrupted after 20 samples,
without final HALT or complete MM/backing cleanup. No KCORE bug
is established by this unresolved late stall.

The original v1338 read-only sampler dropped genuine guest `@%&`
bytes emitted after Ctrl-E but before SIMH's stop acknowledgment.
Testkit v1340 (`e9dc25c`) fixes this host diagnostic boundary,
retaining those bytes in the canonical guest stream while excluding
debugger prompts. The reproduction selftest, normal/early HALT,
linked process inspection and truncated-postflight controls PASS.
The next full guest observes the production `proc_timer_next`
frontier to distinguish a timer wake issue from another dependency.
Original source and traces, validated manifest:
`$TMPDIR/daimos-swap-independent-release-v1340-evidence-r1.tar.gz`,
SHA256 `cb5ebee862a0129e9955ef3beaba4635bca8f796aeda19166855b5241f63573e`.
Documentation `pdp10-doc/KERNEL/SWAP-INDEPENDENT-OWNER-RELEASE-V1340.MD`
(`311121c`). No production KCORE/MM/PI6/SWAP/MRES edits or RAM growth;
v1208/v1211 fallback remains rejected.

## 2026-11-07 v1336: real 256K no-partner pressure observed, recovery open

The retained, source-linked v1335 live 256K KCC PDP-6 guest actually
entered the previously unexercised swap-in MM resource-pressure branch.
With four genuine early `MM_TYPE_KERNEL_DYNAMIC` 8K-word owner extents
(32K total), the twelfth read-only SIMH sample contained 93 typed
`MM_ERR_NOMEM` failed swap-in allocations, 93 slot-0 no-partner
cooldowns, zero exchanges and fatal swap-ins, 54 normal swap-outs
and 37 normal swap-ins. Production MM, scheduler, swap service and
KCORE/MRES code were unchanged. The sample's process request was
26624 words, owner slot 5, with no matching free LOW/HIGH fit.

Independent testkit `c642d28` parses the raw linked SIMH counters
and rejects eight unsafe mutations. `swapout_fail` at this stage
also counts rejected reclamation attempts and is not automatically
a fatal published swap-out. The stricter final full-guest rollback
and leak requirements remain unchanged. Documentation:
`pdp10-doc/KERNEL/SWAP-NO-PARTNER-LIVE-PRESSURE-V1336.MD` (`d143f08`).

**NOT full acceptance:** after 17 additional children, INIT waited
on the next child RUN/ACK while retaining the 32K test reservation.
No independent owner release could occur. The run was interrupted
without HALT, restore-after-release or final MM/backing cleanup.
The separate testkit v1336 (`745690a`) stages a real, pinned,
independently scheduled spinner process to observe true cooldown
and release all four owner-tagged extents through production
`mm_free`/`mm_unpin`. Its guest preflight PASS, with full-system
recovery still awaiting a complete and strictly verified run.
No +16-word v1208/v1211 fallback has been merged.

## 2026-11-07 v1330: 1K late owner failure; native resident pins under test

The booted v1323 256K F07 guest completed all 18 additional
24K-image child admissions and 51 ordinary swap-outs / 35 swap-ins.
Even one test-only APR-02000-aligned 02000-octal-word (1024-word)
MM_TYPE_KERNEL_DYNAMIC owner extent was refused by the real
nonreclaiming MM allocator at the **late** owner-reservation stage.
INIT printed `!t` and HALTed. Its final counters: test owner
prepared=0, count=0, freed=0, error=-1; actual swap-in MM pressure,
exchange and no-partner cooldown all zero. Normal cleanup did NOT
execute; 3440 backing blocks remained at this early fatal test HALT.
This is a completed **negative setup** result, not production swap
liveness acceptance or evidence of a kernel defect. The original
source/map/logs and validated manifest are retained in
`$TMPDIR/daimos-swap-physical-owner-v1323-setup-failure-r1.tar.gz`,
SHA256 `04944fa347f6b259ca779ea96126ab440fa1eeeb9081794b56f0ed4d1bf3cb25`.

Late owner allocation cannot force real swap-in resource pressure
because physical MM is already too full for the test reservation.
Testkit `1a0693e` instead uses genuine production `mm_pin` and
`mm_unpin` on existing resident process extents, keeping the
actual requester excluded so its OOM lifecycle remains possible.
The v1329 source-authentic full-guest wrapper `5a0adb8` uses the
new v1326 SIMH prompt-safe sampler (`e436459`). Its 256K booted
guest had started and **no final no-partner outcome** was available
at this checkpoint. Preflight PASS is not full-system acceptance.

Authoritative documentation: `pdp10-doc/KERNEL/SWAP-PHYSICAL-MM-OWNER-ADMISSION-V1330.MD`
(`bbfeefd`). Production KCORE, MM, scheduler, swap MRES and permanent
RAM unchanged. The speculative v1208/v1211 fallback stays excluded.
Full-system physical no-partner liveness remains OPEN.

## 2026-11-07 v1325: 8K owner admission negative and 1K full guest

The completed v1322 disposable 256K PDP-6 guest reached all 18
additional real process admissions and performed 51 swap-outs and
35 swap-ins. Its real `mm_alloc_aligned_noreclaim` attempt to hold
one additional 020000-octal-word (8K-word) APR-aligned physical
test-owner extent returned failure. Original test INIT emitted `!t`
and HALTed. Authoritative postflight: owner error=-1, prepared=0,
count=0, freed=0; swap-in MM failure=0, cooldown=0, exchange=0.
This was a failed **test-owner admission**, NOT a production swap-in
failure or no-partner acceptance. The independent testkit v1325
postmortem confirms the exact counters, HALT and four negative
mutations; its versioned evidence archive checksum is
`388b09a28389749931eb8208ac008ff2f642fba753aeff933de44ad3ef56fd55`.

Testkit `c43fe7b` added v1323, one instead of 8K **1K-word**
APR-aligned, real owner-tagged physical extent under the same
24K-process F07 workload. Injector, strict owner accounting and
generated guest preflight PASS. Its first full guest was running
at this checkpoint; no final result or genuine no-partner pressure
claim is available yet.

The v1306 diagnostic sampler falsely rejected a normal guest HALT
before its configured maximum sample budget. New versioned v1324
(`7bc46c0`) retains original raw/guest transcript separation but
accepts fewer samples **only** when a real HALT and complete SIMH
postflight EXIT occur. Missing HALT, truncated postflight and failed
simulator still fail; both original and extended self-tests PASS.
The sampler cannot itself grant any functional guest acceptance.

Documentation: `pdp10-doc/KERNEL/SWAP-OWNER-ADMISSION-NEGATIVE-V1325.MD`.
Independent postmortem: testkit `e6e0d26`, docs
`pdp10-doc/KERNEL/SWAP-PHYSICAL-MM-OWNER-ADMISSION-V1325.MD`
(`2f990d6`), and second independently retained negative archive
`$TMPDIR/daimos-swap-owner8k-rejection-v1325-evidence-r1.tar.gz`,
SHA256 `bf5304eb2ce0de91d463b8137c601e9c77f4223bea7cf67ebb3a3e4582800e31`.
No production kernel code, RAM footprint, KCORE/MRES policy or swap
fallback was modified. Full-system physical no-partner liveness
remains OPEN.

## 2026-11-07 v1322: booted no-partner MM-owner stress in progress

Testkit `6dab196` introduced the distinct v1321 full-guest F07
combination: 18 additional 24K-word physical process images and
owner-tagged 8K-word non-process MM extents, allocated by the real
`mm_alloc_aligned_noreclaim` and released by the real `mm_free`.
Its original v1207-only guest policy stays untouched; the temporary
runner has its own explicit real-concurrent/swap/owner gate. The
v1321 first execution was deliberately interrupted after three
read-only SIMH samples (15 swap-outs, 8 swap-ins, zero MM failures)
and BEFORE the additional owner MM allocations. It is not a guest
PASS, no-partner test, or evidence of a kernel problem.

The preceding completed 24K-word full-guest v1308 physical MM
snapshot showed 15840 free managed words at BRK OOM. Six additional
8K-word extents would need 49152 words and likely fail during test
setup. Testkit `b961c8f` therefore adds versioned v1322 using ONE
8K-word owner-tagged extent instead, keeping the same real allocator,
original process lifecycle and strict full-guest MM/swap checks.
The v1322 injection, syntax and generated-runner preflight PASS.
Its new 256K physical guest was building/running at this checkpoint,
with NO final result or no-partner liveness acceptance yet.

Both fixtures are testkit-only; no production kernel source, resident
footprint or scheduler/swap policy has changed. The rejected
v1208/v1211 fallback stays excluded. A full 256K guest must still
demonstrate real swap-in allocation failure without an eligible
exchange partner, later independent process progress, actual backing
restoration and zero leaked resources to close the OPEN liveness gate.

## 2026-11-07 v1320: native DEVICE-backed swap-in gateway PASS

Testkit `d65899c` extends the already accepted native v1317
production `proc_swap_in`/MM/pinning/rollback test through the
unchanged `fs_mres.s` DEVICE backing gateway. A test-only DSK270
hardware endpoint verifies eight translated sectors, logical
backing blocks 1..8 mapped to physical sectors 021..030, and
0200-word physical destination progression per sector. The
production swap-in transaction successfully allocates/pins,
transfers the image via the gateway, unpins, publishes RESIDENT
mode, frees the backing bitmap and offers READY_ONCE. Both a
first-sector and a third-sector read error exercise full production
MM rollback, preserve the DISK record/bitmap, and invoke one TERM.
An executable PDP-6 mutation suppressing the production gateway
buffer increment is correctly rejected. The v1317 test suite is
also rerun by the new fixture, and the v1319 target PASSes twice.

This is **native integration, not full-system acceptance**: actual
DSK270 hardware, guest PI6 scheduling, independent user process
progress and genuine exchange eligibility are still test-controlled.
Production kernel source, resident footprint and v1208/v1211
fallback policy remain unchanged. See
`pdp10-doc/KERNEL/SWAP-DEVICE-GATEWAY-NATIVE-V1320.MD` (`a4fa226`).
Evidence `$TMPDIR/daimos-swap-device-gateway-v1320-evidence-r1.tar.gz`,
SHA256 `c63d3f14a09d0873bb4958bbf40de7dd61bff84c3a4cf7901d1c54e06348bbab`.

**OPEN:** booted 256K physical no-partner swap-in pressure followed
by independently runnable progress, real disk restoration, and final
leak-free termination or recovery. Do not count the native test as
closing that acceptance gate.

## 2026-11-07 v1318: production swap-in native commit/rollback PASS

Production `system/kernel` remains unchanged from the accepted v1315
baseline. daimos-testkit `7ffd840e` adds native v1316/v1317 PDP-6
execution of the **actual** `proc_swap_in` transaction, slot-0 service,
MM allocator and pin/unpin/free, packed DISK/RESIDENT storage mode,
real backing bitmap release, and PI6 countdown/scheduler gate.
Physical MM occupancy drives a typed NOMEM; all three test-controlled
exchange peers refuse admission, backing remains intact and cooldown
0100 is entered. The production clock prefix reaches tick 63 with
one tick outstanding, then expires it at tick 64. After the physical
owner's genuine `mm_free`, the subsequent production swap-in transaction
allocates and pins a process extent, reads via a test disk provider,
unpins, publishes the resident base/storage mode, and releases the
eight backstore blocks using the production bitmap. Correct VM owner,
two endpoint payload words, zero outstanding pins and zero block usage
are verified. A second test returns an I/O error from the disk stub;
the production rollback unpins and frees the new physical extent,
preserves the original DISK record and bitmap, clears TRANSITION after
rollback, and emits exactly one fatal TERM. Three executable mutants
(leaked MM extent, leaked backing blocks, missed storage publication)
are rejected. Older v1315/v1301/v1300/v756 regressions remain PASS.

These are **native testkit integration** results, NOT a fully booted
256K guest: disk read, candidate admission, independent runnable-path
execution and hardware PI6 interrupt entry are controlled fixtures.
The full-system real no-partner liveness and authentic disk rollback
gate remains OPEN. No kernel RAM/code growth, no kernel changes, and
the rejected v1208/v1211 fallback stays excluded.

Native evidence:
`$TMPDIR/daimos-swap-in-real-transaction-v1318-evidence-r1.tar.gz`,
SHA256 `4459b6c916c52d7cc71c5062ff727bb76989edb4a2b6e93cb056f962dc47d8eb`.
Documentation: `pdp10-doc/KERNEL/SWAP-IN-TRANSACTION-NATIVE-V1318.MD`.

## 2026-11-07 v1315: production PI6 aging/swap gate native PASS

The native PDP-6 real-MM no-partner fixture now also extracts and
executes the production `proc_sched_pi_tick` timekeeping section
and `proc_select_rt_normal_swap_v879` gate without any DAIMOS
kernel edits. After an actual MM_ERR_NOMEM, bounded reclaim and
three rejected exchange candidates, its cooldown starts at 0100.
Exactly 63 clock-prefix calls leave cooldown 1. The actual
scheduler gate chooses the independent runnable path and does not
request swap-in. The occupying owner releases physical memory via
real `mm_free`. The 64th production tick expires cooldown, and
the production scheduler gate raises cursor 0402, followed by
production MM allocation and slot-0 READY_ONCE success. No TERM,
exchange or ownership leak occurs in this native scenario.

Six executable negative mutants are rejected (typed MM shortage,
FS reclaim, process reclaim, cooldown publication, clock decrement
and scheduler gate bypass); neighboring native v756/v1111/v1260/
v1300/v1301/v1303 tests PASS. Testkit commit `bfb629ae`; source
documentation `pdp10-doc/KERNEL/SWAP-NATIVE-REAL-MM-NO-PARTNER-V1314.MD`
commit `12f899a`. Versioned positive and negative evidence archive:
`$TMPDIR/daimos-swap-native-clock-gate-v1315-evidence-r1.tar.gz`,
SHA256 `17f4d19c56b7776c608cfc88a0e94f0c3674cb62ce2d09094e503193ebb87134`.

**LIMIT:** the native fixture stops before PI6's hardware dispatch
and still substitutes disk transfer and exchange eligibility.
Booted full-system no-partner liveness, real backstore read and
rollback cleanup remain OPEN. The accepted 64/64 production OOM
gates are unchanged, as are KCORE/MRES footprint and kernel source.

## 2026-11-07 v1314: native real-MM no-partner/retry integration PASS

Production DAIMOS `9475f50a` remains unchanged in `system/kernel`.
The new daimos-testkit `d40a1158` v1313 native PDP-6 test extracts
the authoritative MM find-fit/noreclaim/bounded allocator, arena
feasibility, compaction and free code and unchanged slot-0 swap
service. A full 010000-octal-word managed physical arena is occupied
by a real kernel-dynamic extent owned by slot 3, while a pending
swapped RUN process (slot 2) requests 02000 words. Production MM
returns typed MM_ERR_NOMEM (-1) after exactly two filesystem and
one process reclaim calls (all three intentionally refuse to free
the owner). The production slot-0 service checks three candidates,
finds no eligible exchange, retains the backing image, avoids TERM,
and sets the 0100-octal cooldown. A genuine `mm_free` releases the
original typed/owned extent; a subsequent service request allocates
the process extent with correct owner 2 and grants READY_ONCE.

The actual disk transfer/publication wrapper and candidate-admission
decisions are **test stubs**, not production swap-in implementation.
This is therefore a real-MM + production-scheduler-service *native*
recovery result, NOT a full-system physical swap-in/deadlock or
backstore rollback acceptance. Four executable PDP-6 mutations
(collapsed NOMEM, missed FS reclaim, missed process reclaim and
missing cooldown) all fail their assertions. Neighboring native
v756, v1300/v1301, v1111 and v1260 remain PASS. No kernel source
or RAM overhead change is justified.

The versioned source/map/report and negative-case evidence is
`$TMPDIR/daimos-swap-real-mm-no-partner-v1314-evidence-r1.tar.gz`,
SHA256 `c30cccf491176a4488ffcf8933dff0b0485ce85dc198c3d10db60e06c884ed92`.
The archive's internal manifest verifies. Final documentation is in
`pdp10-doc/KERNEL/SWAP-NATIVE-REAL-MM-NO-PARTNER-V1314.MD`.

**OPEN:** a booted 256K guest must naturally or through a temporary
physical occupancy fixture experience actual swap-in MM pressure
with no valid exchange partner, preserve runnable independent
processes through the PI6 cooldown, then perform authentic
backstore swap-in and complete cleanup/rollback. Do not substitute
v1313's mocked disk completion for this gate, or merge historical
v1208/v1211 fallback without a genuine reproduced kernel defect.

## 2026-11-07 v1312: completed F07 trace, offline full postflight replay

Production DAIMOS `417fbade` (kernel source unchanged from `f78145db`)
has no newly demonstrated swap-in MM defect. The first instrumented
24K-BSS 256K KCC PDP-6 DSK/D6FS full guest, launched on the earlier
v1308 testkit, recorded all 15 Ctrl-E read-only live samples, all 18
RUN admissions and 18 TERM/WAIT reaps, requester-only BRK OOM,
provider/witness milestones and HALT. Its final recorded counters
show 51 swap-outs, 51 swap-ins, 0 real swap-in allocation failures,
0 exchanges, 0 fatal swap failures and 0 no-partner cooldowns.

The original v1308 guest runner returned failure after the v1075 OOM
PASS because interactive SIMH `sim> ` prefixes in its final memory
EXAMINE output prevented v1150 from finding a provider counter. The
later v1309 testkit correction for that PTY formatting PASSes the
complete protocol selftest, but was committed after this runner had
started. Do not report this original invocation as an ordinary
full-system runner PASS.

Testkit `9ddb748` v1311 replays the UNMODIFIED original v1075
postflight validation source against a separate prompt-normalized
copy of the captured guest output. All v1150 provider, v1159
protected-process, v1203 physical MM, v1205 actual process/BRK,
v1207 swap/stack, and v1160 release-media evidence checks PASS.
The nested E13 evidence SHA256 manifest verifies. A disposable
negative test replacing the provider's real armed counter with zero
fails the original v1150 verification, as required. This is **offline
validation of completed recorded guest behavior**, not a new direct
F07 runner PASS. All original files and the full raw debugger trace
are retained unmodified.

Versioned evidence:
`$TMPDIR/daimos-swap-no-partner-evidence-v1312-r1.tar.gz`, SHA256
`e5224db0e4266bea68c7e953c22f49c6074443ea930a5b360665725ab8a1b669`.
Both outer and inner evidence manifests are verified. Operator/source
documentation: `pdp10-doc/KERNEL/SWAP-SIMH-LIVE-TRACE-V1309.MD`.

**Still OPEN:** real full-system no-equal-partner swap-in MM pressure
and later conditional forward-progress/leak-free cleanup. All present
clean-source 24K and 16K loads progress via ordinary disk swap, never
entering the no-partner path. Historical v1208/v1211 16-word SWAP
fallback remains excluded; no production code, KCORE/MRES size or
profile gate was changed during this diagnostic acceptance.

## 2026-11-07 v1309: live swap-pressure tracing, full no-partner still open

Production requester-only OOM remains enabled and accepted (F12/F13
64/64) on the strict 256K KCC PDP-6 DSK/D6FS profile. No production
kernel, scheduler, MM, swap or size-optimization source changed.
The original 16K-BSS 24-process F07 physical guest on `f78145db`
completed all 18 RUN/reap lifetimes, requester OOM and final MM
cleanup: 41 real swap-ins, 0 allocator pressure, 0 no-partner
cooldowns. A separate 24K-BSS guest admitted all 18 extra children
and was stopped deliberately before its old 900s timeout and before
cleanup: 49 swap-outs, 33 swap-ins, 0 swap-in MM failures and 0
cooldowns. This INTERRUPTED run is not accepted as a completed guest.

The daimos-testkit `5e23ff0` adds v1303 physical counter and
negative-mutation classifier, v1304 disposable 24K pressure fixture,
and v1306 live read-only SIMH sampler. Its PTY Ctrl-E/EXAMINE/GO
protocol and two-sample real PDP-6 smoke PASS. Separate
`simh.out.raw-v1306.log` and `simh-sampler.log` hold debugger output;
`simh.out` retains the strict F07 guest-only record. Testkit `3fa4792`
fixes interactive `sim> ` prompt spill into post-HALT resource
records; a protocol selftest verifies cleanup and untouched boot.ini.
The v1308 24K full guest with 15 periodic samples was still active at
this checkpoint and has NO accepted final result yet. Do not count
an interrupted, unexercised, or short diagnostic as proof of
full-system no-partner forward progress. The native v1301 no-partner
branch test stays PASS, while actual full-system no-partner liveness
remains OPEN. Do not merge the rejected v1208/v1211 fallback.
The new operator reference is
`pdp10-doc/KERNEL/SWAP-SIMH-LIVE-TRACE-V1309.MD` (`9ff31eb`).

## 2026-11-07 v1302: current-source swap-in pressure audit (partial)

Production DAIMOS `283b54dc`, with the v1298 requester OOM release
already accepted, was rechecked for swap-in pressure and forward
progress. No production kernel source was changed in this audit.
The clean v852 real 256K PDP-6 GCC/DSK/D6FS scheduler-pressure
guest completed a 16-child post-INIT pressure cycle and HALT: PASS.
Native v855 incoming-image setup, v958 failed-unpin/free rollback,
v666 late swap-out rollback and v757 scheduler-pressure wiring PASS.

daimos-testkit v1300 now checks the current native no-partner
candidate scan, pressure/fatal classification, cooldown, backing
ownership and scheduler retry gate; six unsafe source mutations are
rejected. Native PDP-6 v1301 executes the current slot-0 service
with a synthetic allocator PRESSURE and every pair rejected: the
backing remains owned, the request is consumed, cooldown is 0100,
all three candidates are tested, and no TERM occurs. Separate real
assembly service outcomes confirm fatal TERM and successful one-shot
dispatch. The previously stale v756 test was repaired to check the
native assembler rather than removed C implementation; compilation,
the pressure/fatal contract and four unsafe mutations PASS.
Testkit commit: `3cc0882a62d26a6a06e4a7904af4ff9d4e3f0f56`.
Retained partial-acceptance logs and tool/source hashes:
`$TMPDIR/daimos-swap-pressure-audit-v1302-r2.tar.gz`, SHA256
`6a59446e07ec2dc5c6b16753ce789f04b26f26b43d1de7b9f39295b9555cd003`.

This is NOT a full-system test proving eventual progress with no
viable exchange partner. The clean v852 guest completed but did not
prove that exact pressure branch was reached. The old v750-v754
boot-directory traces and the v1208/v1211 experimental 16-word
fallback do not establish a current production defect. A separate
physical 256K no-partner pressure fixture must demonstrate actual
forward progress or a reproducible stall, including no recursive
reclaim, backing-space admission and transaction rollback, before
any new production swap algorithm is authorized. The older v850
trace wrapper also depends on obsolete DAS_REPO setup; its initial
failure was a fixture/build dependency, not evidence of kernel
failure. Functional completion remains OPEN; optimization deferred.

## 2026-11-07 v1298: F12/F13 requester-only OOM release accepted

F12 production activation and F13 post-enable signoff are accepted
for the frozen 256K KCC PDP-6 DSK Stage 1, four-member D6FS root,
PCLK60, headless CTY, writable RUN/EXEC release profile. All 64/64
mandatory OOM gates are now CLOSED. The sole production kernel change
is `PROC_OOM_BRK_ENABLED_V1000=1` in syscall_dispatch_pdp6.s.

The native v1296 phase passed 14/14 F03 positive/negative booted
policy cases, 18/18 P0-D native stages and 17/17 individual booted
P0-D physical faults (the interrupted initial P0D_FULL sequence is
not called an uninterrupted pass). Both 64-slot F07 policy cases
also pass with 63 users, 127/136 MM descriptors, eight swapped,
31642 APR-rounding words and safe stack watermarks.

The original archived v1285 independent post-enable Android and
Linux release-media tests were recovered locally, then verified.
Their candidate `78259106e267636130a4ab41f01d020981fe8c41`
has parent `e6993a764d9fc82f2421c8b71862c1aa80e2fa14` and only
the OOM policy switch differs. Both policy guest archives on both
hosts record production_oom=ON and identical testkit revision
`74baec8b9be739d20ba55780491410556b8448f6`. All four archived
guest manifests pass SHA256, cross-host KCORE maps match byte-for-byte
per policy, and distinct PDP-6 binary hashes prove independent
simulator builds. Strict F10 build diagnostics pass all four logs,
with no unexplained errors or warnings.

Versioned combined evidence archive:
`$TMPDIR/daimos-f13-release-evidence-v1298-r1.tar.gz`, SHA256
`5dcb4d75ea6ea2291e5ec01f54874ce8d704713c1608fff9dcd64ab8aab83221`.
Other platforms, raw MEMFS XIP, full 32K DSH, and strict 16K
permanent residency are not certified by these release gates.
Next feature completion: D6CODEC, BSTORE, restricted XIP and
opt-in compressed process swapping. No KCORE optimization performed.
DTFS tests are not acceptance blockers during the planned rewrite.

## 2026-11-07 v1296: F12 complete local guest matrix, host signoff pending

The production-enabled requester-only BRK OOM source (switch=1)
passed all 17 P0-D physical guest cases individually: six in the
partial v1293 monolithic matrix and eleven in the sequential v1294
continuation. All eighteen v1096 native release stages had already
passed. This does not retroactively turn the interrupted monolithic
P0-D run into a continuous PASS. The full F03 fourteen-guest
disabled/enabled matrix also passed, combining earlier v1075,
v1110-ON and v1120-OFF/ON results with the nine v1295 guests.
All cases ran on the current working source with production OOM ON.

The v1190 diagnostic audit passed on both fresh 64-slot F07
release-media build logs: twelve approved diagnostic classes per
profile, no unexpected compiler/assembler/media diagnostics. The
complete local evidence, source diff and tool identity manifests
are SHA256-protected in the versioned archive
`$TMPDIR/daimos-f12-production-oom-evidence-v1296-r1.tar.gz`,
SHA256 `13411b8dbbf548a425567a6ed2a460e57cf3e8d2155a9ad32e4bbf84ba2a6e07`.

F12 and F13 remain OPEN: the independent post-activation host/media
qualification and final release audit are not signed off. Both
previously configured LAN SSH endpoints were unreachable during this
attempt. Prior F11 E13 acceptance is preserved, not substituted
for a new post-activation dual-host result. No commit/push or
KCORE size optimization was performed; DTFS tests remain excluded.

## 2026-11-07 v1293: F12 full P0-D partial physical guest checkpoint

Production requester BRK OOM remains ON in the uncommitted source.
The full `P0D_FULL=1` v1096 runner passed all 18 native component
stages and the allocation-free v1083/v1087 release graph, followed
by the first six actual 256K PDP-6 physical fault-injection guests:
v1125, v1124, v997, v1103, v1117 and v1118. These cover permanent
wrong-owner MM/provider leases, TTY/VM/uarea MM_BUSY, pre-VM retry,
combined PIPE/FILE/HUP/VM/uarea owner stages, 48 repeated stale
WAIT/PID/event refusals and simultaneous PIPE/VM pin/MM_BUSY with
actual swap detach. Every completed guest reported PASS.

The sequential runner was interrupted while assembling the seventh,
v1104 physical HUP/pinned PIPE case. The interruption is not a kernel
test failure, but the full 17-guest matrix MUST NOT be reported PASS.
Partial log: `$TMPDIR/daimos-f12-p0d-full-v1293-r1.log`.
F03 14-guest, remaining P0-D, grouped host/media diagnostics, F12
release acceptance and F13 post-enable signoff remain OPEN. Keep
source activation uncommitted and do not claim production release.
DTFS tests are excluded and no KCORE optimization was performed.

## 2026-11-07 v1292: production requester-OOM F12 partial activation

The production BRK OOM selection is now enabled in the working tree
(`PROC_OOM_BRK_ENABLED_V1000=1`); this is not yet a committed or
release-signed-off state. The v1283 policy/mutation guard, v1000
dispatch, v998/v999 cause classification, v1113 native actual MM
negative causes and v1123 RUN/EXEC source scope pass. Fresh physical
v1075 and held-provider v1150 enabled/disabled guests both pass.

Fresh enabled and disabled 64-slot F07 full 256K KCC/DSK/D6FS guests
each passed with 63 live user processes, 127/136 MM descriptors,
eight swapped processes, 239576 MM allocated words, 31642 exact
APR-rounding words, private kernel stack 180/206 and idle stack
63/256. Both archived guest evidence directories were created under
`$TMPDIR/daimos-f12-f07-64slot-v1288-r1` and
`$TMPDIR/daimos-f12-f07-64slot-disabled-v1289-r1`.
The v1096 native P0-D release/ownership component matrix and booted
enabled-policy v1110 invalid BRK negative controls also pass.

F12 is OPEN until the complete production-enabled F03/P0D_FULL
and exact-source release diagnostics/host matrix are accepted. F13
remains OPEN; do not commit/push or claim release acceptance until
full signoff. DTFS tests remain excluded for the planned rewrite.
No KCORE optimization was performed.

## 2026-11-07 v1284: F11 exact-source OOM release-candidate review CLOSED

The frozen S07/S08 256K PDP-6 KCC DSK/D6FS candidate has completed
all mandatory preactivation OOM tests and independent F11 integration
review. Current DAIMOS `f110be06` has byte-identical production
`system/kernel` and `system/boot` to the frozen `3978b50a` source;
the F03 Android worktree used frozen testkit `69830c9`. Both Android
and Linux exact-source **F03 14/14 full-system cases PASS**, including
both disposable OOM policies for BRK, invalid causes, failed RUN,
MM slots, fragmentation, RUN cause and 24-PID reuse. Evidence SHA256:
Android `c167a89c44ef2480c0c9d91f92e588b15d1a4c9e928a75113c6c2bca1f62e19e`;
Linux `0252a5900c7e9850e4c6251e7d52a01898c3b9e1dff5f26e1af5b88d832bb88b`.
The uninterrupted Android `P0D_FULL=1` matrix completed 18 native
stages, the allocation-free v1087 release graph and **all 17 genuine
booted physical owner/teardown guests**, with in-order PASS and normal
exit. Archive `$TMPDIR/daimos-f11-android-p0d-v1284-r1.tar.gz`, SHA256
`0565d7e879b432e728ea3262e3cd65774126fda129fe27c6b38ef1233a5819c9`.
An initial Linux P0-D monolithic run suffered an external simulator
SIGKILL/exit137 during v1121; it is **not** called an uninterrupted
PASS. The nine earlier physical cases had completed, and v1121 plus
seven remaining guests were independently rerun with PASS, with the
initial failure preserved: SHA256
`901326d12c562d7bdb313c05009b370b33219cd45312ffea1defae7c5b47685d`.
Both F07 64-slot/127-descriptor/held-provider modes and E13 exact-
revision independent simulator/media hosts remain CLOSED. F10 diagnostic
scan, F04 exact MM causes, v1260 no-side-effect feasibility, v1092/v1282
provider wait quarantine, and v1123 scope/call-graph controls PASS.

**F11 CLOSED, 62/64 mandatory gates complete.** This is an explicit
release-candidate review, not evidence that production OOM has already
been switched on: `PROC_OOM_BRK_ENABLED_V1000` remains **0**. Next are
F12 guarded production activation, complete both-policy regression,
then F13 post-enable signoff/revert on any failure. Raw MEMFS XIP
RUN/EXEC and static 16K KCORE+MRES optimization remain separate.
See `pdp10-doc/KERNEL/OOM-F11-RELEASE-ACCEPTANCE-V1284.MD`.

## 2026-11-07 v1280: both 64-slot F07 OOM policies pass the mandatory provider barrier

Full 256K KCC DSK/D6FS F07 64-slot guests now PASS with the original
production kernel unchanged and testkit `26d0e39` requiring *observed*
foreign-provider contention before initiating the physical requester
BRK OOM. The old fixed sleep could allow the gateway waiter and spinner
to miss the provider-held window; earlier unbarriered disabled guests
therefore either failed the liveness assertions or, in one stopped
diagnostic, stalled during admission. No production scheduler root cause
has been established from that stopped run. The new disposable-only
three-counter, bounded read-only BRK probe does not modify ordinary
production syscall handling. Its host negative mutants and native
assembler check PASS.

Both enabled and disabled guests completed 57 independent RUN admissions,
actual MM pressure and disk swapping, 57 individual TERM/WAIT/reaps,
protected-process and held FS-owner checks, final FILE/MM/swap accounting
and expected HALT within the 1800-second bound. At physical OOM both
show 63 user processes, 55 resident/eight nonresident, 127/136 MM
descriptors, 239576 allocated managed words, 31642 APR rounding words,
180/206 private and 63/256 idle stack high-water. The disabled policy
returns BRK -1, records predispatch=1, spinner=11, zero fatal victim
and reaches `NWP`; the disposable-enabled policy selects *only* PID 3,
records predispatch=1, spinner=38 and reaches `KWP`. Both owner error
counters are zero. All manifest SHA256s and 13 v1274 negative snapshot
mutants PASS. Archives:

* `$TMPDIR/daimos-f07-v1280-64slot-disabled-frozen-r1.tar.gz` SHA256
  `68c719c5901e3874acaeabe2cef6f309df0566bf99619d89dae2f53380994cd7`.
* `$TMPDIR/daimos-f07-v1280-64slot-enabled-frozen-r1.tar.gz` SHA256
  `bb0173c726d4e243edec0a09c977b0e4bb68147a4117f90ab932df2e0936b253`.

DAIMOS profile revisions `3978b50a` and `67fd397f` differ only in
documentation, not kernel or boot source. The testkit was frozen at
`69830c9` for both policy runs (the v1280 barrier itself is `26d0e39`).
The v1280 evidence is documented in pdp10-doc `e1fbad6`. F07 and E13
are CLOSED, 61/64 active mandatory OOM gates are checked, **F11 final
integration remains OPEN**, then F12 production enable and F13
post-enable release acceptance remain OPEN. Production requester OOM
is still OFF; the separate 16K permanent residency target is unaffected.

## 2026-11-07 v1279: F11 toolchain regression identified and fixed; final matrix OPEN

The current DAIMOS `226e7a5f` F03/v1112 failed-RUN guest initially
halted at PC 0 on Linux but passed on Android with byte-identical
INIT DXR. A frozen Linux source bisect (37 intervening commits)
located the first failure at `4dcaca20` (v1257 scheduler locality),
but the actual defect was the Linux **installed** `p10fold` and
`dlink` binaries dated October 13, before the v1141 cross-object
LOCAL relocation fix. The old installed `p10fold` independently
FAILed the native v1141 mutation test, folding distinct object-local
targets. No kernel scheduler code was changed.

DAIMOS tools `61dcb15` commits exact six-field portable fold-plan
parsing in p10fold/dlink and compatible p10run examination (strict
C99 host compile, no undeclared `strtok_r`). Testkit `ce5c266`
adds seven malformed field-plan mutants; both Android and Linux
native v1141 and v1257 tests PASS. Linux tools were rebuilt from
clean source plus the tested parser patch and installed into
`$HOME/cross` from versioned
`$HOME/tmp/daimos-tools-v1277-clean-r1/local.tar.gz`, SHA256
`938caf5c4e72433a470504c8ee10526abe8707c4724db52601c863566f76d37d`;
previous binaries were preserved in `previous-bin/`. The now-correct
Linux v1112 guest completes `<RILDNWP` and expected HALT; KCORE low
end `023267` equals Android's, correcting the former two-word
host-specific link mismatch. F10 host diagnostics remain strict.

The v1113 real MM/VM/BRK native F04 fixture subsequently failed to
link because it omitted the newer production allocator preflight
`mm_arena_fits_v1260`. Testkit `69830c9` extracts the actual leaf;
Android native v1113's -2/-3 and three bad mutants, and v1260's
separate no-side-effects/two mutants now PASS. No production MM or
OOM behavior was changed by this fixture repair.

An additional full 64-slot/63-user OOM-disabled F07 guest completed:
57 real child RUN/BRK and TERM/WAIT, correct requester no-fatal path,
real provider protection, 127/136 descriptors, 232 swap blocks,
180/206 private and 63/256 idle stack peaks, zero final leaks,
`<`+57 `+`+`^@%&=D*?{}|`+57 `~`+`NWP` and expected HALT. Evidence
`$TMPDIR/daimos-f07-v1280-64slot-disabled-frozen-r1.tar.gz`, SHA256
`68c719c5901e3874acaeabe2cef6f309df0566bf99619d89dae2f53380994cd7`;
all internal SHA256 manifest entries verify.

The corrected Linux final F03/F04/P0-D/E13 matrix was started, but
the device lost access to the home-LAN SSH hosts while jobs were
running. Completed prior exact-revision F07/E13 evidence remains
valid; incomplete final F11 matrix rows are NOT counted. The
production `PROC_OOM_BRK_ENABLED_V1000` switch remains 0. Mandatory
readiness stays **61/64**: F11 pre-activation plus F12/F13 after
F11 signoff. The separate 16K KCORE+MRES footprint remains OPEN.

## 2026-11-07 v1276: E13 exact-source dual-platform release evidence PASS

The same full 256K KCC/DSK/D6FS/PCLK60/headless release-media
candidate built from DAIMOS `226e7a5f` and testkit `ee9d92b`
completed both requester-OOM-disabled and disposable-enabled cases
on independent Android aarch64 and Linux x86_64 Open SIMH PDP-6
V4.1-0 binaries. Android simulator SHA256
`601b6e9ec9979a7b457ab1d866072f4e167bd14929b6de26877c6620fcc36ea0`,
Linux `fdf2efcad69bc4fa7c14fa97fa0031923123af184832af6ca986302432b3d72c`.
Both policies have identical KCORE low endpoints on both hosts:
OOM OFF `023503`, test-enabled `023521` (octal). Only PID 3 is
selected for fatal OOM when enabled; the disabled requester returns
the correct error. Original held FS provider, blocked witness,
gateway waiter, independent spinner and final WAIT/FILE/MM accounting
all PASS. Source revisions, boot media, tool hashes, profiles and
raw counter logs are retained with SHA256-verified archives:

* Android `$TMPDIR/daimos-e13-release-media-v1160.dYyc6O.tar.gz`:
  `c87799ba1ecda40f07bbbf718d5ca20e0d5be896549df3be80d075ed0c8990ef`.
* Linux copied to `$TMPDIR/daimos-e13-linux-x86_64-v1272-r1.tar.gz`:
  `a59d9b90338a0c23fd084fb7153160bd4068fd4233eabfcf7cd0205fb8b6a720`.

The F10 v1190 diagnostics audit PASSes four complete builds with
zero unexplained notes. The one optional, already-approved legacy
D6FSR2 media-layout warning appears only on the Linux-produced
media; v1275 excludes ONLY this expected host/media difference from
cross-host *tool* diagnostic-class equality, retaining strict tool
notes and fatal unexpected messages. The first frozen S07/S08
profile deliberately excludes alternate boot/media and emulator
targets; E13 is CLOSED for its exact release scope. OpenBSD host
coverage remains optional follow-up rather than certified by E13.

F07 and E13 are now CLOSED. F11 is NOT approved: a current Linux
v1112 failed-RUN negative guest halts unexpectedly, although an
older frozen DAIMOS source and the current Android source PASS.
An isolated source revision bisect is in progress. Production
requester OOM remains OFF until F11 review, F12 tests and F13
post-enable acceptance. Active mandatory readiness becomes 61/64.

## 2026-11-07 v1274: F07 64-slot/maximum-MM acceptance PASS (CLOSED)

The frozen 256K KCC DSK/D6FS PDP-6 guest built from clean DAIMOS
`226e7a5f` (disposable testkit source `027fc3f`) completed real
requester-only physical OOM with a foreign FS lease held, 57 extra
concurrent RUN children, all 57 individual TERM/WAIT/reaps, exact
post-test resource accounting and expected `KWP` HALT within the
original 900-second simulator limit. Production requester OOM is OFF.

At the same original physical-OOM decision, the independent samplers
record 63 live user slots of 64 configured (including kernel slot 0),
55 resident process extents, eight swapped/nonresident users, 127 of
136 actual MM descriptors occupied, 239576 of 242085 managed physical
words allocated, 2509 free words, 31642 exact APR BRK rounding words,
and one 128-word held provider workspace. Private kernel stack peak
180/206 words and slot-0 idle stack peak 63/256 words are within
capacity. The 57 children had distinct verified BRK deltas. Only the
requester PID 3 was selected; both provider/protected-process error
counters are zero, and final swap blocks/FILE/MM ownership was cleaned.
This supplies *simultaneous* high descriptor, memory, APR, provider,
process-slot and stack evidence, not separate synthetic maxima.

The first 64-slot attempt cloned INIT's 16K-word pressure BSS 57
times and was controlled-stopped after 18 admissions because of
test-only swap traffic. The accepted fixture keeps a real corruption
sentinel in a 1K-word BSS only in the 64-slot mode; the legacy 24-slot
mode stays unchanged. Its actual core, descriptor and swap measurements
remain mandatory. Testkit `89a8efa` further guards exact live u-area,
APR and descriptor occupancy, and rejects 13 unsafe snapshot mutants.
Archive `$TMPDIR/daimos-f07-64slot-acceptance-v1274-r1.tar.gz`, SHA256
`0fb22920bfd9f66c155ef3bb61f8db0c0f146f80f89bbc3e6e1b3406d9599e1b`;
all included evidence manifest hashes verify. F07 CLOSED for S07/S08.
The outstanding E13 and F11 release-candidate signoff plus F12/F13
production-enable/rollback gates remain OPEN, 60/64 complete. The
separate permanent KCORE/MRES 16K size target remains unchanged.

## 2026-11-07 v1270: reject optional v1208/v1211 swap fallback; clean all source tips

The frozen clean DAIMOS `9a841363` and experiment-bearing F07 guests
both passed the same 256K 24-process physical OOM/real disk-swap test.
The clean guest observed 41 swap-outs/41 swap-ins and no allocation
pressure, exchange, fallback or failures. The experimental guest saw
33 swap-outs/33 swap-ins, 8 exchanges, 18 swap-in pressure returns,
10 fallbacks and zero failures. Its SWAP MRES is 16 words larger.
This is not proof that the fallback is needed: a distinct scenario with
no viable exchange partner has not failed on the clean kernel, and the
added reclaim/restore graph needs its own fault-rollback acceptance.
Thus v1208/v1211 are DROPPED from production rather than silently merged.
The exact 58-line reversible patch is preserved in
`design/f07-swap-fallback-experiment-v1208-v1211-r1.patch.gz`, SHA256
`c99088467b245010117cf37e350aca288e22b0f72547a437f69e87fc0ad84806`.
It applies to the clean frozen reference but is not assembled or linked.
Historical F07 diagnostic fixtures are committed in daimos-testkit
`77f156a`; no unresolved test code is left untracked there.
The 64-slot/high-descriptor combined F07 acceptance remains OPEN,
with production requester OOM OFF. This decision does not enable OOM
or establish a permanent KCORE/MRES 16K fit.

## 2026-11-07 v1269: clean-source F07 24-process guest PASS; swap experiments remain separate

The full 256K KCC DSK/D6FS F07 guest has now PASSED against the
unmodified, committed DAIMOS source `9a841363`, with the same
18-child/batch-TERM test and the unchanged 900-second guest limit.
The clean kernel had NO v1208 no-equal-partner fallback and NO v1211
nonrecursive swap-in allocator change. Test-only requester OOM was
enabled only in the disposable guest; production requester OOM is OFF.
All 18 RUN acknowledgments, real swapping, physical requester OOM,
foreign FS owner/gateway cleanup, all 18 TERM/WAIT/reaps, final
process/FILE/MM accounting, and HALT PASSED:
`<++++++++++++++++++^@%&*?{}|~~~~~~~~~~~~~~~~~~KWP`.

The independent v1159/v1203/v1205/v1207 protected-process, physical
MM, actual-process, swap/stack verifiers PASS; v1150 provider error
is zero. Evidence manifest hashes all PASS. The clean swap counters
are 41 successful swap-outs, 41 swap-ins, zero service-level MM
pressure, zero exchanges, zero fallback, zero swap failures/cooldowns.
Full archive: `$TMPDIR/daimos-f07-clean-v1269-acceptance-r1.tar.gz`,
SHA256 `b0883d79d0a7baacb007690f62be6234a458c927afabf08ad1aac2d57510dde2`.

The separately successful v1264 experimental guest used uncommitted
v1208/v1211 edits: 33 successful out/in transfers, eight exchanges,
18 explicit swap-in pressure returns, ten successful fallback entries
and zero failures/cooldowns. The identical instrumented links differ
by 020 octal = 16 decimal SWAP MRES words, with equal KCORE low-end
addresses. Testkit `218bd7e` enables a guarded clean comparison;
`e8c9667` retains the nonrecursive source/mutant guard; `11e6cc9`
checks exercised fallback counters and five negative mutants.
Both implementations assemble natively. This DOES NOT establish
that the extra 16 words are necessary or beneficial in production.
Do not merge the experimental code until a distinct failure case
(no viable equal-size exchange) and its rollback/performance acceptance
justify it. Preserve the uncommitted changes without overwriting them.

The 24-process F07 stress milestone is now reproducible on committed
production source, but **F07 remains OPEN**: 64 process slots and
combined high MM descriptor/APR/provider pressure are still untested.
E13/F11 also remain OPEN; 59/64 active OOM gates and the production
OOM flag remains OFF. Static 16K residency is a separate KCORE/MRES
size objective, not an OOM preenable gate.

## 2026-11-06 v1265: F07 24-process swap/OOM cleanup verified (partial gate)

Fresh 256K PDP-6 KCC DSK/D6FS full guest v1264 completed within the
original 900-second limit: `<++++++++++++++++++^@%&*?{}|~~~~~~~~~~~~~~~~~~KWP`
and expected HALT. The 18 extra processes were admitted, received
real batch TERM events and were each reaped by WAIT with exact status.
Native v1150 owner check records `error=0`, `real_oom=1`,
`holder_return=1`, `fatal_busy=7` (octal). Independent v1159,
v1203, v1205 and v1207 protection, MM, process, swap and stack
verifiers PASS. Evidence archive:
`$TMPDIR/daimos-f07-v1264-acceptance-r1.tar.gz`, SHA256
`0fc008be42997a81138f4513d603bb828a0a978903ef60d1c761dd08e2068fea`.
Testkit v1265 `5881961` commits the 18-child batch TERM and swap
verifier fixtures. pdp10-doc `f6a7f46` documents methods and limits.

**Acceptance boundary:** guest source was derived from DAIMOS
`b6db5372` with *uncommitted experimental v1208/v1211 swap changes*,
not from the clean production tree. The 64-slot and combined
high-descriptor F07 matrix is still untested. F07/E13/F11 OPEN,
59/64 OOM gates; production requester OOM OFF. Do not merge the
experimental swap paths or claim final F07 enablement from this run.

## 2026-11-06 v1262-v1264: 24-process F07 swap/OOM cleanup full guest PASS

The fresh 256K PDP-6 KCC DSK/D6FS 24-process release-media guest
completed its full physical-OOM stress sequence within the original
900-second limit: all 18 additional RUN acknowledgments, requester
BRK/WAIT, real foreign FS lease ownership and release, spinner and
gateway cleanup, 18 individual TERM/WAIT/reaps, witness survival and
final process/FILE/MM accounting. The final markers were
`<++++++++++++++++++^@%&*?{}|~~~~~~~~~~~~~~~~~~KWP` followed by the
expected HALT at PC `021377` octal. Simulator and harness exited 0.

The v1203/v1205/v1207 independent verifiers passed physical MM/APR
extent and logical BRK accounting, 24 live slots, 12 resident process
extents, 12 nonresident/swap processes, 1,792 swap blocks at physical
OOM, and private/idle stack watermarks. v1150 provider-owner and
v1159 protected-process checks passed with both probe error fields 0;
only requester PID 3 was selected for fatal retirement.

The last two test-only failures were stale instantaneous-state
assumptions: during slow F07 swapping INIT and gateway waiter PID 4
can legitimately be RUN at physical OOM, even though the latter had
already entered the genuine serialized FS gateway wait. Testkit v1263
and v1264 distinguish this from a wrong SLEEP event, preserve strict
standalone v1159/v1150 behavior, assemble under native PDP-6 DAS and
pass six negative mutations each. Testkit commits `4eaa329` and
`bb2d35d` are pushed; batch TERM is the opt-in v1262 fixture.

Evidence `$TMPDIR/daimos-f07-v1264-acceptance-r1.tar.gz`, SHA256
`0fc008be42997a81138f4513d603bb828a0a978903ef60d1c761dd08e2068fea`.
The testkit v1207/v1262 full-runner changes remain uncommitted and
separate from the accepted diagnostic commits. pdp10-doc `f6a7f46`
documents the verified boundary. The complete F07 gate remains OPEN
pending 64-slot/combined high-descriptor-pressure acceptance and
test-runner integration; E13/F11 also OPEN, 59/64 gates; production
requester OOM remains OFF. Unaccepted swap experiments remain separate.

## 2026-11-06 v1260: reject impossible physical APR before swap/reclaim

The full 256K F07 request is `BRK 0776000` octal (261120 words).
A read-only v1254 SIMH saved-machine inspection showed exactly one
managed MM arena: base `047445`, length `0730333` words, with at most
`0730000` words available from an aligned 02000-word start. A user APR
of `0776000` words can NEVER fit this arena, even after every other
process is swapped out. The extended v1259 2000s diagnostic guest
printed `&` before entering the real requester BRK but no `=` exit
marker, proving the prolonged delay is within the allocation call,
not loss of the parent release PIPE notification.

v1260 fixes the actual allocator contract: `mm_arena_fits_v1260`
checks each physical arena's *aligned* capacity without allocating or
swapping. `mm_alloc_aligned` invokes it only after the underlying
no-reclaim allocator returns a typed pressure failure, before any
compaction or victim exchange. The PDP-6 BRK growth path additionally
uses it before its earlier adjacent-gap reclamation. An impossible
request returns real MM NOMEM (-1), not invalid (-4), preserving
requester-only OOM cause/ABI with no process/APR/descriptor mutation.
The check adds instructions but no permanent data words.

Native PDP-6 v1260 arena/allocator tests PASS actual alignment boundaries,
multiple arenas, typed validation, ordinary feasible reclaim, no victim
work for impossible requests and two failing historical mutants. A
separate extracted full-`vm_space_brk_current` native test passes -1/-1
return, unchanged VM and saved BRK, zero reclaim/allocation calls,
unchanged-extent update and below-floor -4; two regression mutants
are rejected. Both changed assembler files assemble cleanly. DAIMOS
`e7fdbb0e`, testkit `c06f217` and `4bacea0` are pushed.

The fresh 256K/18-child KCC DSK/D6FS production-OOM-OFF v1260 release
media guest is running with diagnostic `&` / `=` requester markers and
the deterministic holder test fixture; do NOT claim F07 acceptance
without its full RUN/swap/WAIT/MM cleanup evidence. 59/64 checks,
F07/E13/F11 OPEN, production requester OOM OFF. Preserve experimental
v1208/v1211 swap work separately.

## 2026-11-06 v1258/v1259: real provider holder, requester still delayed

Fresh 256K KCC DSK/D6FS production-OOM-disabled F07 v1258 admitted all
18 additional RUN children and reached `^@%*`. At the controlled stop
after approximately 14m42s, no requester WAIT or extra-child reaping
had occurred. The intended foreign FS holder PID 6 had acquired the
real lease (`holder_enter=1`, `holder_lease=000007000000`); PID 4 had
contended (`predispatch=1`), but requester `real_oom=0` and
`ordinary_wait=0`. Thus nondeterministic lease ordering is not by
itself the remaining F07 blocker. Native swap transactions continued:
32 successful swap-outs, 20 swap-ins and 33 exchanges, with zero
recorded swap-in or swap-out failure branches. However, spinner PID 5
was dispatched 2967 times against requester PID 3 only 27 times and
gateway PID 4 three times. The actual cause of the remaining requester
delay requires further user-stage/scheduler evidence; do not invent a
disk timeout or enable OOM based on this partial observation.

Evidence: `$TMPDIR/daimos-f07-v1258-snapshot-r1.tar.gz`, SHA256
`8229ff20d0bd4271f783582554c4c32759c252ec981acb3283ea8eca912bfec1`.
The v1238 process descriptor snapshot is from the last swap-service
copy, not necessarily the live process state at SIGINT. Full native
transfers, scheduler locality, and provider-order injection tests PASS.

Testkit v1258 (`fbffb63`) and v1259 (`50b73e0`) use a disposable
gateway/holder event handshake rather than trusting the two historical
sleep delays to establish ownership. v1259 triggers the gate from PID 4's
own post-start sleep, eliminating a remaining INIT-arming race. No
production filesystem lock or scheduler code is modified by this test
fixture. The v1259 injected full guest assembled, and native source
checks/negative controls PASS. A separate disposable requester tracer
records `&` immediately before physical BRK and `=` on its return;
the full guest acceptance remains open. The earlier v1256 comparison
also reached the held provider but not requester OOM. F07/E13/F11 OPEN,
59/64 OOM gates, production requester OOM OFF; v1208/v1211 swap
experiments remain uncommitted and unaccepted.

## 2026-11-06 v1256-r1/v1259: full guest still slow after resident tie

The uninstrumented 256K PDP-6 KCC DSK/D6FS release-media v1256
candidate (built before the v1257 u-area defensive guard and v1258
fixture-ordering correction) admitted all 18 children and reached
`^@%*`, including three subsequent INIT sleeps, but did not complete
the requester WAIT or any extra child reaps after 14m38s of SIMH
runtime. A deliberate SIGINT before its 900-second test cutoff preserved
all native stop diagnostics. PID 6 acquired the intended filesystem
lease (`holder_enter=1`, saved lease `7,,0`), PID 4 reached the
foreign gateway wait, but no real requester BRK OOM had yet been
observed (`real_oom=0`; its owner-controlled lease release also stayed
zero). 32 swap-outs, 24 exchanges and 20 ordinary swap-ins completed;
there was no recorded swap failure branch. Clock/timer and swap request
remained active. This is an incomplete F07 test, not proof of a hung
single disk I/O or a completed physical OOM.

Evidence: `$TMPDIR/daimos-f07-v1256-incomplete-evidence-r1.tar.gz`,
SHA256 `0cd220a8aefc5d988a7979e7edb10984a8961912226a0ebe8f5c89ce018e725b`.
The next isolated v1258 guest is exercising deterministic PID 6
provider ordering, testkit `fbffb63`. Testkit v1259 (`85c3836`)
adds opt-in user-side `&` (BRK entry) / `=` (BRK return) markers to
distinguish swap/PIPE dispatch delay from a long physical BRK call.
The v1259 instrumented C89 fixture compiles with real PDP-6 GCC and
its negative controls PASS; no kernel tracing state is added.
F07/E13/F11 remain OPEN, 59/64 acceptance, production OOM OFF.

## 2026-11-06 v1254/v1256: equal-score resident dispatch before disk swap

A frozen 256K KCC DSK/D6FS 18-child F07 guest at the requester-release
stage `^@%` proves a scheduling throughput defect: PID 3 (requester)
and PID 4 (filesystem lease owner) are RUN and resident, while the
scheduler requests a disk restore for PID 5 (spinner, VM base zero).
All four runnable slots (1, 3, 4, 5) have identical nice 024, recent
CPU zero, combined score 024; cyclic rank alone preferred swapped PID
5 despite immediately runnable resident work. PIDs 7..24 are actually
SLEEP on the same PIPE event, not a repeated runnable burst. The
provider lease belongs to PID 4, and PID 6 is asleep on its lock event.
This is snapshot evidence, not a successful OOM or WAIT/reap test.

v1256 gives normal nonresident candidates a 0400-octal cyclic-rank
penalty ONLY within the same primary nice+CPU score; strictly better
swapped priorities still win, and a VM=0 fatal-cleanup continuation
is not penalized. It adds no persistent words. Native PDP-6 tests PASS
for resident ties, stronger swapped priority, all-resident/all-disk
round-robin ties, fatal cleanup and a historically failing old mutant.
DAIMOS 51824c41 and testkit 8e480ba are pushed. A fresh uninstrumented
18-child 256K release-media guest is under evaluation; F07/E13/F11
remain OPEN, 59/64 gates, production requester OOM OFF. Existing
uncommitted v1208/v1211 swap experiments remain separate.

The frozen full-machine evidence is at
`$TMPDIR/daimos-f07-v1254-process-snapshot-r2.tar.gz`, SHA256
`dfe1a414cb2fc7ca5a360ae42df003d7cc0ab217f2d184536bb7247d2391ac8d`.

## 2026-11-06 v1253: real PID dispatch versus swap-selection counts

The 256K PDP-6 KCC DSK/D6FS F07 v1253 guest reached all 18 extra
RUN acknowledgments and `^@%*`, but no requester WAIT `?` or child reap.
The native-assembled, testkit-only v1253 probes count actual per-slot
scheduler restorations and normal nonresident swap selections. A
controlled SIMH stop after about 12 guest minutes captured 2,125 INIT
restorations (16 swap selections), 21 requester PID 3 restorations
(seven swap selections) and 3,103 spinner PID 5 restorations (16 swap
selections). Each of the extra PIDs 13..24 recorded exactly 145
restorations and ZERO swap selections; PIDs 7..12 had only one each.
The requester is not completely excluded from the scheduler, but its
dispatch frequency is tiny compared with the spinner. The identical
extra-child counts could reflect repeated PIPE wakeups or incorrect
runnable-state handling, not necessarily a scheduler priority policy.
No relevant fatal invariant or completed genuine BRK OOM was observed.

Evidence: `$TMPDIR/daimos-f07-v1253-dispatch-evidence-r1.tar.gz`,
SHA256 `5a18fec660cb2f955d06ce482fe8edbe44f52128710613e52f40845553066f1a`.
The next diagnostic combines per-slot dispatch counting with the
existing disposable v1238 process-descriptor snapshot to distinguish
RUN/SLEEP state from wakeup churn before any production policy change.
The copied guest still contained the unaccepted v1208/v1211 swap
experiments; F07/E13/F11 remain OPEN, 59/64, production OOM OFF.

## 2026-11-06 v1252-r1: full F07 guest timer recovery, no requester progress

An independent 256K PDP-6 KCC DSK/D6FS, production-OOM-disabled
guest with the first v1252 timer-service correction acknowledged all
18 additional RUN children and progressed through `^@%*`, including
the held-provider release and three later INIT sleep calls. This is
more evidence that the due sleep frontier can now be serviced, but it
is NOT a successful F07 acceptance. After about 14.5 minutes the guest
still had no requester WAIT `?` and no child reaps, so SIMH was stopped
by SIGINT before the 900-second test timeout to retain diagnostics.

The stopped machine had `proc_timer_clock=0146133` octal and
`proc_timer_next=0`: no overdue timer remained. Current slot was 0;
the scheduler requested swap-in of slot 5 (`cursor=0405` octal), while
the runnable-queue head was slot 3 (requester). Swap pressure occurred
42 times, with 32 successful swap-outs, 20 ordinary swap-ins, 32
successful exchanges, nine reclaim fallbacks and ZERO recorded swap-in
or swap-out failure branches. There were 1,800 allocated swap blocks,
46 MM descriptors and a high process slot of 25. The true BRK OOM
snapshot counters remained zero: the requester has not yet reached its
physical allocation fault. These data establish continued swap work,
not a lost disk completion or a proven endless loop.

Evidence: `$TMPDIR/daimos-f07-v1252-fullguest-snapshot-r1.tar.gz`,
SHA256 `71f23face2a5fd0033d716b56fa92a7b8e085bde62dcbcfaecb3e0607f27f175`.
The guest copied the first unconditional v1252 timer-call variant; the
final committed v1252 source additionally skips the call unless DUE.
The copied source also included the still-unaccepted v1208/v1211 swap
experiments. Next F07 work should measure per-slot dispatch and swap
requests, especially whether the runnable requester makes progress;
do not alter policy from an interrupted guest alone. F07/E13/F11 OPEN,
59/64, production requester OOM OFF.

## 2026-11-06 v1252: service expired timers on software PI6 reschedule

F07 v1250/v1251 showed a due process sleep timer remaining pending while
slot-0 swap repeatedly performed synchronous DRM I/O. The normal clock
PI6 scheduling path calls proc_timer_service, but the software PI6
reschedule path (used after slot-0 swap and explicit executive sleeps)
previously called proc_sched_resched_select without first servicing the
due timer frontier. Slot-0 swap can defer the clock scheduling half;
therefore an explicit reschedule must consume an already-due frontier
before selecting another swapped candidate. v1252 adds that call under
an inexpensive DUE-bit test. No new storage and no OOM policy change.

The v1252 native PDP-6 test exercises the actual scheduler/timer/runq
leaves: a due sleeper must become runnable and be queued before
selection; an active but not-due timer must remain unchanged. A
negative mutant removes the software-reschedule timer handling and
reproduces the blocked sleeper. Existing timer frontier (including
wraparound), v884 reaper scheduling and v1247 PI-register tests PASS.
The full 256K 18-child guest test is in progress and has not established
complete RUN/swap/WAIT or provider/MM lifecycle acceptance. F07/E13/F11
remain OPEN, 59/64 OOM gates, production OOM OFF. The experimental
v1208/v1211 swap changes are not accepted by this fix.

## 2026-11-06 v1250/v1251: F07 swap throughput and deferred timer diagnosis

The full 256K KCC DSK/D6FS production-OOM-disabled F07 guest again
admitted all 18 additional RUN children and reached `^@%`, but did
not reach the subsequent `*` sleep milestone before operator stop.
The disposable v1250 slot-0 wait probe sampled the exact active DRM
descriptor `000454047277` and its completion event `047301` (offset
two), which was still zero. The pending DRM descriptor was zero, and
the ordinary storage error state was zero. At the observation,
the last captured wait entry was only two clock ticks old; the active
request still carried the full `0454`-tick watchdog countdown. The
slot-0 wait entry had been reached 7269 times in the same run, with
32 successful swap-outs, 21 swap-ins, nine exchanges, and no recorded
swap-in fail branch. This is evidence for many successive I/O requests,
not proof that a single DRM completion was permanently lost.

The clock had reached `0100245`, while a timer frontier marked due
at `074753` remained pending (1722 ticks, or about 28.7 seconds).
This indicates prolonged deferral of timer processing during slot-0
swap work; it does not yet establish an incorrect I/O completion or
an unbounded scheduler deadlock. Native swap v774/v775/v819/v854/v855/
v856, v1243 canary, and v1247 PI-preservation tests still PASS. The
versioned v1249/v1250 testkit diagnostics and v1251 caller-attribution
correction are committed. The stop was diagnostic only: F07/E13/F11
remain OPEN, 59/64 OOM gates, production requester OOM OFF. Do not
change scheduling priority/timer policy until a complete timed
stress acceptance or stronger state evidence establishes the defect.

## 2026-11-06 v1243: protect u-area swap backing from stack canaries

The stable process u-area has 0410 octal words, with the private PDP-6
PUSH stack based at offset 070. Its last legal pushed word is offset
0405; word 0406 owns the executable backing while swapped, and 0407
owns the process BRK. The previous 0316-word private-stack bound
included word 0406. Worse, the diagnostic PROC_STACK_WATERMARK canary
loops in both native RUN and transient KINIT initialized through 0407,
writing synthetic nonzero addresses into semantic backing/BRK words.

The v1243 correction sets the actual private stack capacity to 0315
octal words in C and PDP-6 assembly, and stops both canary loops at
offset 0406 (exclusive). The native extracted-RUN test verifies that
the first and last stack canaries are present, but saved backing and
BRK remain zero, and rejects the historical upper-bound mutant.
Updated v1239 idle/private exact-boundary tests and the current u-area
layout contract PASS. The first 18-child v1243 guest with fault-owner
diagnostics is underway. F07/E13/F11 remain OPEN, 59/64 gates;
production requester OOM remains OFF. No size reduction is required.

## 2026-11-06 v1240/v1241: swap-exchange saved-backing ownership defect

Full v1240 disposable 256K DSK/D6FS 18-child F07 guest again stopped
after all 18 RUN admissions and the provider-start marker. The v1238
scheduler trace showed millions of genuine swap requests, with PID 1
and PID 5 runnable/nonresident. The v1240 instrumentation proved that
all four swap-service admission rejects were ZERO, both process disk
records were valid/nonzero, but the saved executable-backing words in
their stable u-areas were both ZERO. Thus the repeated swap-in failure
is not a missing scheduler request, invalid process state or BCACHE.

The actual defect is in proc_swap_exchange_records_v786: unlike regular
proc_swap_out, exchange publication previously overwrote A's resident
executable-backing record with its disk span without saving the old
backing in A's stable u-area. It also retained B's consumed saved
backing. v1241 now validates A's old record, both u-area owners and B's
saved backing before publication, transfers A's old backing to its
stable u-area and clears B's. The pre-destructive v797 backing validator
also refuses a nonzero A saved slot or missing resident backing.

The real native PDP-6 v786, v801-v803, v819, v854-v856 transaction tests
PASS with fully owned fixture u-areas. v854 additionally rejects
corrupted A backing before the first destructive disk I/O; the v1241
negative control against v1237 confirms the old path fails the new
backing postcondition. The separate v1241 full 18-child guest is still
under evaluation; do NOT close F07/E13/F11 or enable OOM from native
tests alone. Production requester OOM remains OFF (59/64 acceptance).

## 2026-11-06 v1239: fix verified slot-0 stack overwrite and scan bound

The original 0100-octal (64-word) permanent slot-0 pushdown stack had
overwritten PID 1's process descriptor during the nested swap exchange
call graph (SIMH write watchpoint, v1224). KINIT and native process
assembly now reserve/scan a matching 0400-octal (256-word) idle stack.
The instrumented process-private and idle-stack watermark scans also
stop at their allocation bounds instead of reading capacity+1 when
every canary has been overwritten. Testkit v1239 executes both actual
PDP-6 scanner leaves with a fully touched stack and a partially used
stack; two valid-assembly negative mutants restore the old comparisons
and are rejected. The v1225 diagnostic override and v1207 capacity
verifier accept the new production default. This removes the known
64-word descriptor overwrite, but F07 acceptance still requires an
entire 18-child swap/WAIT cycle and observed highwater below capacity.
E13/F07/F11 remain OPEN, 59/64 OOM gates, production requester OOM OFF.

## 2026-11-06 v1237: isolate slot-0 swap I/O from suspended D6FS reads

The full 256K KCC DSK/D6FS v1236 diagnostic reproduced a zero FCB
metadata word for FCB 037/block 0741 immediately after a successful
filesystem backing read, before BCACHE publication. A direct backing
reread returned the intact REG metadata 100005550020. Slot-0 swap I/O
could reuse fs_block_workspace while the filesystem provider's sleeping
caller still owned its completed DMA data. The v1237 candidate assigns
swap exchange/optional PURE verification an independent 0200-word buffer
inside the removable swap MRES; it does not change the cache policy.

The patched disposable guest admitted all 18 additional RUN children
(the original failure occurred after 9). The same FCB now reads
100005550020 both immediately after I/O and after cache publication;
no zero post-read observation or invalid-FCB rejection was recorded.
Native v774/v775/v854 transfer tests and the v1237 source/negative
control PASS. This is a partial correction only: the 18-child run
subsequently stalled after the provider-start marker, before the next
sleep/reap milestone. The guest was interrupted to preserve diagnostics.
The slot-0 stack is still the original 0100 words in production; the
guest used the diagnostic-only 0400-word stack. F07/E13/F11 remain
OPEN, 59/64 OOM gates; production requester OOM remains OFF.

## 2026-11-06 v1232-v1234: F07 D6FS in-memory FCB divergence (OPEN)

Full disposable 256K PDP-6 guest v1232 proves the tenth RUN's
D6FS payload failure comes *after* a successful FCB block fetch
and FCB decode, at the provider's REG/SYMLINK type gate. v1233
shows a raw FREE FCB (META 0, type 0, size 0) for executable FCB
`037` in block `0741` octal. In contrast, read-only inspection of
the very same DSK set (validated CLEAN by d6fsck) shows on-disk
FCB META `100005550020` octal, type REG (1), at physical member 1
sector `0171`, FCB offset `0160`. Thus guest memory/cached content
diverges from intact backing media. A disposable v1234 BCACHE and
shared-workspace probe is being qualified; do not assume media
corruption, alter the production filesystem or accept F07 without
identifying the bad data path and rerunning independent 256K guests.
Production requester OOM remains OFF; F07/E13/F11 OPEN, 59/64.

## 2026-11-06 v1232-v1234: D6FS cached FCB differs from backing (F07 OPEN)

Fresh disposable 256K PDP-6 KCC DSK/D6FS guest v1232 showed that
the tenth extra RUN fails its payload read because a validly indexed
(`037` octal, limit `0550`) and decoded D6FS FCB has type FREE (0).
There was no FCB block-fetch, decode, or physical-block failure.
The v1233 guest independently captured cached FCB metadata
`000000000000` while a **direct successful backing read of the same
FCB** returned `100005550020` (FCB block `0741` octal). This proves
the D6FS cached/workspace view is wrong, not that its on-disk FCB
was freed or overwritten. v1234 test-only BCACHE hit-source tracing
is being evaluated to isolate slot 0 workspace versus slot 1 slab.
Do not add a disk reread fallback to production: fix the actual
cache ownership/invalidation defect, retaining two-block low-RAM
cache policy. Original production idle stack remains 64 words and
production requester OOM remains OFF; F07/E13/F11 OPEN, 59/64.

## 2026-11-06 v1228-v1231: F07 loader/D6FS failure isolated (OPEN)

With the disposable 0400-octal (256-word) idle-stack upper-bound,
the 256K guest acknowledges 9/18 additional RUN children before a
different failure. v1228 identifies `exec_load_process=-1`, slot 020;
v1229 identifies loader phase 6 (payload read after VM creation).
v1230 records D6FS `vfs_read_words=-1`, with vnode `060101000037`,
offset 3, destination `0734020`, `01572` (= 890) words requested.
v1231 reaches the D6FS provider FCB/type gate and returns `-1`
there; its reader loop and actual block-I/O-failure counters are
both zero. Do not diagnose a physical media error from this trace.
Next distinguish FCB block fetch from FCB decode/type validation
under load and correct the responsible violation. See testkit STATUS.MD
for raw diagnostic values and logs. The production idle stack remains
64 words; the 256-word setting is disposable, not an accepted budget.
F07/E13/F11 OPEN, 59/64, requester OOM OFF.

## 2026-11-06 v1224-v1227: F07 real idle-stack overrun (OPEN)

A fresh v1224 PDP-6 guest, with a SIMH hardware data-write breakpoint
armed after `vm_enter_initial_user`, stopped at runtime PC `033637`:
the second `PUSH 17,011` in the swap MRES's
`proc_swap_exchange_pair_v826` (relative offset `000011`) wrote value
`3` over PID 1's descriptor meta at physical `047062`. The previous
four modifications to the same word only toggled TRANSITION correctly.
This proves a slot-0 stack overwrite, rather than the conjectured
release/teardown or MM descriptor deletion. KINIT reserves only `0100`
octal = 64 words for its permanent pinned idle stack, allocated before
the process table. A disposable v1225 trial with `0140` octal (96)
stack words instead reached the swap transaction's fail-stop invariant:
v1226 CPU history showed the incoming swap partner's VM field
unexpectedly held `340000033445` at the commit check. A 52-word
transaction graph in the service call increases the worst-case stack
depth. The larger `0400` octal (256-word) trial v1227 is diagnostic
only; select the smallest safe permanent capacity after a complete
resident/nonresident/WAIT peak and watermark evidence, not by guess.
Uncommitted source still retains the original production 64-word stack;
requester OOM remains OFF. F07/E13/F11 OPEN, 59/64 gates.

## 2026-11-06 v1219-v1220: F07 swap-pressure root-cause investigation (OPEN)

Full 256K PDP-6 guest diagnostics (not accepted F07 regression) show
genuine PID 1 swap-in `MM_ERR_FRAGMENTED` (-2): 18432 APR words
requested, 28221 free but no LOW/HIGH aligned fit, 35 MM descriptors
(v1219). Independently rebuilt v1220 reproduced -2 with 28204 free.
Afterward the original fatal owner branch observes PID 1 still RUN,
but descriptor `meta=3`, `vm=1`, and invalid `file_table=026`.
The disposable-only probe counted **zero `proc_uarea_release` calls**;
this u-area disappearance is not a normal release. v1218 had seen
the owner descriptor intact before/after disk swap calls. The exact
writer of the corrupt descriptor or table pointer remains unknown.
Experimental v1208 slot-0 reclaim and v1211 nonrecursive swap-in are
uncommitted and NOT accepted; source-linked checks alone cannot prove
safe kernel behavior. Refer to daimos-testkit STATUS.MD for raw log
names. Do not activate production requester OOM. F07/E13/F11 OPEN,
59/64 gates; next isolate descriptor/table writes and retest fresh
release-media guests with enabled and disabled OOM.

## 2026-11-06 v1205: 12 genuine concurrent user BRK/APR/MM extents PASS (partial F07)

Frozen DAIMOS `684d4504`, testkit `f2222e2`; two independent
256K KCC DSK/D6FS OOM policy guests PASS physical BRK fault under
held foreign provider, with **12 actual resident process images**
(six extra RUN children), individual u-area/BRK snapshots, and
one-to-one exact MM owner/APR physical extent reconciliation.
Both have 231012 managed allocated words (95.3% physical occupancy),
31/136 MM descriptors and **5499 actual logical BRK padding words**.
Enabled: 242427 managed, 11415 free; disabled: 242441 managed,
11429 free. All child TERM/WAIT, FS-provider lease and protected
owner/reaper checks PASS; source production OOM remains OFF.
Testkit rejects five malformed process/BRK/MM fixtures.

See `design/F07-REAL-PROCESS-BRK-ACCEPTANCE-V1205.md` and archived
full SIMH/build/maps/checksum evidence at
`$TMPDIR/daimos-f07-real-mm-v1205r4.tar.gz` (SHA256
`db2a1e1069fe6b008aa9ef85d4dc79cd953ce47a17a2b30eac9aad0547b44694`).
Next combine real concurrent-process pressure with near-full MM
descriptor occupancy, then qualify 64-slot/swap and stack budget.
**F07 OPEN, E13/F11 OPEN, 59/64 gates, production OOM OFF.**

## 2026-11-06 v1204: high-load F07 physical MM/provider test PASS (partial)

Frozen DAIMOS `75ccef85`, committed testkit `aa7cb86`: both
independently rebuilt 256K KCC DSK/D6FS full guest variants PASS
real physical BRK OOM with the foreign provider held after adding
12 APR-aligned 8K-word and 96 single-word MM allocations in the
disposable kernel. At the fault: **211092/242518** managed physical
words allocated (enabled; disabled 242532 managed), **127/136**
MM descriptors, six real process extents, 3 free gaps and the original
128-word shared transfer workspace. All 108 test-owned extents were
released exactly once after the original protected-owner and WAIT
tests completed. No non-requester retirement, wrong-owner release,
unexpected MM error or provider-lease theft. Host audit rejects 17 negative
mutants (v1203+v1204). Low APR placement slack measured 706/720 words.

F07 verifier bug fixed: alignment is `02000` octal = 1024 words,
so the per-gap and per-process conservative waste bound is 1023,
**not 511** (corrected v1203 historical report; raw old logs unchanged).
Evidence `$TMPDIR/daimos-f07-high-mm-v1204r2.tar.gz`, SHA256
`00bb9942c78455d616160649b77756823b3715b74ba14fbe5e1dc73aeb91513d`.
See `design/F07-HIGH-PRESSURE-MM-ACCEPTANCE-V1204.md` for exact
metrics, full provenance and limitations. High MM occupancy is now
proven, but maximum real process-slot concurrency and per-process
logical BRK rounding are not yet measured; **F07 remains OPEN**.
E13/F11 remain OPEN; acceptance 59/64, production requester OOM OFF.

## 2026-11-06 v1203: simultaneous F07 physical MM/provider sample PASS (partial)

On DAIMOS `cf4e85d4` and committed testkit `aae6d2d`, independent
256K KCC DSK/D6FS disabled and disposable-enabled physical-OOM
guests PASS live read-only MM-table capture while a foreign FS owner
holds the provider. Both show 19/136 descriptors (six APR process
extents, 13 kernel-dynamic), 112692 managed allocated words, three
free gaps and the existing 128-word shared workspace. Enabled
guest free/managed is 130022/242714 words (disabled 130036/242728);
LOW APR gap-prefix slack 990/1012 words. v1150/v1159 protected
owner/scheduler invariants, 10 host negative mutants, full media/
build/tool checksum checks and exact-source provenance PASS.

Evidence: `$TMPDIR/daimos-f07-oom-lease-v1203r2.tar.gz`, SHA256
`e4fdbd54d04a1052ffec87a621d2abc53840f1f76712e71bf76cca6c71bac0ff`.
Full definitions and limits: `design/F07-LIVE-OOM-MM-PEAK-AUDIT-V1203.md`.
This is one genuine simultaneous fault snapshot, **not** peak-load
qualification: approximately 130K words and 117 MM descriptors
remained free. F07 OPEN, A04/F02 remain closed, checklist 59/64,
production OOM OFF; no production code/RAM/ABI changes.

## 2026-11-05 v1202: A04/F02 full frozen P0-D matrix PASS

DAIMOS `5db15ecf` with testkit `122c61f` completed one uninterrupted
`P0D_FULL=1` run, exit 0: all 18 component checks, v1083/v1087
allocation-free release graph, and all 17 booted PDP-6 256K KCC
DSK/D6FS resource-owner tests PASS. Unique full-script stage/guest
markers and both physical-OOM PID-reuse policy variants were checked.
Log, revisions and SHA256 manifest are preserved at
`$TMPDIR/daimos-a04-final-v1202.tar.gz`; detailed evidence in
`design/A04-F02-FULL-MATRIX-ACCEPTANCE-V1202.md`.

**A04 and F02 CLOSED** for the frozen 256K writable-process profile;
mandatory checklist **59/64 complete, 5 open** (E13/F07/F11 before
activation, deferred F12/F13). This is not full release signoff:
independent simulator/media coverage, combined dynamic peak budget,
and F11 remain outstanding. Production OOM remains OFF. No production
kernel, permanent memory, or ABI changes.

## 2026-11-05 v1201: all individual A04 P0-D booted rows now PASS

On DAIMOS `d9aedf20` and testkit `122c61f`, v1120 OOM disabled
and isolated enabled (24 numeric PID reuse cycles in each), v1106
physical VM MM_BUSY, v1107 disk bitmap refusal, v1077 D6FS SYNC
and physical OOM, and v1101 permanent wrong-owner -4 all PASS.
Complete v1201 logs/revision/sha256 manifest are archived at
`$TMPDIR/daimos-a04-evidence-v1201.tar.gz`; SHA256 and failure
counters in `design/A04-P0D-BOOTED-PROGRESS-V1201.md`.

With the earlier v1197-v1200 tests, all 17 individual `P0D_FULL=1`
booted invocations have now passed in staggered batches; the 18
component/source-linked P0-D checks also PASS. **This is not an
uninterrupted exact-revision full acceptance matrix**. A04/F02 and
final diagnostics/size signoff remain OPEN; 57/64 mandatory OOM
checks complete, production requester OOM disabled. No production
kernel/MRES code, RAM or ABI changed.

## 2026-11-05 v1200: A04 PIPE/DPY/swap full guests PASS

On source `08bf7427` and testkit `122c61f`, v1119 combined
PIPE/DPY HUP, v1121 DPY MM_BUSY with genuine disk swap-out/in,
and v1122 PIPE MM_BUSY followed by physical disk-bitmap refusal
all PASS. Exact owner/WAIT/TTY release and zero guest errors.
Full logs and manifest in `$TMPDIR/daimos-a04-evidence-v1200.tar.gz`;
`design/A04-P0D-BOOTED-PROGRESS-V1200.md` records provenance and
checksum. A04/F02 OPEN pending remaining guests and full frozen
matrix; 57/64, production OOM OFF, no production code changes.

## 2026-11-05 v1199: four more A04 full-system P0-D guests PASS

On DAIMOS `3eb2d0b3` and testkit `122c61f`, four individually
rebuilt 256K PDP-6 DSK/D6FS full guests PASS: v1117 mixed fatal
teardown and 48 stale WAIT/PID/event refusals; v1118 real PIPE+VM
MM_BUSY with exact physical MM/swap recovery; v1104 pinned PIPE
with HUP/TTY handoff; and v1105 pinned Type-340 DPY MM_BUSY with
PI6 refresh and deferred u-area/WAIT. All preserve owners and
finish with zero test errors. Full outputs/manifest archived at
`$TMPDIR/daimos-a04-evidence-v1199.tar.gz`; SHA256 and details in
`design/A04-P0D-BOOTED-PROGRESS-V1199.md`. Further P0D_FULL
rows and final exact-revision full signoff remain mandatory.
A04/F02 OPEN, OOM disabled, checklist 57/64 unchanged.

## 2026-11-05 v1198: three additional current-source A04 physical guests PASS

On frozen DAIMOS `22df1a0c` and testkit `122c61f`, isolated full
256K PDP-6 physical guests v1124 (TTY/VM/u-area MM_BUSY), v997
(pre-VM and post-VM release retries with a held foreign provider),
and v1103 (combined PIPE/HUP/VM/u-area teardown) all PASS with
correct original ownership, final EXIT/WAIT and zero guest errors.
The three raw logs, SHA256 manifest and full revisions are archived
at `$TMPDIR/daimos-a04-evidence-v1198.tar.gz`; see
`design/A04-P0D-BOOTED-PROGRESS-V1198.md`. This does not complete
`P0D_FULL=1`: the remaining guests and exact final-candidate
review are required. A04/F02 OPEN, 57/64 checklist, production OOM OFF.

## 2026-11-05 v1197: partial current-source A04 full-guest matrix

DAIMOS `d3e02fdc` and testkit `122c61f` completed all 18 P0-D
component tests plus v1083/v1087 source-linked release audits under
`P0D_FULL=1`. The first full guest (v1125 real permanent wrong-owner
MM -4 with held foreign FS lease) PASSed. The second guest (v1124)
was interrupted during isolated build before guest evaluation. The
full matrix is **not** certified and A04/F02 remain OPEN. Details,
run command and SHA256 of partial log: `design/A04-P0D-FULL-PARTIAL-V1197.md`.
No production code or OOM policy change; checklist remains 57/64.

## 2026-11-05 v1196: real F07 descriptor/fragmentation stress PASS

Fresh disposable 256K/full DSK/D6FS PDP-6 OOM-enabled guests PASS
testkit v1114 (fill the real live 136-descriptor MM table, refuse
new RUN as descriptor shortage without killing its parent, release
every temporary MM extent) and v1115 (aligned 16K-word extent
pressure, aggregate-free-versus-largest-gap 64K check, BRK return
failure with preserved physical MM cause `-2`, never trigger fatal
requester-only OOM, exact cleanup). Baseline v1075 physical OOM,
v1110 invalid BRK and v1112 failed child RUN also PASS in each
guest. Both process and idle stack watermarks PASS on current
rebuild; current P0-D component source/negative matrix also PASS.

Frozen run outputs and host-tool checksums in
`$TMPDIR/daimos-f07-mm-evidence-v1196.tar.gz` SHA256
`77f7f35141dc1aef84ed0a294b80d6521be8f42de77376f14993891bf54297a8`.
This is **partial** F07 coverage: a 136-slot pressure boundary and
APR-aligned fragmentation are exercised, but a complete peak
allocation/alignment-slack/provider-buffer budget under combined
maximum load is not yet measured. F07/A04/E13/F02/F11 remain OPEN,
production requester OOM OFF; overall count **57/64** unchanged.

## 2026-11-05 v1195: current-source P0-D and stack baseline PASS

On DAIMOS `bcdafeea` with v1194 corrected AUXSTORE converter,
the testkit v1096 component-only P0-D release matrix PASSes all
18 source-linked/native leaves plus v1083 direct release and v1087
transitive allocation-free BRK/APR/EXEC release graph. The run
explicitly did **not** set `P0D_FULL=1`; A04/F02 remain OPEN.

The current-source process-private kernel stack test PASSes with
`0300` octal highwater below the `0310` warning limit (`0316`
capacity). The corrected-media idle-stack guest PASSes with `025`
octal highwater below `060` limit. The first idle attempt failed
at DBOOT -> AUXSTORE conversion, fixed and reverified by v1194.
F07 remains OPEN for complete MM extent/descriptor, APR-alignment
and shared-provider-buffer peak coverage; these stack passes are
only bounded baseline evidence. OOM remains OFF; checklist 57/64.

## 2026-11-05 v1194: AUXSTORE conversion accepts explicit DBOOT marker

The v1192 DBOOT explicit-layout word `020` exposed a strict-legacy
assumption in `mkauxstore --convert-dboot`: it refused the now-valid
flag `1` when converting the deliberately empty auxiliary-drum D6FS
precursor to AUXST1. Its exact-match safety check now allows only
flag `0` (historical) or `1` (explicit reserved ranges) while
retaining the requirement that all other descriptor metadata
outside the expected DBOOT header be zero. Unknown and 36-bit high
flag values remain destructive-conversion refusals.

Testkit v638 conversion and v636 safety PASS with both flag modes,
read-only dry-run preservation, image-byte preservation on refusal
and original filesystem payload preservation on successful conversion.
The corrected host utility is installed under `$PDP10_PREFIX/bin`.
This is a prerequisite regression fix for new v1192 media and does
not change OOM policy, allocator or KCORE/MRES residency.

## 2026-11-05 v1192: D6FSR2 explicit zero-reservation layout fixed

DBOOT's historical D6FSR2 extension had no way to distinguish an
intentionally empty modern set of bootstream/LOGSTORE/BADMAP
reservations from old all-zero legacy metadata. `mkdsk --d6fs-layout`
now writes DBOOT word `020` = 1 to assert explicit reserved ranges
even when all lengths are zero. The host `mkd6fs` and `d6fsck`
readers and the KINIT BLOCKSET decoder accept flag 1, retain the
flag-0 legacy inference, reject all other 36-bit flags and reject
mixed flags across members. This fixes the spurious warning while
creating an auxiliary-drum D6FS precursor prior to AUXSTORE conversion
without disabling the warning for genuine old images.

Testkit `test-daimos-d6fs-explicit-zero-layout-v1192` PASSes modern
zero ranges, historic legacy warning, unknown/high flags and mixed
member negative cases with strict-warning host-tool builds. KINIT
BLOCKSET decoder strict compile check PASS. The independent v1190
OOM diagnostics auditor now tolerates the warning on archived legacy
images and its absence on modern images; four archived release-media
profiles and diagnostic-negative mutants still PASS. No OOM policy,
KCORE residency or MRES lifetime changes; requester OOM remains OFF.

## 2026-11-05 v1190: F10 OOM build diagnostic audit CLOSED

The testkit v1190 audit of complete release-media build logs for
Android/aarch64 and Linux/x86-64, both disabled and isolated
test-enabled OOM, PASSes four profiles with the same 13 explicitly
catalogued diagnostic classes and zero unexplained compiler/assembler
diagnostics. Three negative warning/error mutations are rejected.
Known legacy KCC identifier/unused notes and D6FSR2 layout warning
are *reported*, not silenced; actual D6FS checker reports CLEAN.
No production source or OOM switch changed. F10 is CLOSED for
this audited build set; A04/F02/F11 must recheck the exact final
candidate and F07 must retain independent dynamic peak evidence.

With F05 out of OOM scope, **57/64 complete, 7 open** (5 pre-enable:
A04/E13/F02/F07/F11; plus deferred F12/F13). Production OOM OFF.

## 2026-11-05 v1189: F05 retired from requester-OOM acceptance

Per release-scope decision, the static 16K permanent KCORE/MRES
address-space budget is **not an OOM acceptance target**. Remove
F05 from the mandatory requester-OOM gate denominator, without
marking it as passed. The measured 2717-word 16K overflow in the
instrumented 256K guest remains a real, separately tracked
KCORE/MRES size-optimization issue; no fit or RAM saving is claimed.
OOM-specific *dynamic* physical MM extent/descriptor, APR-alignment,
stack and provider-buffer peak safety remains mandatory under F07.

Revised OOM checklist: **56/64 complete, 8 open** (6 pre-enable:
A04/E13/F02/F07/F10/F11; F12/F13
are deferred activation/post-activation). Production requester
OOM remains OFF.

## 2026-11-05 v1188: F06 permanent MRES inventory audit CLOSED

The v1174/v1176 identical-source Linux/Android 256K DSK/D6FS
cross-platform evidence verifies the live SYS_MEMINFO resident count
against KINIT's patched instruction, 136 allocated MM descriptor
slots (272 permanent words), and the sum of precisely 22 boot-
installed MRES packages. Android MRES is 8837 words; Linux is 8839.
In the disabled *instrumented* Android guest, 9944 KCORE + 272
MM table + 8837 installed MRES = 19053 words, with exclusive last
address 045235; the octal 040000 permanent-address budget is
exceeded by 2717 words. Test-enabled physical OOM adds 14 KCORE
words, increasing the shortfall to 2731. Package inventory and
KINIT's actual installed set are **not** interchangeable; none
of these MRES objects is dynamically unloadable or OOM reclaimable.

**F06 CLOSED as a measured occupancy/reconciliation task only.**
F05 is explicitly **OPEN and failing the 16K memory requirement**
until an optimized, uninstrumented full target meets the budget;
the present booted 256K test image is functional but cannot
constitute a 16K-fit acceptance. This does not enable production
OOM. Total mandatory checklist **56/65 complete, 9 open**
(7 pre-enable plus F12/F13).

## 2026-11-05 v1187: F08 deferred 32K separation CLOSED

F08 formally records that passing 256K/full KCC DSK/D6FS OOM
guests is **not** evidence for a strict 32K KCORE/DSH boot.
That optimization is separate, deferred until after requester-OOM
release acceptance; its size/boot results remain unclaimed. The
independent permanent 16K executive/MRES limit remains mandatory
for the selected OOM profile and F05/F06 are still OPEN.
**55/65 checks complete, 10 open (8 pre-enable plus F12/F13).**
No code or production OOM policy change.

## 2026-11-05 v1185: F04 exact errors and F09 operator policy CLOSED

F04 PASS: on the same DAIMOS `96e6c695` and corrected KCC
`b9cd764` release source, the full F03 -2/-3/invalid and requester
OOM pair is supplemented by eight fresh Linux tests. Actual
backing exhaustion leaves the BRK requester alive (v1156 on/off),
RUN's independent parent survives descriptor -3 and separately
triggered physical requester OOM (v1158 on/off), v1113 validates
real -2/-3 and rejects three unsafe mutants, v1106 checks actual
VM/u-area MM_BUSY retry, v1077 exercises physical OOM under D6FS
SYNC/MM_BUSY contention, and v1157 validates APR postcommit EXEC
and unfinished child RUN cleanup under a foreign FS lease. All eight
additional tests PASS; Linux log archive
`$HOME/tmp/daimos-f04-cause-evidence-v1184.tar.gz` SHA256
`5c12dce02fe1877a47c19afa9e9339e1a466a39f79526d1960617a048ee69096`.
Only normal writable process VM is covered, not optional raw XIP.

F09 CLOSED: `pdp10-doc/KERNEL/OOM-FAIL-CLOSED-RECOVERY-V1185.MD`
documents owner-private provider failure, HUP/TTY/PIPE/DPY refusal,
VM/u-area MM_BUSY, permanent wrong-owner -4 quarantine, backing
bitmap/media refusal, boot-time MRES installation failure and
impossible-owner HALT, together with read-only TD/RETRY/DETAIL
diagnostics and permissible owner-side recovery or controlled
restart. No force unlock, fabricated WAIT, speculative free,
or runtime MRES loading/unloading is permitted. This documents
known noncompletion without promising bounded cleanup.

**54/65 mandatory checks now complete, 11 open (9 pre-enable plus
F12/F13).** Full Linux P0-D matrix with corrected KCC is running
for F02; the earlier Android full candidate matrix exceeded its
execution timeout after many PASS stages and is not counted as a
completed run. F05/F06 16K residency remains unresolved. The
production requester OOM switch is still OFF; the test/compiler/
documentation commits are retained locally because remote GitHub
SSH currently lacks permission to push to theseus-chimaira.

## 2026-11-05 v1181: F03 complete; production requester OOM still OFF

The Linux 256K/full PDP-6 OOM acceptance matrix now PASSes all 14
positive/negative full-system combinations for v1075, v1110, v1112,
v1114, v1115, v1116 and v1120, each with production-disabled and
isolated test-enabled requester OOM. Provenance: DAIMOS `96e6c695`,
KCC `b9cd764` (live preserved-register peephole repair), Open SIMH
`edd19b17`. The test-only v1114 descriptor cap was corrected from
32 to 136 entries after the compiler fix exposed the obsolete limit;
its disabled and enabled reruns PASS, and actual MM cause -3 and
fragmentation cause -2 are separately verified. All 14 individual
logs, SHA256 manifest and toolchain/source hashes are archived at
Linux `$HOME/tmp/daimos-f03-verified-evidence-v1181.tar.gz`
(SHA256 `dab45fce94cd6350bfb32e7b1683010f85bb7d93e4a6fd60b4996d17d49bb6bc`).

**F03 CLOSED: 52/65 checklist checks complete, 13 open (11 pre-enable
plus F12/F13).** New testkit v1174-v1176 independently verifies the
runtime SYS_MEMINFO resident immediate, actual MM descriptor capacity
and boot-installed MRES package sum. Android and Linux test archives
from the same DAIMOS `96e6c695` source PASS the v1163 cross-platform
E13 source/counter checker. E13 remains OPEN for remaining required
platform/media coverage. The full booted P0-D matrix PASSed on Linux;
F02 final acceptance remains OPEN until exact-candidate signoff.
The production KCORE/MRES 16K memory budget remains an F05/F06 blocker
and may NOT be relaxed by redefining boot-installed MRES as dynamically
reclaimable. MRES is always installed during KINIT, permanent and
unloadable. KCC/testkit commits are present locally; upstream GitHub
push currently fails due the available SSH identity lacking write
permission. Production `PROC_OOM_BRK_ENABLED_V1000,0` unchanged.

## 2026-11-05 v1163: E13 Linux x86_64 cross-platform evidence PASS

The second independent PDP-6 simulator target, Linux x86_64 Open SIMH
V4.1-0 build `a6b511e5`, now PASSes two fresh v1160 256K KCC DSK270/
D6FS release-media guests on DAIMOS `90dfc8d0`: default-off returns
BRK -1 without fatal selection; isolated enabled selects only the
genuinely exhausted requester slot 3. Both preserve INIT, independent
spinner, blocked witness, foreign gateway waiter and original in-flight
provider owner through the shortage; original-owner lease release,
WAIT/FILE/MM accounting, 160 spinner advances and zero errors PASS.
Linux archived complete SIMH, boot, build, maps and counter evidence
in `$HOME/tmp/daimos-e13-release-media-v1160.wPB6WE.tar.gz` (SHA256
`b80eb98991845b89fb56344cb621c22eb3e8fd79aff185912163c5406f3e86fd`).
A verified copy is at Android `$TMPDIR/daimos-e13-linux-x86_64-v1163.tar.gz`.

New testkit v1163 cross-platform audit PASSes both Linux guests against
the previous Android aarch64 Open SIMH `edd19b17` v1160 archive. Both
hosts match instrumented KCORE low endpoints: OOM off `023410`, test-on
`023426` octal. These are NOT uninstrumented permanent KCORE/MRES size
measurements. The Android earlier archive used DAIMOS `27acb950`, so
the revision pair is *not* an exact final-candidate E13 signoff.

The Linux release-media build first uncovered a stale installed host
formatter and a genuine strict-C99 portability defect: `fileno`,
`fdopen` and `lstat` were implicitly declared without a POSIX feature
test. DAIMOS `90dfc8d0` adds `_POSIX_C_SOURCE 200809L` before headers
in `tools/host/mkauxstore.c`. Clean C99 builds and v636 destructive
format safety plus v638 DBOOT conversion PASS on Android and Linux;
the current formatter is installed at Linux `/usr/local/bin` with
the earlier binary backed up. No production KCORE/MRES or ABI change.

**E13 still OPEN** pending exact revision retest on all required
simulator/media targets (OpenBSD SSH currently unavailable). Linux
KCC notes about long identifiers/unused symbols and a legacy D6FS
layout warning remain F10 review items. The current full booted
P0-D matrix is also ongoing, not accepted. Mandatory checks stay
**51/65 complete, 14 open** (12 pre-enable plus F12/F13), OOM OFF.

## 2026-11-05 v1161: A04/P0-D component preflight PASS, final gate open

On frozen first-profile DAIMOS `fe0f3c0e`, testkit
`test-daimos-p0d-release-matrix-v1096` PASSes 18 real native and
source-linked PIPE, DPY, SWAP, HUP, VM, TTY, u-area, PREVM, SIDL,
XIP and VM_NONE error/release tests. The suite also reruns v1083
direct fatal-release scope and v1087 allocation-free transitive
BRK/APR/EXEC -> FS/FILE/HUP -> MM/SWAP/WAIT graph, both PASS.
Negative mutation checks reject unsafe ownership/release shortcuts.
The test explicitly skipped its `P0D_FULL=1` 256K booted matrix.

This verifies useful A04 preflight evidence on the frozen candidate
but does NOT close A04 or F02. Their final full booted release
regressions and exact revision signoff remain open. The completed
checklist remains 51/65 (12 pre-enable plus F12/F13 open), and
production requester-OOM remains OFF. No kernel changes made.

## 2026-11-05 v1160: S07/S08 scope frozen; E13 DSK/D6FS platform PASS

The first bounded requester-only BRK OOM release-candidate profile is
frozen in `pdp10-doc/KERNEL/OOM-RELEASE-PROFILE-V1160.MD`: full 256K
PDP-6 with KCC, 60 Hz PCLK, DSK270 Stage 1, `ROOT=disk`/`ROOTSET=1`,
actual four-member D6FS media and DRM236 AUXSTORE backing, CTY in
headless mode. Normal writable RUN/EXEC only; MEMFS raw-XIP admission,
strict 32K GCC/DSH, alternate/tape/drum roots or Stage 1 sources and
GUI/multiTTY are not in this first *OOM acceptance* profile. Existing
DAIMOS support for excluded features is not removed. The hardware
image can still contain additional packages; permanently resident
MRES and actual swap usage remain separate F05-F07 proofs.

Two fresh disposable 256K PDP-6 KCC E13 v1160 guests, on the local
Android aarch64 Open SIMH V4.1-0 `pdp6` (`edd19b17`), PASS actual
DSK/D6FS boot and physical BRK OOM with a held foreign filesystem
gateway in production-disabled and test-only enabled modes. The
original holder resumes and unlocks, INIT and other unrelated users
survive, the enabled policy selects only requester slot 3, disabled
selects nobody, and both record 160 spinner advances, zero integrity
errors and exact WAIT/FILE/MM cleanup. The versioned archive
`$TMPDIR/daimos-e13-release-media-v1160.qel3Zm.tar.gz` has full
simulator/counter and build logs, actual boot.ini, maps and SHA256
provenance. v1123 raw-XIP/OOM scope guard PASSes anew.

**S07/S08 CLOSED for the frozen first scope: 51/65 mandatory checks
complete, 14 open** (12 pre-enable plus F12/F13). **E13 remains
OPEN** until other required simulator/media combinations are
independently reproduced; A04 and F02-F11 remain open. The guest
has a test-only INIT, so actual default INIT/swap policy operation
is not being signed off. Production OOM remains OFF and no permanent
KCORE/MRES, RAM or ABI changes were made.

## 2026-11-05 v1159: E12 protected live owners under requester OOM

Two independently built full 256K PDP-6 testkit v1159 guests PASS
with opt-in requester-only BRK OOM enabled and production-disabled.
The actual physical -1 BRK decision snapshots the real process table:
INIT slot 1, blocked witness slot 2, gateway waiter slot 4,
independent spinner slot 5 and original held gateway owner slot 6
all remain active while requester slot 3 fails. The enabled guest
records exactly one fatal selection, of slot 3 only; the disabled
guest records zero fatal selections and a normal -1 BRK return.
The original in-flight provider retains its lease (`7,,0`) until
its own continuation unlocks it, with 160 spinner advances in each
guest and zero integrity errors. Existing physical MM, FILE close,
provider retry, WAIT and process accounting checks also PASS.
The test does not assert an exemption for INIT when INIT *itself*
requests a failing physical BRK; the proven invariant is protection
of all nonrequester critical/ordinary processes.

E12 CLOSED for the supported writable-process requester-only scope:
**49/65 mandatory checks complete, 16 open** (14 pre-enable plus
F12/F13). E13, S07/S08, A04 and F02-F11 remain. Production requester
OOM OFF; no production KCORE/MRES, ABI or permanent RAM changes.

## 2026-11-05 v1158: E11 independent RUN parent survives BRK OOM

Fresh independent 256K PDP-6 full-system guests PASS testkit
`test-daimos-oom-run-parent-v1158` in both isolated requester-OOM
enabled and default-disabled modes. In one guest, a separate live
process uses real MM descriptor pressure to fail a child RUN with
the production allocator's -3 result, confirms no published process
slot or process-word growth, restores descriptors and signals INIT
through a pipe. It remains blocked while another process triggers
genuine BRK physical OOM. Enabled policy retires only that requester;
disabled policy returns -1. The original RUN parent then completes
normal exit and exact WAIT. No production kernel or permanent RAM/ABI
changes; production OOM remains OFF.

E11 CLOSED for writable-process scope: **48/65 mandatory checks
complete, 17 open** (15 pre-enable plus F12/F13). E12/E13,
S07/S08, A04 and F02-F11 remain open.

## 2026-11-05 v1157: E10 APR/EXEC/RUN under foreign FS lease accepted

Fresh full 256K PDP-6 `test-daimos-fatal-exec-provider-v1157`
PASSes against DAIMOS `a7b721b4` and testkit `436552f`.
The disposable guest holds a real foreign filesystem-provider lease,
then exercises asynchronous APR fatal handling during a post-commit
EXEC FILE failure and an unpublished RUN u-area MM_BUSY rollback.
The real provider-backed FD3 survives staging, encounters foreign
lock contention, and is released only after the original holder
returns through its normal unlock path. Final private-owner WAIT,
FILE and u-area accounting PASS with zero test errors. Recorded
VFS calls/rejections are 6/5, FILE passes/sleeps 7/6, and physical
u-area pin/busy/unpin 1/1/1. The earlier retained v1157r11 log
and this independent rebuild agree.

E10 CLOSED for writable-process scope: **47/65 mandatory checks
complete, 18 open** (16 pre-enable plus F12/F13). E11-E13,
S07/S08, A04 and F02-F11 remain open. Test-only probes modify only
the copied guest; production KCORE/MRES/RAM/ABI and requester OOM
default OFF are unchanged.

## 2026-11-05 v1156: E09 physical MM and backing error classes accepted

Testkit `test-daimos-oom-error-matrix-v1156` PASSes all nine
independent real PDP-6/native error-class cases against frozen
DAIMOS `9f51326e` and testkit `81f40eb` with fixed host-tool
checksums, retained under `$TMPDIR/daimos-oom-error-matrix-v1156.gzw27g`.
The matrix distinguishes genuine BRK physical-MM exhaustion (-1),
MM fragmentation (-2), descriptor shortage (-3), invalid/wrong
ownership (-4) and pinned owner (-5). The additional v1156
test actually exhausts the production backing-store bitmap,
observes genuine allocation refusal with unchanged accounting,
then verifies continued ordinary RAM BRK growth/shrink and clean
backing recovery. Both disabled and disposable-enabled requester
OOM modes PASS; disk exhaustion never triggers requester killing.
The native backing-capacity test rejects unsafe ownership mutants.

E09 CLOSED for the currently supported writable-process scope:
**46/65 mandatory checks complete, 19 open** (17 pre-enable plus
F12/F13). E10-E13, S07/S08, A04 and F02-F11 remain open. No
production kernel, RAM, ABI or OOM switch change; requester OOM OFF.

## 2026-11-05 v1155: E08 concurrent sessions/group lifecycle accepted

Two independently booted 256K PDP-6 full-system v1155 guests PASS
simultaneously live foreign sessions, two separate two-member process
groups, actual group STOP/CONT/terminal events and parent CHLD/WAIT
notifications. The original unrelated sessions remain alive while
both groups stop, continue and exit. Exact per-PID WAIT status is
checked after each transition, with no second status or stale
event accepted. After those groups are reaped the guest repeats
64 numeric PID lifetimes, rejecting stale WAIT/TERM at each boundary
while the foreign sessions remain live, then reaps each session.
Final process slots, process words and FILE slots equal the baseline.

Testkit `test-daimos-oom-groups-sessions-v1155` PASSes on the
independent run plus the previously retained passing guest; the
unmodified v979 group/fatal-event regression also PASSes freshly.
An earlier in-development v1155 run timed out before the foreign
session phase; the finished test's two full PASS runs complete.
No production kernel, MRES, RAM or ABI change was required.

E08 is CLOSED for the writable-process scope: **45/65 complete**,
20 open (18 pre-enable plus deferred F12/F13). E09-E13,
S07/S08, A04 and F02-F11 still block activation.
`PROC_OOM_BRK_ENABLED_V1000` remains disabled.

## 2026-11-05 v1153-v1154: E07 qualified PI6 and bystander CLOSED

Full 256K PDP-6 v1153 exercises the held foreign FS provider with
four distinct physical MM_BUSY owners (PIPE, Type-340 DPY, process
VM, u-area). Qualified PI6 advances from 1297 to 1352 (55 ticks)
between the first real PIPE MM_BUSY and slot-5 exact finalization;
the independent runnable user makes three sleep/wakeup advances
inside that interval. All MM release and publication witnesses PASS
with zero errors, while the provider retains its original lease.

The separate full 256K v1154 guest extends the original v1122 test
to start an independent separate-session user while the second HUP
member encounters a genuine DISK bitmap verifier refusal. Actual
first and second verifier calls record PI6 1015 -> 1032 (17 ticks)
and eight unrelated user sleep/wakeup operations. The bitmap repair,
one physical PIPE pin/busy/unpin/free, exact DISK backing release,
and parent WAIT/TTY accounting all PASS with zero integrity errors.
The v1148 full-system TTY/HUP case already PASSes 32 actual PI6 ticks
and 48 bystander sleeps across real pinned TTY/VM/u-area retries.

E07 is CLOSED for the currently supported writable-process scope.
The mandatory checklist is now **44/65 complete, 21 open** (19
pre-enable blockers plus deferred F12/F13). E08-E13, S07/S08, A04
and F02-F11 including actual RAM/size and final release-candidate
checks remain open. No production KCORE/MRES/data/ABI change was
made; `PROC_OOM_BRK_ENABLED_V1000` remains zero.

## 2026-11-05 v1151r1: E05/E06 sustained physical-OOM recovery accepted

Independently rebuilt 256K PDP-6 full disk-root guests PASS both
testkit v1151 endurance targets with the production OOM switch OFF
and with requester-only OOM enabled exclusively in a disposable copy.
Each guest passes 33 genuine BRK physical-MM exhaustion attempts
(the original v1075 one plus 32 additional waves), 32 successful
fresh RUN and BRK grow/shrink recovery checks, 256 numeric PID-reuse
lifetimes and 512 stale WAIT/event refusals. Process slot/word, FILE,
disk swap allocation, and non-cache physical MM descriptor accounting
is exact at every boundary; at least 1024 real PI6 ticks advance.

The first run exposed a false assumption in the new *test*: the raw
MM extent count grows 7 -> 8 after successful filesystem access.
The preserved guest's physical descriptor table identifies a single
clean `0201`-word BCACHE slab, type kernel-dynamic, owner 6, base
`0777577` octal. This is intentional bounded read-cache residency,
not an orphaned process extent. The final test uses a test-only
read-only probe of the real BCACHE base and demands both exact
non-cache MM accounting and its subsequent genuine OOM reclamation.
The disabled guest originally passed all internal assertions but
the host wrapper mistakenly omitted the 32 normal `D` BRK-return
markers; v1151r1 checks their exact number and order. Both final
rebuilt guests PASS, with logs preserved in the testkit STATUS.MD.

E05/E06 are now CLOSED for this scope: **43/65 checks complete, 22
open** (20 pre-enable blockers plus F12/F13). E07-E13, final
release configuration, full warning and actual resident/peak RAM
measurement remain open. This phase changes no production KCORE,
MRES, per-process state, ABI or default-disabled requester OOM policy.

## 2026-11-05 v1150: B06 real OOM with held provider accepted

Two independently rebuilt disposable 256K PDP-6 full guests PASS a
combined genuine BRK physical-MM exhaustion, held foreign provider
lease, pre-dispatch gateway wait and owner-side FILE cleanup test.
The OOM requester opens a real `/SYSTEM/INIT` FILE before another
process (slot 6) acquires the serialized provider lease `7,,0`.
That original provider owner sleeps on its own private executive
continuation, while slot 4 waits before provider dispatch on the
real gateway event; a separate user continues running. At the
requester's real BRK -1/physical-MM-cause -1, the test witnesses the
original unchanged `7,,0` lease and sleeping waiter.

With requester OOM opt-in enabled in the disposable kernel only,
one fatal requester is killed, seven genuine foreign-provider
FILE-close contention attempts leave the original lease intact,
and slot 6 eventually resumes and unlocks it normally. With OOM
disabled, BRK returns -1 and the requester then waits on the
foreign-provider event during ordinary EXIT. Both cases show one
holder entry/return, one real physical OOM, one pre-dispatch waiter,
160 independent spinner advances, zero integrity errors, exact
terminal WAIT, surviving unrelated witness and stable MM/slot
accounting. The test changes no production source or switch.

B06 is CLOSED for the current writable-process scope. The 65-item
OOM checklist is now **41 complete, 24 open** (22 pre-enable plus
F12/F13). S07/S08, A04, E05-E13 and F02-F11 remain open, including
sustained reclaim, actual RAM/size and final release-candidate tests.
Production `PROC_OOM_BRK_ENABLED_V1000` is still zero.

## 2026-11-05 v1149: fail-closed B07 retry lifetime verified

The v1149 testkit source-linked contract PASSes current PDP-6 fatal
owner FILE/HUP retry, foreign FS lease contention and slot-0
finalization, rejecting eight unsafe mutations. The 01000-tick
backoff is only a maximum *interval*: a failed FILE/SYNC or foreign
provider never authorizes reclaiming the in-flight owner, private
executive stack, VM, FILE table or occupied provider lease. Native
v885 executes 14 actual FILE refusals with repeated capped delay
and eventually a single successful owner EXIT; native v1102 tests
the slot-0 backoff and rejects four additional unsafe mutants.

B07 is checked, without claiming eventual liveness for a provider
which never returns. Checklist: 40/65 complete, 25 open (23
pre-enable blockers plus F12/F13). B06 physical-OOM concurrency,
E05-E13 endurance/liveness and F02-F11 memory/release acceptance
remain open. No production code/RAM/ABI change, OOM still OFF.

## 2026-11-05 v1147-v1148: C04 actual PI6 and deferred HUP completed

Two independent v1148 full-system 256K PDP-6 builds PASS one combined
owner-continuation adversarial test. A live same-session member retains
its private syscall stack when INIT's first real EXIT broadcasts HUP
and a physically pinned canonical TTY causes MM_BUSY (-5). The HUP
defer and the member's owner-return HUP are counted exactly once;
INIT waits for the actual HUP terminal status before retrying EXIT.
The original TTY session survives refusal. The second owner EXIT
then reaches real VM and u-area MM_BUSY, with one pin/unpin each,
preserved owner state, slot-0 deferred retry and one final commit.
Separate-session user sleep progress is 48 in each guest, while the
real PI6 clock progresses 810->842 and 805->837, respectively. All
recorded hook errors are zero. v1147 standalone recheck and v1098
PREVM clock contract/three unsafe mutants also PASS.

C04 is now checked. The mandatory OOM tally is 39/65 complete,
26 open (24 pre-enable plus F12/F13). B06/B07, sustained E/F,
RAM sizing and diagnostics review remain mandatory. All v1148
instrumentation is in disposable testkit copies; no production
kernel, MRES, persistent RAM, ABI or OOM switch changed.

## 2026-11-05 v1146r3: full D16 exact-once matrix PASS

The sequential nine-stage `test-daimos-oom-exact-once-matrix-v1146`
PASSes on clean frozen DAIMOS `2e12c1da` and daimos-testkit
`253a813`, using fixed installed P10FOLD/DLINK/DAS/PDP-6 simulator
checksums. Full logs and `revisions.txt` are retained in the
versioned `$TMPDIR/daimos-oom-exact-once-v1146.mV2FJf` directory.
The matrix verifies the transitive fatal-release graph and negative
mutations, real PIPE/DPY/VM/u-area pin/busy/unpin/free, physical TTY
HUP, swap-out/in and bitmap-refusal recovery, retained foreign FS
provider lease, exact FILE->VM->u-area->ZOMB publication and parent
WAIT, nested APR/EXEC/post-VM failed-RUN deferred discard, bystander
progress, and process/owner bookkeeping. Disabled and isolated-enabled
OOM guests each complete 24 PID lifetimes with stale event refusal,
alternating WAIT status and stable MM/slot counts. Only the requester
is terminated in the enabled physical-OOM case.

D16 is now complete: **38/65 mandatory OOM checks**, 27 open (25
pre-enable blockers plus F12/F13 post-signoff). C04, B06/B07, E/F
stress/signoff and actual resident/peak 256K and deferred 32K budgets
remain open. Existing KCC notes during full image builds include a
truncated TSFS helper identifier and unused utility functions; F10
still requires a final diagnostics review. No production KCORE/MRES
instructions, persistent data, ABI or requester-OOM policy changed;
`PROC_OOM_BRK_ENABLED_V1000` remains 0.

## 2026-11-05 v1145-v1146: D16 exact-once acceptance in progress

The corrected installed host tools and unchanged DAIMOS production
kernel PASS the full testkit v1087 transitive-release suite, including
its unsafe mutations and native FILE/HUP, PIPE, DPY, TTY, VM, SWAP
and provider-lease checks. Two outdated native test expectations were
repaired in the testkit: v912 must return the real MM_BUSY (-5) on
owned DPY rollback, and v995 must provide current wait-state/event
symbols with an unexpected-runqueue guard.

A newly rebuilt 256K PDP-6 v1122 session guest independently PASSes
one physical PIPE MM pin/busy/unpin/free and the other member's real
disk-backing bitmap refusal/restoration, with one swap detach,
correct terminal WAIT/TTY and zero test errors. This is fresh D16
component evidence, not complete D16 acceptance.

Testkit v1146r1 now stages the sequential exact-once release matrix.
It captures the tested DAIMOS/testkit revisions and installed host
tool checksums, rejects dirty production/fixture trees and detects
version changes between guests. Its dirty-source negative control
PASSes; the full multi-guest matrix is NOT yet executed. Mandatory
OOM checklist stays 37/65 complete, 28 open (26 pre-enable plus
F12/F13). Production requester BRK OOM is still disabled; there is
no kernel/MRES code or persistent RAM/ABI change.

## 2026-11-04 v1144: D14 mixed physical resources PASS

Full-system PDP-6 `test-daimos-oom-pipe-dpy-vm-provider-v1144`
now PASSes one real-physical pin/BUSY/unpin/free sequence each for
PIPE, Type-340 DPY list, process VM and executive u-area. The
test's 256K guest retains a real foreign FS gateway lease and its
blocked waiter while performing actual FILE/HUP retries. Its
terminal checks reject premature parent WAIT, verify chronological
FILE/PIPE/DPY -> VM -> u-area -> ZOMB publication, one swap detach,
correct WAIT/CHLD, 48 stale-PID/event refusals and bystander progress.
All four MM owners are distinct and zero hook errors are recorded.
The original independent v1118 full guest also PASSes after this
testkit extension, as does the v1087 transitive-release/native suite.

D14 is now checked. Mandatory OOM checklist: **37/65 complete**;
28 open (26 pre-enable blockers plus later F12/F13). D16, C04,
E/F and actual RAM budgets remain open; production requester-OOM
switch remains OFF. Test injections add no production RAM or ABI.

## 2026-11-04 v1143: D15 scope closure, XIP process admission excluded

Fresh native testkit v1108/v1109 MEMFS XIP release tests PASS, with
their unsafe mutants rejected. The v1123 scope/mutation audit also
PASSes: ordinary production EXEC/RUN cannot admit an XIP process,
and requester-only BRK OOM remains disabled. The current OOM
acceptance explicitly EXCLUDES scheduled XIP processes. D15 is
therefore checked, making 36/65 mandatory checks complete; 29
remain open (27 pre-enable plus F12/F13). This does not close the
separate DAIMOS 0.9 trusted-XIP-launch requirements X03-X08.
D14/D16, C04, E/F and RAM/signoff work remain open. No resident
kernel, MRES, permanent data or ABI change.

## 2026-11-04 v1142: D13 integrated physical MM_BUSY rollback passes

With v1141's corrected cross-object fold/link validation installed,
the v1142 full 256K PDP-6 guest combines an APR-faulted child,
postcommit EXEC fatal FILE refusal/retry and an unpublished failed
RUN whose actual allocated child u-area is physically pinned.
The first real u-area MM-free returns MM_BUSY (-5); a verified unpin
allows slot-zero deferred discard to complete a second real free.
Two independently rebuilt full guests report identical target
counters: calls=2, pins=1, busy=1, unpins=1, errors=0.
Parent RUN return, process-word/slot recovery and the
EXEC child's final ZOMB/WAIT also PASS with no unexpected UUO.

This satisfies the remaining physical-u-area component of D13.
The current OOM mandatory checklist is **35/65 complete**, with
30 open (28 pre-enable blockers, plus the later F12/F13 actions).
D14-D16 and other C/E/F gates remain open. No production KCORE/MRES
code, persistent data, ABI or OOM policy change was made; the
requester-only BRK OOM switch remains disabled.

## 2026-11-04 v1141: unsafe local relocation fold repaired

The original uninstrumented 256K PDP-6 D13 combined APR fatal,
postcommit EXEC FILE refusal and unpublished failed RUN regression
now PASSes with corrected installed P10FOLD/DLINK: one late RUN,
one terminal EXEC ZOMB, no integrity error and normal parent WAIT.
The historical v890 uninstrumented EXEC/FILE/WAIT regression and
native v1129 VM_NONE regression also PASS with corrected tools.
The independently rebuilt v197 RUN rejection control PASSes 16 real
post-VM failures with stable process ownership, and the second fresh
uninstrumented combined v1126 guest independently PASSes as well.

P10FOLD had considered local relocations in different DOBJ1 objects
equal whenever their section/addend numbers matched, even when the
branch target was outside the fold. The actual broken plan folded
proc_pdp6's `proc_discard_ok` onto syscall_dispatch's unrelated
`SETZ/JRST` block. DLINK repeated the same erroneous equivalence.
Both implementations in pdp10-tools and daimos-tools now require
identical original object identity for external local references,
while preserving safe relative in-span targets. Testkit v1141
reproduces the old invalid fold and forged-link-plan acceptance,
and passes with each corrected tool tree. A v1141 local.tar.gz
installed the verified pair under PDP10_PREFIX.
The corrected post-fold full-guest KCORE low endpoint is `023413`
octal versus `023411` with the unsafe toolchain, a two-word cost
for correctness; this is not a dynamic RAM high-water measurement.

No DAIMOS production source, KCORE/MRES data, ABI or OOM setting
changed. The full D13 MM_BUSY admission-rollback variant, D14-D16,
P0-E/F stress/signoff and resident RAM audit remain OPEN. Production
`PROC_OOM_BRK_ENABLED_V1000` remains zero. Earlier debugger-sensitive
v1138-v1140 observations describe the old broken toolchain only.

## 2026-11-04 v1140: debugger breakpoints alter D13 outcome

In the latest fresh 256K guest with simulator-only MM-return and
discard-return breakpoints, the late failed RUN resumes through
`proc_discard_ok` and the RUN epilogue, and the EXEC ZOMB counter
becomes one. The immediately preceding v1138 stage-breakpoint guest
did not reach that return and produced invalid UUO/ZOMB=0. No
production bytes were deliberately modified in either run. Treat
v1140 as a timing-sensitive diagnostic and preserve D13 as OPEN;
neither a breakpoint-induced pass nor source padding is a valid fix.

## 2026-11-04 v1139: fresh MM-free breakpoint in D13

The separately rebuilt 256K diagnostic adds a linked `mm_free`
execution breakpoint without modifying production code. It confirms
the failing unpublished child's post-VM u-area-release path invokes
the real MM-free routine. It does not yet prove successful return or
descriptor clear, and it does not resolve the invalid UUO. Replaying
an already booted writable image was invalid (`?RT`); fresh media are
required for each acceptance run. D13, MM_BUSY and E/F gates remain
open, with requester OOM disabled.

## 2026-11-04 v1138: nested RUN reaches u-area release

The fresh unchanged-production 256K PDP-6 v1138 SIMH stage-breakpoint
fixture completes and reports a late failed RUN through claimed-child
rollback, VM-done and u-area-release entry. It does not reach the late
RUN return before invalid UUO. Later release calls for slots 1 and 2
occur after the trap and cannot be interpreted as a failed-child retry.
No production source, KCORE/MRES/ABI or OOM policy was changed; D13
and physical MM_BUSY rollback remain open. Next isolate `mm_free` and
`proc_uarea_release` return state under the same uninstrumented guest.

## 2026-11-04 v1139: late RUN rollback advances through slot trimming

Further unchanged-kernel SIMH breakpoints confirm the late claimed
child's u-area MM free is followed by runnable-queue removal and
high-slot trimming. The invalid UUO remains, and the late discard
return/parent RUN epilogue is not observed. Repeated MM events in the
full capture are not proven to belong to the same slot. A separate
versioned testkit v1139 wrapper records the extended release stage
trace, retaining the narrower v1138 mode. This is diagnostic evidence,
not a repair or production acceptance. D13 OPEN; OOM remains disabled.

## 2026-11-04 v1138: RUN rollback reached u-area release

Fresh unchanged-production 256K PDP-6 D13 guest with simulator-only,
post-fold stage breakpoints reproduced late RUN rejection and invalid
UUO. Its rollback passed TTY/VM release and entered u-area release,
without visiting discard completion or late RUN return. Three
`proc_uarea_release` entries in the full capture include other
processes; this does not establish repeated failure of the late
child's MM free. The cause remains unproven. D13 remains OPEN,
production OOM remains OFF, and KCORE/MRES code/data were untouched.

## 2026-11-04 v1134-v1137: late RUN rollback is not returning

The still-OPEN D13 nested APR/EXEC/unpublished-RUN guest was rebuilt
four more times against unchanged production code. v1134 inserts 043
octal unreachable text words after the test-only fdmap branch; it
still prints `<EA`, records one late RUN rejection and zero terminal
EXEC ZOMB, then hits the same unexpected UUO. This particular code
placement change does not reproduce v1131's instrumented success.

v1135 builds the uninstrumented guest and samples existing SIMH words
after HALT. `mach_invalid_uuo_word` is 000000000014 and the saved
trap return word is 340000774047; post-halt process slot is zero.
The word's high bits are PDP-6 status flags, not an ordinary 250 JRST
opcode: a v1136 live breakpoint records a valid return word
350000000104 in parent slot 1. Do not diagnose corruption from the
340/350 prefix alone. No source execution is added for these probes.

v1136's first arrangement captured two earlier successful RUN
epilogues before the late failure. v1137 instead stops at the late
`proc_run_claimed_bad` with parent slot 1, saved return
350000000104, user SP 000053002057 and kernel SP 000000046565.
A second breakpoint at the same RUN's return epilogue is never hit;
the simulator reaches the invalid UUO and HALT first. Thus the late
failed RUN does **not** return from its `proc_slot_discard` path;
the two v1133 RUN-return counts came from earlier successful RUNs.

The next step is to isolate TTY, VM, u-area and discard-return stages
without source-code instrumentation, then validate any actual source
fix against fresh uninstrumented and isolated control guests. D13
remains OPEN; no speculative padding, OOM enablement or new KCORE/
MRES/ABI data was committed. Mandatory checklist still 34/65 done,
29 pre-enable blockers.

## 2026-11-04 v1130-v1133: D13 mapped-RUN return boundary unresolved

The full v890 postcommit-EXEC failure baseline now independently
PASSes on the corrected v1129 `VM_SPACE_NONE` production destructor:
one destroyed old VM, a failed replacement, two real VFS close
refusals, successful third close, final EXIT and correct parent WAIT.
The v1129 native source-linked sentinel/detach test also PASSes.

The distinct nested D13 guest with APR fault and unpublished late RUN
still does not complete. Without additional tracing, its actual
fdmap failure is seen once, but the guest reaches only `<EA` followed
by simulator UUO word `340000774047`, with zero terminal EXEC ZOMB;
the parent never writes R. Independently booted v197 inherited-group
and v1130 NEW-group controls both PASS sixteen invalid fdmap RUN
rollbacks under three blocked children with exact MM accounting and
later successful RUN/WAIT. NEW-group selection alone is not the cause.

Three independent disposable guest diagnostics narrow the difference:
v1131 full rollback-stage tracing produces `<EARH`, with each real
rollback entry/VM destroy/u-area free/return visited once, no error
deferral, and terminal EXEC ZOMB=1. v1132 adds 13 *unused BSS words*
only and reproduces the `<EA`/UUO/ZOMB=0 failure. v1133 adds only
two RUN-label counters, sees the late claimed-child rollback once
and `proc_run_return` twice (including the previous successful APR
child), but still fails with `<EA`/UUO/ZOMB=0. The added tracing
changes observed behavior; do not infer a safe production fix from
the instrumented PASS. Further investigation must inspect the
mapped-user syscall return, parent saved continuation and APR/stack,
and whether code placement or scheduling contributes.

D13 remains OPEN; the v1131-v1133 tests are diagnostics, not D13
acceptance, and the combined physical MM_BUSY failed-RUN rollback is
still outstanding. The rest of the 29 pre-enable OOM blockers remain.
No v1130-v1133 production KCORE/MRES/ABI or resident RAM change;
`PROC_OOM_BRK_ENABLED_V1000` remains zero. No padding workaround.

## 2026-11-04 v1129: original v890 EXEC teardown fixed; D13 still OPEN

The exact original, unmodified full 256K PDP-6 v890 guest previously
timed out at both historical 65-second and extended 180-second host
limits. A new disposable stage probe verified that after VFS close
success, slot 2 experienced one real `vm_space_destroy` -1 failure,
zero failed TTY release, no VM success, no deferred u-area stage
and no final ZOMB. Source inspection established the root cause:
post-commit EXEC had already published `VM_SPACE_NONE` (-1), but
`vm_space_destroy` used `hrrz` and treated its low 18 bits as a live
physical base, then attempted invalid MM teardown (-4).

Production `system/kernel/proc/vm_pdp6.s` now recognizes the exact
full-word sentinel after storage validation and continues through
the usual checked backing detach. No process ABI, MRES, persistent
storage or data words changed; the fix costs two VM text instructions.
The new testkit v1129 native PDP-6 regression PASSes two consecutive
VM_NONE destroys with zero attempted MM/XIP free, two backing detach
calls, unchanged sentinel, invalid-mode refusal and two unsafe
mutants rejected. Existing v1109 XIP/VM and v880 swap-detach native
tests PASS. The independently rebuilt **unmodified v890 full guest
now PASSes**, including real VFS FILE refusal/retry, cleanup and
parent WAIT. This closes the specific baseline regression.

The concurrent v1126 APR+EXEC+unpublished RUN guest remains a distinct
OPEN gate: after VM fix it reaches `<EA` but subsequently causes an
unresolved simulator UUO before the late RUN marker. Its test-only
finalizer trace reports owner entry but no terminal ZOMB and cannot
be considered an accepted result. The original v1126 failure
diagnostics are retained for audit. D13 is NOT checked; the source
readiness audit remains 34/65 mandatory complete, 29 pre-enable
blockers plus F12/F13 post-signoff actions. Production requester OOM
stays default OFF.

## 2026-11-04 v1126: D13 nested fatal/rollback acceptance remains OPEN

**v1126r1 baseline requalification:** The exact, unmodified v890
test C and shell source from the previously committed daimos-testkit
revision was restored to an isolated TMPDIR fixture and rebuilt
against the current DAIMOS production tree. It timed out after
printing `<E` with its original 65-second simulator limit. A second
independent disposable guest used the identical original sources,
changing only the host timeout to 180 seconds; it also printed `<E`
and timed out (SIMH exit 124, SIGTERM, PC 016424 octal in that run).
This disproves the hypothesis that the *new v1126 guest extension
alone* caused the observed EXEC owner noncompletion or that merely
increasing the 65-second timeout is sufficient. It does NOT prove
whether the remaining failure is in production finalization, the
PI6/clock/reaper path, or simulator-specific progress. The baseline
must be investigated directly before accepting D13 or changing
the production policy. The original v890 test is not currently a
passing full-system regression on this configuration.

An opt-in testkit `daimos-fatal-exec-apr-run-v1126.sh` now builds an
isolated 256K guest combining genuine post-commit EXEC VM destruction,
test-injected VFS FILE-close refusals on the preserved process-private
kernel continuation, a second child's real APR protection fault,
and a subsequent invalid-fd-map unpublished RUN rejection with
unchanged parent process-slot/word accounting. The APR WAIT status
must be flagged TERM, as established by v889; an initial expectation
of plain status 1 was corrected in the guest, not in production.

The guest reaches `<EAR` and observes the EXEC provider ultimately
accept one close after four refusals (5 VFS calls, descriptor/witness
cleared, one EXIT entry, zero ownership-error counters). However the
parent's later bounded NOHANG WAIT did not see the EXEC child reaped;
the guest emitted `<EAR1!t` and stopped. A diagnostic-only follow-up
with extra finalizer instrumentation instead encountered `<EA` plus
a simulator UUO and timeout, so it cannot be used to assert either
a production fault or a successful recovery. Neither run meets D13.

The `daimos-testkit` STATUS records the failed variants precisely.
Existing v893/v895 full-system physical RUN VM/u-area MM_BUSY tests
are independent evidence, not a combined D13 result. Diagnose the
final child/WAIT/clock stage and the test-only tracing before adding
physical rollback MM_BUSY in the integrated guest. D13, D14-D16,
P0-E and P0-F remain OPEN. OOM remains disabled in production;
no production code, RAM, ABI or MRES layout changed by v1126.

## 2026-11-04 v1125: booted wrong-owner MM while foreign FS lease held

The accepted 256K PDP-6 disposable guest combines two separately
proven real production behaviors. Slot 2's genuine u-area MM extent
is changed once to the wrong owner in the actual MM descriptor table;
the unchanged `mm_free` checks then return -4 on all seven real
reaper attempts, leaving the correct process/u-area and WAIT state
unreaped. Independently, slot 4 enters the actual serialized FS
gateway and holds its original lease and private process stack across
80 scheduler-yielding sleeps. `fs_provider_ready` contains `5,,0`
both during the targeted MM free and at HALT. Slot 3's independent
spinner advances; six reaper visits, one real APR fault, one wrong-
owner poison, zero synthetic MM returns and zero test errors are
observed. The initial guest used the historical immediate-F marker
assertion despite delayed F; its MM/FS counters passed but the
outdated transcript check failed. The corrected test also samples
the lease at the MM refusal and PASSes in a separately rebuilt guest.

D12 is checked: **34/65 mandatory OOM gates complete, 31 OPEN**,
29 are pre-activation blockers and F12/F13 are post-signoff actions.
The original v1101 wrong-owner guest was independently rebuilt and
rebooted on the same source: PASS with seven -4 returns and original
WAIT quarantine. The entire source-linked v1096 P0-D matrix PASSes;
the larger `P0D_FULL=1` boot matrix has not yet been rerun as a whole.
v1125 is now included in that optional full matrix. All additions are testkit
only (kernel-copy probes, user guest and counters); production KCORE,
MRES, memory footprint, ABI and the disabled OOM switch are untouched.
The remaining D13-D16 and whole-system P0-E/F gates remain OPEN.

## 2026-11-04 v1124: booted physical TTY, process VM and u-area MM_BUSY

The DAIMOS OOM enablement checklist now has **33/65 mandatory
tasks checked, 32 OPEN**; 30 are pre-activation blockers and F12/F13
are deliberately post-signoff actions. D11 closes with two
independently built disposable full 256K PDP-6 guest PASS results.

The v1124 guest first attaches the INIT session's real canonical TTY
input MM extent; the owner-side HUP preflight on the first real EXIT
pins it and receives genuine MM_BUSY (-5). User EXIT returns -1,
preserving session/TTY ownership and the process's still-running
private kernel stack. A second real user EXIT releases exactly that
TTY extent and enters normal fatal finalization. The process's real
VM base is physically pinned before its first checked destruction,
producing a second MM_BUSY and preserved PREVM/VM owner. The actual
qualified-clock slot-0 service later unpins the VM and pins the
still-owned u-area, whose checked release produces a third MM_BUSY.
The later reaper cycle observes VM_NONE, retains the exact u-area
owner, unpins it and commits final process-slot release once.

The full image PASS counters are canonical TTY/VM/u-area physical
busy=1 each; three real pin/unpin pairs=1 each; TTY retained/retry=1,
VM retained/destroyed=1, u-area retained=1, terminal release=1,
test ownership errors=0. The historical v969 TTY-only regression
was also reworked for the current owner-side HUP preflight and
independently rebooted: PASS with 1 pin, 1 retained owner and 1
exact unpin/retry. Initial v1124 experiments using a late v969 arm
or synchronous EXIT-failure cleanup were invalid; the accepted test
uses the real owner-side EXIT plus clock-driven reaper stages.

Production KCORE, MRES, process ABI and the OOM flag remain
unchanged (`PROC_OOM_BRK_ENABLED_V1000,0`); all instrumentation is
confined to testkit disposable copies. D12-D16, sustained P0-E
pressure and P0-F final sizing/activation are OPEN. The optional
full P0-D boot matrix includes v1124, not yet fully rerun as a whole.

## 2026-11-04 v1123: fine-grained OOM release checklist and XIP boundary

The production requester-only OOM branch remains OFF. A new active
top-of-TODO v1123 list separates 65 mandatory tasks S/A/B/C/D/E/F
from eight optional raw-MEMFS-XIP admission tasks. The audited
baseline is **32/65 mandatory checked, 33 mandatory OPEN**;
raw-XIP X01-X02 are checked, X03-X08 optional and OPEN.

S covers policy/qualification scope, A allocation-free release
(v1087 CLOSED), B fail-closed held-provider ownership (v1092
CLOSED *for safety*, not guaranteed completion), C PI6-safe slot-0
finalization (v1086 CLOSED), D physical error/owner cross-products
(OPEN), E sustained system acceptance (OPEN), and F image-budget,
full-matrix and actual production activation (OPEN).

Of the 33 unfinished mandatory tasks, F12 (production switch) and
F13 (post-enable regression/commit) are deliberately executed only
**after** pre-enable signoff. The actual activation-readiness audit
therefore reports **31 open pre-activation blockers** and two later
actions, avoiding a circular release prerequisite.

The existing v1122 real PIPE MM_BUSY plus separate DISK bitmap
refusal/repair and v1120 physical OOM PID-reuse evidence are retained
as completed subsets, not counted as full P0-D/E acceptance. All
normal production RUN/EXEC paths use the writing `vm_space_startup`;
no privileged raw MEMFS-XIP process admission is wired. The new
testkit v1123 source guard detects this boundary and rejects six
unsafe mutations. Optional XIP RUN/EXEC is excluded from the current
ordinary-process OOM activation scope; adding it later requires
separate trusted non-writing startup/VM/swap/teardown acceptance.

The full current source-linked v1096 P0-D component matrix PASSes,
including MEMFS XIP release/retry, TTY/VM/SWAP/PIPE/DPY and the
v1087 transitive allocation-free graph. New testkit readiness audit
PASSes structural inventory and five unsafe mutations, reporting
33 unfinished mandatory items, of which 31 block pre-enable signoff;
its --require-ready mode correctly exits 2.
The independent v981 requester policy, v999 BRK-cause, v1113
real MM -2/-3 (three unsafe mutants rejected), v1092 provider
quarantine (14 mutants rejected), and v1086 slot-0 PI6 closure
source-linked controls also rerun PASS.
This is a **not-ready** result, not a replacement for an enabled
256K guest or the optional P0D_FULL booted suite. A comprehensive
release checklist and constraints are documented in
`pdp10-doc/KERNEL/OOM-ENABLEMENT-CHECKLIST-V1123.MD`.

No production KCORE code, MRES, process ABI, resident RAM or default
OOM policy was modified; the known KCC 256K full-image low endpoint
remains `023171` octal pending a new final sizing run. The historical
v513 full/auto package-budget overflow must be assessed against
the actual live MRES set before activation, not treated as measured
runtime peak. Strict 32K DSH and KCORE `<020000` goals remain
separately deferred, not implied by 256K OOM acceptance.

## 2026-11-04 v1122: actual cross-owner PIPE MM and DISK bitmap refusal

The new testkit `daimos-session-hup-pipe-swap-v1122` full 256K
PDP-6 guest PASSes with two separate actual resource-release faults
within one leader's session teardown. The first session member
retains a private PIPE. Its first HUP attempts real typed MM free;
an injected physical MM pin makes it return -5. The initial HUP
fails without committing FILE/PIPE ownership or TTY/session release.
The next HUP revisits that member once, unpins/frees the exact PIPE
and continues to a distinct second member. The second member has
a genuine DISK backing record created by normal SWAP MRES swap-out.
Its real bitmap verifier rejects a test-corrupted physical allocation
bit, retains the original record and block accounting, and slot 0's
clock-backed PREVM retry later restores the bit and releases once.

Counters in the boot: PIPE calls=2, physical pin=1, MM_BUSY=1,
unpin=1, free=1, owner errors=0; first member HUP repeats=1,
pending commit=1; swap-outs=1, bitmap verifier calls=2,
corrupt=1, restore=1, backing free=1 and swap errors=0. At final
HUP, the original slot 4 DISK record is zero and backing-block
usage is restored to baseline. First-member pinned u-area has one
pin and one unpin. All intermediate nonwaitable leader, final
leader/member WAIT and foreground TTY tests PASS. Three leader
HUP attempts intentionally reach the two distinct physical faults.

Unlike v1121, this scenario really reaches the checked DISK bitmap
failure while a different member has also failed its physical PIPE
close. The two original v1104 and v1107 tests remain available in
unchanged single-provider modes. All hooks/observability are scoped
to disposable test images; the production kernel, MRES size, RAM,
ABI and disabled OOM flag remain unchanged. Booted MEMFS XIP,
long-running mixed-provider liveness and complete P0-D/E/F
acceptance remain OPEN.
After the v1122 combined guest PASS, the original 256K v1104 PIPE-only
and v1107 bitmap-only guests were rebuilt and independently booted.
Both PASS their previous exact pin/busy/release and bitmap-verifier
corrupt/restore/checked-detach counters respectively. No baseline
regression, production change or new resident RAM was introduced.

## 2026-11-04 v1121: booted DPY MM_BUSY with real DISK out/in lifecycle

The full disposable 256K PDP-6 v1121 mixed session guest PASSes with
genuine resident Type-340 DPY final-close MM refusal and a DISK
swap-out/restore of the same second member. The first version reached
HALT but failed the old v1107 terminal-DISK assumption: `second=1`
rather than zero, and no bitmap verification occurred. This was a
valid difference in process lifecycle, not an observed kernel defect:
the physical DPY FILE-close refusal kept the private continuation
live; the process had to be restored from disk before close could
complete. It was not valid to count the unreached bitmap fault.

The testkit revised its isolated source generator to instrument the
real SWAP MRES `proc_swap_in` entry and zero-status successful return
for slot 4. One real swap-out, one real successful swap-in and one
backing free now PASS; the terminal record and block count are zero,
and no direct bitmap-verifier injection is claimed. The DPY raw-list
extent is pinned once, produces exactly one real MM_BUSY (-5), and
is later unpinned and freed exactly once. Both release attempts are
accounted, with zero owner errors. The display engine executes 118
PI6 refreshes, including 90 during the held-list interval. Partial
HUP, exactly one later second-member HUP, first-member u-area owner,
complete leader/member WAIT statuses and TTY handoff all PASS.

This is P0-D evidence for physical SWAP restoration plus DPY release,
not for simultaneous DISK bitmap failure and DPY MM_BUSY. Testing an
actual bitmap-detach fault while another independently owned resource
refuses will require a distinct valid owner/lifecycle setup. Booted
MEMFS XIP admission, concurrent fault/liveness acceptance and final
P0-E/F policy activation remain OPEN. All instrumentation is confined
to disposable guest code; DAIMOS production KCore, MRES, resident
RAM, ABI and default OOM policy are unchanged.
An independent fresh v1107 swap-only full guest PASSes after the
testkit generator extension: two real bitmap-verifier calls, one
physical bitmap refusal/corruption, one restoration, one backing
free, no duplicate HUP and no final backing record or word leak.

## 2026-11-04 v1120: booted numeric PID recycling after OOM

The `daimos-oom-pid-reuse-v1120` regression boots complete disposable
256K PDP-6 images under both OOM-disabled and test-only enabled
policies. The v1075 real physical exhaustion requester behaves
correctly in both: the disabled guest returns BRK -1, while the
enabled guest kills only the true exhausted requester. The blocked
independent witness survives and is reaped; exact parent process
slot and process-word accounting returns to baseline.

Each guest then sequentially creates and reaps 24 new processes with
alternating 062/063-octal exit statuses. All 23 adjacent child pairs
reuse the exact numeric PID after the prior process has finished.
Every repeated WAIT and directed TERM issued *after reap and before
the next RUN* returns -1; every iteration returns to the same MM
process-slot and process-word baseline. The simulator outputs are
`<DNWQP` (default off) and `<KWQP` (private enabled), both PASS.
This proves live numeric PID reuse rather than merely repeated
requests to an already-reaped, never-reallocated slot. It does not
invent a PID generation contract or claim stale PID handles remain
invalid after another process legitimately occupies the number.

The v1109 real MEMFS XIP/VM native fixture and v1087 transitive
teardown scope audit also PASS on current source. Testkit adds C89
guest logic, two dedicated Make targets and optional P0D_FULL rows;
the production kernel, linked memory footprint, MRES, process ABI,
and disabled OOM gate remain unchanged. Complete P0-D/E mixed fault
liveness and final P0-F policy activation are still OPEN.

## 2026-11-04 v1119: actual PIPE plus DPY session-HUP release faults

Two independent full 256K PDP-6 boot runs PASS the new testkit
`daimos-session-hup-pipe-dpy-v1119` fixture (testkit `42736cd`). The
second member of a live session owns both a private PIPE final-close
extent and a published Type-340 raw display list when the first
session HUP arrives. Both real MM free error paths are armed in the
same delivered HUP; the test-only generator composes v1104 and v1105
entry/busy/delete instrumentation without changing production MM.

The two MM extents have separate physical addresses and expected
types/owners. Each records two free attempts, one real pin-induced
MM_BUSY (-5), one unpin and one successful free; both ownership-error
counters stay zero. The Type-340 PI6 refresh has 14 counted entries,
including 8 while the display list remains pinned after the initial
busy error. The first session member retains its pinned u-area and
already committed HUP; the leader receives a partial HUP failure,
then finalizes correctly. No duplicate first-member HUP or improper
early WAIT is observed. Leader and both child WAIT statuses, TTY
reattachment and foreground ownership PASS after release.

The final rerun used the corrected C89 test source and versioned
Make target. The initial physical fault composition required test-
only generator anchor changes; production KCore, MRES, persistent
RAM, process ABI and OOM selector were not modified. This closes
one additional physical P0-D cross-product, but concurrent
XIP/VM/swap bitmap faults, numeric PID reuse, and full integrated
P0-E/F remain outstanding. Production OOM stays OFF.

## 2026-11-04 v1118: booted combined physical PIPE and VM teardown

The full 256K PDP-6 `daimos-oom-physical-mixed-v1118` regression
PASSes in a disposable test kernel. Previously separate v1103 and
v1106 `mm_free` fault hooks now compose, with each typed MM owner
check preserved. The initial fixture failed while preparing private
kernel source because the two test injections shared an incompatible
entry anchor. The corrected test generator permits both callbacks
to reach the real MM allocator in the same dying process.

The boot observes exactly two PIPE final frees, one refused -5 and
one success, and two process-VM frees, one physical pin-induced -5
and one actual MM deletion after unpin. The real VM destroy reaches
swap detach once. It also reports three FILE close attempts/two
refusals, two HUP stage entries/one injected refusal, two u-area
release calls/one MM refusal, and one EXIT, CHLD and finalization.
The independent runnable spinner advances 37 times, while a different
process retains its original serialized FS-provider lease. After
reap, 48 stale WAIT and PID-directed TERM requests return -1 without
increased process-slot/process-MM-word accounting. No ownership
error is reported. This is full booted evidence for one genuinely
simultaneous physical PIPE/VM release-failure sequence.

The HUP failure itself is synthetic; this does not cover physical
DPY/HUP, live MEMFS-XIP pin/VM, concurrent DISK swap bitmap faults,
numeric PID reuse, or indefinite provider recovery. Do not conflate
this positive P0-D/E subset with the complete required matrix.
P0-D/E/F remain OPEN, production OOM remains OFF, and no production
KCore/MRES/ABI or persistent data was changed. Testkit commit is
`7df48f0`.

## 2026-11-04 v1117: booted stale WAIT/PID after multi-stage fatal release

The disposable 256K PDP-6 v1117 guest extends the validated v1103
PIPE MM_BUSY / FILE close / HUP-stage / VM / u-area fatal-release
sequence. Once the faulted child is fully retired and reaped, the
parent performs 48 repeated WAIT requests and 48 PID-directed TERM
requests against that exact reaped child. Each must return -1; every
iteration must leave process-slot and process-word accounting no
higher than at the start of the post-reap checks. The unrelated
spinner and foreign held FS-provider lease remain live throughout.

The full boot PASSes, preserving v1103's exact two PIPE frees/one
physical MM_BUSY, three FILE close calls/two refusals, two HUP-stage
calls/one refusal, two VM calls, two u-area calls/one refusal,
one EXIT/CHLD/finalization, no ownership errors, and 36 counted
bystander cycles. This is independent evidence against duplicate
WAIT and stale event delivery following the multi-stage teardown.

The initial test tried creating new child processes after the FS
provider was deliberately held. RUN legitimately requires that same
serialized provider to load an executable, so that design made no
post-fault progress; it was replaced with provider-independent
stale-WAIT/event checks, not a production workaround. The accepted
test does not establish numeric PID reuse or all concurrent P0-E
cross-products. Full P0-D/E and the final P0-F enable decision remain
OPEN. Production kernel code, resident RAM, MRES and OOM default are
unchanged.

## 2026-11-04 v1116: booted direct RUN descriptor-pressure cause -3

Two independent disposable 256K PDP-6 guests PASS with actual
MM descriptor-table saturation and a structurally valid child RUN.
The native allocator result is captured immediately at the actual
process-VM creation allocation and at the later child u-area MM
allocation in private source copies. The test accepts exactly one
genuine MM descriptor exhaustion (-3) at the reached failing stage;
the other stage must either have succeeded (0) or not have executed
(-7 test sentinel). The public RUN ABI still returns -1, with no
child process VM, u-area or process slot retained after the refusal.

The same test INIT frees each tracked one-word MM_TYPE_KERNEL_DYNAMIC
extent by exact base/type/owner and compares parent process-slot and
process-word accounting before continuing. The default OOM-disabled
guest PASSes and returns normally for true physical exhaustion. The
disposable OOM-enabled guest PASSes and kills only the later genuinely
exhausted requester, not the descriptor-pressured RUN caller or the
invalid BRK/RUN negative controls. The independent blocked witness,
expected WAIT results, and recovered MM accounting PASS in both.

All v1116 instrumentation, 36-bit counters, sentinel calls and
the optional OOM switch are restricted to isolated test kernels.
Native v1111 MM cause and v1113 MM-to-VM/BRK classification
compatibility tests PASS, including their unsafe mutants.
There is no production kernel/MRES/ABI change and no new resident
RAM consumption. The remaining P0-D resource-retry cross-products,
P0-E integrated fault/liveness/WAIT/PID run, and P0-F final
production-enable decision remain OPEN. Production OOM stays OFF.

## 2026-11-04 v1115: booted fragmented-core BRK -2 without OOM kill

The 256K PDP-6 v1115 full guest creates real fragmented physical
memory using disposable-kernel calls to the production MM allocator:
allocate aligned 040000-word kernel-dynamic extents until no further
such extent fits, then free alternating extents while tracking every
retained physical base. The actual `mm_find_fit` search (exported only
in the disposable copy) verifies at least 0200000 words free in total
but no contiguous 0200000-word replacement image. The parent requests
a 64K BRK; `vm_space_brk_current` returns -1 and real internal cause
-2, observed by a test-only AC2 capture immediately after the VM call.
The requester survives, its logical break is unchanged, and exact
typed MM frees release every recorded test extent.

The first 40K attempt exposed an important valid VM behavior: BRK
could still grow in place despite the lack of a complete 40K free
extent. Increasing the target to 64K excludes that delta-only path.
One MM descriptor was reclaimed during the BRK retry in the observed
guest. A strict before/after global descriptor equality check was
therefore incorrect; all tracked test extents were already freed.
The final test rejects increased descriptor counts, checks every
typed base/owner release, and restores parent process-MM accounting.

Both independent disposable boots PASS. Default OOM disabled yields
`<FRILDNWP`; test-only enabled OOM yields `<FRILKWP`. The -2 BRK
refusal never fatally retires the process, while the subsequent real
physical-OOM requester dies only with the isolated policy enabled.
The independent blocked witness, failed-RUN and invalid-BRK controls,
WAIT statuses and final process-word accounting all PASS.

No production assembly, resident kernel code/BSS, MRES or ABI changed.
Production OOM remains disabled. Direct internal RUN -3 cause tracing,
mixed P0-D/E fault integration and final P0-F acceptance remain OPEN.

## 2026-11-04 v1114: booted actual MM descriptor-table pressure

The disposable 256K PDP-6 v1114 guest extends v1075/v1110/v1112
with a test-only BRK sentinel protocol compiled into a private copy
of `syscall_dispatch_pdp6.s`. Production BRK dispatch stays unchanged.
The sentinel uses real `mm_alloc_aligned_noreclaim` to allocate
tracked, one-word MM_TYPE_KERNEL_DYNAMIC extents until
`mm_extent_count == mm_extent_capacity`. The parent attempts the
same structurally valid child RUN used by the successful v1110 tests;
RUN returns -1 with the MM descriptor table full. A second sentinel
releases every tracked physical base through production `mm_free`
with the exact type and owner. Parent process-slot and process-word
accounting return to their baseline before other regressions run.

Both separately built and booted guests PASS. Default-disabled OOM
prints `<MRILDNWP`; disposable OOM-enabled prints `<MRILKWP`.
Invalid RUN and BRK requests remain nonfatal, the subsequent physical
OOM requester dies only under the enabled policy, and the independent
blocked witness survives. The internal MM failure code returned by
the failed RUN was not directly recorded; full descriptor occupancy
at the beginning of RUN is proven by the sentinel. This closes the
booted physical descriptor-pressure child-RUN observation, but NOT
booted real BRK fragmentation or explicit RUN -3 cause tracing.

All additional code/BSS is compiled solely into the disposable test
image; production code, resident KCore and MRES occupancy, ABIs and
the default disabled OOM policy are unchanged. P0-D/E/F remain open.

## 2026-11-04 v1113: real MM -> VM BRK -> syscall error propagation

The native PDP-6 v1113 fixture links actual `mm_find_fit`,
`mm_extent_insert`, `mm_alloc_aligned_noreclaim`, `mm_alloc_aligned`,
`mm_compact`, `vm_space_brk_current` and the OOM-enabled
`native_sys_brk` entry. A live process VM at base 02000 cannot grow
in place because another extent immediately follows it. Fragmented
MM gaps total 06000 words but no individual gap fits the requested
04000-word replacement image. The real allocator returns -2 all the
way through VM and BRK, with no fatal dispatch and unchanged original
VM, process break and extent count. The fixture then uses two live
extents and descriptor capacity two: a large fitting gap exists but
the real insertion rejects it with -3, likewise without killing the
requester or changing the original VM/BRK/MM ownership.

Three deliberately unsafe mutations (VM cause lost, -2 collapsed to
-1, -3 collapsed to -1) are rejected by the same native test. FS and
swap reclaim callbacks, refused process relocation, and fatal selection
are fixture witnesses; the MM allocator, VM BRK and syscall decision
branches are genuine production assembly. This does not replace the
remaining booted fragmented/descriptor-pressure BRK and RUN coverage,
or the open P0-D/E gates. Production OOM remains disabled. No
production kernel text, resident data, MRES or ABI is changed.

## 2026-11-04 v1112: booted failed child RUN with OOM disabled and enabled

The optional v1112 real 256K PDP-6 guest extension tries a complete,
syntactically valid RUN V2 request to a nonexistent absolute executable.
The request returns -1 with no child slot remaining allocated. The parent
checks unchanged process-slot and MM process-word accounting, then
continues through the v1110 below-floor/address-limit BRK negatives
and the original v1075 genuine physical OOM requester/witness case.

Two separately booted images PASS. Production default disabled:
`<RILDNWP`, with the physical-OOM request returning -1. Test-only
OOM enabled: `<RILKWP`, with only the genuinely exhausted requester
fatally retired. The invalid RUN and invalid BRKs never cause a fatal
event, the independent blocked witness survives, all WAIT statuses
are as expected, and final MM accounting returns to the baseline.

This certifies a missing-executable RUN rejection, not an allocation-
pressure RUN failure. P0-F still requires booted MM descriptor and
fragmentation failures through BRK/RUN before enabling production OOM.
No production code, resident data, MRES, ABI or default policy changed.

## 2026-11-04 v1111: real MM shortage, fragmentation and descriptor limits

The native PDP-6 v1111 regression links actual production
`mm_find_fit`, `mm_extent_insert` and `mm_alloc_aligned_noreclaim`.
Two extents in one 010000-word arena leave two separate 02000-word
gaps. A 05000-word request fails as insufficient total RAM (-1);
03000 words fails as fragmented (-2) despite 04000 total free; a
01000-word request finds a gap but fails because both descriptor slots
are occupied (-3). In all three cases both original extents and the
extent count remain unchanged. Raising only descriptor capacity from
two to three permits the same 01000-word request to insert a new
sorted middle extent. Mutants that collapse -2 or -3 to -1 are rejected.

The separate production BRK-dispatch v1000 native test also PASSes:
only AC1=-1 together with the physical-shortage cause AC2=-1 triggers
the opt-in fatal path, not -2 fragmentation or validation failures.
This v1111 test covers the real allocator failure distinction; it
does not yet integrate all three errors with a booted BRK requester.
Production OOM is still disabled and no kernel code, resident RAM,
MRES or process layout changed. P0-F remains OPEN for booted MM
fragmentation/descriptor checks, failed-child RUN and P0-A..E gates.

## 2026-11-04 v1110: booted invalid BRK versus physical OOM classification

The optional v1110 extension of the existing v1075 full 256K PDP-6
guest runs two additional short-lived children before the physical
exhaustion requester. One attempts BRK below its original image floor
(`1`), and the other attempts the 18-bit address limit (`0777777`
octal), whose required APR rounding overflows that limit. Each must
receive `-1`, retain its original break, exit with a normal expected
WAIT status and return process-slot/word accounting to the baseline.

Both disposable configurations boot and PASS. With the production
default-off switch left at zero, the invalid children finish normally
and real physical exhaustion also returns `-1` to its requester; the
independent blocked witness survives and all process-MM accounting
returns to baseline. With the OOM branch enabled ONLY in a disposable
kernel copy, the two invalid children still finish normally, while
only the genuinely exhausted requester is fatally retired. The witness
and final MM accounting checks again PASS. Test-only console sequence
is `ILDNWP` when disabled and `ILKWP` when enabled, following the
initial `<` sentinel; the legacy v1075 behavior remains the default
without the v1110 option.

No production code, KCore data, MRES, process ABI or OOM policy changed.
This closes only the invalid-floor, address-limit and physical OOM
classification subset of P0-F; MM descriptor exhaustion, fragmentation,
failed child RUN and integrated P0-E/D acceptance remain OPEN.

## 2026-11-04 v1109: integrate VM destroy with physical MEMFS XIP refusal

The source-linked native PDP-6 v1109 regression executes production
`vm_space_destroy` together with the real `fs_provider_reg_call`
MEMFS direct XIP dispatch, `memfs_xip_owns_base`,
`memfs_xip_unpin_base`, and `mm_pin`/`mm_unpin` leaves. The fixture
starts with a MEMFS-owned XIP reference to an actual MM extent whose
pin count has been cleared. The first VM destroy obtains the provider's
owned-XIP -4 refusal, returns -1, retains the same VM descriptor and
MEMFS reference, and does not attempt an ordinary process-type MM
free or call the backing-detach witness.

Repairing the original extent using production `mm_pin` permits the
second VM destroy to release its exact MEMFS reference and MM pin,
invoke backing detach once, and publish VM_SPACE_NONE. The unrelated
serialized FS-provider lease remains held; no APR clock wait occurs.
Two unsafe variants (falling through to process MM free on an owned-XIP
error, and reversing the swap-detach success gate) are rejected.
The expanded native v1096 P0-D release-stage matrix PASSes with v1109;
its optional full booted guest set was not rerun.

The resident storage-mode check, swap-detach return and forbidden
process-MM fallback are counted test witnesses, not production-backed
allocations. The fixture prepublishes an owned XIP reference: it does
not exercise a trusted MEMFS XIP admission/certified executable image
or a booted process/WAIT/PID lifecycle. A real booted XIP admission
requires the no-write certification explicitly demanded by the existing
private `memfs_xip_admit` API; ordinary DXR EXEC does not confer it.
This remains a P0-D OPEN gate, alongside broader fault cross-products
and P0-E/F. No production text, BSS, MRES or OOM activation changes.

## 2026-11-04 v1108: real MEMFS XIP pin-state refusal and retry

The source-linked PDP-6 v1108 test extends the v1086 direct MEMFS
provider/XIP/MM leaf fixture with an actual MM extent whose pin count
has been cleared while its MEMFS XIP reference still exists. The
production `memfs_xip_unpin_base` calls real `mm_unpin`, which rejects
the unpinned extent with -4. The MEMFS release gateway preserves that
-4 as an owned-XIP refusal, retains the original XIP reference and MM
extent, and does not acquire the unrelated serialized provider lease
or wait for an APR clock tick.

The fixture repairs the exact MM extent with production `mm_pin` and
retries through the same XIP provider gateway. Release then succeeds,
the original reference is cleared and the physical pin is removed.
A repeated release correctly returns not-owned without double-unpin.
The v1108 physical XIP fixture additionally rejects two unsafe
mutations: dropping the reference after a failed MM unpin and retaining
the reference after a successful unpin. The v1086 baseline and v1094
owned-XIP/MM release and its four unsafe mutation controls PASS
independently. The expanded native v1096 P0-D
release-stage matrix also PASSes with v1108 included. No production source or memory
layout changed. A booted real process/VM MEMFS-XIP lifecycle fault
remains OPEN under P0-D, as do P0-E/F; OOM activation stays disabled.

## 2026-11-04 v1107: isolate SIDL discard flag from live run-queue link

The slot-0 finalizer first checks the process state before interpreting
scheduler RH bit `02` as an unpublished RUN's SIDL discard-VM marker.
For non-SIDL slots, the same RH field is a run-queue successor; a live
PREVM+FINALIZE process with successor slot 2 must still reach its VM
release retry rather than being skipped. The existing SIDL retry and
post-VM paths retain their original ownership and dispatch behavior.

The daimos-testkit v1107 source-linked PDP-6 test passes for both a
live PREVM owner with runq successor 2 and a real SIDL discard owner;
its old-order unsafe mutation is rejected.

The v1107 disposable 256K PDP-6 session/HUP guest also PASSes with
genuine DISK backing. Test INIT enables SWAP through STORAGECTL, and
the actual SWAP MRES reclaims session member slot 4, allocating a
16-block backing run beginning at valid block zero. The first HUP
enters the regular swap-detach verifier. A test-only change to one
allocated bitmap bit makes the verification fail, retaining the
nonresident process and its exact PREVM+FINALIZE retry owner. Slot 0
subsequently restores the bit and completes teardown without repeated
HUP delivery. The ordinary backing-store free is called once with the
original 16-block extent; the final process swap record is zero and
the allocation count returns to baseline. INIT receives the expected
HUP/WAIT statuses for the leader and both members and can reclaim
the controlling TTY.

Booted counters: one real swap-out; bitmap verification 2 calls, one
corruption/repair, one physical backing free, five slot-0 reaper scans,
zero repeat HUP deliveries, one first-member MM pin/unpin and zero
ownership errors. Original booted v907, pinned PIPE v1104, active
DPY v1105 and real resident-VM v1106 compatibility guests PASS;
the native P0-D matrix PASSes. The full production KCC image builds;
its KCore endpoint remains `023171` octal. No new permanent resident
state or MRES dependencies were added; production requester OOM stays
disabled.

This is an allocated-DISK *bitmap ownership verification* refusal,
not a hardware storage-I/O failure. P0-D remains OPEN for genuine
MEMFS-owned XIP release and broader simultaneous fault/WAIT/PID/leak
acceptance; P0-E and P0-F remain OPEN.

## 2026-11-04 v1106: booted physical process-VM MM_BUSY and owner retry

P0-D gains a booted real-MM process-VM release refusal in the existing
v997 held-provider/WAIT/bystander guest, with fault injection restricted
to a disposable 256K PDP-6 kernel image in daimos-testkit. This mode
replaces v997's synthetic initial `vm_space_destroy` rejection: the
first slot-5 VM teardown physically pins the actual MM_TYPE_PROCESS
extent recorded in the process VM descriptor (base `0752000` octal).
The unchanged `vm_space_destroy` queries XIP ownership through its
provider gateway and then calls production `mm_free` on that extent,
which returns MM_ERR_BUSY (-5) because the extent is physically pinned.
The PREVM/FINALIZE owner survives for slot-0 retry, without releasing
the original process VM or prematurely publishing VM_SPACE_NONE.

On the second VM destroy, the test unpins the *same* extent. Production
`mm_free` validates the exact owner/type and deletes it once, then the
real `proc_swap_detach` stage executes once and VM_SPACE_NONE commits.
This is a resident process with no allocated disk-swap backing: it does
not establish the disk-swap bitmap failure path. The existing v997
test simultaneously refuses FILE close twice, retains a genuine live
foreign FS-provider lease, and rejects the first actual u-area MM free
(-5) before successful slot-0 completion. The independent bystander
executes 36 scheduling cycles; one CHLD/EXIT/finalizer commits.

Booted v1106 PASS: VM calls 2; process VM `mm_free` attempts 2,
physically pin/unpin 1/1, MM_BUSY 1, exact MM delete 1, swap-detach
stage executions 1, instrumentation errors 0; u-area calls 2 with
one real MM_BUSY; FILE calls 3/refusals 2; bystander 36, one CHLD and
one EXIT, no provider ownership errors. The unmodified v997 booted
guest and the optional v1103 multistage booted guest both rerun PASS;
the native v1096 release-stage matrix PASS. All hooks and counters are
test-only; production source,
KCore low endpoint (`023171` octal), resident data and OOM policy are
unchanged. P0-D/E/F remain OPEN for physically held MEMFS-XIP and
allocated disk-swap backing, broad PID/WAIT/leak cross-products and
combined OOM activation gates.

## 2026-11-04 v1105: booted physical session HUP and Type-340 DPY MM refusal

P0-D now has a booted physical Type-340 cross-resource refusal test in
daimos-testkit, extending v907's real two-member session/TTY guest without
altering production kernel code. The first member receives a real HUP and
commits VM_NONE+FINALIZE while its exact stable u-area remains physically
pinned, pending slot-0 release. The second member installs a genuine 32-word
Type-340 raw display list through `/DEV/DPY0` before its own remote HUP.

The second HUP reaches the production FILE DPY close dispatch and
`dpy_write_words` STOP, which quiesces PI7 and calls actual `mm_free`
on the published MM_TYPE_KERNEL_DYNAMIC / owner 014 display-list extent.
The test-only hook pins the exact extent once; production `mm_free`
returns MM_ERR_BUSY (-5) and retains the FILE descriptor and original
raw-list owner/IOWD for retry. INIT verifies the leader cannot yet be
reaped with WAIT_NOHANG. During the refusal interval the display clock
replays the same list through the live PI6 refresh path.

On the second HUP, production session teardown skips the already
irreversible first member. The original DPY extent is physically
unpinned and freed by the unchanged MM allocator; its FILE descriptor
then closes. TTY session commit unpins the first member's stable u-area.
The guest verifies all three HUP/WAIT statuses and TTY handoff with no
duplicated HUP, premature WAIT or owner error.

Booted PDP-6 v1105 PASS: DPY physical MM free attempts 2, one pin,
one unpin, exactly one MM_BUSY rejection and one successful free, 14
actual PI6 raw-list refreshes (8 after busy and before unpin), zero
errors; first-member u-area pin/unpin 1/1 and repeated member HUP 0.
Independent reruns of the booted v1104 PIPE/HUP and original v907
physical HUP fixtures PASS, as does the complete v1096 native P0-D
release-stage matrix.
Test-only fault hooks and counters exist solely in the disposable
guest kernel. Production KCore remains `023171` octal, OOM remains OFF.

P0-D remains OPEN for simultaneous MEMFS-XIP, swap backing and VM
failure combinations, broader failure cross-products and final resource
leak/WAIT/PID acceptance. P0-E/F remain OPEN.

## 2026-11-04 v1104: real session-member HUP plus physical PIPE MM_BUSY

P0-D gained a booted physical cross-resource refusal case using the
existing v907 two-member session/TTY guest. Unlike v1103's synthetic
HUP-stage rejection, this exercises real HUP delivery and actual FILE,
PIPE and MM allocator release transactions on the same session.

Each session member creates a private anonymous PIPE. On the leader's
first HUP, the first member commits its EXIT/VM teardown but retains
the original physically pinned u-area, owned by the slot-0 finalizer.
The second member's final PIPE descriptor reaches production `mm_free`;
the isolated test hook pins that exact MM_TYPE_KERNEL_DYNAMIC extent
(owner PIPE=5), causing one real MM_ERR_BUSY (-5). Its last writer
reference and FILE descriptor remain live and the leader's HUP reports
incomplete. The parent checks that the leader is not yet waitable.

On the second HUP, the session code skips the committed first member,
retries the second member's exact PIPE, unpins the physical extent and
frees it exactly once through the unmodified MM validation. The TTY
session commit then unpins the first member's u-area; WAIT reports the
leader and both HUPed members once, with no repeated committed HUP.

The v1104 booted 256K guest verifies PIPE calls 2, pins 1, unpins 1,
MM_BUSY 1, successful free 1 and zero ownership errors; first member
deferred u-area pin/unpin 1/1, HUP rejection/retry 1/1, repeated HUP
0, committed first member 1 and normal TTY handoff. Native v1096
release-stage matrix PASS; original v907 booted physical-HUP baseline
rerun without the new option PASS. All v1104 hooks and counters exist only in
daimos-testkit's disposable kernel copy. The production source and
KCore endpoint (`023171` octal) remain unchanged from v1102.

This closes the physical session HUP + PIPE final MM refusal leaf but
not P0-D overall: combined DPY, MEMFS-XIP/VM and swap failures, and
additional long-duration PID/WAIT and no-leak combinations remain.
P0-E/F remain OPEN; production requester OOM termination is OFF.

## 2026-11-04 v1103: booted multi-stage fatal owner fault matrix

The new daimos-testkit v1103 optional full-guest regression extends the
disposable v997 256K PDP-6 image rather than copying its boot harness.
It exercises sequential failure and retry in the **same real child**:
FILE close is refused twice by the original v997 test hook; a private
anonymous PIPE survives its first real final `mm_free` returning
MM_ERR_BUSY (-5), with the expected exact reference counts preserved,
and is freed after one subsequent retry. The child then receives a
test-only refusal at the HUP-stage boundary in `proc_session_teardown`,
followed by success. This refusal does not represent actual physical
TTY HUP delivery; the booted v907 regression covers real session
members and partial HUP commit independently. Next, the existing v997
test hook rejects the first VM destroy and the production MM allocator
rejects the first stable u-area free (-5), both with successful retries.

The v1103 booted guest PASSes with FILE calls 3/refusals 2, PIPE MM
calls 2/refusals 1, HUP-stage calls 2/refusals 1, VM calls 2, u-area
calls 2/MM refusals 1, CHLD/EXIT/finalization exactly once and no
ownership error. The held foreign FS-provider lease never changes;
the independent bystander executes 36 cycles. No repeated HUP is
triggered after PREVM publication and the original session/WAIT owner
is finalized only after all release stages accept completion.
The original v997 full guest was rerun without the v1103 option and
PASSes; the complete source-linked v1096 native P0-D matrix also
PASSes against the same production tree.

This is a daimos-testkit-only, isolated-source fault injection. The
production DAIMOS assembler/C source remains unchanged from v1102,
including its disabled OOM gate, KCore endpoint `023171` octal, and
existing resident owner/backoff state. The original v997 regression
remains available separately and v1103 is included in optional
`P0D_FULL=1` validation. P0-D/E/F remain OPEN: simultaneous physical
HUP+PIPE/DPY, XIP/swap and exhaustive WAIT/PID fault cross-products
still need booted acceptance.

## 2026-11-04 v1101-v1102: invalid-owner MM quarantine and idle reaper

P0-D's new booted v1101 physical-MM wrong-owner diagnostic exposed a
reaper backoff regression despite retaining the correct process/u-area
owner. In a disposable 256K PDP-6 guest, the MM hook poisons the exact
dying slot-2 u-area descriptor once (owner 01002 becomes 01003); all
subsequent original `mm_free` calls therefore return MM_ERR_INVAL -4.
The process remains quarantined, the MM extent remains allocated and
the read-only `GETTEARDOWN` detail reports WRONG_OWNER (0402). The
initial guest nevertheless recorded 41 invalid free attempts and 40
idle-reaper invocations during the test interval, not the capped
exponential retry frequency expected from v885.

Root cause: v1098 correctly distinguished a prepublished EXIT
FINALIZE+PREVM marker without a retry clock from an already armed
pending owner. But the idle loop still cleared the *shared pending bit*
before entering `proc_finish_service_v881`, as it had done before the
v1098 distinction. Every failed retry then observed the cleared bit
and reinitialized a 020-octal-tick first retry; backoff advancement
never exceeded the first step. This affected both permanent MM_BUSY
(-5) and MM_ERR_INVAL (-4), not just the new wrong-owner test.

v1102 fixes the shared retry ownership protocol at the source. Slot-0
keeps the pending bit set while scanning. The existing saved AC11
records both completion (bit 0) and any failed retry (bit 1). After
the full scan, the pending bit is cleared only if no retry owner
remains refused. Bit 1 is removed before returning, preserving the
old completion/last-user ABI. `proc_finish_backoff_advance_v885` now
sees a stable pending decision and doubles 020,040,0100...01000 octal
on persistent failures. This changes only instructions, no new BSS,
per-slot retry flags, MM state or MRES module.

Native source-linked PDP-6 v1102 validates two consecutive failures,
16/32/64-tick backoff and successful retirement. Its four unsafe
source mutants include the old early-clear path and are rejected.
Existing v881 and v1098 native finalizer tests and v1096 full native
release-stage matrix PASS. The repaired booted v1101 wrong-owner guest
PASSes with one poisoned extent, seven -4 refusals, six reaper scans,
read-only wrong-owner MM status, no premature WAIT, runnable bystander
and zero unrelated MM errors. The unchanged v909 pinned-uarea guest
also PASSes: one physical pin, seven -5 refusals, six scans and live
owner/bystander. Full 256K production KCC image builds; KCore low
endpoint moves `023162` -> `023171` octal (+7 code words), with no
persistent data increase. OOM production kill remains OFF.

P0-D/E/F are not closed: simultaneous multi-resource/provider fault
and exhaustion negatives still require independent guest acceptance.

## 2026-11-04 v1100: preserve typed old-DPY-list MM refusal

P0-D found another typed release boundary. `dpy_write_words` keeps
the previously published raw display list, immutable IOWD and clock
refresh owner if `mm_free` rejects its extent, but its successful
rollback of the newly allocated replacement overwrote the original
MM error with generic -1. This hid `MM_ERR_BUSY` (-5) versus invalid
MM type/owner (`MM_ERR_INVAL`, -4) from the FILE final-close path and
its error diagnostics.

The rollback now stores the original old-list MM result in the
already-allocated temporary stack word before freeing the new extent.
After a successful rollback it restores that result verbatim; a
second failed free still publishes the new orphan in the preexisting
`dpy_list_orphan_v912` owner and reports the latter MM result. Neither
the old display program, its IOWD, refresh state, descriptor, nor
the original owner are published as freed on a failed release.
No additional global word, dynamic owner, MRES or process field.

Native PDP-6 v874 replacement rollback/retry PASS; v1100 executes
actual production driver source for both -5 and -4 old-list failures,
verifies later successful replacement and stop, and rejects two
unsafe mutations that lose the first error. The v1100 component matrix
and combined booted v997/v1077 fault guests PASS on the prior release;
the current v1100 full KCC 256K production image build also PASSes,
with unchanged `__kcore_low_end` `023162` octal and no new BSS.
This is an individual P0-D leaf improvement, not closure of the
entire multi-stage matrix. Production OOM kill remains disabled.

## 2026-11-04 v1099: preserve PIPE/FIFO final-close MM error type

P0-D source audit found a real error-classification loss in the final
PIPE/FIFO FILE close path. `pipe_close_ref` correctly delays publishing
reader/writer decrements and restores a temporarily unlinked named
FIFO if its last MM extent free fails. But its failure path routed both
`MM_ERR_BUSY` (-5, pinned) and `MM_ERR_INVAL` (-4, wrong type/owner)
through `pipe_close_bad`, replacing the original error with -1. That
prevents FILE and fatal-close diagnostics from distinguishing permanent
owner corruption from a retryable physical pin.

The failure path now returns directly through `pipe_close_done`. For
anonymous pipes, no reference count or FD is cleared. For named FIFOs,
the nonallocating head insertion uses scratch AC3 rather than AC1;
the restored list and original reader/writer count survive while AC1
retains the exact MM return to FILE. Invalid *pipe arguments* still
use the existing generic bad path. Neither successful reference
decrement nor last-free commit ordering is changed. The extant v899
and v900 booted real-MM PIPE pin and FIFO reopen tests establish the
original owner protocol separately.

daimos-testkit native v1099 uses production source for the FILE close
entry, FIFO unlink and PIPE last-close while injecting exact -5 and
-4 allocator refusals. The original descriptors, counts and named
FIFO list remain intact and can later be released once; four unsafe
mutations are rejected (status collapse twice, lost FIFO owner,
premature count publication). The extended P0-D v1096 component
matrix PASSes, as do v1087 no-new-allocation checks and all eleven
original unsafe source mutations. The production full KCC 256K
image PASSes: KCORE low end remains `023162` octal, with no new fixed
state, process fields, MRES or syscall changes.

P0-D remains OPEN for simultaneous booted cross-stage failures and
permanent wrong-owner diagnostics. P0-E/F remain OPEN; production
requester OOM termination stays OFF.

## 2026-11-04 v1098: prepublished PREVM exit now arms retry epoch

P0-D full-system validation exposed a genuine preexisting unbounded
reaper stall. `proc_exit_current` publishes FINALIZE+PREVM under the
brief PI-disabled stack handoff as required by v1086; that is an owner
publication, **not** a pending timeout request. When the first VM
destroy failed, `proc_finish_mark_pending_v892` saw the marker and
jumped straight to rearm without initializing the first delay. This
published the global pending flag with an unset delay. The idle
reaper's explicit zero-delay safety guard correctly ignored that
malformed state indefinitely while unrelated users continued running.

Before v1098, the booted v997 disposable 256K guest observed one VM
destroy attempt, zero u-area attempts, correct descendant CHLD and
261 bystander cycles, but it timed out waiting for the marked owner
to complete. That failure was not caused by v1097 typed return codes.
v1098 makes `proc_finish_mark_pending_v892` distinguish the existing
per-owner FINALIZE marker from the global pending-bit/clock lifecycle:
an already pending prior retry preserves its delay, while a first
refusal with a prepublished marker initializes the same 020-octal-tick
first retry and bounded APR epoch as a new owner. No new state word,
forced release or device sleep is introduced.

Source-linked native PDP-6 v1098 exercises prepublication/no clock,
existing retry backoff, a new finalizer with old backoff and a stale
epoch; three deliberately unsafe mutations fail. Native v1081/v1085,
v881, PI6 safety v1086 and allocation closure v1087 also PASS after
the test fixtures supply their existing kernel clock word.

The repaired booted v997 guest PASSes: two VM calls including one
initial refusal, two final u-area MM calls with one MM_BUSY refusal,
three FILE-close attempts with two refused, one descendant CHLD,
one owner EXIT, an untouched held foreign filesystem lease and
36 independent bystander cycles. Zero errors, premature frees or
duplicate finalization observed. Production OOM remains OFF pending
remaining P0-D/E/F acceptance.
The v1077 disposable physical OOM/D6FS/MM guest also PASSes on the
same v1098 sources (two rejected SYNC, one rejected MM free, correct
requester and independent witness recovery). Full production KCC
build PASS; KCORE end `023156` -> `023162` octal (+4 code words), no
additional BSS, timer, process or MRES state. All v1096 component
matrix tests, v1086 PI6, v1087 allocation closure and v1098 native
clock/backoff negative controls PASS. The production OOM bit is OFF.

## 2026-11-04 v1096-v1097: typed final u-area release and stage matrix

P0-D found a genuine loss of MM error classification in final process
resource release. `proc_uarea_release` called the actual `mm_free`
but returned generic -1 for every failed result, hiding the difference
between a physically pinned extent (-5) and an impossible owner/type
or missing extent (-4). The v1097 implementation keeps the exact
allocator error after restoring saved AC10, and returns -4 for a
corrupted stable-u-area descriptor whose owner flag is set but whose
base is zero. Neither branch clears the descriptor or tries to free a
different owner's memory. Successful MM deletion and the already-
absent case still return zero; subsequent finalizer stages are gated
on that success. Existing process/finalizer callers test nonzero and
retain the same PREVM/POSTVM retry owner and commit ordering.

The v1097 native test runs the production `proc_uarea_release` and
PDP-6 MM leaves on the simulator, covering invalid owner/type, a
pinned u-area, corrupted zero base, repaired metadata, exactly-once
free, and a second harmless release. All PASS. Three deliberately
unsafe mutations (collapsing errors, reversing the commit guard, and
misclassifying invalid base) are rejected. The v1096 source-linked
component matrix additionally PASSes PIPE last close, DPY replacement,
swap bitmap release, HUP retry, both TTY PREVM finalizers, typed TTY
release, XIP/VM ownership and allocation-free fatal graph checks.

The full KCC 256K production image builds successfully. KCORE end
changes `023154` -> `023156` octal (+2 code words), with no extra
KCORE BSS, owner table, FILE/MM descriptor, MRES or ABI storage.
The first v997 booted acceptance attempt on v1097 timed out because
of the first PREVM timer defect detailed in v1098 above. On v1098,
both v997 and v1077 guest regressions PASS as independent executions.
P0-D/E/F remain OPEN; OOM requester termination stays OFF.

## 2026-11-03 v1093-v1094: typed MM refusal and XIP/VM commit guard

P0-D identified two release-stage ambiguities, fixed without adding
process metadata or permanent storage.

v1093: `proc_tty_line_try_reset_v897` previously invoked the void
canonical-line reset and returned generic -1 for any surviving base.
That prevented a direct caller from distinguishing retryable MM_BUSY
(-5) from invalid type/owner/missing extent (-4). The checked helper
now performs the authoritative `mm_free` directly and propagates its
real return; zero is published only after successful free or for a
genuinely absent line. The historical `proc_tty_line_reset` void ABI
uses that same implementation and intentionally ignores the result.
On any failure, the original per-TTY base and associated session
remain owned by their previous process; finalizers retain the PREVM
stage, and no new allocation or retry field appears. The actual MM
native v897/v968 tests pass for -5/-4 and subsequent exact free; two
unsafe mutants (collapse to -1, premature base-clear) are rejected.
The v898 disposable booted PDP-6 physical-MM TTY pin/refusal/unpin
guest passes with one pin, one unpin, one busy and no errors. Its
instrumentation sees five checked helper calls instead of four because
the unchecked reset now shares the same leaf; no duplicate pin occurs.

v1094: `vm_space_destroy` previously treated all MEMFS XIP release
returns other than 1 as a reason to try MM_TYPE_PROCESS freeing. But
an *owned* MEMFS XIP unpin error used -1, indistinguishable from the
unloaded MRES's absent-provider sentinel. The provider now maps an
owned unpin failure to -4; VM teardown accepts the MM_TYPE_PROCESS
fallback only after status 0 (not XIP) or -1 (provider absent).
Other negative statuses fail closed before swap-bitmap detach or VM_NONE
publication, leaving the XIP reference and process slot as retry owner.
The native v1094 source-linked real VM/MEMFS release leaf test covers
owned refusal/retry, normal and absent provider fallback, MM -4/-5,
failed swap detach/retry and four unsafe mutations. Its initial
absent-provider test oracle needed a corrected PDP-6 SETO AC syntax;
after fixing the fixture, all cases PASS. Existing v1086 PI6, v880
swap detach, v1081 finalizer and v1087 release-graph checks PASS.

Matched production 256K KCC full-image `__kcore_low_end` changes from
octal `023161` to `023154`, a **five-word resident decrease**. No
fixed-data, MRES, FILE, process or MM extent count is added. P0-D is
still OPEN for the combined HUP/TTY/XIP/swap/PIPE/DPY/u-area retry
and permanent invalid-owner matrix. Production OOM kill remains OFF.
The v1077 booted 256K requester/D6FS/MM fault guest also PASSes on
the updated tree with two D6FS SYNC refusals, one MM_BUSY refusal,
correct requester exit/reclamation and independent witness survival.

## 2026-11-03 v1092: P0-B owner-safe provider quarantine policy CLOSED

P0-B is CLOSED as a **safety/containment** gate, deliberately not a
guarantee of bounded provider completion. The remaining case is an
executing provider that never returns or a FILE provider whose CLOSE
continues to reject with -4/-5. In both cases it is impossible to
free a process-private executive stack, its live FILE/MM references,
the owner-encoded `fs_provider_ready` lease or the shared provider
workspace without proof that its actual caller has unwound. An
arbitrary watchdog, remote fatal delivery or capped retry interval is
not such proof.

Chosen kernel contract is **fail-closed quarantine** using the
existing owner and scheduler state, not a new quarantine table:

* An unreturned provider keeps its lease (LH=owner slot+1, RH=0),
  stable u-area/private continuation, process slot and resources;
  only the provider's own successful return unlocks and wakes normal
  waiters. No forced cancel, fake completion or PID reuse is allowed.
* A fatal event on a process waiting before provider dispatch can
  withdraw that *unstarted* request as in v1090/v1091. An ordinary
  waiter may remain blocked until the provider returns; it is not a
  provider owner and the kernel does not promise its wait will end.
* A returned, permanently failing CLOSE keeps the process and FILE
  ownership in `proc_fatal_close_current_v879`. Its private backoff
  reaches 01000 octal (512) clock ticks, then retries at capped
  frequency; this bounds CPU load, not the number of retries or the
  amount of time to reclaim. MM wrong-owner/type failures are separate
  P0-D stage/idempotence work and cannot be treated as MM_BUSY.
* Existing `SYS_PROCCTL_GETTEARDOWN`/`PS` stage 1 (FILE), descriptor
  detail and ERR expose actual returned close refusal without private
  addresses. For a live provider that has not returned, examine the
  `fs_provider_ready` word via its KCORE map address in a diagnostic
  simulator/console: RH nonzero is free; RH=0 and LH nonzero encode
  owner slot+1. Inspection is read-only: NEVER DEPOSIT a free bit,
  wake its event artificially or free its owner's u-area.
* Administrative response is to diagnose and fix the owning provider
  or media and allow its real return/retry, or perform a controlled
  full-system restart when it cannot quiesce. Online recovery is
  permitted only after an independently proven provider-specific
  quiescence/return protocol; none is asserted here. Reproducible
  software nonreturn is still a correctness bug to fix at its source.

daimos-testkit v1092 source-linked fail-closed contract PASSes with
14 unsafe control-flow/ownership mutations rejected; v954, v1088,
v1090, v1087 transitive owner graph and current native lease tests
PASS. On current source, booted v891 indefinite VFS -5 rejection
PASSes: 11 CLOSE attempts, 11 private sleeps, FD/error witness held,
512-tick delay cap, zero premature EXIT, and independent user progress.
Booted v914 also PASSes for eleven permanent -4 provider rejections,
preserving the FD, returned-error witness, live owner and bystander
without pretending an invalid provider result was a completed CLOSE.
Booted v953 provider revival after the same eleven refusals PASSes:
twelfth provider call succeeds; FD/witness are cleared and EXIT
publishes exactly once. The v1091 booted held-owner/foreign-waiter
guest establishes owner-stack survival and safe fatal waiter retreat
while the lease remains held. No production code or resident data is
changed at v1092; KCORE endpoint remains 023161 octal.

P0-B closure does not enable automatic OOM termination: inability to
reclaim a quarantined requester is explicitly possible. P0-D/E/F
must still prove their independent fault-stage, integrated guest,
negative classification and production enable conditions. Production
OOM requester kill remains OFF.

## 2026-11-03 v1090-v1091: fatal event can retire a pre-dispatch FS waiter

P0-B identified an independently recoverable part of the nonreturning
provider case. A DTFS/D6FS/TSFS register provider can suspend while
holding `fs_provider_ready` owner encoding and a process-private stack.
Another process blocked in `fs_provider_lock_sleep_v915` owns no active
provider transaction or its shared buffer; previously an accepted TERM,
HUP, INT or PIPE only set that waiter's pending event bits. It could
remain stuck forever if the foreign owner never released the lease.

The existing `proc_event_fatal_other_defer_term_v926` now wakes exactly
one pending-fatal target *only* if its descriptor records the normal
FS gateway wait-event address and precise SLEEP/WAIT_EVENT class.
WAIT_CHILD, WAIT_INTR, job-stopped and unrelated event waiters are not
touched. The awakened `fs_provider_lock_retry` reads the existing
u-area FDCTL bitmap, refuses an unstarted fatal-marked request with -5
before acquisition, and unwinds its own saved seven-word frame. User-
return fatal event consumption performs ordinary owner-side cleanup.
An established PROC_FATAL_CLOSE_BIT cleanup owner bypasses pending
bits so its FILE release can continue when the provider is free.
No foreign provider stack is aborted or lease unlocked by either path.

Native source-linked PDP-6 v1090 proves the waiter wake, exact owner
retention, unrelated event/job STOP/interrupt exclusions, subsequent
owner-side free-lease dispatch and four unsafe mutation controls.
The expanded v1087 allocation closure includes the existing, bounded,
allocation-free `proc_runq_add` direct edge from this new wake path.
The booted v1091 disposable 256K guest holds slot 3 indefinitely inside
the true serialized gateway (`4,,0` lease), sends TERM to its waiting
slot 4, and observes slot 4 consume the event, exit, and be reaped via
correct parent WAIT status. A separate slot 2 stays runnable across
73 sleep cycles. There are two deferred fatal events and zero errors;
holder lease, owner continuation and device/provider workspace remain
unchanged. The full production KCC image rebuild PASSes; octal KCORE
low end increases `023127` -> `023161`, +26 resident words and no BSS,
MRES or process-table growth.
The full v1077 256K physical-OOM requester/D6FS/MM fault guest also
PASSes on the v1091 tree (two D6FS SYNC refusals, one MM_BUSY
refusal, correct requester recovery and surviving independent witness).

This is deliberately *not* a general kill or cancellation of a live
provider. Permanently rejecting FILE/MM and arbitrary nonreturning
provider code remain fail-closed with intact ownership and throttled
retry. P0-B and P0-D/E/F remain open; production OOM kill stays OFF.

## 2026-11-03 v1089: bound Type-136 tape data-transfer completion

P0-B investigation found a reachable driver-specific lifetime defect:
DECtape search and MTC control had wall-clock timeouts, but DTC/MTC
data transfers awaited their event after entering negative storage_state
without any clock-based completion guarantee. Lost PI3/PI5 completion
could indefinitely retain the live Type-136 buffer, process-private
stack and a DTFS/TSFS serialized filesystem provider lease.

The tape MRES now exports `tape_watchdog_tick_v1089`. KINIT patches a
direct `storage_clock_tape_jump` into the existing qualified APR tick
sequence; unloaded tape remains a zero-work no-op. The existing
`dtc_search_retries` word becomes a watchdog after search succeeds,
and doubles as the MTC runtime data timer. DECtape gets 0454 octal
(300) ticks; Type-516 tape gets 012430 octal (5400) ticks. Normal
PI completion is unchanged. Timeout marks `storage_state=7`, stops
the Type-136/tape controllers, invalidates DTC motion, accounts the
device error and wakes the existing tape event. Only the actual
waiting owner resumes, unwinds and releases the provider lease. No
foreign lock reset, synthetic wakeup, process retirement or additional
persistent timer word is introduced. The future module-mover binding
list includes the new direct tape-clock call.

The `daimos-tape-data-watchdog-v1089` native source-linked PDP-6
fixture verifies DTC read/write, MTC read/write, exact countdown,
single timeout completion, unrelated storage states and a negative
mutant. Production tape-driver online regressions for Type 516 record
I/O/control, two-block Type 551 streaming and reverse block I/O PASS.
Full KCC 256K `ROOT=auto` image build PASS; booted full-media SIMH
reports DTC and MTC `OK`, then reaches `INIT V1` and `LOGIN` before
an intentional 35-second harness timeout. Comparable KCC KCORE end
increases `023126` to `023127` octal, one resident instruction word;
the countdown reuses an existing MRES word and adds no permanent BSS.
Same-linker baseline/current tape MRES comparison gives source text
360 -> 390 words and packaged resident 375 -> 405 (+30), unchanged
15-word BSS. Combined KCORE/tape runtime residency grows 31 words.
The independent v1077 256K physical-OOM requester/D6FS/MM guest also
PASSes against the revised tree: SYNC refusal 2/3 calls, MM_BUSY
refusal 1/2 calls, and original independent witness recovery intact.

This resolves the specific data-controller stall under healthy APR
ticks, not an arbitrary software-stalled provider or permanently
refusing FILE close. P0-B/D/E/F remain OPEN; production requester OOM
termination remains disabled.

## 2026-11-03 v1088: hold a real live provider frame through contention

The new `daimos-provider-inflight-owner-v1088` PDP-6 native test uses
the actual source-extracted register gateway, and enters a provider
oracle on the owning kernel stack before making recursive and foreign
fatal requests. Both fail with -5, never borrow the provider workspace,
do not sleep and cannot release the owner-encoded lock. The original
provider's error return unlocks and wakes exactly once; a later
successful request reacquires/releases it. Two negative controls that
turn either refusal into a blocking wait fail as expected. The v1078
fatal-contention and v327 provider-lock regressions PASS. No DAIMOS
source change or new permanent RAM at this stage. This is an ownership
test, not a proof of recovering a provider call that never returns.

## 2026-11-03 v1087: P0-A transitive fatal release graph CLOSED

No new production code is required. An executable fail-closed source
contract in daimos-testkit now follows the currently reachable
requester-only BRK, APR and post-commit EXEC fatal entries, normal
EXIT and immediate/deferred fatal events through the private owner
FILE closure and session HUP phase, and onward through marked TTY,
VM/XIP, MM/u-area, swap backing and parent-notification finalization.

Current VFS_SYNC is reachable only for DTFS and D6FS provider numbers
5 and 6. DTFS SYNC performs vnode validation; D6FS SYNC is a zero
return. Other providers are not sent a SYNC from file_close. The
serial register gateway preserves the private owner stack while
waiting or returns -5 to a fatal owner behind another provider lease.
Its lock is released only by the actual returning provider owner.
PIPE last-ref mm_free happens before dropping counts; DPY STOP retains
the FILE owner if MM refuses its published raw-list extent. PIPE
non-final close wakeup, FIFO list unlink and parent runqueue/wakeup
callbacks do not allocate or invoke storage I/O.

The HUP member scan retains the live TTY/session on failed event,
checked TTY-line free or a pending foreign release; it skips only
already committed TRANSITION+VM_NONE+FINALIZE members. Fatal-other
FILE close failure preserves the victim's descriptor and does not
enter VM destruction. Unpublished RUN does not publish file-reference
ownership until validation succeeds and uses the SIDL retry owner on
MM refusal. The private fatal owner retains both FILE refs and its
stack across failed CLOSE/HUP with a clock-backed retry; after success
v1086 publishes PREVM before PI6-inhibited finalization. All exposed
MM/VM/TTY/XIP, u-area and swap backing failures retain their existing
process or FD retry owner. No new MM or backstore allocation is
reachable on these current fatal release and retry paths.

The v1087 graph asserts exact callback edges, source result/commit
ordering and absence of fresh allocations. Eleven injected mutations
to FILE/HUP/foreign fail guards, VFS/D6FS SYNC, PIPE/DPY, TTY/VM,
RUN discard and owner retry are rejected. Native PDP-6 v870, v904,
v912, v957, v905, v922, v995, v1078, v1081 and v1085 PASS; v922's
historical no-TTY-line fixture gained its checked v897 success stub.
The v1077 booted 256K physical-OOM requester/provider-MM guest also
PASSes its two D6FS and one MM_BUSY injected failures. There are no
new KCORE/MRES instructions or permanent RAM words; the comparable
production KCORE end remains 023126 octal.

P0-A is CLOSED for the actual current fatal teardown call graph and
error/owner boundaries, **not** for arbitrary future MRES code or
infinite in-flight provider quiescence. The latter is explicitly
P0-B: an already-running foreign provider must retain its live
stack/lease until its own completion or an independently proven safe
provider-specific quiescence protocol. P0-D still owns detailed
failure-stage/idempotence combinations, P0-E the combined booted
matrix, and P0-F the final enable decision. Production OOM stays OFF.

## 2026-11-03 v1086: P0-C PI6-masked slot-0 teardown CLOSED

The full reachability audit uncovered a real second HUP path: after
private-stack HUP preflight, `proc_exit_current` switched to the idle
stack and inhibited PI6, but the unmarked `proc_finish_slot_owner_v932`
could call `proc_session_teardown` *again*. A deferred/foreign process
HUP may close FILEs or sleep for an active provider while PI6 is off.
Likewise, immediate self-fatal event delivery bypassed the preflight
owner-retry route and jumped directly to `proc_exit_current`.

v1086 publishes the already defined PREVM+FINALIZE marker and the
TRANSITION owner bit atomically under disabled global PI *before*
switching the dying process to the permanent idle stack. The marked
finalizer dispatch bypasses HUP and only performs TTY canonical-line
checked MM release, MEMFS direct XIP unpin, VM/bitmap detach,
u-area release and process-table notification. Immediate self-fatal
events now enter the existing private owner FILE/HUP retry path.
Normal EXIT already does private-stack HUP preflight; APR/EXEC fatal
owner also does so. No new permanent retry state or global timer.

The separate periodic `proc_idle_loop` PI6-masked reaper invokes only
explicitly marked PREVM/post-VM finalizers or SIDL RUN rollback; it
cannot dispatch HUP or disk-backed swap-in. The latter executes only
after PI6 restoration. MEMFS provider index 4 uses the nonserialized
gateway path, and operation 026 dispatches directly to XIP ownership
lookup and MM unpin. Disk swap-detach checks/changes the bitmap, not
media I/O. The C TTY canonical reset likewise only invokes mm_free.

The v1086 executable source graph checks these exact gates and all
direct calls, with seven injected unsafe-source mutations rejected.
Native PDP-6 tests PASS for real gateway/MEMFS XIP/MM unpin despite a
foreign-held serialized lease, EXIT marker handoff and old-source
negative control, and marked idle finalizer with an HUP-refusal trap.
The v974 self-fatal, v926 deferred owner, v970/v995 owner-HUP, v993
EXIT preflight and full v1077 physical OOM, session teardown and
process-events guest tests PASS. Comparable KCC KCORE low end grows
`023117` -> `023126` octal (+7 instruction words); no new persistent
data, MRES exports or changed process ABI.

P0-C is CLOSED for the defined *slot-0 fatal finalizer* entry graph.
This does not certify every unrelated kernel driver against deadlock:
P0-A transitive owner allocation/cleanup, P0-B foreign in-flight
provider quiescence, P0-D error commit/idempotence, and P0-E/F broad
acceptance/activation still remain open. Hardware APR clock failure
is outside the DAIMOS OOM recovery contract. Production OOM stays OFF.

## 2026-11-03 v1085: unpublished RUN TTY ownership and OOM clock scope

An unpublished RUN child can be left in SIDL/TRANSITION with VM
rollback pending while its parent later leaves the inherited session.
The old `proc_slot_discard` released VM first, discarded the result
of `proc_tty_release_session`, then freed the last stable u-area.
A final-session canonical TTY buffer whose MM free refused would keep
its session record but lose the natural process retry owner.

v1085 checks the final-session TTY release *before* VM destruction.
Its refusal enters the existing `PROC_SIDL_DISCARD_VM_RH` and shared
slot-0 retry clock, preserving the child and its still-live VM/u-area.
Subsequent service rechecks the TTY and commits VM/U-area teardown
only after release succeeds. The no-uarea case retains its zero-session
no-op, and post-VM retry does not repeat TTY or VM destruction.
Production assembly gains one instruction word, no permanent state:
KCC KCORE low end `023116` -> `023117` octal. Source-linked native
PDP-6 positive and pre-v1085 negative acceptance PASS. v881/v892,
v894 and v1081 rollback/finalizer tests PASS on corrected fixtures;
the full v1077 physical-OOM provider/MM guest also PASSes.
The booted v969 256K physically MM-pinned last-session TTY exit
guest also PASSes retained line/session, real unpin and successful
finalizer retry. No TTY ownership is silently discarded.

OOM readiness is distinct from a hardware clock failure. The PDP-6
APR line clock is assumed operational, just like MM and instruction
execution. No timeout, fallback timer or recovery protocol for a
physically broken clock is required. A DAIMOS-created PI6 inhibit,
an infinite software spin, or a device wait which requires inhibited
PI6 ticks *is* a kernel defect and still requires a reachable-path
proof or repair. In the current code the idle PI6 inhibit encloses
only marked PREVM/post-VM and unpublished RUN retry; HUP is staged
on the private owner stack, and swap-in service is after PI6 restore.
The source audit does not yet certify all nested provider callbacks.

Remaining OOM production blockers: transitive no-allocation and
owner-preserving teardown across FILE/HUP/TTY/VM/MEMFS-XIP/PIPE/DPY,
in-flight provider lease/quiescence policy, PI6-safe progress of
slot-zero callbacks, deterministic permanent-refusal containment,
and full-system combinations of faults and competing live processes.
The requester-only physical shortage/exit and independent witness
have already been demonstrated with a disposable OOM-enabled guest.
`PROC_OOM_BRK_ENABLED_V1000` remains zero in production.

## 2026-11-03 v1084: interrupted FIFO OPEN retains its MM retry owner

An interrupted named-FIFO OPEN has already reserved a provisional FILE
slot and incremented the PIPE reader or writer count. Its rollback
invokes `pipe_close_ref`. When this is the last reference, MM may
refuse the free (physical pin or wrong owner), keeping the reference
and FIFO object intact. The old path ignored that refusal and returned
an OPEN error; the FILE layer then erased its provisional descriptor.
This orphaned a live PIPE MM allocation with no retry owner.

The fix changes only the final branch in `pipe_fifo_open_intr`: a
successful rollback still returns the interrupted-OPEN error; if
rollback refuses, OPEN commits instead, returning its already reserved
FD. The existing normal FILE close is the exact retry owner. This is
an irreversible-operation completion contract, not a new reaper or
background retry mechanism. Neither data nor executable word count
changes: one JRST became JUMPE. Comparable KCC KCORE low end remains
`023116` octal; no MRES or persistent data growth.

The new v1084 native PDP-6 test links production `file_open`,
`file_new_fd`, `pipe_fifo_open`, `pipe_close_ref`, FILE close and actual
MM `mm_free`/`mm_unpin`. It checks a pinned final rollback, the
published descriptor and counts, a second close refusal, physical
unpin, successful final close, and an ordinary unpinned interrupted
OPEN. The original v1083 source fails the negative test with a
stranded extent and cleared FILE FD. All native cases PASS after fix;
the full named-FIFO guest and direct fatal-release leaf tests PASS.
The full physical-OOM 256K guest also PASSes two D6FS SYNC failures,
one actual MM_BUSY refusal and independent witness recovery.

The historical word-FIFO test also required an update: since v901,
PIPE READCHAR/WRITECHAR transfer one complete native word instead of
being unsupported. The obsolete `!2` assertion was unrelated to v1084.
The corrected full word-FIFO guest now PASSes its character round-trip
and all prior wrap/EOF/dup/RUN/blocking scenarios. The testkit's
anonymous-PIPE Make target now invokes the dedicated v902 guest,
instead of only the word-FIFO prerequisite.
The full v902 anonymous PIPE guest PASSes create, character I/O, EOF,
EPIPE, duplicate/RUN references and blocking wakeup.
The production automatic OOM killer remains disabled pending the
separate transitive teardown and provider-liveness proofs.

## 2026-11-03 v1083: allocation-free direct fatal-release leaf contract

The v1083 daimos-testkit audit enumerates the current source-linked
FILE close, PIPE/FIFO final close, zero-length DPY STOP, checked TTY
session/line release, U-area free, VM destroy, MEMFS XIP release and C
swap-detach leaves. It rejects new direct MM/backing allocations in
these release bodies and asserts the established error/owner ordering.

Native PDP-6 v870 final PIPE/FIFO MM refusal/retry, v904 pinned/wrong
DPY STOP, v912 double-failed DPY MM rollback and host-compiled v957
real C swap-detach verifier/retry all PASS. No production code was
altered, so KCORE, MRES, permanent RAM and ABI growth are all zero.

Scope restriction: this is a direct-leaf contract, not a transitive
OOM-kill readiness proof. Session HUP cascades, foreign in-flight
provider callbacks, permanent MM pins and clock-dependent retry
progress require separate closure. `pipe_fifo_open_intr` also drops
the return from `pipe_close_ref` while `file_open` clears its temporary
FD: a fault-injected final MM refusal there can leave no FD owner for
retry. Unlike an ordinary FILE close, this is a provisional FIFO-OPEN
abort, and normal physical pin reachability remains to be established.
No blind memory free or speculative global reaper has been installed.
Production `PROC_OOM_BRK_ENABLED_V1000` remains zero.

## 2026-11-03 v1082: NEWSESSION checks TTY MM release before publishing identity

PROCCTL_NEWSESSION previously changed the caller's pgrp, session ID and
TTY state before invoking proc_tty_release_session. If the caller was
the last member of the old session and MM refused to free its canonical
TTY input, the original TTY session remained attached with no live
process able to retry the release. The syscall still returned success.

v1082 calls the existing last-member TTY release with the leaving slot
excluded from the session scan *before* writing any process identity.
An MM refusal returns -1 with old pgrp/session/TTY state untouched;
retry after the owner becomes releasable succeeds. When other session
members remain, their TTY remains attached and the caller may leave.
The checked MM routine is non-sleeping; the successful path reloads
the current descriptor/slot after the callback instead of saving
caller-scratch registers on a persistent stack. No new owner table,
retry state, MM allocation, or permanent data is introduced.

Native source-linked PDP-6 positive and original-v1081 negative
controls pass, including true LAST-member refusal, success after
MM unpin, and another-member session retention. Existing process/TTY
full guest passes. Matched noninstrumented KCC KCORE low-end:
`023112` -> `023116` octal (+4 words); zero data growth.
The complete TTY-session-state and session-teardown simulator guests
PASS, as does the 256K v1077 physical-OOM guest with two D6FS SYNC
refusals, one MM_BUSY refusal, and recovered independent witness.
The P0 transitive allocation-free closure and foreign provider owner
proof remain open. Production OOM termination remains disabled.

## 2026-11-03 v1081: final-member TTY MM refusal uses existing PREVM retry

The final session member (including a non-leader) must not free its
process VM/u-area after a failed TTY canonical-line MM release.
proc_tty_release_session now returns refusal, and proc_finish_slot
does the TTY check before vm_space_destroy, reusing PREVM owner retry
on refusal. The prior VM failure handler is shared, avoiding duplicate
retry logic; no added persistent allocation state. Native v967/v968,
v881 and v1081 owner-retry tests pass. The full v1077 physical-OOM
256K guest also passes. Production OOM kill remains disabled.

Actual FILE close SYNC on D6FS/DTFS/TSFS does not issue media
transfers, so generic no-clock provider cancellation is not a
meaningful new P0 requirement for these close operations. However,
the shared provider lock, fail-preserving FILE ownership, and
provider-in-flight protections remain necessary. TTY, PIPE, DPY,
MEMFS-XIP and VM/MM still need complete transitive allocation-free
closure proof before enabling OOM kill.

Comparable, noninstrumented KCC full-image low-end size:
`023110` -> `023112` octal (+2 words). No persistent data growth.

## 2026-11-03 v1080: correct full DRM236 pending-queue contention

The v1079 watchdog changed a queued request pointer read from SKIPN
to HRRZ but inadvertently left an unconditional retry jump in
`drm236_runtime_queue_full`. A third process could therefore spin
indefinitely while a live request and one pending request occupied
the DRM engine. The v1080 fix uses `JUMPE` after RH descriptor
extraction: only an absent active owner triggers retry; otherwise the
third caller sleeps on that owner's completion event, which preserves
the private stack until the owner returns. It adds comments and no
additional code/data words.

A real PDP-6 source-linked v1080 three-request test PASSes full-queue
wait, renewed claim and correct event ownership; v1079 source FAILS
the negative control. v1079 watchdog and real-SIMH DRM read/write/
queue regressions remain PASS, as does the full 256K physical-OOM
D6FS/MM refusal guest. The unsolved clock-inhibited slot-0 device
wait and general provider quiescence remain explicit P0s.

## 2026-11-03 v1079: DRM236 runtime five-second completion watchdog

The Type-167/236 drum previously had bounded boot-time polling but an
unbounded runtime PI2 wait on process-private three-word I/O
descriptors. DRM MRES now exports a 300-tick runtime watchdog driven
through the existing 60-Hz KCORE storage clock path after the DSK
watchdog. The active request's existing LH encodes remaining ticks;
RH retains the descriptor pointer. Expiration clears Type-167/236,
records the actual request failure, wakes its owner and promotes the
single pending request with a fresh deadline. Actual PI completion and
timeout share one descriptor lifetime/finalization routine. A
nonreturning hardware request can thus complete with error without
stealing the serialized filesystem provider lease.

Native PDP-6 v1079 test PASSES for two successive exact-deadline
failures, queue promotion, accounting and no double-finish; disabling
its watchdog correctly FAILS the negative control. Existing real
Type-167/236 online read/write/queue test PASSES after repairing an
obsolete test-only read-trampoline symbol. Full 256K v1077 physical
OOM with provider/MM fault injection PASSES on this clock/DRM code.
Independent `ROOT=drum` boot with DSK0 unattached also PASSes through
DRM236, D6FS, INIT and LOGIN, confirming active MRES installation.
The full KCC KCORE low end grows one word (`023157` -> `023160`,
octal); DRM MRES grows 17 words (124 -> 141), no new persistent
data words. Total 18 additional resident words.

This is device-specific bounded runtime completion, not generic
provider cancellation. Clock-disabled slot-0 transfers, permanently
rejecting providers and transitive allocation-free fatal teardown are
still open. OOM termination remains disabled in production.

## 2026-11-03 v1078: fatal FILE closure avoids foreign-provider lock wait

Fixed `fs_provider_lock_wait`: a process in private fatal FILE cleanup
now returns `-5` when another slot owns the shared DTFS/D6FS/TSFS
lease, instead of sleeping inside the serialized gateway indefinitely.
The existing `file_table[-2]` fatal-close bit is consulted only on
contention. The real provider lease is never stolen or released;
`file_close_all` preserves the FD, and its private owner retry/backoff
retains the stable executive stack. Ordinary wait/revalidation, owner
recursion, free dispatch and MEMFS bypass remain unchanged.

The new native PDP-6 v1078 test passes the foreign-fatal, normal-wait,
free-dispatch and self-recursion matrix. It fails against the original
gateway as a negative control. Older native gateway/rename regressions
v326/v327/v866/v915/v916/v917/v428/v681/v466 and static owner audits
pass. No new permanent data; linked KCORE low-end increases by six
words (`023151` -> `023157`) in the comparable full-profile image.
The full 256K v1077 guest OOM/D6FS/MM retry test also PASSes on this
code: two provider refusals, one u-area MM_BUSY, successful retirement
and unrelated PIPE-process progress, without altered memory accounting.
This does NOT allow forced cancellation of an active provider, prove
transitive allocation-freedom, or enable production OOM termination.

## 2026-11-03 v1077: combined provider/MM refusal on real requester OOM

An isolated `daimos-testkit` 256K PDP-6 image with test-only OOM
activation runs a real physical-memory shortage while the requester
owns an open D6FS executable vnode. Its first two provider SYNC calls
fail, the third succeeds; the subsequent u-area MM free fails once
with MM_BUSY and succeeds on retry from the idle-stack finalizer.
KCORE-observed counters confirm 3/2 FILE attempts/refusals and 2/1
MM attempts/refusals. The parent reaps only the requester (OOM status
1); a separately blocked PIPE child resumes, completes, and the
process memory accounting returns to baseline. The guest ends with
`<KWP` and controlled HALT. PASS.

Finite failure/retry correctness is now covered across both provider
and MM in the same genuine OOM event. A genuinely nonreturning or
indefinitely rejecting provider still lacks proven safe cancellation;
transitive allocation-free termination remains unproved. The
production OOM switch stays disabled. No kernel changes in v1077.

## 2026-11-03 v1075: full 256K PDP-6 requester-only OOM proof (test-only activation)

The native `daimos-testkit` v1075 fixture uses an isolated copy of the
kernel with the existing `PROC_OOM_BRK_ENABLED_V1000` compile-time guard
changed from 0 to 1. A valid 18-bit BRK request encounters real
insufficient physical memory. The requesting child exits with the
kernel's OOM status 1, a separate blocked PIPE reader stays alive and
resumes after its writer closes, and the process memory accounting
returns to the parent's initial value. The full guest reaches a
controlled HALT (`<KWP`). A separate full-guest control with the
default flag 0 returns -1 from BRK instead of terminating its caller;
the child exits normally, the PIPE witness resumes, and the parent
verifies recovered process memory (`<DNWP`). Both use disposable
256K simulator images, without changing
production source or persistent KCORE memory.

This accepts only the ordinary provider-success path. A forever-busy
FILE/MM provider can still prevent bounded reclamation, and complete
allocation-freedom through all fatal teardown callbacks is not proved.
Automatic OOM termination remains DISABLED in production.

## 2026-11-03 v1074: reject orphaned provisional BRK MM allocations

Corrected the PDP-6 BRK full-relocation fallback: a failed old-extent
`mm_free` now retains its exact error code, and an additional failure
while releasing the unpublished replacement triggers the resident
storage-invariant HALT rather than losing the replacement allocation.
No persistent metadata, user ABI, or automatic OOM-kill policy change.
The source-linked PDP-6 test passes all three branches, while the
old source and a deliberately missing second-free guard fail the test.
GCC KCORE and full KCC boot-image builds pass; the KCC boot reaches
INIT/LOGIN. The full `daimos-brk-v1` KCC 256K guest runtime regression
also passes growth, preserved heap data and shrink.
Permanently refusing providers and allocation-free OOM teardown remain P0.

## 2026-11-03 v1070: complete cross-repository branch cleanup

The full `~/git` branch audit (twelve repositories) found additional
toolchain and simulator work beyond DAIMOS, testkit and pdp10-doc.
Every remaining development tip has now been incorporated in its own
repository's main/master history, either by a tested code integration
or by a history-only merge when current production already superseded
the old implementation. The old branch references have been retired.
No KCORE or 32K-specific optimization was resumed.

The final uncommitted DSH two-word-fold experiment from detached
DAIMOS commit cb1e6094 has been preserved byte-for-byte as
`design/HISTORICAL-DSH-FOLD-M2-UNACCEPTED-V1070.patch`. Its historical
claims of 28 linked words saved are *not* acceptance: a 256K PDP-6
conformance run failed in APROPOS with invalid UUOs, while the four-
word control passed. The unsafe patch is not applied to production.
Its former detached worktree was restored clean after checking that
the archived patch reverses the original modifications exactly.

The separate untracked PDP-6 headless simulator binary, which differed
from the earlier archived copy, was preserved locally at
`~/archive/daimos-git-cleanup-v1070-20261103/pdp6_headless-v1070`.
This generated binary is not a new source commit. Historical missing
Git worktree registrations were pruned without removing any existing
worktree directory or source content.

## 2026-11-03 v1065: close the last independent historical tips

Final DAIMOS KCORE source tips `b2f92fd8` (selective MRES) and
`bafe9fd8` (resident startup snapshot) were reviewed together with
testkit v72 tip `68b9169`. Their unique historical KCORE code is
not current-production compatible or independently RAM-accepted.
Detailed disposition is in `design/FINAL-KCORE-TIPS-AUDIT-V1065.md`.
Both original source tips are now main ancestors using history-only
merges. No active syscall dispatch, swap, init, KCORE, DSH, or module
runtime source was taken from those old trees.

Two separate test issues were recovered before retirement: the v101
swap assembly dependency list (v1063) and the native FILE test's
missing mount-extension table space (testkit v1064). The latter
passes on the PDP-6 without warnings. Current VFS parent-AC3 v1037
and GCC MonitorFS compiler-selection v1033 checks also pass. Those
are targeted source/target tests, not a new KCORE RAM-size or 32K
boot claim. Further KCORE optimization and 32K tests remain deferred.

## 2026-11-03 v1063: repair stale outer swap-assembly dependencies

The final KCORE startup-snapshot test branch contained a useful
host-only source-graph regression from v101. Against current main it
correctly found a build defect: the outer `system/boot/pdp6/Makefile`
KERNEL_INPUTS omitted four direct prerequisites of the inner
`system/kernel/Makefile` generated `proc_swap.s`:
`proc_pure_backing_pdp6.s`, `proc_storage_runtime_v30.s`,
`proc_pure_attach.c`, and `proc_storage_map_v28.h` (all in `proc/`).

All four are now explicit outer KERNEL_INPUTS prerequisites, preventing
an incremental outer boot build from leaving the assembled swap image
stale when any of them changes. The versioned testkit target
`test-daimos-kcore-swap-input-deps-v1063` checks the dependency lists.
It passes against the repaired worktree with five direct composite
`proc/` inputs and fails against unchanged prior main, identifying
those four missing inputs. This is an incremental build correctness
repair, not an alteration to KCORE source, linked size, or RAM use.
No guest boot or 32K-specific test was run.

## 2026-11-03 v1061: consolidate retained Git archives into main ancestry

All four historical DAIMOS archive tips have been reviewed for
uncommitted files and are being folded into `main` via ancestry-only
merges. The original archive summaries are now ordinary files in main:

- `design/LEGACY-BOOT-STAGE1-CHECKPOINTS-V1057.md`
- `system/kernel/core/D6CODEC-HISTORICAL-ARCHIVE-v1028.md`
- `design/LEGACY-DSH-INIT-ARCHIVE-V1050.md`
- `design/LEGACY-SHRINK-KCC-DIAGNOSTICS-V1059.md`

The archives contain unaccepted and superseded source trees, including
KCORE, codec, boot, DSH and MAKE diagnostics. `ours` merges intentionally
preserve exact commit ancestry without changing current executable,
kernel, loader or toolchain code. The unique historical v059 codec
readiness report remains reachable in Git history and is not installed
as a current design assertion. Old archive branch labels may be
removed after the pushed main commit is verified as their descendant.

The only independently retained DAIMOS optimization tips after this
cleanup are the two explicitly deferred KCORE experiments. No new
KCORE optimization or 32K-specific test is implied by Git ancestry.

## 2026-11-03 v1059: consolidate native VM patches and old shrink studies

`merge/native-kcc-vm-20261008-v1` contains five commits, all reported
patch-equivalent by `git cherry main`: native KCC brk-backed VM,
MEMFS partial-chunk free, incremental MEMFS growth, VM peer reclaim
before BRK relocation, and libc nonadjacent free-list insertion.
The single `native` branch commit for compact PDP-6 process teardown
is also patch-equivalent. Their historical commits are preserved in
main ancestry without source changes and their branch labels retired.

Three further divergent tips are retained in the versioned archive
`archive/legacy-shrink-kcc-diagnostics-v1059-20261103`:
`backup/pre-push-20261005-v1`, `checkpoint/shrink-20261006-v1`,
and `experiment/kcc-swap-in-diag-20261008-v1`. The original commits
were verified reachable on the remote archive before old local/remote
refs were deleted. Their old source is not merged: picture staging,
Type 340 scrolling, native MM/MEMFS, accounting and EXEC behavior
already have newer implementations in current source.

The old KCC instrumentation did **not** establish a swap-in defect.
Its historical `MAKE -N ALL` recursion failure needs a fresh check
after the separate v1054 native MAKE lazy-scratch change. No full
KCC self-hosting run was performed during this consolidation. No
new kernel code, KCORE optimization or 32K-specific tests were added.

## 2026-11-03 v1058: superseded KCC source staging branch

The old `push/kcc-native-source-20261008-v1` branch staged a minimal
five-argument C-SIX/S6REC converter and a placeholder native KCC
Makefile. Current main has a six-argument source staging pipeline
with native DAIMOS.MK, current ABI headers, and packaged bootstrap
objects. The old packaging code has no missing implementation to
replay. It also carries unrelated historical VM/memfs/LOGSTORE work,
which remains available through a history-preserving `ours` merge
but has not been reintroduced into current kernel source.

The older branch's original commit tip was preserved in main ancestry
before its obsolete local/remote branch refs were retired. No native
KCC self-hosting acceptance or 32K guest test was performed, and
KCORE optimization remains deferred.

## 2026-11-03 v1057: archive obsolete Stage-1 and early-boot refs

Seven remote-only October 14-15 boot, EXEC, and Stage-1 checkpoints,
plus local `v155-dtc-stage1-wip-20261015`, are now ancestors of the
versioned branch `archive/boot-stage1-checkpoints-v1057-20261103`.
Their original exact commit tips were checked against the pushed
archive before seven remote names and one local name were removed.

Current KCORE already places the D6LZ36 fixed entry first; current
PURE backing scans guard u-area dereferences with `PROC_F_UAREA`.
The old staged EXEC transition and DTC Stage-1 word-limit experiments
use superseded caller and loader layouts and were not replayed.
The historical SIMH GO-PC regression was independently recovered and
passed in testkit v1048. The eight ancestry merges were archival
(`ours`) only: no production code or boot configuration changed.

The migrated Type-340 C testkit fixture referencing deleted
`dpy_text.c` was moved unchanged into testkit
`tests/archive/type340-c-obsolete-v1056/`, with a SHA-256 manifest.
The applicable native display and detached-TTY regressions pass.
Further KCORE optimization and 32K-specific testing are deferred.

## 2026-11-03 v1056: consolidate superseded TTY and display branches

`ttyroute-20261002-v1` and `fix/dpyview-process-20261006-v1`
were reviewed against current main. The historical TTYOUT command,
root-only TTYCTL SETOUT/GETOUT service, output routing, DPY character
sink, KILL event command and DPYVIEW terminal-read interruption are
already integrated. Current production has evolved further with the
separate retained /DEV/TTYDPY0 terminal and native Type-342 blocks.
The historic DPYVIEW and KILL SIXMD files match current main exactly.
No old device or syscall assembly should be replayed over newer code.

Testkit current detached-TTY bounds v1, native DPY blocks v1, and
DPY interrupt-policy v1 checks all pass. The older migrated Type-340
C fixture could not build because the historical `dpy_text.c` is no
longer present; the current native-block regression is the applicable
replacement. This is an obsolete fixture, not proof of a current
display failure. The two old source tips are preserved as main
ancestry, without modifying the current TTY/DPY implementation.

KCORE optimization and all 32K-specific testing remain deferred.

## 2026-11-03 v1055: historical worktree and staged-codec branch cleanup

The old testkit SIMH v115 worktree's four untracked DTC3 diagnostic
files were copied byte-for-byte into the versioned testkit main
archive `tests/archive/dtfs-v199-v200-unaccepted-v1052/`, with SHA-256
verification. They do not implement their claimed two-boot persistence
test and were not promoted to active tests. The source v115 history
was already reachable from testkit main; its local branch was deleted
and the old DevSpace2 worktree was left clean and detached.

Likewise the historical nested-UUO testkit v54 worktree's twelve
untracked probes and locally edited runtime report are preserved with
checksums in `tests/archive/dsh-nested-uuo-v1053/`. Its original
commits are now ancestors of the retained
`archive/legacy-dsh-supervisor-tests-v1049-20261103` branch. The old
local v54 branch was deleted, its source worktree cleaned and detached.
The existing SIMH media-isolation test was rerun and passed. Old
nested-UUO failure reproductions remain historical, not accepted
current regressions.

The original remote-only codec v053 and v054 staging branches were
preserved as ancestors of DAIMOS
`archive/d6codec-experimental-cutover-v1028-20261103`; their old remote
refs were deleted. Main still contains the current staged codec, with
the original v053 decoder/staging note preserved and newer encoder/
integration-checklist revisions retained. No production codec switch,
KCORE optimization or 32K-specific test was performed.

## 2026-11-03 v1054: native MAKE lazy recursion-scratch consolidation

The historical `experiment/native-make-recursive-frame-20261008-v1`
identified a useful userland RAM reduction but was never accepted
after its self-hosting timeout. Current MAKE had subsequently moved
the recursion scratch outside the string arena, yet still allocated
all 33 large implicit-rule records after parsing. Its current code
has now been adapted to cache just one record per recursion depth
actually visited, without altering dependency traversal semantics.

Native KCC MAKE compiles and links, and a freshly built 256K
full-profile PDP-6 image passes both v1054 guest probes: a six-level
dependency chain and a dependency cycle that must be rejected. The
unmodified baseline passes the same behavioral probes. The new testkit
`test-daimos-make-lazy-depth-v1054` target also verifies the lazy
allocation source contract and rejects the old bulk-allocator source.
It requires an explicitly supplied 256K image and is not a default
boot or 32K test. GCC also links the userland binary, with unrelated
existing target-libc/include warnings in that toolchain path.

KCC emits a 212-character implicit scratch record (53 PDP-6 words).
The previous unconditional 33-record heap allocation reserved 1749
words plus one heap header. At six visited depths the new layout uses
318 record words, six heap headers, and a 33-word static pointer array.
Including two-word heap headers, this calculates to about 1389 fewer
process words for that example; this is a source/ABI estimate, not a
measured guest peak-RAM difference. KCC linked MAKE text changed from
025124 to 025116 octal (-6 words), and BSS from 001526 to 001566
octal (+32 words). At the full 33-depth bound the multiple headers and
pointer array could instead cost 96 words more than the old full slab;
deepest-case peak remains a follow-up memory test. A full native
`MAKE ALL` self-hosting run was not repeated. KCORE source is unchanged,
and 32K-specific testing remains deferred.

## 2026-11-03 v1050: archive superseded DSH/INIT source branches

Four divergent historical DAIMOS userland/INIT branches are now retained
through `archive/legacy-dsh-init-experiments-v1050-20261103`:
`dsh-phase0-history-acceptance-20261012-v7`,
`supervisor-idle-tty-activate-20261013-v5`,
`diag-dsh-v43-entry-20261022-v52`, and
`dsh-v53-acceptance-state-20261022-v55`. Original commits are ancestors
of the versioned archive, while their obsolete local and available
remote branch names have been removed. The archival `ours` merges
preserve history only; they do **not** integrate incompatible source.

Current INIT retains its own INITTAB loop. The historical separate
supervisor, EDCORE, lock-based DSH history repair, and related PI
diagnostics are not current production features. The useful pipeline
child-reaping fix was independently recovered and tested in v1019.
The companion testkit history is retained as
`archive/legacy-dsh-supervisor-tests-v1049-20261103`; both source and
test archives have documented boundaries. The D6FS bounded-undo host
experiment passes its 5,805 modeled interruption cases but provides
no proven hardware persistence guarantees or implemented journal.

The independent PDP-6 simulator console-PC test has also been restored
to testkit main as `test-daimos-simh-console-pc-v1048`, passing its
startup, interactive, breakpoint and step checks. The historical
`pdp6-simh-console-pc-20261021-v115` local branch must remain checked
out for now: its separate DevSpace2 worktree has untracked DTFS
diagnostic files. Main already preserves its commit history, but the
dirty worktree must not be deleted or modified without reconciling
those files. KCORE optimization and 32K-specific tests stay deferred.

## 2026-11-03 v1047: non-KCORE consolidation, rejected DSH fold threshold

Further KCORE optimization remains deferred. The historical DSH
`fix/dsh-fold-full-objects-v1` branch contains a two-word p10fold
threshold experiment, whereas current main folds C and assembler
objects at four words. On the identical current KCC source object set,
two-word folding reduced DSH linked image from 035743 to 035707 octal
(28 decimal words), BSS unchanged at 001763. However, the 256K
full-profile guest with explicit disk-root selection passed the first
22 DSH conformance probes and then failed `APROPOS ASSEMBLER` with
invalid UUOs (including `UUO 000000000000 340000000001`). The same
command passed on a separately built current-source four-word image.
The small plan is therefore **rejected as unaccepted**: do not merge
the `-m 2` setting or count those savings. The exact fold-plan defect
requires separate p10fold/DLINK diagnosis before reconsideration.

The testkit branch `tsfs-aap-64k-regression-v1-20261015` was redundant:
its 64K three-member test script and Makefile target already exist in
current testkit main. Its historical commit was recorded in main's
ancestry and its local/remote branch refs removed without rerunning the
AAP boot. No 32K testing or KCORE change was performed.

The historical `group027-functional-acceptance-20261022-v172` branch
contains only PROJECT-STATE updates. Its later v197 account reports
success for tape-root DSH nested RUN/WAIT, but explicitly leaves
DTC3 FSCK/MOUNT timeouts unresolved. Preserve this as historical
evidence rather than turning it into a new acceptance claim.

## 2026-11-03 v1042: consolidation-only phase; KCORE work deferred

Further KCORE optimization, including the historical syscall-dispatch
folding and selective-MRES experiments, is deferred by request. During
this phase no speculative KCORE changes or 32K-specific tests are made.
Keep unreviewed KCORE branch tips until that optimization work resumes.

The separate `daimos-testkit` `experiment/mm-packed-20261008-v1`
experiment was compiled and run before retirement. Its one-word
encoding correctly round-trips aligned physical extents, but discards
owner identity and limits pins to two bits. Its 1.5-word encoding
round-trips two 18-bit metadata values but limits owner IDs to 12 bits
and pin counters to four bits. Neither preserves current MM's full
18-bit owner and eight-bit pin fields. The experiment's original commit
was retained in testkit main ancestry, and its branch ref removed.

Historical DSH PTR/TSFS evidence code was recovered as the opt-in
testkit `test-daimos-simh-ptr-tsfs-evidence-v1042` target. It checks
the bounded DSH image, five uppercase literal callers, boot media
identity and captured SIMH milestones. Its static source check passes
and rejects injected lowercase callers; it refuses invocation without
a real image build and console log. No PTR/TSFS boot was performed in
this consolidation. The old `dsh-ptr-tsfs-size-20261016-v1` literal
change is already integrated and tested by DSH v1016, so its obsolete
branch label can be retired. A second supervisor branch label with an
identical commit tip was removed without dropping any source history.

## 2026-11-03 v1040: D6FS checker source consolidation

The historical `d6fsck-fcb-owner-test-v154-20261015` branch exposed a
host-tool source divergence rather than a KCORE issue: the installed
`d6fsck` and DAIMOS's current host source pass 260 FCBs, but the
independently maintained `pdp10-tools/d6fsck.c` still stored block
owner IDs in unsigned bytes. FCB index 254 has owner ID 256 and its
byte representation wraps to zero, making a live allocated block look
leaked. The pdp10-tools checker now uses a full-width (32-bit) block
owner array and retains the owner ID in all mark/repair comparisons.

The recovered daimos-testkit v1040 source-built checker regression
passes against both DAIMOS host source and corrected pdp10-tools
source. The original installed-checker owner-width regression also
passes. The historical v154 test's swap-fixture source changes are
already superseded by the current testkit production-ABI fixture.
No guest kernel, KCORE, or 32K-specific test was changed or run.

## 2026-11-03 v1038: reject DSK/DRM extraction, accept VFS parent fix

Reassessed `dee93223` from the historical selective-MRES integration
branch against current full-profile KCC KCORE. Both Type-270 and
Type-167/236 address/validation wrappers can move into their optional
MRES packages, but the combined resident RAM effect is unfavorable
when both controllers are installed. Measured linked images (octal
36-bit word counts, unchanged BSS in each comparison):

                   KCORE      DSK MRES    DRM MRES
  baseline         022403     000312      000174
  branch layout    022344     000335      000214
  reduced layout   022344     000334      000212

The reduced version saves 31 decimal KCORE words but adds 18 DSK and
14 DRM words, net **one extra permanent word** with both present.
Optional-hardware-absent cases do save RAM, so the underlying design
is valid, but the net growth with both loaded fails RAM-first
acceptance. Experimental changes were discarded, their versioned
`$TMPDIR/daimos-dskdrm-*-v1035/v1036` link results retained. No
production disk/drum assembly or MINIT change was accepted.

The same historical checkpoint also contained a VFS improvement.
`vfs_parent_mount` now extracts the mount slot directly into AC4,
instead of clobbering AC3 and moving AC3 to AC4. Native PDP-6 v1037
negative control exits 3 on old source (AC3 corrupted); fixed code
passes. Full KCC KCORE link shrank 022403 -> 022402 octal text, exactly
one permanent word, with BSS 000415 unchanged. Existing native VFS
mount-mutation v9 and static mount-slab v688 checks also pass after
repairing the old v9 test fixture to include the real `vfs_readonly`
routine. No 32K tests were run.

## 2026-11-03 v1033: preserve smaller MonitorFS MRES GCC leaf

Historical selective-MRES branch v6 proposed compiling the cold
`monitorfs_swap_v63.c` reader with the selected kernel compiler.
Reassessment against current KCORE shows this would be a resident-RAM
regression: under the same KCC KCORE map, a KCC-built reader produces
`monitorfs-mres.dxr` image 003265 octal words, versus 003070 for GCC.
The 0175-octal difference is 125 decimal pinned-MRES words. The
reader's entry is called from a stable AC1..AC4 PDP-6 assembly ABI;
the GCC leaf links against KCC and GCC KCORE maps. Consequently the
current intentional GCC-only helper policy is retained even when
`COMPILER=kcc`, rather than replaying the historical compiler change.

The boot Makefile now documents that exception, uses the GCC system
include directory without a duplicate-search warning, and depends on
the GCC executable. New testkit v1033 assembles the helper for both
kernel compiler selections, checks the selected GCC invocation, and
verifies identical generated objects; it passes warning-free. These
are build/link checks, not a new native boot or swapped-reader runtime
acceptance. Total production MonitorFS MRES footprint remains 003070
octal words, with no new permanent-RAM growth. 32K tests remain deferred.

## 2026-11-03 v1030: recover MM descriptor-capacity regression

Historical MM v125 documented KINIT-reserved descriptor backing and an
adjustable descriptor ceiling. Both are already part of current main:
`kcore_end.s` exposes `mm_extents` without allocating resident BSS,
`kinit.c` reserves and clears `2 * mm_extent_capacity` words before
publishing arenas, and `mm_pdp6.s` checks that capacity on insertion.
No allocator code was changed during this consolidation.

The historical native PDP-6 31/32/31 descriptor boundary test was
recovered into `daimos-testkit` as the v1030 acceptance target. Its
standalone backing now explicitly reserves 64 words because current
KCORE no longer defines a static MM array; its synthetic core ceiling
was raised to 256K to avoid deferred 32K-specific tests. Native PDP-6
v1030 passed. The old experimental pointer-replacement MM proposal
does not provide a further proved RAM saving and remains deferred.

## 2026-11-03 v1028: consolidate grouped D6CODEC development branches

Four obsolete DAIMOS grouped-codec development branch names and six
corresponding daimos-testkit branch names were removed locally and on
origin after their independent commits were made ancestors of two
versioned archive branches:

- DAIMOS: `archive/d6codec-experimental-cutover-v1028-20261103`
- daimos-testkit: `archive/d6codec-grouped-tests-v1027-20261103`

These archives retain test and implementation history, not active
functionality. Current production still uses D6LZ36 for boot, EXEC,
VFS, PURE backing and swap. The grouped encoder/decoder and v055 ABI
wrapper remain staged and inactive. The existing native PDP-6
`d6codec-kcore-abi-v055` test passed against current main; its testkit
fixture path was corrected to resolve within the executing testkit
worktree. Historical grouped PURE swap v091 did not link against the
newer backing store and process-state interfaces and was not accepted
as a current regression. No 32K-specific tests were run.

The grouped codec format must be switched atomically only after all
producer, boot, consumer, ABI, memory and failure-injection acceptance
specified in `D6CODEC-INTEGRATION-TODO-v054.md`. No production kernel,
boot-loader or codec code changed in this consolidation, and no resident
RAM saving is claimed. Both archival histories remain accessible.

## 2026-11-03 v1019: recover failed-pipeline child reaping

Historical DSH v53 exposed a real gap still present in current main:
`dsh_exec_pipeline` terminated a partially launched process group on a
later-stage error but failed to WAIT for its children, leaving a ZOMB
process slot occupied. The recovered cleanup uses the existing
`dsh_wait_pgrp` with the exact count of successfully launched stages,
after TERM and pipe-descriptor closure, while preserving the original
launch error. No per-process or kernel state is added.

The new daimos-testkit `test-daimos-dsh-pipeline-failed-launch-reap-v1019`
passes 3/3 on the 256K full-profile KCC/SIMH image: error status 127,
no state-4 process in PS, and successful subsequent pipeline. The
historical SIMH media-isolation helper and harness switch were recovered
to keep that boot from altering shared writable media; its host fixture
passes independently. KCC/DAS/DLINK generated DSH image 035743 octal,
versus the v1016 035730 baseline (+013 octal / 11 decimal text words),
BSS still 001763. This correctness expense is in userland, not KCORE.
No 32K-specific tests were run; those remain deferred.

## 2026-11-03 v1018: historical branch recovery, DSH literal shrink

The historical PTR/TSFS DSH size repair was re-evaluated against current
main rather than replayed blindly. All five `dsh_s6_literal` call sites
pass compile-time uppercase strings, so its redundant lowercase branch
was removed. A fresh same-toolchain KCC/DAS/DLINK before/after build
measures linked DSH image 035736 -> 035730 octal (six fewer words), BSS
unchanged at 001763 octal. The testkit host guard
`test-daimos-dsh-uppercase-literals-v1016` passes. This is not a native
runtime or 32K boot acceptance; all 32K-specific tests remain deferred.

The repository branch cleanup preserves distinct ancestry tips. Historical
EXEC/DCS/SIMH/TSFS knowledge has been integrated into pdp10-doc; the
remaining independent tips still require source-aware review, not bulk
merges. Source-level DSH changes were committed and pushed to main.

## 2026-11-01 v926 native owner-return deferred TERM validation

The existing TERM bit in the process control word records a foreign TERM
when its target still owns a private executive continuation. The owner-side
monitor-UUO return gate checks this bit before user return and dispatches
through the private fatal FILE-close retry path. New testkit
`daimos-deferred-owner-return-v926` executes that helper and actual extracted
fatal-close assembly: two FILE refusals, two owner sleeps, successful close,
one TERM exit and no second cleanup on repeat check. Native PASS. Earlier
v926 coalescing and v925 handoff tests remain separate coverage; actual
full-system CHLD wakeup/TERM handoff and other fatal-event precedence remain
open. OOM killing is disabled.

## 2026-11-01 v925: protect saved executive context after wait wakeup

The v924 wait-class guard missed the interval after a child's CHLD
wakeup clears WAIT_CHILD but before the noncurrent parent's saved
executive stack resumes. `proc_restore_kernel_tail` restores and clears
the saved CTX_PC only when the owner runs; while noncurrent, its stable
u-area CTX_PC still carries the PI return mode. An executive return
lacks LH 010000 (the user-mode return bit). The OOM selector, remote
fatal path, and direct foreign finalizer now refuse such a context,
in addition to existing wait-class/lease and fatal-FILE protections.
The current owner may still unwind its own stack. Twelve assembly
instructions, no new KCORE data words, per-process state, or ABI.

Native PDP-6 v925 pre-fix negative controls reproduced unsafe OOM
selection (exit 010), remote fatal (032), and direct finish (034).
The repaired source passes these three tests, existing WAIT_CHILD and
INTR checks, and historical FILE, RT, session/finalizer and rollback
native regressions. Extracted-source fixtures now explicitly initialize
saved user PI returns; the v921 static audit asserts all three CTX_PC
checks as well as the wait-class masks.

Booted 256K PDP-6 v924 provider/WAIT_CHILD compatibility on v925 PASS:
three fatal refusals (one real WAIT_CHILD), stable owner lease 4,,0,
73 bystander cycles, zero errors, controlled HALT. This is booted
nonregression rather than direct handoff injection. Cooperative pending
TERM delivery, genuinely stalled provider recovery and allocation-free
OOM termination remain P0. OOM killing stays OFF.

## 2026-11-01 v924: protect all private wait-class continuations

The v918/v920 remote teardown and OOM guards incorrectly tested only
PROC_WAIT_EVENT_LH (020000). A child-WAIT owner instead retains its
private executive stack with PROC_WAIT_CHILD_LH (040000); the old OOM
selector could select it and remote TERM/direct finish could free its
live stack. All three guards now use PROC_WAIT_LH_MASK (060000): EVENT,
CHILD and interruptible waits. These are only operand changes to the
same three instructions: no additional KCORE instructions, permanent
RAM words, per-process state or syscall ABI changes.

Native PDP-6 v924 negative baselines all exposed the defect (OOM exit
010, remote fatal exit 032, direct finish exit 033). Post-fix tests
cover CHILD and INTR, with existing EVENT and teardown regressions
passing. The v921 static OOM inventory now verifies all wait encodings
and all three full-mask guards. Historical extracted-assembly tests
were updated for the shared constant and missing finalizer fixtures.

Full-system 256K PDP-6 v924 PASS: real child-WAIT parent and living
child alongside the held serialized FS owner, foreign gateway waiter,
and runnable bystander. Three actual fatal guard refusals, exactly one
WAIT_CHILD-class refusal, lease 4,,0 stable at all three samples,
72 independent bystander sleep cycles, zero errors and controlled HALT.

Safe refusal is not automatic deferred TERM delivery: a sender must
retry once the private wait completes. A genuinely nonreturning FS
provider still retains its lease and resources; no forced unlock or
automatic OOM kill is permitted. Establish cooperative pending fatal
behavior, bounded cancellation, and allocation-free OOM cleanup before
relaxing these safety guards. Automatic OOM killing stays OFF.

## 2026-11-01 v923: repeated FILE SYNC refusal and partial close retry

A new native PDP-6 v923 test retries remote fatal FILE cleanup across
two consecutive SYNC refusals. The failing target descriptor remains
owned, a successful independent descriptor stays closed and is never
replayed, global `file_table` remains associated with its original
caller, and final successful retry closes the sole survivor before
one fatal-slot completion. Native v923, v867, v865 and v885 PASS.

The test extends the existing explicit-target close fixture and proves
this particular partial-close retry sequence without new production
KCORE instructions, data words or ABI changes. Infinite provider
refusal, an operation that never returns, and allocation-free OOM
termination remain unresolved P0. Automatic OOM killing stays OFF.

## 2026-11-01 v922: repeated partial session HUP retry proof

The v905 session-HUP retry path already recognizes an irreversibly
committed member only through TRANSITION+VM_NONE+FINALIZE. Native PDP-6
v922 repeatedly fails the second member's HUP after the first has
committed VM destruction but retained its pending u-area allocation.
Both failed attempts preserve TTY ownership and the first member's
finalization marker without redelivering its HUP. The eventual success
only delivers HUP to the previously failing member before TTY release.
A test-only pre-v905 negative control fails as required (exit 023).

Native v922, v905, v875 and v918 tests and the v921 source audit PASS.
No production KCORE, RAM, or ABI changes; this expands test coverage
rather than claiming comprehensive OOM cleanup. A permanently stalled
provider or failed FILE SYNC still prevents bounded teardown; automatic
OOM termination remains disabled.

## 2026-11-01 v921: OOM teardown safety-path inventory

The resident OOM victim selector remains read-only; automatic OOM
termination is disabled. A new executable testkit static audit checks
the current PDP-6 fatal/finish teardown edges and private-wait guards.
It finds FILE close -> VFS_SYNC (potentially sleeping or failing),
process finish -> session HUP (partial delivery possible), and VM
release -> MEMFS XIP provider, MM free and backing detach. The u-area
is freed through the resident MM table. Individual MM deletion uses no
allocation but does not establish transitive allocation-freedom or
bounded completion for all provider and session callbacks.

v921 testkit audit PASS for current call-chain inventory and existing
private-stack protection; it explicitly reports the termination chain
NOT CERTIFIED. v917-v920 native and booted acceptance remains valid.
No production KCORE/RAM/ABI changes in this stage. Safe reclamation of
nonreturning providers, failure-atomic multi-stage teardown and full
allocation-free OOM kill proof remain open P0. OOM killing stays OFF.

## 2026-11-01 v920: protect direct noncurrent process finalization

The direct `proc_finish_slot` entry can destroy a target VM without
passing through the v918 fatal-event gate. It now refuses a foreign
target with `PROC_WAIT_EVENT_LH` set before transition marking or
resource cleanup. A self-exiting process remains allowed to unwind its
own stack normally. This adds nine assembly lines with no new BSS,
per-process state, syscall or module ABI changes.

Post-v920 booted PDP-6 compatibility acceptance: the full 256K v919
remote-fatal provider test PASSes against v920. One suspended lease
owner, one foreign waiter, exactly two refused fatal deliveries,
stable lease 4,,0, 72 runnable bystander sleep cycles, zero errors
and controlled HALT. This confirms no scheduler/provider/fatal regression
from the direct finalizer guard; it does not inject a direct
proc_finish_slot request from another guest process.

Native PDP-6 v920 verifies that a direct attempt does not release VM or
u-area while the foreign private executive continuation is live, and
that normal fatal teardown remains functional after the event wait
clears. Native v883, v918 OOM/fatal, and v868 regressions pass. Full
booted v919 provider/fatal compatibility is being checked separately.
Reclaiming a genuinely unresponsive provider remains open; automatic
OOM killing stays disabled.

## 2026-11-01 v919: booted remote fatal protection for provider waits

A full 256K PDP-6 guest now verifies the v918 private kernel-event
stack lifetime guard with real TERM delivery. One process owns a live
serialized filesystem gateway lease while suspended in proc_wait_event,
and a second process sleeps on the occupied provider lock. INIT sends
TERM to both. The fatal-event path refuses both before remote FILE
close, VM destruction, or private-stack release. Disposable test-only
instrumentation records exactly two actual guard rejections.

The filesystem lease remains encoded 4,,0 at acquisition, contention,
and final controlled HALT. The independent third job runs 72 sleep
cycles, and the guest reports zero errors. The existing v917 scheduler
regression and v918 native safeguards remain accepted. v919 adds no
production KCORE instructions, resident RAM, ABI, or per-process state.
Recovery or eventual reclamation of genuinely nonreturning provider
resources remains an open P0 gate, as does allocation-free OOM killing.
Automatic OOM victim termination stays OFF.

## 2026-11-01 v918: preserve private kernel-event continuations

A process in PROC_WAIT_EVENT_LH sleep may still own an active private
executive stack, including a blocked filesystem-provider or MM call.
Non-current fatal delivery now refuses to close such a victim's FILE
references or destroy its VM; OOM selection omits the same protected
waiter. Both changes use existing resident scheduler state. Twelve
assembly lines added, no BSS, process-table, syscall or module ABI
changes. This is safety containment, not forced recovery.

Post-v918 full-system compatibility: the existing isolated 256K PDP-6
v917 held-provider/waiter/bystander boot regression PASSed against v918
KCORE. One held owner, one foreign waiter, stable lease 4,,0, 72
unrelated bystander sleep cycles, zero errors and normal controlled
HALT. This is a provider/scheduler non-regression, not an injected
full-system remote fatal delivery against that suspended waiter.

Native PDP-6 v918 separately verifies refusal without close/VM mutation,
normal fatal lifecycle after the event state clears, and exclusion from
OOM candidate selection. Existing v866, v883, v879, v868, v867, v865
and v885 native regressions PASS. Full booted v918 fatal delivery under
a live private-event wait and allocation-free OOM termination remain
unverified. Automatic OOM killing stays OFF; TSFS v1.2 is deferred.

## 2026-11-01 v917: fix owner-encoded provider event busy retry

The real PDP-6 scheduler checks the entire `proc_wait_event` word.
The filesystem gateway formerly passed `fs_provider_ready` as that
word, but a held lease stores owner_slot+1 in its left half. The
nonzero value made a foreign waiter retry without sleeping. A full
256K booted guest exposed 8193 repeated lock attempts and stalled
independent work. The gateway now uses one separate resident event
word, zero while held and signaled when the owner unlocks. The lock
word continues to identify its original owner. Slot 0 returns -5
rather than entering the bootstrap wait loop on a foreign lease.

Native v917 validates the zero event and slot-zero refusal, with v915,
v916, v866, v326 and v327 provider tests passing. Full booted v917
shows one holder and one waiter blocked, owner encoding 4,,0 stable
at acquisition, contention and controlled HALT, 72 bystander sleep
cycles and zero errors. The first post-fix guest timeout came from a
fault-injection hook accidentally using the old lock word again; the
corrected hook and genuine scheduler path passed. No forced unlock,
per-process state or syscall ABI change. One resident event word was
added. A nonreturning owner's resources remain owned; automatic OOM
victim termination is not yet enabled.

## 2026-11-01 v916: foreign FS provider lock cannot be stolen

Native PDP-6 testkit v916 exercises the production fs_provider_reg_call
through four repeated waiter attempts while a different process retains
the existing FS lease word. It checks that none of those calls can
enter the provider, corrupt its owner, or release its shared workspace.
The fifth simulated wait observes normal unlock, then dispatches with
all seven request registers and stack restored. The v915 recursive
self-owner rejection and v866 owner encoding regressions also PASS.
No production code, kernel BSS or ABI change. The test uses a native
simulated wait/retry oracle, not an actual permanently stalled physical
provider or full-system scheduler fault. A genuinely nonreturning owner
still cannot be freed or terminated safely; that full-system progress
and OOM cleanup acceptance remains P0. OOM killing stays OFF.

## 2026-11-01 v915: provider lock detects same-owner recursion

DTFS/D6FS/TSFS share a serialized KCORE gateway with one existing
`fs_provider_ready` word encoding owner_slot+1 in LH while held.
Previously a second gateway call on the *same owner's* kernel stack
entered `proc_wait_event` awaiting its own lease and could never make
progress. The lock-wait path decodes the existing owner-slot field
and compares it with `proc_current_slot`; self-ownership returns -5
without sleeping, restores all saved request registers and the stack,
and preserves the original lease. Other processes still wait/resume.
MEMFS remains direct. Nine resident KCORE instructions are added, but
no new BSS, per-process metadata, heap or provider ABI changes.

Native PDP-6 v915 verifies recursive -5, unchanged lease ownership,
zero owner sleeps and subsequent cross-process wait/release. The v866,
v326 and v327 lock/dispatch/AC-preservation regressions also PASS.
Final integrated v915 rerun: booted 256K PDP-6 with the compacted
nine-instruction gateway PASSes the full v914 APR-fault/permanent FILE
provider refusal test: 11 rejected provider calls, 11 preserved FILE
ownership checks, 11 private sleeps, 512-tick cap, zero errors and a
runnable bystander through controlled guest HALT. This checks system
regression, not a booted nested-lock injection.

Post-v915 full 256K PDP-6 v914 guest regression PASSes: real APR
fault, 11 permanent invalid FILE-provider refusals, 11 private sleeps,
512-tick capped retry, FD retained, independent process runnable and
normal controlled HALT (zero errors). This is not a test of a separate
process permanently holding the filesystem provider lease.

This establishes local self-deadlock avoidance, not recovery when a
separate process holds a provider lock indefinitely. Full booted
provider-lock failure/PI6 and allocation-free OOM fatal cleanup remain
P0; OOM killing remains disabled and TSFS v1.2 work stays deferred.

## 2026-11-01 v914: persistent invalid FILE-provider errors remain owned

The v891 full-system 256K PDP-6 APR-fault continuation already proved
safe retention under permanent VFS provider error -5. The v914 testkit
extends that exact real hardware-fault and bystander workload to a
permanently invalid provider result -4, without changing production
code. A copied kernel returns -4 only from the victim's FD 0 vfs_sync.
The first build deliberately caught a stale v891 guest assertion
(expect class 5); the corrected v914 guest expects class 4 and PASSes.
Measured: one real APR fault, 11 provider calls/rejections, 11 FILE
cleanup passes, 11 owner-stack sleeps, 11 retained-FD checks, zero
premature closes, witness clears, victim exits or test errors. The
retry cap reaches 01000 octal (512 decimal) ticks. GETTEARDOWN reports
FILE stage and the actual negative provider error, while an independent
user remains runnable. Controlled guest HALT does not reap the owner.
Native v888 MM_BUSY/INVAL classification, v909 fatal stage/finalizer
and v885 fatal FILE retry/backoff regressions PASS.

The test proves containment of a permanently invalid FILE provider,
not recovery of corrupt or missing MM extents or bounded reclamation.
Automatic OOM termination remains disabled. Fatal provider-lock and
allocation-free victim retirement acceptance are still P0. No new
kernel instructions, RAM, syscall ABI or module dependency.

## 2026-11-01 v913: booted Type-340 double-MM refusal accepted

Full-system 256K PDP-6 with a live raw Type-340 program exercises the
v912 double-MM-release path, not only an isolated MM stub. A test-only
copied kernel pins the active old list (physical base 19760 decimal)
and the new unpublished list (base 19792 decimal) at their actual MM
free boundaries. Both free attempts return MM_BUSY. A subsequent
WRITE_WORDS and final CLOSE each return MM_BUSY before allocating or
unpublishing anything further. Runtime ownership witnesses confirm
`dpy_list_base == 19760` and `dpy_list_orphan_v912 == 19792` after
all four refusals. FILE/process slot accounting remains unchanged.
There are two physical pins, zero unpins, four rejected frees, zero
errors, 18 live PI6 refreshes, 18 PI7 BLKO completions and 12 refreshes
after first MM_BUSY. The guest prints <D and HALTs normally. Native
v912/v874/v904 regressions pass. Fault hooks and ownership witnesses
live only in the v913 testkit's disposable kernel copy; production
image, kernel ABI and permanent RAM are unchanged since v912.
This proves safe retained ownership under simultaneous permanent
refusal, not reclamation of the two still-pinned MM extents. Permanent
fatal FILE teardown, provider lock failures, and allocation-free OOM
victim retirement remain P0. OOM killing remains OFF; TSFS v1.2 and
final KCORE shrinking remain deferred.

## 2026-11-01 v912: DPY double-MM-free refusal retains both owners

Type-340 replacement previously executed HALT if freeing the old active
raw list failed and freeing the unpublished new list also failed. The
normal MM refusal contract makes this possible when both extents are
physically pinned. The driver now retains that unpublished extent in
one DPY-MRES word, `dpy_list_orphan_v912`; the original active list
remains published. The next DPY WRITE_WORDS or STOP first retries the
orphan release, without allocating further memory or changing PI7.
If MM still refuses, it propagates the error without losing ownership;
if MM succeeds, it clears the orphan before the requested operation.
The original list's clock refresh is rearmed after double failure.

PDP-6 native v912 proves double refusal, no HALT, one tracked orphan,
no second allocation under continued refusal, successful orphan drain,
subsequent replacement/STOP and unchanged original rollback policy.
Existing v871/v874/v904 and PI7 policy regressions PASS. One optional
DPY-MRES BSS word is added; no KCORE BSS or syscall ABI change.
v912 post-commit full-image regression: the existing v910 256K PDP-6
physical-MM refusal guest was rebuilt against the v912 DPY-MRES and
passed again (base 19743 decimal, 3 BUSY, 1 pin, 0 unpins, 18 PI6
refreshes, 18 PI7 completions, 12 post-error refreshes, 0 errors,
normal HALT). This is a regression against the changed driver, not a
full-system injected double-free test.

Permanent refusal can retain both allocations; this is safe ownership,
not eventual reclamation. Full booted PI7/double-fault acceptance, fatal
FILE teardown, provider locks, automatic OOM and KCORE size gates remain
open. OOM termination stays disabled; TSFS v1.2 remains deferred.

## 2026-11-01 v911: SYS_MEMINFO counts all 16 FILE slots

Full-system 256K PDP-6 exposed two independent resident SYS_MEMINFO
errors. FILE capacity and the counted scan used octal 015 (13) instead
of FILE_NFILE=16 (octal 020). More seriously, the routine wrote the
FILE count into result word 7 *before* an in-place filesystem MRES
request initialized result words 2..8, destroying the count. Fix both
limits and move the existing FILE scan to after the optional MEMFS
request, leaving syscall ABI and resident storage unchanged. No new
instructions or per-process state.

A C89 testkit guest duplicates CTY descriptors until all 16 FILE slots
are occupied, checks MEMINFO after each DUP, exercises the final FD 15,
then closes every duplicate and checks baseline counts. Corrected
full-system guest marker <F and normal HALT PASS. Two independent
negative guests PASS: the copied 13-slot kernel reports !1 and the
copied pre-fix FILE-count ordering reports !4. Native v708/v874/v904
PASS. Re-running v910 with production v911 PASSes permanent DPY MM_BUSY
(3 refusals, 1 pin, 0 unpins), 18 PI6/PI7 frames, 12 after refusal,
stable *now genuinely populated* FILE/process counts and normal HALT.
The former v910 counter comparison alone could not establish positive
FILE occupancy because the old routine zeroed both samples.
Permanent provider release, rollback double-fault, OOM and KCORE size
gates remain open; OOM killing stays disabled, TSFS v1.2 deferred.

## 2026-11-01 v910: persistent Type-340 MM refusal retains ownership

Full-system 256K PDP-6 (`DPY OK`) now proves the raw-list final CLOSE
can encounter *permanent* physical MM_BUSY without destroying its owner.
A test-only copy of the kernel pins one real MM owner-014 dynamic-list
extent and never unpins it. Three consecutive CLOSE attempts each return
MM_BUSY (-5); the FILE count and process count remain unchanged from
the post-OPEN sample. PI6 clocks keep replaying the same published list:
18 starts, 18 completed PI7 frames and 12 refreshes after the first
refusal. Exact physical target 19732 decimal, three frees, one pin,
zero unpins, three busy results, zero errors; normal test HALT. Native
v871/v874/v904 and PI7 policy regressions PASS. All v910 instrumentation
lives in daimos-testkit, with no production kernel, size or ABI changes.
The still-pinned allocation cannot be reclaimed; replacement double
failure, locked providers and allocation-free OOM termination remain P0.

## 2026-11-01 v909: actual permanently pinned MM u-area, bounded reaper

Full-system 256K PDP-6 v909 exposed an idle-loop P0 bug: with one
permanently pinned, finalized u-area, the old slot-0 reaper attempted
`mm_free` 1,281 times in 1,280 elapsed maintenance ticks rather than
respecting the shared capped retry interval. The kernel now gates the
idle loop on wrapped elapsed timer ticks, using its existing packed
retry word. Another post-VM correctness change clears the expired
fatal-FILE-close phase bit when retaining a finalized u-area; the
TRANSITION+VM_NONE+FINALIZE owner remains protected. Production
GETTEARDOWN now identifies the outstanding MM stage and one physical
pin instead of reporting stale FILE ownership. The corrected guest
records 7 refused frees, 6 reaper passes, one pin, no errors, a
continuously runnable bystander, nonreapable WAIT and normal test HALT.
Test-only fault hooks remain in daimos-testkit. No new permanent data,
no resource forced free, no size optimization. Permanent provider
refusal still owns RAM indefinitely; automatic OOM termination stays
OFF. Tests and negative controls are in daimos-testkit v909.

## 2026-11-01 v908: live DPY MM_BUSY retains an active raw list

Full 256K PDP-6 with a headless but present Type-340 (`DPY OK`) accepts
normal raw-list WRITE_WORDS, actual PI6 clock replay and PI7 BLKO
completion. A disposable copied-kernel MM hook pins the exact owner-014
physical extent on final CLOSE, causing real MM_BUSY (-5). The FILE
entry and published raw list survive and *six* further raw frames
refresh before unpin. The next CLOSE unpins/frees the original extent;
another DPY OPEN/WRITE/CLOSE succeeds, process/FILE accounting stays
stable and the guest HALTs normally. Testkit v908 counters: base 19730
decimal, 3 tracked MM frees, 1 pin, 1 unpin, 1 busy, 18 raw refreshes,
18 PI7 raw completions, 6 post-error/pre-unpin refreshes, 0 errors.
Existing native v709/v871/v874/v904 and PI7 policy tests PASS. No
production change, ABI adjustment, new permanent data, or size sweep.
Permanent FILE/MM refusal, fatal-process cleanup and OOM termination
remain unaccepted.

## 2026-11-01 v907: physical MM session-HUP retry accepted

Full-system 256K PDP-6 session/HUP failure injection now passes
against production v905. A test-only kernel copy pins the first
child's actual u-area, allows fatal HUP to commit with deferred MM
release, refuses the second member's first HUP, and exercises the
normal parent PROCCTL retry. It verifies exactly one HUP for the
pending child, one successful second-member HUP, retained TTY until
commit, physical unpin, eventual WAIT and normal TTY reattachment.
Guest counters: first child 3, leader 2, base 20020 decimal, pins 1,
unpins 1, refusals 1, second HUP 1, duplicates 0, pending 1,
errors 0, normal HALT. Testkit v907 contains all instrumentation;
no KCORE production code or size change. Permanent FILE/MM failures
and safe OOM victim retirement remain unresolved.

## 2026-11-01 v906: TSFS backlog legacy items superseded; session smoke PASS

The remaining V13/V15/V22 unchecked TSFS provider/format experiments
were marked superseded/deferred after KCORE, rather than left as active
KCORE prerequisites. Existing TSFS boot ABI regression duties remain.
The v905 production image passes the existing full-system PDP-6
session-leader teardown and TTY reattach test. Injected partial
HUP/FILE/MM provider failures still require dedicated full-system tests.
No new production kernel changes in v906.

## 2026-11-01 v905: prevent duplicate committed session HUP

Fixed retry following a session leader's partial member-HUP success:
an already-finalized fatal child may retain its u-area due to MM_BUSY,
so its descriptor stays TRANSITION until background cleanup. A second
HUP would be rejected and block the still-live controlling TTY.
The scanner now recognizes only TRANSITION+VM_NONE+FINALIZE as a
committed event and skips it during retry. Native PDP-6 v905 tests
and original v875 PASS, old-order negative FAILS as expected.
Other live-member failure handling remains conservative. Full-system
partial-fault and indefinite-provider acceptance remain open.

## 2026-11-01 v904: DPY STOP publishes no stale PI7 ownership

P0 DPY STOP now disables the Type-340 controller/PI7 before
unpublishing its raw-list pointer, removing a PI7 interrupt window.
No code size or permanent memory increase. The v904 native PDP-6 test
uses actual `mm_free` and `mm_unpin`: pinned MM_BUSY and wrong-owner
invalid failures preserve the descriptor and list; correction permits
exact free; absent-list close succeeds. Pre-v904 ordering negative
control rejects old source; v709/v871/v874 PASS. Real live-PI7
full-system fault and permanent-provider failure remain open.

## 2026-11-01 v903: TSFS deferred until after KCORE

Decision v903: TSFS v1.2 is not a KCORE feature, optimization, or
acceptance prerequisite. KCORE functional completion, regression tests
and its final size pass come first. TSFS v1.2 sharded metadata follows
as separate pre-0.9 work. Existing TSFS boot compatibility tests remain
required for relevant KCORE changes. No kernel source was changed.

## 2026-11-01 v902: original anonymous PIPE integration PASS

The v901 VFS character-stream fix passes the unchanged historical
`daimos-pipe-anon-v1.c` full-system workload under headless PDP-6.
This covers FIFO-style word and character use, pipe creation,
EOF/EPIPE, dup, RUN inheritance and sleeping peer wakeup. It provides
an independent broader check beyond the native and focused v901
regressions. No production change or size work in v902. Outstanding
OOM/FILE/MM/DPY/session teardown gates remain open.

## 2026-11-01 v901: native PIPE character syscalls PASS

Restored missing PIPE vnode dispatch in resident VFS READCHAR and
WRITECHAR. A character transfer now uses the existing one-word
PIPE read/write queue, without new per-process or resident storage.
The v899 test's initial character-EOF observation was an interface
gap, not a failure of the retained MM owner or PIPE descriptor.
Native positive and pre-v901 negative tests PASS; real full-system
256K PDP-6 guest with alternating character/word I/O, EOF and
stable FILE counts also PASSes and HALTs normally. No shrinking work
was run. Permanent provider refusal, DPY and session lifetime
acceptance, and allocation-free OOM termination remain unfinished.

## 2026-11-01 v900: named FIFO real-MM rollback PASS

Full-system 256K PDP-6 acceptance now covers both anonymous PIPE
(v899) and linked named FIFO (v900) last-reference MM_BUSY rejection.
For v900, the disposable test kernel pins the FIFO extent and the
normal MM provider refuses one free. The existing v870 rollback
restores its linked-list membership and preserves the FILE descriptor:
a second descriptor opens the same pathname and shares the stream.
After physical unpin, final close and a fresh FIFO OPEN/CLOSE succeed.
Guest counters: one pin, one busy, one unpin, zero errors, HALT.
No production code, permanent data or size optimization changed.
The FILE/MM provider and safe automatic OOM retirement gates remain
unfinished.

## 2026-11-01 v899: PIPE last-reference physical MM recovery PASS

Feature-first cleanup testing now accepts v870's anonymous PIPE
last-reference rollback in a booted 256K PDP-6 image. An isolated
kernel hook pins the exact PIPE object; one real `mm_free` returns
MM_ERR_BUSY and the original FILE descriptor remains usable. A
second close unpins and frees the resource; a separate new PIPE
opens and closes successfully. Counters: 3 tracked final-close
calls, 1 pin, 1 unpin, 1 busy, 0 errors, normal HALT. The native
v870 test passes too. No DAIMOS kernel change or size sweep was
required. Named FIFO, DPY and permanent-provider teardown and the
allocation-free OOM termination gate remain unfinished.

## 2026-11-01 v898: real PDP-6 TTY pinned-release test PASS

The production v897 canonical line owner guard is additionally
accepted in a booted 256K PDP-6 guest with the actual MM allocator.
An isolated test kernel allocates and pins the exact TTY line, proves
first `TTY_DETACH` fails without changing session ownership, unpins
before retry, then observes successful detach, reattach, independent
child behavior and normal HALT. Counters: 4 checked calls, 1 pin,
1 blocked MM release, 1 unpin, 0 errors. Test-only hooks are not
committed into production. KCORE size optimization remains deferred.
Automatic OOM victim termination is still disabled pending complete
allocation-free FILE/MM/session cleanup acceptance.

## 2026-11-01 v897: feature-first TTY owner containment

At the user's direction the KCORE shrinking effort is deferred until
feature work and tests are complete. The next teardown audit found a
real TTY lifetime error: `TTY_DETACH` could remove a session after
its canonical input `mm_free` failed, and another session could
subsequently claim that terminal without releasing the old line.
The checked, allocation-free C helper now refuses ownership change
while the exact line MM base persists. PDP-6 native attach/detach
preserves output routing, original session and retryability on refusal.
An unowned terminal can be claimed only after successfully releasing
its leftover input. TTY mode changes share the same release contract.

Native positive and old-code negative controls PASS. A separate native
test compiles the actual C helpers and links real PDP-6 MM operations:
BUSY/invalid owner/missing extent preserve the pointer; unpin/correct
owner permits exact release. Full-system PDP-6 attach/detach/reattach and child session
acceptance (`daimos-tty-session-state-v897.sh`) also PASSes. Existing fatal FILE/MM provider inconsistencies,
allocation-free OOM termination, and several other feature gates remain
unresolved. No forced reclamation or automatic OOM kill was introduced.

## 2026-11-01 v896: publish RUN u-area ownership before startup

The unpublished PDP-6 RUN path now records its newly allocated,
zero-filled kernel-dynamic u-area in the SIDL process descriptor
before invoking `vm_space_startup`. On startup rejection, the normal
`proc_slot_discard` path can therefore release the exact MM extent,
with v892/v894 retry ownership preserved on provider refusal. This
reorders three existing assembly instructions, with no new permanent
state or process ABI. The production startup function presently has no
normal error return, so a test-only injected rejection is required.
Full-system 256K PDP-6 startup-error injection PASS: one discard,
real-MM u-area probe shows zero leaked extents, subsequent RUN/WAIT and
normal HALT. The identical test with isolated pre-v896 publication
ordering detects exactly one leaked extent; occupied process slot
accounting alone misses it. Native v885/v892/v894 cleanup tests PASS.
The v197 booted PDP-6 16-case FD-map rollback regression under three
living children PASSes with headless simulator console configuration.
Same-profile GCC disk-root lowmem KCORE remains 9,426 words,
KCORE+MRES 15,912 and permanent endpoint 15,959 (no growth).
Permanently broken provider recovery and safe allocation-free OOM
termination remain open.

## 2026-11-01 v895: genuine PDP-6 pre-VM RUN MM recovery PASS

The v894 deferred VM cleanup now passes a real full-system
PDP-6 fault-injection sequence. An unpublished RUN child fails
its fourth FD mapping with a real process VM already allocated.
The disposable kernel pins that VM before its real destructor;
two actual `mm_free` returns are MM_ERR_BUSY -5. The v894 SIDL
retry marker retains ownership without a spurious WAIT event.
Slot-zero retry dispatch eventually unpins, completes VM teardown
and frees the unpublished child. An independent child executes.

SIMH testkit v895 PASS: target 3, pins 1, VM calls 3, busy 2,
unpins 1, reaper 2, errors 0, guest CTY `<RFBP`, normal HALT.
No DAIMOS code/data change. Last same-profile KCORE measurement
remains 9,426 words (1,234 over 8,192). This closes the particular
v894 real-process-VM release gate but not permanent provider recovery
or the general allocation-free OOM termination requirement.

## 2026-11-01 v894: preserve unpublished pre-VM RUN owners

Fixed a second rollback liveness gap after v893: `proc_slot_discard`
could return from failed VM destruction before the child's u-area
existed, leaving SIDL allocated but without a retry continuation.
The PDP-6 kernel now uses SIDL scheduler RH bit 1 and the existing
TRANSITION flag to mark the VM owner, retries via the slot-zero
reaper/backoff, and clears the marker only upon committed VM destroy.
Further u-area release failures switch to v892's distinct post-VM
FINALIZE|DISCARD marker. No new resident data, process slot width,
syscall number or removal of provider ownership.

`PROCCTL(GETTEARDOWN)` reports read-only stage 3 RUN_VM even if
the unfinished RUN child has no u-area. Native PDP-6 pre-VM test
PASSes two failing VM releases for each of two children, followed
by successful teardown, independent retry of a rejected u-area
release, exactly-once FREE and no zombie/HUP/WAIT. Native v894
stage-3 query, old queries and cleanup regressions PASS; v893
negative control FAILS as expected. Paired GCC lowmem disk-root
KCORE 9,381 -> 9,426 words (+45), KCORE+MRES 15,867 -> 15,912,
permanent endpoint 15,914 -> 15,959; zero new permanent data.
Booted real VM-pin rollback acceptance and permanent provider policy
are still open. Automatic OOM termination remains disabled.

## 2026-11-01 v893: full-system real RUN MM_BUSY recovery accepted

The v892 unpublished-RUN deferred u-area fix now passes a 256K
PDP-6 full-system acceptance with actual MM pin and free operations.
An invalid fourth FD mapping fails a real user RUN after the child's
VM and u-area were allocated. An isolated test-kernel hook pins the
u-area after real VM destruction, causing two real `mm_free` -5
responses. The existing slot-0 retry mechanism calls the release
twice, unpins before its third attempt, and publishes FREE without
zombie/WAIT. A second independently running child exits normally.

SIMH guest PASS: target slot 3, pins=1, releases=3, busy=2,
unpins=1, reaper=2, errors=0, CTY `<RFBP`, HALT. No production
source, size, new resident data or syscall ABI change. KCORE's
last measured v892 lowmem disk-root endpoint remains 9,381 words.
Permanent provider refusal, pre-VM rollback and automatic OOM
termination remain open; this pass is not an OOM-killer acceptance.

## 2026-11-01 v892: unpublished RUN post-VM cleanup is retryable

Found and fixed a lost u-area ownership edge in `proc_slot_discard`:
failed `mm_free` after committed VM destruction previously left an
unpublished RUN slot in TRANSITION/VM_NONE without the v881 cleanup
marker. The v892 path tags word 067 with FINALIZE|DISCARD and reuses
the existing slot-0 MM retry throttle. It never redoes VM/TTY teardown
or turns the uncommitted child into an EXIT zombie/parent WAIT event.

Isolated native PDP-6 fault-injection PASS; pre-v892 negative control
FAILS, and v881/v883/v885 native regressions PASS. Full-system
RUN v197 PASSes 16 ordinary post-VM rollback cases; it does not
inject a rejected u-area free. Identical GCC
lowmem disk-root package builds show KCORE 9,351 -> 9,381 words
(+30 text); KCORE+MRES 15,837 -> 15,867; endpoint 15,884 ->
15,914; permanent data unchanged. This is ownership correctness,
not a reduction toward the 8,192-word target. A booted guest with
actual RUN rollback MM rejection remains to be tested. Persistent
FILE/MM refusal and automatic OOM termination remain unresolved.

## 2026-11-01 v891: permanently failing FILE owner survives under load

The new testkit v891 full-system guest injects persistent -5 from
VFS-sync for the selected real APR-faulted process's FD 0, while a
separate user program continues running. In a 02400-octal-tick
observation, the actual fatal FILE continuation performs exactly
11 failed close passes and 11 private timer sleeps, reaches the
01000-octal-tick (512 decimal) exponential delay cap, and never
drops its FD, u-area witness or owner. The user's existing
`GETTEARDOWN(pid|0400)` check reports stage FILE, first FD 0,
error class 5. No premature victim EXIT or test errors. The
disposable kernel deliberately halts on INIT exit for measurement;
the test does not simulate provider recovery or claim reclamation.

Full-system PDP-6 SIMH PASS. No production changes, kernel size
or static-RAM growth: KCORE 022124 octal = 9,300 words, +1,108
relative to 8,192. A permanently refusing or metadata-invalid
provider still has no universally safe automatic release action;
automatic OOM process termination stays disabled pending verified
allocation-free lifetime handling. Continue with actual MM owner
inconsistencies and release-provider recovery policy before a
final KCORE feature/size freeze.

## 2026-11-01 v890: irreversible EXEC/FILE retry full-system PASS

The remaining v889 full-system post-commit EXEC fatal recovery
gate now PASSes. In an isolated PDP-6 guest, a valid user
`dsys_exec` stages a replacement; the test injects
`EXEC_LOAD_NOMEM` at the staged-load boundary. The real
low-memory fallback executes `vm_space_destroy` of the old
process VM, verified as `VM_SPACE_NONE` before a test-only
replacement-loader -1 response. The production
`EXEC_REPLACE_FATAL` and `native_sys_exec` handlers then
enter the existing process-owned private-stack fatal FILE loop.

The guest proves one stage fallback, one VM destruction, one
post-commit failed loader, three close passes, two VFS-sync -5
rejections, two sleeps at 1/2 ticks, retained ownership and
real FILE stage/ERR 5 diagnostic, successful close and cleared
word-067 witness, one exit, parent WAIT status 1, CTY `<EH`,
normal HALT, zero faults in test assertions. All instrumentation
remains in a disposable test kernel.

Native fatal EXEC/backoff/provider/MM regressions PASS. DAIMOS
source and resident allocation are unchanged. KCORE `022124`
octal = 9,300 words (+1,108 relative to 8,192), zero new RAM.
Permanent provider failure still has unbounded ownership
lifetime, so automatic OOM termination remains disabled.

## 2026-11-01 v889: genuine full-system APR fault/FILE recovery

A full PDP-6 guest now validates v879/v885/v888 fatal FILE
continuation, rather than only isolated assembly calls.
Test user slot 2 generates a real APR address fault, routed
through CLK PI6 to the production private owner-stack fatal
cleanup. A temporary overlay rejects two VFS-sync requests on
its FD 0 with -5. The resulting full-system counters show
one APR fault, three FILE passes, 3 VFS calls / 2 refusals,
two sleeps (1 and 2 ticks), two checks that the FD remained
owned, one successful descriptor release, one cleared
word-067 error witness, and one final exit. Parent `WAIT`
receives the expected fault status; CTY `<FH` and HALT PASS.
The original `GETTEARDOWN` FILE stage/ERR 5 is observable
while the refusal is active. Injection and diagnostic counters
live only in the copied test kernel, never in production RAM.

No production code changes. KCORE stays `022124` octal
(9,300 words), 1,108 above 8,192. Native v888, v885,
v878/v879, v887 regressions PASS. The separate full-system
post-commit EXEC fatal route and indefinitely rejecting
provider cases remain open. OOM killing is disabled.

## 2026-11-01 v888: first actual FILE-close rejection is observable

The private-stack fatal FILE close scanner preserves its first
failed FD and provider's original return value in the already
reserved u-area word 067 while FILE ownership remains live.
Each retry replaces the witness with that pass's first failure;
successful closure clears it. Nonfatal FILE closes do not touch
word 067. Later MM finalization reuses the word exactly as before.

Existing `GETTEARDOWN(pid|0400)` packs the actual FILE failure
class in bits 15..17 (1..5 = -1..-5, 6 = other, 0 = no witness),
and `ps` now displays a numeric ERR column. v887's read-only MM
extent probe already distinguishes currently pinned, missing,
wrong-type and wrong-owner allocations. No forced release, new
resident word, process descriptor or syscall operation is added.

Native PDP-6 actual-error/retry and previous fatal and diagnostic
tests PASS; the real MM allocator's busy/invalid distinctions
and unpin/release also PASS. Pre-v888 negative control FAILs
as expected. Full-system session-leader HUP/TTY detach and
reattach regression PASSes without injected FILE rejection. KCORE
`022124` octal = 9,300 words (+26 from v887, +1,108 above 8192),
without permanent RAM growth. Automatic OOM killing stays off.

## 2026-11-01 v887: read-only provider diagnostics, no persistent state

The v886 `procctl(GETTEARDOWN,pid)` stage and retry ABI stays
backward compatible. Adding 0400 octal to the PID argument requests
actual provider-owned state: a fatal FILE owner's remaining
descriptor count/first FD and VFS/PIPE/DPY provider class; or a
deferred-MM u-area owner's exact MM allocator pin count, subject
to type/owner verification. Invalid MM metadata produces distinct
missing/type/owner codes, never a fabricated zero pin count.
No mutation, raw addresses, permanent words or new syscall slot.
The `ps` utility displays the packed numeric `DETAIL` alongside
the existing TD and RETRY columns, with zero additional kernel RAM.

Native PDP-6 detail and v886 compatibility tests PASS.
The pre-v887 source FAILs the native negative control. A
full-system PDP-6 guest PASSes live MM pin count 1, two real
MM_ERR_BUSY results, eventual unpin and successful reaping,
and session HUP/TTY while an independent user remains runnable.
KCORE links at `022072` octal = 9,274 words (+72 over v886, 1,082
above 8192), with no added permanent data. The result is a
current snapshot only: a provider might later complete cleanup.
Automatic OOM killing remains disabled.

## 2026-10-31 v886: visible, non-destructive teardown status

The existing `procctl` UUO 077 uses the previously unassigned
and correctly forwarded sub-op 041 to expose
`GETTEARDOWN(pid)` as a read-only integer:
0 normal/zombie, 1 owned fatal FILE close, or MM-pending stage 2
with shared reaper retry interval in upper bits. Invalid/FREE
slots return -1. These stages are derived from the existing
fatal-close bit and exact TRANSITION + VM_NONE + u-area-word-067
marker. No addresses, new permanent state or forced-release
capability are exported. `ps` displays `TD` and `RETRY`.

Native PDP-6 query/dispatch tests PASS; reserved-op negative
control FAILs as expected. Full-system native-UUO acceptance
PASSes with real MM_BUSY, independent spinner and session HUP/TTY.
PDP-6 PS compilation and KCORE link PASS.
KCORE `021762` octal = 9,202 words (+37 v885,
+1,010 above 8,192), zero new static RAM. Permanent MM/FILE
provider failure is diagnosable but still unreclaimable until
the original provider accepts release. OOM remains disabled.

## 2026-10-31 v885: bounded retry CPU load without abandoning owners

Fatal APR/EXEC FILE-close continuation now retries with a
private-stack exponential delay, doubling from one tick up to
01000 octal ticks (512, about 8.5 seconds at 60 Hz). The
post-VM quarantined u-area reaper shares the same capped maximum,
starting at 020 octal ticks (16). Its existing resident word
packs pending, last service epoch, and delay; no new permanent
RAM or process descriptor bits. A newly failing owner resets the
MM delay; a repeated failure preserves backoff. Success clears
the MM interval and completes normal zombie/free ownership.

Native fatal close, MM backoff, scheduler/RT, v881 deferred
finalization, v883 fatal HUP, v880 swap rollback and older
regressions PASS. Pre-v885 constant-interval FILE negative control
FAILs as expected. Full-system PDP-6 real MM_BUSY and continuously
runnable bystander session test PASSes. KCORE `021715` octal
(9,165 words, +30 v884, +973 above 8192). This bounds frequency,
not eventual resource reclamation; persistent provider refusal
still retains resources and automatic OOM killing remains disabled.

## 2026-10-31 v884: no idle-only starvation of deferred MM cleanup

v881-v883 deferred u-area cleanup was possible only in slot 0:
continuously runnable user work could indefinitely postpone MM
reclamation. v884 packs a pending flag into the existing
`proc_finish_last_tick_v881` word. The PI6 scheduler grants one
short slot-0 maintenance turn after 020 octal ticks with a pending
owner, then the idle loop software-kicks ordinary selection.
Runnable RT jobs and disk swap preempt maintenance; dormant
RT leases no longer block it. No new static data or queue.

Native PDP-6 throttle, timestamp-wrap, runnable-user, RT/disk
priority and dormant-RT tests PASS; pre-v884 negative control FAILs
as expected. The guest session/HUP/MM pin test with an independently
spinning bystander PASSes: 2 reaper passes with a populated run
queue, 3 u-area releases with 2 real MM_ERR_BUSY responses, 1
pin/unpin and normal HALT. KCORE `021657` octal (9,135 words),
+39 from v883 with no extra permanent data. Permanent errors,
guaranteed progress with an indefinitely running real-time owner,
and automatic OOM termination remain unresolved.

## 2026-10-31 v883: non-current fatal HUP commit status corrected

Fixed the non-current fatal-event caller contract: after FILE close
and successful VM destruction, a rejected final u-area MM free
quarantines the target for v881 slot-0 completion. This is now a
committed event and returns success to session teardown. An event
rejected before VM destruction still fails. A duplicate fatal event
against the pending target remains forbidden; the original exit
status is preserved. Native PDP-6 testkit coverage PASS, and the
pre-v883 version FAILs the new negative control as expected.
The C fallback enforces the same commitment test. KCORE link PASS
at `021610` octal, 9,096 words (+17 over v882), no new static RAM.
Sustained runnable-load fairness remains a separate acceptance
gate; OOM stays disabled.
The final full-system non-current injected-MM-busy acceptance now
PASSes (slot 2, 3 release attempts, 2 busy, pin/unpin once each,
no error, normal session/HUP/TTY HALT), as does the preserved
current-EXIT v882 scenario. Sustained-load fairness is still open.

## 2026-10-31 v882: guest MM pin/failed-free/retry accepted

Full-system testkit fault injection on the actual PDP-6 kernel
establishes that `mm_pin` prevents u-area `mm_free` twice and
`mm_unpin` permits subsequent deferred finalizer completion.
The first experiment intercepted non-current fatal teardown and
showed that its caller reports a transient failure. The accepted
scenario selects a process entering its own `proc_exit_finish`,
which passes ownership to the slot-0 retry. SIMH readback:
the selected current-EXIT slot (1/2 across runs), three frees,
two MM_ERR_BUSY, one pin/unpin each, no errors,
and normal session HUP/TTY final HALT. No production kernel changes
or KCORE size growth: `021567` octal. Sustained-load fairness and
permanent-MM-error policies remain unresolved; OOM stays disabled.

## 2026-10-31 v881: retryable post-VM u-area finalization

The PDP-6 process finalizer can now defer an MM-rejected u-area free
after VM destruction, preserving the process's TRANSITION and VM_NONE
state and saving status in the unused per-process u-area word 067.
The slot-0 idle loop scans pending finalizers at clock-tick cadence,
with PI6 inhibited while touching shared process/MM state; after a
successful release, the normal zombie/free path runs exactly once.
The saved marker does not consume any session/domain control bit.
Native PDP-6 fault injection and compiled production C EXIT handoff
PASS. The older finalizer/session/VM regressions PASS. KCORE end
`021567` octal (9,079 words; +100 against v880, one resident tick
word). A genuine full-system MM-busy injection, bounded cleanup and
automatic OOM kill remain unaccepted. See TODO.MD and swap design.
Ordinary full-system session leader HUP, TTY detach and reattach also
PASS with the v881 build; actual MM-busy guest fault injection remains open.

## 2026-10-31 v880: preserve process ownership on backing-detach error

Repaired silently ignored backing-release failure at the
`proc_swap_detach` / `vm_space_destroy` boundary. Disk swap records
and block accounting remain intact if the existing SWAP MRES verifier
rejects any block of the allocation. VM_SPACE_NONE is published only
after successful detach. SWAP MRES exports its existing verifier at
index 6 through a single new resident patchable jump; generated MRES
and KCORE link PASS. Native VM and actual C detach tests PASS,
pre-v880 negative-control VM test FAIL as expected. KCORE end
`021423` octal, +16 resident words; no persistent data allocated.
U-area MM failure after VM teardown and complete OOM termination
remain unresolved in TODO.MD.
The full-system headless session/HUP/TTY regression also PASSes with
the newly exported verifier installed in SWAP MRES.

## 2026-10-31 v879: owned fatal-close continuation before slot-0 EXIT

PDP-6 user APR and post-commit EXEC fatal paths now preserve their
private executive stack until FILE resources close successfully.
They retry a rejected close after timer-yielding rather than lose
descriptors in slot-0 teardown. Existing process control bit 2 marks
the continuation, blocking duplicate event termination, swap-out
and OOM victim selection without a new permanent table. A VM-less
post-commit EXEC fatal process remains selectable and restores its
executive continuation without programming APR from VM_SPACE_NONE.
Native retry/VM-less selection and APR restoration, direct swap-out
guard, EXEC/FILE/session/OOM selector tests PASS; KCORE link PASS at
`021403` octal (+108 words, no new persistent data). Untagged
VM_SPACE_NONE (-1) is excluded by native/C scheduling instead of
being mistaken for a resident map. The old native swap-out guard
FAILs the new negative-control test, as expected.
The final-source full-system ordinary session HUP/TTY integration test
also PASSes (headless display isolation); injected fatal-provider
rejection is still not covered by that test.
No full-system forced-provider-failure or total-latency guarantee,
and final release after VM destruction remains unproven. Automatic
OOM killing stays disabled. See TODO.MD and swap-pressure design.

## 2026-10-31 v878: missing post-commit EXEC FILE close corrected

The unrecoverable PDP-6 EXEC branch now runs `file_close_all` on the
surviving process-private stack before jumping to slot-0 EXIT. Without
that call the v869 FILE invariant would force a HALT with otherwise
closable descriptors after the old VM had gone. Native successful
close, failed close, normal EXEC and pre-commit failure PASS; older
source FAILs the new negative-control test as expected. Other
FILE/EXIT/finalizer tests PASS. KCORE endpoint `021227` octal (+1
word). Persistent failure still requires deferred reaping and is not
OOM-safe; no kill activation. See TODO.MD and swap-pressure design.

## 2026-10-31 v877: post-v876 full-system HUP/TTY regression PASS

The full-system session teardown guest successfully completed on the
v876 process finalizer. Leader exit HUPed both running/stopped session
members, detached its controlling TTY and allowed reattachment.
The test isolates CTY from unrelated SDL graphics backends; the old
unisolated simulator crash (139) was pre-INIT. All native v875-v876
failure/rollback tests also PASS. No OOM kill, APR/EXEC deferred cleanup
or full-system fault-injection acceptance is implied. KCORE size stays
`021226` octal, with no source changes in v877.

## 2026-10-31 v876: fatal finalizer scheduler/RT safety

Fixed early RT-lease revocation and a period of schedulability during
potentially sleeping session HUP. `proc_finish_slot` now rejects an
existing VM transition, publishes its own TRANSITION before member
callbacks, clears it after failed HUP/VM destruction, and drops RT
ownership only once the VM is destroyed. Native failure/rollback and
successful-zombie tests PASS, with old-source negative control FAILing.
KCORE lowmem link PASS at `021226` octal. No extra resident data.
Nonreturning fatal APR/EXEC cleanup remains an OOM prerequisite.

## 2026-10-31 v875: session controlling-TTY teardown ordering

Reordered the resident `proc_session_teardown` commit: it now delays
controlling-TTY detachment until member HUPs succeed. If an HUP fails
because FILE or other victim cleanup cannot finish, the original TTY
state remains for retry. The native v875 test PASSes failed-second-HUP,
retry, nonleader and detached cases, while pre-change code FAILs the
early-detach check. Lowmem KCORE link PASS at `021222` octal.
The recursive HUP sequence is still not fully atomic, and fatal APR/
EXEC cleanup plus OOM victim killing remain disabled/open in TODO.MD.

## 2026-10-31 v874: atomic DPY list replacement

Fixed the Type 340 old-list MM allocation leak: replacement now frees
the previous extent before publishing the new program, with rollback
that frees the unpublished replacement on an old-free failure.
Native PDP-6 fault injection PASS for failure, recovery, successful
replacement, no-old-list installation and allocation failure. The
pre-v870 negative control FAILs as expected. Existing v709/v871 DPY
tests, v870 PIPE close, Type 340 PI policy, direct DPY MRES link and
lowmem KCORE link PASS; KCORE exclusive endpoint `021216` octal.
If both rollback releases fail, an explicit MM invariant HALT remains.
Actual full-system replay/combined-failure and OOM kill acceptance
remain open in TODO.MD and design/swap-pressure-recovery-v755.md.

## 2026-10-31 v873: named-FIFO full-system baseline comparison

Full-system KCC named-FIFO validation times out (124) after D6FS
startup on current v871 and on the isolated pre-v870 v869 source
`8b4e7d9`. Neither run reaches its completion marker, so no FIFO
full-system acceptance is claimed and the cause is still unknown.
Separately, the native v870 rollback fixture detects the old bug:
it FAILs against v869 source and PASSes after the repair. No kernel
code changes in v873. See TODO.MD and the swap-pressure design.

## 2026-10-31 v871: Type 340 final-close resource ownership

Fixed ignored DPY raw-list release failure and loss of the old
display-list MM base. The FILE close now reports service failure;
DPY stop retains the original list/IOWD and resumes clock replay
after a rejected free. Native v871 fault injection, revised v709
gateway checks, direct DPY assembly, and lowmem KCORE link PASS,
endpoint `021216` octal. Separate DPY replacement failure and
full-system stop/replay fault injection remain open. Automatic OOM
termination is still disabled; see TODO.MD and swap-pressure design.

## 2026-10-31 v870: PIPE/FIFO release rollback repaired

Fixed a retry bug in final PIPE close: before v870, failed MM release
could leave a live FD with a zeroed pipe reference count. Now the final
free precedes ref-count publication, and named FIFO list removal is
reversible without allocation when `mm_free` rejects the request.
Native PDP-6 testkit v870 verifies anonymous and FIFO failed-free
retry, list preservation and normal non-final close: PASS. Lowmem
KCORE link PASS at `021217` octal. OOM teardown is incomplete; no
automatic process killing or broad full-system OOM acceptance claimed.

## 2026-10-31 v868-v869: process-finalization guard

Current-process fatal event and SYS_EXIT return failure without destroying
an unclosed FILE descriptor after failed SYNC (v868). The PDP-6 final
process teardown rejects missing u-area or any live descriptor before
changing RT, session, VM or scheduler state (v869). C reference paths
have matching error handling and final guard. Native v868 and v869
fault/guard fixtures PASS and lowmem KCORE link PASS, exclusive endpoint
`021211` octal. No permanent data table was added; OOM killing is still
OFF because forced nonreturning APR/EXEC exits, potentially blocking
providers and reclaim progress are not yet OOM-safe. Track in TODO.MD.

## 2026-10-31 v867: victim FILE close failure is now visible

The static fatal-other event path no longer destroys a process u-area
after any descriptor close fails. `file_close_all_at_v865` checks all
16 descriptors and reports the aggregate failure; the victim remains
live and retryable. Testkit v867 native PDP-6 failed/successful-sync
fault-injection PASS, previous v865 explicit-target regression PASS,
lowmem KCORE link PASS. No new persistent data or backing sectors.
This is a precondition for, not activation of, allocation-free OOM kills.
Held provider leases, nested session HUP, current-process EXIT and
failure-atomic no-progress policy remain open in TODO.MD.

## 2026-10-29 v670: D6FS root mount-stage diagnosis

The scheduler-pressure boot's `?RT` occurs after BLOCKSET discovery,
physical block reads and entry to D6FS superblock selection, before
VFS mounting. The diagnostic v670 test reproduces this stage; it
does not pass the full-system scheduler test. Root cause remains open.

## 2026-10-29 v669: scheduler fault-acceptance blocker

The historic full-system pressure runner v298 has stale fixed-KCORE
assumptions for SWAP MRES functions. A corrected v669 diagnostic
runner reaches native SIMH boot but the guest emits `?RT` and halts
at `065206` before workload completion. This is NOT a passed swap
or rollback test. Isolated v664-v668 remain PASS; diagnose the startup
failure before full-system scheduler acceptance. No production edits.

The `?RT` code is emitted by `kinit_boot()` on root mount or regular
`/SYSTEM/INIT` validation failure, before scheduler pressure begins.

## 2026-10-29 v668: recoverable late MM-free rollback verified

The testkit v668 isolated native PDP-6 test PASSes: after successful
`mm_unpin`, injected `mm_free` failure retains resident VM/swap record,
releases provisional BACKING, clears the staging word and TRANSITION.
Full-system scheduler and I/O fault integration tests remain open.

## 2026-10-29 v667: fix unsafe swap-out unpin failure cleanup

Both `mm_unpin` results in production PDP-6 `proc_swap_out` are now
checked. Unexpected failures enter the existing swap invariant
handler rather than unlocking a potentially pinned resident process.
Testkit v667 and updated v666 source checks PASS; isolated native
PDP-6 branch fault tests PASS at both unpin sites. Fresh GCC 32K
disk-root boot PASS. Full-system MM fault handling remains open.

## 2026-10-29 v666: swap late rollback audit

Production PDP-6 swap-out control flow confirms MM unpin/free failures
return through provisional BACKING release before DISK publication.
Testkit v666 source regression PASS; native late-MM fault injection
still outstanding. No KCORE optimization or production change.

## 2026-10-29 v665: BACKING partial-write failure contract

Native PDP-6 gateway test PASS: injected second-block error stops a
three-block write and returns failure. Source audits confirm swap and
MEMFS do not publish newly allocated backing extents after write
failure. Physical writes are not atomic; broader rollback remains open.

## 2026-10-29 v664: native VOLUME mapper absence confirmed

daimos-testkit v664 executes production BACKING assembly on PDP-6:
missing VOLUME mapper returns -1 with zero device calls, while direct
DEVICE mode remains operational. Live full-system mapper removal and
partial physical transfer faults remain separate validation work.

## 2026-10-29 v663: native multi-member VOLUME regression

Two- and four-member D6FS HDD runtime and missing/mismatched-member
rejection PASS after correcting stale x86-64 host relay executables on
an AArch64 host. Testkit v663 launcher creates native relay binaries
for each run. Targeted runtime mapper-loss fault injection still open.

## 2026-10-29 v662: BACKING naming in runtime consumers

Process swap and MonitorFS both use `backing_region_blocks()`;
the old BLOCKSET query remains an alias. One shared DEVICE/VOLUME
hardware path remains. No runtime layer or storage-format change.

## 2026-10-29 v661: LOG remains independent of BACKING/CACHE

Reclaimable LOG boot I/O now directly uses AUXSTORE REGION read/write
with explicit LOG selection; redundant AUXSTORE LOG wrappers removed.
Legacy root-backed LOG remains supported. No change to LOG on-media
record format or persistent MRES ABI.

## 2026-10-29 v660: region state vocabulary without additional RAM

Existing backing transfer config words have direct-device selector,
optional VOLUME mapping span, and BACKING region-capacity names.
Legacy aliases remain at identical addresses. Swap boot now queries
`backing_region_blocks`; source regressions v658-v660 PASS.

## 2026-10-29 v659: fail closed on unavailable VOLUME mapper

The permanent backing bridge now rejects missing multi-member mapping
instead of silently addressing the first physical device. Existing
direct DEVICE path unchanged; native 32K disk-root boot reaches
`proc_boot_init` with BSET/D6FS loaded. Targeted native mapper-loss
fault injection remains pending. No size-optimization work.

## 2026-10-29 v658: direct DEVICE vs multi-member VOLUME split

Named direct DEVICE configuration for AUXSTORE backing and singleton
root swap tails; VOLUME configuration used for multi-member roots.
Legacy BLOCKSET ABI is preserved and hardware dispatch remains shared.
This is storage feature reorganization, not RAM optimization.
Fresh native 32K D6FS/disk lowmem GCC image reached `proc_boot_init`
with BSET and D6FS loaded. Multi-member regression remains open.

## 2026-10-29 v657: BSTORE shared-region feature implementation begins

The production AUXSTORE code now provides a range-checked, shared
region-relative raw I/O interface for BACKSTORE, LOGSTORE and
CACHESTORE. LOGSTORE bootstrap wrappers use it. Testkit host region
bounds, AUXSTORE selection and D6FS overlap PASS. BACKSTORE integration
and native recovery remain unfinished; no size-sweep work was done.
The online D6FS packer and agreed storage/admin extensions are 0.9
release scope (not post-0.9). DTFS/TSFS work is deferred for now.

## 2026-10-29 v656: mm-core host regression PASS

The stale `mm-core` testkit fixture now builds and passes with C99
test-only reference VM lifecycle functions, runtime-sized descriptor
checks, and current 8-to-64 process-slot expectations. No kernel
source changes or permanent RAM growth.

## 2026-10-29 v655: DSK0 media loss PASS; mixed-mode KINIT build fixed

Fresh 96K GCC full-profile native PDP-6 SIMH, 256-word KINIT stack:
detaching DSK0 at KINIT entry reaches fatal HALT and records valid
`0172` octal (122-word) high-water; source D6FS remains CLEAN.
Versioned mode stamp now rebuilds both KINIT C and assembler sources
when watermark or stack reserve changes. A preliminary mixed-build
`0577` reading was an instrumentation artifact. Mode-toggle 1/0/1
regression PASS. KCORE permanent memory unchanged.

## 2026-10-29 v653: missing/nonregular INIT root validation PASS

Native 96K GCC full-profile D6FS root: absent `/SYSTEM/INIT` entry
and nonregular (directory-type) INIT FCB both emit `?RT` and HALT;
no INIT V1 startup. Two testkit v653 scenarios PASS using isolated
disk copies, original media CLEAN. No production changes.

## 2026-10-29 v652: KINIT 256-word reserve accepted

KINIT bootstrap stack default reduced from `0600` (384 words) to
`0400` octal (256 words). Previously verified fatal paths reached at
most `0172` octal (122 words). A fresh 32K GCC lowmem native SIMH
boot reached `proc_boot_init`, and the invalid-root HALT recorded
`0164` octal. Image end `077260`, reserve end `077660`, safely below
`0100000`. Unrelated unmeasured error paths remain open.

## 2026-10-29 v651: BLOCKSET read-I/O failure watermark PASS

Native 96K GCC full-profile KINIT with simulator-only BLOCKSET read
failure during D6FS mount emits `?RT`, HALTs, and records `0172`
octal (122 words) stack usage. CLEAN input media is preserved; testkit
v651 PASS. No production changes; missing-INIT remains open.

## 2026-10-29 v650: KINIT MM allocation-failure bound PASS

Native 96K GCC full-profile KINIT idle-stack `mm_alloc` failure forced
via simulator-only instruction replacement after Stage1 loading.
Fatal HALT watermark `0172` octal (122 words); `0600` reserve intact.
Media CLEAN pre/post, testkit v650 PASS. No production changes.

## 2026-10-29 v649: D6FS mount-validation stack bound PASS

Native 96K GCC full-profile fatal-path watermark `0172` octal
(122 words) on D6FS superblock-validation failure; original disk media
preserved, copies only corrupted. Testkit v649 PASS, no production
change. MM allocation, I/O and missing-INIT fatal paths remain open.

## 2026-10-29 v648: KINIT disk-selector failure watermark PASS

The native instrumented 48K tape-root image with disk-root selector
`1` HALTs with recorded `0164` octal (116 words) stack high-water.
Testkit v648 PASS; no production changes. More complex KINIT root-mount
and MM-failure stack measurements remain open.

## 2026-10-28 v647: four full-profile interactive DSH gates PASS

All GCC/KCC x 96K/256K native PDP-6 full-profile ROOT login, DSH
command, external TRUE and orderly HALT tests PASS, with verified
CLEAN D6FS after shutdown. Testkit v647 isolates writable media and
rejects reusing a DIRTY diskset. No production code change. Remaining
KINIT failure-path stack measurement coverage is still open.

## 2026-10-28 v646: full-profile GCC 256K KINIT PASS

Native 256K GCC full-profile disk-root build and kernel-only SIMH
boot PASS. All 96K and 256K full-profile KINIT GCC/KCC gates now
PASS. DSH and further KINIT error-path stack testing remain open.

## 2026-10-28 v645: KCC 256K full-profile KINIT PASS

Native KCC 256K full-profile disk-root build and kernel initialization
PASS through `proc_boot_init`. Testkit v645 PASS. No production source
change. 256K GCC and interactive userland/DSH gates remain open.

## 2026-10-28 v644: KCC 96K full-profile KINIT PASS

Native KCC PDP-6 96K disk-root full-profile build and SIMH kernel
boot PASS through `proc_boot_init`. KINIT end `113573` octal, stack
reserve `0600` intact; testkit v644 PASS. No production source change.
DSH and 256K full-profile functional checks remain open.

## 2026-10-28 v643: GCC 96K full-profile KINIT PASS

Native PDP-6 SIMH full-profile GCC disk-root build and kernel-only
boot PASS, reaching linked `proc_boot_init` after D6FS/BLOCKSET/MEMFS
initialization. The `0600`-octal stack reserve fits. Userland/DSH,
KCC and 256K full-profile remain open. See `design/kinit-full96-gcc-v643.md`.

## 2026-10-28 v642: KINIT lowmem image with larger installed core

Native PDP-6 SIMH KINIT-only testkit v642 PASS at 48K, 96K and 256K
installed core using the same GCC lowmem TSFS-root image. It verified
exact memory banners, DTC/DSK270 startup, and a map-derived breakpoint
before process initialization. No production change. Full-profile and
DSH acceptance remain open; see `design/kinit-memory-profiles-v642.md`.

## 2026-10-28 v641: 32K KINIT-only native acceptance

Native testkit v641 PASS: standard GCC 32K TSFS boot satisfies the
unchanged `0600`-octal stack reserve, initializes DTC/DSK270/DTFS,
and stops at the linked `proc_boot_init` breakpoint before any
userspace execution. No new kernel mode or permanent code change.
Other-profile and remaining error-path checks remain open. See
`design/kinit-kernel-only-v641.md`.

## 2026-10-28 v640: native KINIT fatal-path stack sample

Optional watermark-enabled HALT paths now save the stack high-water
before stopping; production paths remain unchanged. A native PDP-6
48K invalid-root HALT recorded octal `0164` (116 decimal) words, below
the octal `0600` reserve. Testkit v640 PASS. Other failure paths and
32K kernel-only acceptance remain open. Full details in
`design/kinit-failure-watermark-v640.md`.

## 2026-10-28 v639: optional KINIT watermark boundary corrected

The optional stack-watermark fill and scan used an inclusive upper
bound even though `__kinit_image_end + reserve` is exclusive. At 32K
this could reference nonexistent physical address `0100000`.
Both loops now stop at endpoint minus one. Native 48K GCC lowmem
watermark-enabled TSFS boot PASS (MEM 48 K, INIT V1), testkit v639.
No permanent KCORE growth or reserve reduction. The separate 32K
failure-path watermark and kernel-only acceptance are still open.

## 2026-10-28 v638: 32K KINIT native bootstrap restored

Transient PDP-6 KINIT diagnostic/probe assembly recovered 30 decimal
linked words: image end `077236` -> `077200`, meeting the existing 32K
`0600`-octal stack reserve exactly. No permanent KCORE change or weaker
stack bound. Native 32K TSFS+AUXSTORE v493 PASS; standalone SIMH boot
shows `MEM 32 K` and reaches INIT V1. The auxiliary image generator's
DBOOT-to-AUXST1 conversion is now explicitly confirmed, signature- and
layout-validated, preserving D6FS. Host v636/v638 safety tests PASS.
Other profiles, stack watermark and failure-path acceptance remain open.
See `design/kinit-lowmem-footprint-v638.md`.

## 2026-10-27 v512: final KCORE optimization waits for functional freeze

The consolidated functional freeze checklist is now in TODO.MD, with a
matching SIZEOPT.MD gate and KCORE.MD chronological note. Required
acceptance/correctness includes timekeeping, MKSWAP, mixed storage/BLOCKSET
inventory, swapped MonitorFS process metadata and scheduler/testkit
regressions. Before declaring the source ABI frozen, decide inclusion of
the D6 codec production cutover, TSFS v1.2, BSTORE remainder, production
MEMFS XIP and compressed process swap. Post-0.9 module relocation and
user overlays remain deferred. This is planning/documentation only; no
runtime changes or measured savings. Preserve the v508 accepted size base.

## 2026-10-27 v513: fresh permanent-size measurements and XIP/512-word TODO

Rebuilt the current source's GCC `permanent-size` target for three PDP-6
profiles (not historical v219 binary sizes). 32K lowmem TSFS/tape:
KCORE `020544` = 8548 words; KCORE+packaged MRES `032227` = 13463;
permanent endpoint `032306` = 13510; 2873 words free below `040000`.
32K lowmem D6FS/disk: same KCORE; packaged MRES `032343` = 13539;
endpoint `032422` = 13586, 2797 free. Disk multi-root LAST is
`032463` = 13619, 2764 free.

256K GCC full/auto: same KCORE, packaged MRES `043537` = 18271;
single-root LAST `043616` = 18318, **1934 words above `040000`**;
multi-root LAST `043657` = 18351, **1967 words above**. The full/auto
`permanent-size` reporter consequently returns FAILURE; do not portray
this as a passing full 256K permanent address map. Optional packaged
MRES counts do not by themselves establish simultaneously installed
MRES or physical runtime RAM. No production source changes or savings.

Consolidated incomplete XIP RUN/EXEC work at the front of TODO.MD; the
existing MEMFS admission/release and native proof tests are implemented,
but general scheduled production XIP remains disabled. Added a deferred
study of a 512-word allocation quantum. Crucial units: source `02000`
is octal (1024 decimal), and both PDP-6 APR models hardwire ten low bits
for process relocation/limit. Software allocations may be more flexible;
do not promise native 512-word process mappings on original hardware.
Preserved the unrelated working-tree `userland/dsh/dsh_exec.c` edit.

## 2026-10-27 v511: P-1 KCORE backlog reconciliation

Rechecked current v510 source/documentation before optimizing. v507-v508
already established validated AUXST1 and native 32K DSK/DRM swap; v509-v510
established that new raw-AUXSTORE D6FS superblock parsing is unnecessary.
`TODO.MD` and `SIZEOPT.MD` now separate completed/superseded storage work
from remaining combined-provider coverage and the first active `proc_swap`
object audit. Before each TODO, verify whether implementation is already
present or the premise is stale. Native 32K testing covers boot without
DSH; DSH functional checks run at 48K/96K/256K. Full 32K DSH command
acceptance is deferred to a separate shell optimization run. No kernel
implementation or RAM saving is claimed. Preserve the v508 full linked
baseline until an exactly matched fresh build supersedes it.

## 2026-10-27 v510: validation scope closed; proc_swap audit initiated

Confirmed D6FS-root KINIT already performs BLOCKSET descriptor discovery
and dual-superblock selection. Non-root raw AUXSTORE uses its own
validated descriptor and may have unformatted D6FS space; no additional
automatic D6FS parser is justified. The separate `proc_swap` fixed
KCORE object audit has begun, distinguishing mandatory process/PURE
metadata cleanup from the optional swapv481 MRES. No source behavior
changed and no memory reduction is yet accepted. Baseline remains v508.

## 2026-10-27 v509: AUXSTORE/D6FS format separation verified

`mkauxstore` does not format its optional D6FS partition. For TSFS
raw-swap AUXST1, block-zero descriptor validation (v507) is sufficient
for isolating BACKSTORE/LOGSTORE/CACHESTORE; reading or requiring valid
D6FS superblocks there would regress valid unformatted media. D6FS-root
KINIT already performs dual-superblock verification through
`d6fs_boot_mount_root()` and `d6fs_super_select()` after BLOCKSET
discovery. New testkit v509 host regression passes; no kernel code or
permanent RAM change. Further formatted non-root D6FS/BLOCKSET
validation must have an explicit consumer, not be part of bare AUXSTORE
discovery. Prior v508 size baseline unchanged.

## 2026-10-27 v508: pre-chat closeout, positive DRM swap and fresh sizes

32K GCC TSFS root with a separately provisioned DRM0 AUXST1 passed the
native swap-out, wake, slot-0 swap-in and WAIT workload. The test first
failed a *test-media* tail-layout check (CACHESTORE was zero rather than
filling the rest of the 020000-block drum); fixing that fixture and
allowing 180 seconds for drum I/O produced PASS. This is real native
DRM0-backed process swapping, not merely successful driver probing.
The separate v507 32K DSK AUXST1 positive regression also passed.

Final fresh full linked GCC lowmem reports: KCORE 8548 words; tape
KCORE+packaged MRES 13463; disk 13539. Optional DRM MRES is 124
words included in those package totals, installed only when detected.
Default KINIT stack 0600 octal = 384 words; measured successful boot
watermarks are 129 (TSFS), 136 (TSFS+DRM) and 201 (full 256K).
Full-image package totals do not include additional persistent runtime
MM allocations or prove worst-case failure-path stack depth. Strict 32K
acceptance is deferred by policy. Lowmem MEMFS autoload remains cancelled.
TODO.MD/SIZEOPT.MD reviewed. Next work: read-only D6FS superblock and
BLOCKSET reservation validator in KINIT, followed by ordered SIZEOPT
objects. Preserve unrelated DSH working-tree edits.

## 2026-10-27 v507: AUXST1 D6FS partition validation

KINIT AUXST1 discovery now rejects D6FS partition overlap against
BACKSTORE/LOGSTORE/CACHESTORE, out-of-medium partition bounds,
bad/duplicated relative superblock offsets and inconsistent absence.
No additional permanent state or linked-size growth: GCC 32K lowmem
KCORE 8548, packaged tape 13463. New testkit v507 invalid-descriptor
test PASS, existing AUXSTORE choice PASS, native 32K TSFS+AUXST1 swap/
wake/WAIT PASS. Full D6FS superblock/BLOCKSET read-only validation
remains open; 384-word KINIT stack default retained.

## 2026-10-27 v506: 384-word default KINIT stack and lowmem DRM

Default KINIT stack reduced to octal `0600` (384 words), based on
instrumented prior TSFS 129, TSFS+DRM 136, full 256K 201-word boot
highwaters. Lowmem now includes optional hardware-probed DRM236 MRES
(124-word resident payload when installed), while MEMFS remains
excluded. Native 32K TSFS/DRM boot reaches INIT, KCORE unchanged at
8548, packaged tape total 13463. Testkit v506 regression added.
Positive DRM AUXST1 swap and expanded exceptional-path stack testing
remain open; 384 is a policy choice with 183-word margin over the
maximum successful boot measured, not a proof for every boot path.

## 2026-10-27 v505: maximal KINIT boot measured, 256 not adopted

Fresh 256K GCC full/auto SIMH image booted through DRM236, BLOCKSET,
LOGSTORE, MEMFS and D6FS to INIT; KINIT stack watermark octal `0311` =
**201 words**, compared with prior 32K TSFS 129 / TSFS+DRM 136.
Only 55 words would remain with a 256-word reserve; this is not
"clearly below" 256 and cannot cover untested worst-case/error paths.
No stack-size default changed. 512 remains a candidate subject to wider
coverage; original 1024-word stack reserve remains active.

## 2026-10-27 v504: native 512-word KINIT watermark measured

Fresh 32K GCC lowmem builds with temporary 512-word stack and watermark
reached INIT in SIMH. Native `kinit_stack_highwater` yielded **129 words**
for TSFS/DSK, **136 words** for the experimental DRM-enabled TSFS image;
the DRM probe reported `DRM236 OK`. The instrumented DRM image passed
the 512-word stack-bound build check. Neither the default 1024-word
stack reservation nor optional lowmem DRM has been accepted/changed;
both require broader boot and failure-path coverage. Experimental code
was reverted. Unrelated DSH working tree change was preserved.

## 2026-10-27 v503: KINIT 512-word stack exploratory boot passed

With `KINIT_STACK_RESERVE_WORDS=01000` only as a make-time override,
the 32K GCC lowmem TSFS image built and reached `INIT V1` on SIMH.
No default stack-size or kernel source change; watermark and maximal
boot-path stack usage remain unverified. The earlier lowmem DRM experiment
remains reverted until separately retested with measured stack safety.

## 2026-10-27 v502: lowmem DRM blocked by KINIT image overlap

Optional DRM support was prototyped by enabling the existing 124-word
DRM MRES and MINIT on a 32K tape profile. The linked permanent image
fit (KCORE 8548, packaged tape 13463), but KINIT ended at octal
`076144`, **100 words over** the `076000` stack-safe boundary. Native
boot tests could not run. All code changes were reverted; accepted tape
packaged total 13339 remains. Next work: reduce transient KINIT image
by more than 100 linked words without compromising the stack reserve,
then retry optional DRM and native swap tests. The separate SIZEOPT
KCORE object backlog remains deferred.

## 2026-10-27 v501: automatic MEMFS/backingstore work cancelled

Automatic MEMFS enablement whenever backingstore exists is cancelled,
including for systems below 64K. There is no active lowmem MEMFS
delivery/auto-mount implementation task. Keep the v496 accepted behavior;
investigate general loadable modules only after KINIT size optimization,
with explicit loading rather than device-triggered MEMFS autoloading.
Existing swap work and deferred SIZEOPT audits remain in scope.

## 2026-10-27 v500: optional MEMFS delivery constraint quantified

Accepted 32K tape KINIT ends at octal 075637, only **97 words** below
the stack-safe octal 076000 boundary (1024-word stack reserve). The
1710-word MEMFS MRES cannot be embedded there, regardless of whether
KINIT would later skip installation. Existing MINIT source reclamation
happens after image loading and cannot solve the overlap. No RAM-growth
experiment was accepted. External, validated package delivery after root
selection is prerequisite to backed lowmem MEMFS; preserve contiguous
MRES placement, loader ABI, native swap regression and boot stack budget.

## 2026-10-27 v499: lowmem MEMFS experiment blocked by KINIT headroom

MEMFS-on-backed-32K prototype failed `mkimage` stack-reserve acceptance
even though linked KCORE remained 8548: including the 1710-word MEMFS
MRES source in transient KINIT exceeds available image/stack space.
No workaround or stack-budget relaxation accepted. All experimental
source changes reverted; the accepted tape full link is 13339 words.
Need new optional-package source/reclamation design before MEMFS can be
activated on backed lowmem roots. SIZEOPT object backlog remains deferred.

## 2026-10-27 v496: TSFS AUXST1 native swap regression resolved

Corrected TSFS `SYSTEM_INIT_DXR` override rather than changing KCORE
or swap internals. Native 32K TSFS+AUXST1 swap-wait now PASSes swap-out,
wake, restoration and WAIT. Suppressed false MEMFS warning if not
requested by fstab, and set no-MEMFS FSTAB for configured RAM below 64K.
Full linked GCC lowmem KCORE 8548; tape packaged 13339, disk 13415.
Lowmem drum and KINIT-only D6FS reservation/BLOCKSET discovery remain
open. The unrelated DSH working-tree changes are untouched.

## 2026-10-27 v495: native TSFS swap test INIT mismatch isolated

The v494 custom INIT is recorded correctly in the TSFS manifest, but
SIMH boots the normal `INIT V1` program instead. The swap regression is
therefore blocked before proven process-swap execution. Preserve
test artifacts and trace the actual boot executable-selection path
before modifying VM/swap logic. KCORE and permanent RAM unchanged.

## 2026-10-27 v494: positive AUXST1 process-swap test not yet passing

The TSFS-root native swap-wait diagnostic with explicit AUXST1 DSK0
booted to INIT but timed out (45 seconds, status 124), without a verified
swap/wake/WAIT completion. This is an open blocker, not a passing test.
Diagnostic v494 is kept in daimos-testkit; no kernel implementation
change is accepted. Inspect INIT workload execution and restoration.

## 2026-10-27 v493: native positive AUXST1 boot fixture

Introduced a daimos-testkit native fixture for a 32K TSFS root using an
explicitly formatted, disposable AUXST1 DSK0 image with nonzero backing
reservation. Initial native boot reaches INIT V1; actual swap-out/in,
resident MEMINFO boundary and optional DRM remain unverified. This is a
test-only step; KCORE and kernel service code remain unchanged.

## 2026-10-27 v492: validated AUXST1 swap preference correction

KINIT discovery no longer treats a log-only DRM descriptor as a reason
to ignore usable DSK swap. The probe still prefers a DRM reservation
when present, preserves log-only DRM when neither device has swap, and
does not modify media. Testkit v492 host fixture exercises the actual
discovery code. Native lowmem tape-root still boots to INIT; KCORE 8548,
tape packaged 13339, disk packaged 13415 unchanged. Positive native
AUXST1 swap and full lowmem tape+drum are pending.

## 2026-10-27 v491: TSFS AUXST1-based optional SWAP selection

In D6FS-disabled images, SWAP MINIT uses reclaimable KINIT AUXST1 discovery
after physical storage driver MINITs. Valid nonzero backing capacity is
necessary before installing the 412-word pinned SWAP MRES. Unprovisioned
32K tape-root native boot reaches INIT; D6FS module is not loaded. GCC
lowmem linked KCORE 8548, packaged tape 13339 and disk 13415 unchanged.
Do not assume this implements lowmem tape+DRM: the lowmem profile still
excludes DRM and positive native AUXST1 tests are pending, along with
safe D6FS reservation and BLOCKSET discovery in KINIT.

## 2026-10-27 v490: swapless lowmem TSFS bootstrap

KINIT suppresses pinned SWAP/BACKSTORE MRES installation on the existing
device-free lowmem TSFS root configuration. The v489 zero-capacity process
boot preserves executable backing with no allocator service. Static tape
package-linked combined 13339 (includes uninstalled SWAP 412), actual
installed MRES is therefore expected to be 12927 based on `mres_install`
semantics. Native 32K TSFS root reached INIT; KCORE unchanged at 8548.
Disk lowmem full link unchanged at 13415. This is not yet runtime
selection for TSFS+auxiliary DSK/DRM; KINIT read-only discovery and an
explicit AUXSTORE-capable lowmem build remain required.

## 2026-10-27 v489: zero-capacity backing independence

`proc_swap_boot_init` in reclaimable KINIT now initializes executable
backing records and packed storage state without calling optional
BACKSTORE allocator exports when its discovered block capacity is zero.
Both full linked permanent baselines are unchanged (disk 13415,
tape 13339; KCORE 8548). Native 32K tape-root reaches INIT V1 and disk
swap-wait passes. Conditional swap MRES installation and the independent
KINIT-only reservation reader remain pending; no D6FS module was added
to TSFS-root images.

## 2026-10-27 v488: preparatory SWAP installation ordering

SWAP MINIT/package-source order now follows DSK/DRM physical driver
MINITs, instead of preceding them. The KCORE ABI and source are untouched;
GCC 32K lowmem disk KCORE 8548/combined 13415, tape KCORE 8548/combined
13339. Both native root boots reach INIT; TSFS eight-member test passes.
Actual conditional SWAP/BACKSTORE installation remains open: KINIT must
discover valid auxiliary backing space without loading the D6FS filesystem
provider on TSFS roots, and retain minimal process backing with no device.

## 2026-10-27 v485: storage-state runtime relocation rejected

The existing process backing records and packed storage-mode map are
semantically active and already compact (one record per process and two
bits per process). Moving the two storage-mode native routines to SWAP
MRES reduced fixed KCORE by 40 words but grew combined permanent RAM by
two words, **13415 -> 13417**. All v485 implementation edits were reverted
and the accepted v484 disk-root full link rebuilt: **8548 KCORE**, **412
SWAP/shared MRES**, **13415 combined**. No runtime tests were needed for
the rejected no-growth experiment. Continue optimizing only if a measured
full-image reduction or a truly size-neutral change can be demonstrated.

## 2026-10-27 v484: accepted BACKSTORE KCORE-to-MRES split

Corrected prior v482/v483 failed bootstrap experiments: KCORE linker
boundary `kcore_end.dobj` must be **last** in the full fixed link. Earlier
attempts placed BACKSTORE BSS over `__kcore_low_end`/MM descriptors. In v484
the four state words remain in KCORE (`020620`-`020623`), the allocator
TEXT joins pinned SWAP MRES, and `__kcore_low_end=mm_extents=020624`.
GCC lowmem disk KCORE **8548** (v481 8652), SWAP/shared MRES **412**
(v481 308), combined permanent **13415** unchanged; tape combined **13339**
unchanged. Native 32K swap-wait, swap-stress, PDP-6 backstore fixture and
KCORE-boundary guard PASS. No process-swap boot-data layout change. Strict
DSH 32K acceptance remains deferred.

## 2026-10-27 v482: BACKSTORE relocation unaccepted

Shared BACKSTORE allocator TEXT (108 words) was experimentally separated
into MRES with KCORE state and four patched entry points. Both lowmem
disk and tape full linked resident totals were unchanged, but native
32K disk-root boot halted in KINIT after D6FS initialization, before
swap-wait process execution. Reverted all v482 implementation files.
Accepted v481 KCORE **8652**, disk-root KCORE+MRES **13415** unchanged.
The bootstrap failure needs direct root-cause tracing; do not adopt v482
from link-only evidence. Process-swap boot record/map allocation remains
unchanged and is still used independently of physical swap capacity.

## 2026-10-27 v481: measured KCORE swap relocation

Process swap transfer/reclaim has moved to a pinned boot `swapv481` MRES;
KCORE keeps only two patchable entry instructions and its existing PURE
metadata/storage-state runtime. Fully linked GCC lowmem disk-root KCORE
is **8652 words** (was 8960). Swap MRES adds **308 words**, leaving total
permanent KCORE+MRES unchanged at **13415**. The separate tape-root link
also gives KCORE **8652**, combined **13339**, unchanged versus its prior
root-specific package footprint. Swap-wait, swap-stress, and native PURE
image reconstruction/verifier tests PASS. SWAP MINIT must be installed
early in physical package order for the 32K boot to succeed. Strict 32K
DSH acceptance remains a separate issue. `backstore` and swap boot-state
relocation remain unimplemented, pending consumer/lifetime analysis.

## 2026-10-27 Proposal B v480: optimization phase closed

Ten final bounded resident source searches found no credible safe
>2-word cut beyond v479. Native SYMLINK v448 and RENAME v455/v457
PASS. No production change. KCORE 8960, KCORE+MRES 15507, permanent
last 15554 remain the accepted baseline. Full-system SYMLINK and
within-RENAME PI6 switching are pending targeted acceptance coverage;
P0 strict 32K remains failing. See
`design/proposal-b-final-ten-searches-20261027-v480.md`.

## 2026-10-27 P0 v480: 32K DSH builtin does not execute

Reproduced lowmem ROOT -> DSH startup; the v478 `MEMSTAT` check
failed. New testkit v480 diagnostic first tested `DATE` and then
corrected a terminal-echo false positive: exact-line `ECHO V480OK`
produces no output although the shell returns a prompt. Failure is
upstream of MEMSTAT specifically; DSH parse/execute and available
heap remain suspects, not established causes. Preserved unrelated
uncommitted DSH work. No production edits or new RAM acceptance;
KCORE baseline **8960**, strict 32K P0 FAIL. See
`design/p0-32k-dsh-command-diagnostic-20261027-v480.md`.

## 2026-10-27 Proposal B v479: RENAME closed at verified boundary

RENAME correctness and current size reduction work complete. Retain
syscall/FILE/VFS/provider split: unproven removal risks blocking-I/O
ownership and has no measured RAM benefit. Revalidated v450, v468,
native v455/v461/v466: all PASS; earlier native full-system v471 PASS.
Actual PI6 resumption *inside* RENAME is an unverified future targeted
test, not a known defect. No production edit. Accepted KCORE **8960**,
KCORE+MRES 15507, PERMANENT_LAST 15554. Resume strict-32K P0 work.
See `design/rename-closure-20261027-v479.md`.

## 2026-10-27 v472: documented DSH 32K size-cut budget

Separated the unchanged DSH executable image gate (`036000` octal,
15360 words; v440 KCC image `035757`, 17-word headroom) from the
strict 32768-word *physical* boot requirement. Historical stripped
kernel + aligned shell estimate exceeded 32K by **210** words before
other costs; applying the old shell VM allocation to v466-v470 full
linked PERMANENT_LAST gives a **conditional**, non-current 3268-word
deficit. Historical 48K workload exceeded 32K by 8397 words, but is
not a minimum boot requirement. Exact current DSH cut requires matched
clean-image and live allocation measurements. No production changes;
KCORE baseline **8962**. See
`design/dsh-32k-size-cut-budget-20261027-v472.md`.

## 2026-10-27 P0 v474: strict 32K gate still failing; P2 paused

Reprioritized to strict P0; P2 optimization/profiling is paused.
Fresh default GCC PDP-6 lowmem KCORE **8960**, KCORE+MRES 15507,
PERMANENT_LAST 15554: **769** words must be removed to get strictly
below 8192. Explicit single disk/tape profiles KCORE **9008**, need
**817** words removed; footprint KINIT+02000-stack free 764/209 words.
Auto-root full 32K boot image fails `mkimage` stack reserve; explicit
single-disk boots to DCS LOGIN but ROOT reports LOGIN INCORRECT,
with DSH never reached. Root cause of login failure not established.
Strict 32K gate FAIL; P0 remains open, no production modifications.
See `design/p0-strict-32k-gates-20261027-v474.md`.

## 2026-10-27 Proposal B v473: CREATE fallthrough neutral, reverted

Removed an apparently redundant jump at the shared CREATE/SYMLINK
provider dispatch and adjusted layout guards temporarily. CREATE,
SYMLINK and native DTFS tests PASS, but the full linked GCC PDP-6
lowmem result was unchanged: KCORE **8960**, KCORE+MRES 15507,
PERMANENT_LAST 15554. Reverted both source and test changes; no RAM
reduction accepted. See
`design/vfs-create-fallthrough-size-experiment-20261027-v473.md`.

## 2026-10-27 Proposal B v472: VFS RENAME three-word frame

Removed redundant saved AC4 across two `vfs_readonly` calls; AC4 is
unchanged by the current helper. Native RENAME v455 and integrated
gateway v466 PASS; v450 namespace and new v472 AC4 contract guards
PASS. Full GCC PDP-6 lowmem KCORE **8960** (-2), KCORE+MRES 15507,
PERMANENT_LAST 15554. See `design/vfs-rename-ac4-stack-20261027-v472.md`.

## 2026-10-27 Proposal B v471: two native physical RENAME transactions

Full-system PDP-6 SIMH regression PASS: two separate user processes
create, rename, verify and remove their own `/CONFIG` D6FS files while
the remaining processes exercise PI6 scheduling and disk I/O. Does not
prove a context switch during RENAME. FILE/VFS/provider and filesystem
implementations are open to redesign when permanent RAM decreases and
ABI/filesystem semantics remain intact. No production edit; accepted
KCORE **8962**. See `design/rename-two-physical-processes-20261027-v471.md`.

## 2026-10-27 Proposal B v470: successful native UUO RENAME

Full-system PDP-6 SIMH multiprocess v470 regression PASS: slot 5
creates private `/CONFIG/R470A`, writes one 36-bit word, renames to
`R470B`, checks disappearance of source and content of destination,
unlinks test file, and completes the existing failed-RENAME and
disk-wait/preemption checks. It does not establish an actual scheduler
switch *within* RENAME. No production edit; KCORE **8962**.
See `design/rename-successful-uuo-multiprocess-20261027-v470.md`.

## 2026-10-27 Proposal B v469: native UUO RENAME with scheduler

Full-system PDP-6 multiprocess scheduler regression now invokes actual
monitor-UUO RENAME on nonexistent `/SYSTEM` names, verifies -1 and
continues with native disk I/O and PI6 scheduling. PASS. This is the
nonmutating error path, not a successful RENAME across provider sleep.
No production edit; KCORE stays **8962**. See
`design/rename-real-uuo-multiprocess-20261027-v469.md`.

## 2026-10-27 Proposal B v468: PI6 scheduling baseline validated

Full PDP-6 multiprocess scheduler regression PASS (preemption,
private process context, nice ABI, sleep/wakeup). New testkit v468
static guard PASS for RENAME syscall mapping lease, VM hold bit,
sleeping executive save/restore and provider transaction wait/retry.
The actual combined RENAME + cross-process PI6 sleep remains untested.
No production edits; accepted KCORE **8962**. See
`design/rename-pi6-scheduler-contract-20261027-v468.md`.

## 2026-10-27 Proposal B v467: native VM mapping hold bits

Testkit v467 executes actual PDP-6 VM mapping hold/release code;
PASS on set/clear, preservation of the full 36-bit control word,
preexisting hold, and boot/no-user-area path. Actual PI6 rescheduling
across provider wait remains untested; no production changes and
KCORE stays **8962**. See `design/mapping-hold-native-20261027-v467.md`.

## 2026-10-27 Proposal B v466: RENAME provider ABI regression fixed

Found a real regression in consolidated VFS RENAME: AC7 retained the
namespace XOR rather than the provider ID required by FS_MRES. Reload
AC7 after namespace comparison. Actual extracted provider gateway
and mapped FILE/VFS RENAME now PASS native PDP-6 wait/retry and error
tests, plus earlier v455/v461/v464 regressions. The fixture uses a
synchronous wait callback, not true scheduler context switching.
Mandatory correctness repair increases full linked KCORE to **8962**,
KCORE+MRES 15509, PERMANENT_LAST 15556. See
`design/rename-provider-ac7-correctness-20261027-v466.md`.

## 2026-10-27 Proposal B v465: gateway experiment not accepted

Tried to combine mapped FILE/VFS RENAME with the actual FS_MRES
provider serialization gateway and a deterministic lock wait.
Standalone PDP-6 fixture linked but failed its first RENAME assertion;
root cause undetermined. Exploratory test files discarded; no kernel
changes or RAM acceptance. Baseline KCORE **8961**. Investigate
gateway binding and lock wait before further cutover; see
`design/rename-real-provider-gateway-experiment-20261027-v465.md`.

## 2026-10-27 Proposal B v464: simulated provider interruption

Native integrated mapped FILE/VFS RENAME test extended with a provider
callback that simulates a sleep/wakeup register clobber. PASS: mapping
lease remains held during interruption, balanced on success and
failures. This is not actual scheduler wait/retry validation. No
production change; KCORE **8961**. See
`design/rename-mapping-simulated-wait-20261027-v464.md`.

## 2026-10-27 Proposal B v463: mapped FILE/VFS RENAME integration

Native PDP-6 test now joins actual two-path mapping helper, FILE
RENAME, VFS RENAME and recording provider. PASS on both mapping
failures, both parent and permission failures, provider failure and
success, checking exactly one hold/release and provider hold lifetime
for every invocation. Full monitor dispatch and sleeping provider
remain open. No production change; KCORE baseline **8961**.
See `design/rename-mapping-file-vfs-integrated-20261027-v463.md`.

## 2026-10-27 Proposal B v462: native dual-path mapping gate

Added native PDP-6 test for actual `native_sys_two_paths` helper.
PASS for valid dual mapping and failure of first or second pointer;
each acquires/releases exactly one VM mapping hold. Integration with
real FILE/VFS RENAME and sleeping provider remains outstanding.
No production edit; KCORE baseline **8961**. See
`design/rename-two-path-mapping-native-20261027-v462.md`.

## 2026-10-27 Proposal B v461: native FILE + VFS RENAME test

New `daimos-testkit` PDP-6 native test links real FILE RENAME with
real resident VFS RENAME and a recording provider. PASS: success,
both pathname failures, both permission failures and provider failure.
The syscall VM mapping lease and sleeping provider remain untested.
No production edit or size change: KCORE **8961**, KCORE+MRES 15508.
See `design/file-vfs-rename-integrated-native-20261027-v461.md`.

## 2026-10-26 Proposal B v460: RENAME integrated cutover gate

Mapped complete syscall -> FILE -> VFS -> provider ownership,
including the user mapping hold and sleeping I/O transaction lease.
An integrated native test must precede a coordinated removal of
existing wrappers; independent leaf tests are insufficient. No
production edit, no new size saving. Linked baseline KCORE 8961,
KCORE+MRES 15508, PERMANENT_LAST 15555. Strict 32K deferred.
See v460 design note.

## 2026-10-26 Proposal B v459: rejected XOR operand order

Full linked PDP-6 lowmem KCORE remained 8961 after reversing VFS
RENAME XOR operand order; reverted size-neutral production edit.
Baseline KCORE 8961, KCORE+MRES 15508, PERMANENT_LAST 15555.
Next pursue structural FILE/VFS pathname savings, not operand-order
churn. Strict 32K deferred. See v459 design note.

## 2026-10-26 Proposal B v458: rejected RENAME unwind experiment

Separated RENAME success and failure cleanup experimentally; full
linked lowmem KCORE rose 8961 -> 8963 (+2). Reverted production source.
No accepted memory change, baseline KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. See v458 design note. Strict 32K deferred.

## 2026-10-26 Proposal B v457: native FILE RENAME failure handling

Native PDP-6 production FILE RENAME assembly now passes independent
parent lookup, permission and provider-error injection in daimos-testkit;
native VFS and DTFS RENAME fixtures also PASS. Complete integrated
transaction remains outstanding. No production changes or new RAM
savings; linked v452 KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. Strict 32K deferred. See v457 design note.

## 2026-10-26 Proposal B v456: VFS RENAME failure coverage

Extended daimos-testkit native VFS RENAME fixture with readonly
source/destination mount rejection, writable control and provider
error propagation. Native VFS and DTFS RENAME tests PASS. Production
unchanged; full linked v452 baseline KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. Integrated FILE syscall transaction pending;
strict 32K deferred. See v456 design note.

## 2026-10-26 Proposal B v455: native VFS RENAME dispatch

New testkit native PDP-6 fixture links production `vfs.s` and tests
RENAME opcode 012, provider call on same namespace and suppression on
cross-provider/cross-mount cases. PASS. Full FILE/VFS integration and
readonly mount cases are still pending. No production change or RAM
saving; v452 linked KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. See v455 design note; strict 32K deferred.

## 2026-10-26 Proposal B v454: RENAME ownership coverage

Testkit now checks every provider/mount identity against twelve
single-bit namespace mismatches, plus the FILE RENAME two-parent
lookup, access and frame contract. Static, native PDP-6 FILE/walk
and DTFS RENAME provider tests PASS. No production change or RAM
saving; full linked v452 KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. Integrated FILE/VFS transaction pending;
strict 32K deferred. See v454 design note.

## 2026-10-26 Proposal B v453: DTFS RENAME native fixture

daimos-testkit's extracted DTFS RENAME fixture lacked the resident
`kconst_1_1` link constant. Supplied the production-equivalent word;
native PDP-6 provider RENAME regression PASS (ITS/TENEX,
destination-exists, cross-mount). No kernel modification or saving;
linked v452 baseline KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. See v453 design note. Full FILE/VFS RENAME
transaction coverage remains open, strict 32K deferred.

## 2026-10-26 Proposal B v452: masked-XOR RENAME namespace

Replaced separate masked vnode comparisons with masked XOR/TDNE.
Full linked lowmem KCORE **8961** (two words saved), KCORE+MRES
15508, PERMANENT_LAST 15555. No permanent state or ABI change.
Testkit v450/v452 namespace and PDP-6 FILE, pathname-walk and
provider-lock tests PASS. End-to-end RENAME transaction remains open;
strict 32K deferred. See
`design/vfs-rename-namespace-xor-20261026-v452.md`.

## 2026-10-26 Proposal B v451: RENAME namespace regression closure

Added pairwise collision-freedom coverage for all 4096 combined
provider/mount identities in `daimos-testkit`. Static namespace,
PDP-6 FILE and pathname-walk and native provider lock PASS. No
production code change; accepted full linked v450 KCORE 8963,
KCORE+MRES 15510, PERMANENT_LAST 15557 remain the baseline.
No new saving claimed. See
`design/vfs-rename-namespace-regression-20261026-v451.md`.

## 2026-10-26 Proposal B v450: compact RENAME namespace check

VFS RENAME compares combined provider/mount vnode namespace identity
under `0777700000000`. Full linked lowmem KCORE **8963**, KCORE+MRES
15510, PERMANENT_LAST 15557; three permanent code words saved with
no storage growth. Exhaustive 4096-identity test and focused FILE/PIPE,
UMASK and native provider-lock tests PASS. A smaller intermediate
mask was detected and corrected prior to acceptance. See
`design/vfs-rename-namespace-20261026-v450.md`.

## 2026-10-26 Proposal B v449: shared TRUNCATE/WRITE guard

Unified duplicated readonly, swapped-backing and provider-dispatch
logic for TRUNCATE and WRITE_WORDS. Full linked lowmem GCC PDP-6
KCORE **8966**, KCORE+MRES 15513, PERMANENT_LAST 15560, saving six
resident words. TRUNCATE now uses two additional transient stack
words, no permanent storage. Static ABI guard and native provider,
FILE/PIPE and VFS STAT tests PASS. Syscall end-to-end still open.
See `design/vfs-write-truncate-guard-20261026-v449.md`.

## 2026-10-26 v448: native SYMLINK provider dispatch regression

New testkit v448 PDP-6 executable drives resident VFS SYMLINK into a
recording provider. Native simulation PASS for opcode 010, unused
provider E=0, counted target and vnode publication. Full backing FS
symlink lifecycle is separate. No production edits or KCORE saving;
baseline **8972**. See `design/vfs-symlink-native-validation-20261026-v448.md`.

## 2026-10-26 Proposal B v447: shared CREATE/SYMLINK dispatch

Use PDP-6 conditional execution for the SYMLINK-only unused provider
argument E=0 instead of branching around it. Full linked GCC lowmem
KCORE **8972**, KCORE+MRES 15519, PERMANENT_LAST 15566; saving one
permanent word. Updated SYMLINK guard, DTFS CREATE and FILE/PIPE and
provider tests PASS. Native SYMLINK runtime test still pending.
See `design/vfs-symlink-provider-e-20261026-v447.md`.

## 2026-10-26 Proposal B v446: SYMLINK/CREATE policy cutover

VFS SYMLINK now shares `vfs_create_op` permission checking and vnode
publication, preserving the separate SYMLINK opcode and provider e=0.
Linked lowmem KCORE **8973**, KCORE+MRES 15520, PERMANENT_LAST 15567,
saving eight resident words without new permanent allocations.
FILE/PIPE, provider lock, DTFS CREATE and mapping guard PASS; full
SYMLINK runtime test still pending. Two earlier source experiments
were rejected or neutral; see
`design/vfs-symlink-create-policy-consolidation-20261026-v446.md`.

## 2026-10-26 Proposal B v445: CREATE/SYMLINK call consolidation

Shared physical provider call and vnode publication for CREATE and
SYMLINK. Linked lowmem KCORE 8981, KCORE+MRES 15528,
PERMANENT_LAST 15575; one KCORE word saved, no new data. FILE/PIPE,
UMASK, provider and common-dispatch checks PASS. See
`design/vfs-create-symlink-common-dispatch-20261026-v445.md`.

## 2026-10-26 v444: native DTFS UNLINK fixture repaired

The standalone extracted DTFS unlink routine had an unresolved shared
`kconst_2_2` stack constant. Testkit now supplies the same 2,,2
word as `core/ret.s`; `test-daimos-dtfs-unlink` PASS with native/ITS,
TENEX, corruption and rollback paths. No production change; KCORE
baseline **8982**. See `design/dtfs-unlink-testkit-link-contract-20261026-v444.md`.

## 2026-10-26 Proposal B v443: shared VFS mutation path

Removed duplicated read-only validation and provider dispatch across
two- and three-argument VFS mutations. Full linked KCORE 8982 (−5),
KCORE+MRES 15529, PERMANENT_LAST 15576. Native provider and FILE/PIPE
tests plus new mutation-frame guard PASS. The 2-argument path pays one
additional process-stack word, with no permanent storage growth.
See `design/vfs-mutation-dispatch-consolidation-20261026-v443.md`.

## 2026-10-26 Proposal B v442: direct-op cutover rejected

Shared FILE/VFS MKDIR/MKFIFO opcode cutover increased linked GCC PDP-6
lowmem KCORE 8987 -> 8990 (+3 words), KCORE+MRES 15534 -> 15537;
fully reverted. No change accepted. See
`design/file-vfs-direct-op-cutover-20261026-v442.md`.

## 2026-10-26 Proposal B v441: OPEN directory argument

Accepted one KCORE word reduction (full linked GCC PDP-6 lowmem
KCORE=8987, KCORE+MRES=15534, PERMANENT_LAST=15581) by deriving
`file_new_fd` directory flag from VFS stat type without an extra
initialization. Native FILE/PIPE direct-mount, UMASK and OPEN lifetime
checks PASS; new type projection guard PASS. DSH KCC gate remains
fixed separately by v440. See `design/open-directory-projection-20261026-v441.md`.

## 2026-10-26 v440: KCC DSH code-size fix

Unified fixed SIXBIT and input-line word clearing in one helper.
Clean KCC PDP-6 DSH image 036013 -> 035757 octal, saving 034 octal
(28 decimal) words, unchanged BSS 012751 and EXEC gate 036000.
The FIFO fixture explicitly selects KCC; the full PDP-6 FIFO runtime
regression PASS. No GCC substitute or limit increase. See
`design/dsh-kcc-clear-loop-size-fix-20261026-v440.md`.

## 2026-10-26 v439: DSH FIFO test size gate recovered

The v438 test's DSH 036013 > 036000 failure was due to a FIFO fixture
boot submake selecting KCC despite a GCC-built INIT. Testkit now uses
`BOOT_PROFILE=full COMPILER=gcc` and native PDP-6 FIFO integration
PASSES (MKFIFO, OPEN, rename, unlink, mount-busy, D6FS persistence).
This does **not** optimize KCC DSH or relax its size limit. KCORE
unchanged at 8988. See `design/dsh-fifo-size-gate-validation-20261026-v439.md`.

## 2026-10-26 Proposal B v438: FIFO OPEN completion

The FIFO success path reuses `file_open_success` instead of publishing
the descriptor twice. Full linked KCORE 8988 (one word saved),
KCORE+MRES 15535, PERMANENT_LAST 15582. No extra allocation; OPEN
lifetime guard PASS. FIFO integration build blocked by `DSH image
036013 exceeds EXEC limit 036000`; runtime result unknown. See
`design/open-fifo-result-unification-20261026-v438.md`.

## 2026-10-26 Proposal B v437: OPEN lifetime audit

Testkit v437 source guard PASS for OPEN's create/stat frame phases,
append-size lifetime, FIFO rollback and syscall VM mapping release.
No production changes or KCORE saving: baseline remains 8989 words.
See `design/open-frame-ownership-analysis-20261026-v437.md`.

## 2026-10-26 Proposal B v436 OPEN flag normalization

Accepted one KCORE word saving: GCC PDP-6 lowmem KCORE 8989,
KCORE+MRES 15536, PERMANENT_LAST 15583. New exhaustive 18-bit OPEN
flag semantics guard and EXEC ABI PASS. No FILE/VFS ownership redesign
yet. See `design/open-flag-register-abi-20261026-v436.md`.

## 2026-10-26 Proposal B v435: OPEN vertical-slice design

Recorded a new unified pathname-request ABI candidate joining syscall
mapping, FILE OPEN publication and VFS provider selection while
preserving distinct I/O buffer ownership. Requires atomic removal of
the replaced veneers and native success/error/rollback tests before
acceptance. No code change or KCORE saving yet. Baseline 8990.
See `design/file-vfs-syscall-unified-contract-20261026-v435.md`.

## 2026-10-26 Proposal B v434: linked KCORE architecture census

Map spans: VFS-to-bridge 823 words, FILE-to-PIPE 1050, native syscall
to getchar bridge 787, provider bridge 30. Spans are bounded address
intervals, not exclusive object size or potential savings. Next
candidate is unified FILE/VFS/syscall interface with measured linked
KCORE reduction. No production edit; KCORE 8990. See
`design/kcore-resident-layer-census-20261026-v434.md`.

## 2026-10-26 Proposal B v433: provider bridge bounded at 30 words

The complete `fs_provider_reg_call` to unlock region has 30 resident
instruction words, excluding four service bindings and lock state.
That ceiling rules out a large KCORE saving from this bridge alone.
No kernel change, baseline KCORE=8990. Continue architectural census
of larger FILE/VFS and syscall resident code. See
`design/provider-bridge-size-ceiling-20261026-v433.md`.

## 2026-10-26 Proposal B v432: 2 KCORE words saved

Direct MEMFS dispatch uses its normal missing-service trampoline,
eliminating a duplicated check. Full linked KCORE 8990, KCORE+MRES
15537, PERMANENT_LAST 15584. Native PDP-6 provider dispatch including
unbound MEMFS, lock and workspace interleaving PASS. See
`design/provider-memfs-unbound-dispatch-20261026-v432.md`.

## 2026-10-26 Proposal B v431: provider bridge saves 3 KCORE words

Accepted common missing-service call/unlock in `fs_provider_reg_call`.
Full linked GCC PDP-6 KCORE=8992, KCORE+MRES=15539,
PERMANENT_LAST=15586, all three below v430. PDP-6 native provider
lock/lease and missing-service unlock tests PASS. A faster fast-path
experiment increased KCORE by four words and was reverted; a test-
caught fall-through bug was fixed before acceptance. Details in
`design/provider-bridge-no-service-unification-20261026-v431.md`.

## 2026-10-26 Proposal B v430: TSFS READDIR projection

Accepted removal of a redundant TSFS child-record pointer copy.
Full linked image KCORE=8995 (unchanged), KCORE+MRES=15542,
PERMANENT_LAST=15589 (one MRES word saved). TSFS multiextent, D6LZ
and source-projection tests PASS. Provider locking unchanged; see
`design/tsfs-readdir-projection-20261026-v430.md`.

## 2026-10-26 Proposal B v429: TSFS value-record cost gate

New testkit source guard PASS establishes eight-word TSFS file records,
four-word extent records and buffer pointer consumers in parent,
READDIR and D6LZ paths. Copying entire records requires 8/4
additional per-call stack words, without inherently shortening
sleeping I/O ownership. No production change or saving accepted.
See `design/tsfs-value-record-lease-cost-20261026-v429.md`.

## 2026-10-26 Proposal B v428: native provider interleaving test

New daimos-testkit PDP-6 test PASS: a simulated wait preserves a
leased 128-word workspace while request registers are clobbered, then
the resident bridge restores arguments and dispatches after release.
TSFS record pointer returns constrain early unlock. No production edit
or size savings. See `design/provider-workspace-interleaving-20261026-v428.md`.

## 2026-10-26 Proposal B v427: live buffer across device sleeps

Traced D6FS backing read/write, TSFS table/word reads and DTFS DTC
veneer into shared `fs_block_workspace` use. Physical DSK/DRM/tape
drivers can wait; buffer ownership therefore must continue through
completion, consumption and cache publication. New testkit guard PASS.
No kernel edit or claimed RAM saving. See
`design/provider-io-lease-boundaries-20261026-v427.md`.

## 2026-10-26 Proposal B v426: provider lock and workspace baseline

Native PDP-6 provider lock wait/retry and static contract tests PASS.
New daimos-testkit inventory reports 60 source references to the shared
filesystem/BCACHE workspace across seven files. DSK/DRM/tape driver
waits are indirect from provider paths, so lock removal remains unsafe
without tracing I/O continuations and concurrency tests. No kernel
change or size saving; see
`design/provider-transaction-call-boundaries-20261026-v426.md`.

## 2026-10-26 Proposal B v425: provider transaction redesign scoped

Identified a previously unimplemented architectural candidate:
transaction-scoped ownership for the single 128-word filesystem/BCACHE
workspace, permitting eventual replacement of the resident provider
serialization/wait bridge only if all blocking lifetime constraints
can be proven. This is a source-grounded candidate, not a code change
or measured saving. See
`design/provider-transaction-ownership-redesign-20261026-v425.md`.

## 2026-10-26 Proposal B v424: FLOCK state update saves 3 words

The three FLOCK operations now select their final state directly from
`file_lock_modes`, including UNLOCK=REGULAR, eliminating a special
branch. Full GCC PDP-6 lowmem KCORE 8995, KCORE+MRES 15543,
PERMANENT_LAST 15590: three words below v423, 114 words below v388.
Native FILE/directmount tests PASS. Details in
`design/file-flock-mode-table-20261026-v424.md`.

## 2026-10-26 Proposal B v423: one-word FILE mount-unlock saving

Accepted `file_unlock_mount` redundant load elimination. Full-linked
PDP-6 GCC lowmem image KCORE 8998, KCORE+MRES 15546,
PERMANENT_LAST 15593 (one word below v417, 111 below v388). Native
FILE/directmount tests PASS, no new storage. See
`design/file-unlock-mount-live-word-20261026-v423.md`.

## 2026-10-26 Proposal B v422: disk/drum/tape root-login PASS

Full `test-daimos-root-login-lifecycle` passed in the PDP-6 simulator
with GCC userland and the v421 tape-root 15-minute response timeout.
All three root backends DSK/D6FS, DRM/D6FS and DTC/TSFS completed
invalid-login rejection, ROOT login, PWD, EXIT/respawn and relogin.
This resolves the previous incomplete TSFS `PWD` observation without
changing kernel source or the DSH size limit. See
`design/file-direct-mount-root-login-validation-20261026-v422.md`.

## 2026-10-26 Proposal B v420: D6FS provider post-cutover checks

Committed FILE v417 passes native PDP-6 D6FS multimount, remount and
direct/multi-DRM backing regressions. Validated independent mount
identities/readers, RW/RO publication and unmount cleanup, failure
atomicity, and missing-driver behavior. No production source or memory
size change; full root-login end-to-end acceptance remains separate.
See `design/file-direct-mount-provider-validation-20261026-v420.md`.

## 2026-10-26 Proposal B v419: RUN lifecycle acceptance

The committed v417 direct-mount FILE conversion passes full PDP-6
`test-daimos-process-lifecycle` and `test-daimos-run-late-rollback`
with supported GCC userland and unchanged DSH image limit. Validated
RUN/WAIT/zombie/orphan handling, explicit descriptor mappings and 16
late RUN mapping failures under three live children with successful
recovery. No production change or size difference from v417's
KCORE=8999, KCORE+MRES=15547, PERMANENT_LAST=15594. See
`design/file-direct-mount-lifecycle-validation-20261026-v419.md`.

## 2026-10-26 Proposal B v417: direct-mount FILE accepted

The atomic FILE format conversion stores all six mount bits directly
within each two-word descriptor, removes the extension-map machinery
and duplication-family token, and implements approved process-local
vnode-wide FLOCK. Full GCC PDP-6 lowmem link reduces KCORE from 9109
to 8999, KCORE+MRES 15657 to 15547, PERMANENT_LAST 15704 to 15594:
110 permanent words saved, without claiming a smaller process u-area.
Focused PDP-6 FILE/path/directmount/PIPE/UMASK tests PASS. Broad boot
and multiuser acceptance is still pending. See
`design/file-direct-mount-cutover-20261026-v417.md`.

## 2026-10-26 v418: DSH-size blocker fixed in PIPE regression

The pipe-wordfifo-v2 test inadvertently built a KCC DSH image 036013
against the unchanged 036000 production gate, although its target INIT
was compiled with GCC. Corrected the testkit boot submake to
`BOOT_PROFILE=full COMPILER=gcc`. Full-profile GCC DSH image=031313;
native PDP-6 PIPE word-FIFO regression PASS. No gate override, no KCC
code modification, and no change to strict 32K acceptance. See
`design/dsh-size-fixture-compiler-fix-20261026-v418.md`.

## 2026-10-26 Proposal B v400 decisions locked; next-chat handoff v401

Recorded the four approved v400 decisions in
`design/file-descriptor-mount-redesign-20261026-v400.md`:
(1) DUP/DUP2 and RUN descriptor remapping copy offsets independently;
(2) use minimal, demonstrably sufficient descriptor flags;
(3) boot/hardware/provider-dependent capacity must support at least 26
mounts when available, without preallocating 26 slots every boot, and
remain expandable; (4) retain existing locking and provider serialization
initially. Canonical full-vnode plus full 36-bit offset in two words is
the proposed B1 target. No implicit shared offsets, semantic relaxation,
or production change is authorized. Next chat: audit actual metadata
users, propose an exact no-growth encoding, quantify complete RAM/speed,
and prepare the full regression matrix **before seeking approval to edit
code**. Latest measured linked baseline: KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. See the design document for detailed handoff.

## 2026-10-26 Proposal B descriptor/mount design review v400

User-selected architecture: at least 26 mount slots with future expansion,
canonical-vnode two-word per-process descriptor (alternative B), retain
useful DUP and shared-offset capabilities. Discussion-first constraint:
no production edit without design agreement and an explicit functionality
and regression review. See `design/file-descriptor-mount-redesign-20261026-v400.md`
for the current code inventory, three candidate formats, offset-sharing
clarification, migration gates and open decisions. This is a design draft;
no production code changed, no new size or test acceptance claimed.
Latest measured baseline remains v388 KCORE 9109, KCORE+MRES 15657,
PERMANENT_LAST 15704.

## 2026-10-26 Cross-proposal architectural redesign policy v399

**Applies to the entire A–F KCORE architectural shrink roadmap, not just
Proposal B.** Existing interfaces, internal and public ABIs, descriptor
semantics, locking protocols, error/rollback rules and historical
compatibility requirements are **not immutable constraints**. The team
may rewrite producers, consumers, callers, filesystem providers,
userland and tests together when a simpler design has a demonstrable
memory/performance benefit. Do not reject a proposal solely because
its behavior or interface differs from the current implementation.

The invariants that **do** remain mandatory are:

- Correctness and explicit resource ownership: no corrupted state,
  leaks, deadlocks, use-after-free or half-published objects. Rollback
  may be redesigned or replaced by a simpler transaction protocol, but
  partial failure must leave a well-defined recoverable state.
- Keep the necessary capabilities available to the OS; alternatives
  need not preserve historical invocation or descriptor semantics.
  EXEC and RUN remain resident essential syscalls, notwithstanding
  permitted changes to their calling interface or internals.
- Optimize **actual physical RAM consumption first**, including KCORE,
  MRES, per-process memory and MM-managed allocations, rather than
  moving memory cost between accounting categories. Execution speed is
  nearly as important; disk size is secondary.
- No workaround-driven redesign or feature creep: fix underlying bugs,
  preserve a readable, documented implementation and measure the
  post-fold fully linked image as well as runtime memory costs.
- Before accepting changed semantics, define the *new* contract,
  migrate all affected call sites and userland, and add appropriate
  tests in `daimos-testkit`. Test both normal operation and concurrency,
  failure, resource cleanup, boot and process lifecycle as applicable.

**A:** process/wait/signal/session and teardown semantics may be
simplified together, but essential scheduling/ownership correctness
remains. **B:** FILE/VFS/MonitorFS APIs, descriptor representation,
DUP/DUP2 and offset semantics, mount policy, locking and rollback
may all change in a coordinated rewrite. Compare packed descriptors,
independent local descriptors and minimal shared handles, including
per-process storage and decode costs. **C:** EXEC/RUN validation,
startup and publication APIs may change, but essential resident
EXEC/RUN and sound replacement/creation semantics stay. **D:** MM
registry and allocation rules may be redesigned based on actual
high-water and fragmentation evidence. **E:** inactive-process
representation and swap interfaces are redesignable if correct state
restoration and total RAM benefit are demonstrated. **F:** scratch
workspace ownership/lock conventions may be rewritten after a proven
lifetime and reentrancy analysis.

This policy supersedes *compatibility-only* restrictions in older
proposal reviews, **not** their factual measurements or correctness
warnings. No production code or ABI was changed by this documentation
update. The latest accepted post-link baseline remains v388: KCORE
**9109**, KCORE+MRES **15657**, PERMANENT_LAST **15704**.

## 2026-10-26 Proposal B bounded structural review v389–v398 (ten passes)

User requested ten further review iterations after v388, except that an
unexpected large-saving opportunity should extend the effort. Ten
independent candidate boundaries were inspected in the current
`system/kernel/fs/vfs.s` and `file_runtime.s`, using the existing
A–F roadmap. **These are source-level review passes, not ten new
compiled patches or runtime acceptance results.** No proposed change
met a credible >=10–20 net resident-word saving threshold with a
proved unchanged ABI and lifetime, so no production edit was made.

1. **v389 LOOKUP namespace table:** both three-record namespaces now
   share loop-count initialization (v387); a further common loop would
   add branching without a demonstrated saving.
2. **v390 READDIR namespace merge:** built-in `/MONITOR` and mounted
   root offsets depend on different input traversal states and
   distinct stack frames. No safe common-entry reduction established.
3. **v391 PARENT_NAME:** MonitorFS device/PROCESSES/DOMAINS paths
   publish different counted names, while provider parent lookup has
   its own mount inheritance and ABI. Existing provider dispatch is
   shared where feasible (v386).
4. **v392 MOUNT slot search:** free-slot selection rejects existing
   mounts and uses MM-backed target/root tables; changing scan order
   risks duplicate target acceptance.
5. **v393 MOUNT publication:** target/root table entries and RO/SWAP/
   LOGSTORE owner policy have distinct publication requirements.
   No redundant full-sized resident table is present.
6. **v394 UNMOUNT retirement:** busy checks, sync, prepare-unmount,
   namespace-root substitution and descriptor unlocking precede
   shared slot retirement. Folding into publication would conflate
   failure and lifetime states.
7. **v395 CREATE/MKDIR/MKFIFO:** common `vfs_create_common` and
   `vfs_create_store_result` already share mode handling, provider
   result publication and error unwind; remaining differences are
   provider/operation policy.
8. **v396 SYMLINK/RENAME:** their readonly checks resemble CREATE,
   but arguments, namespace transitions, provider operations and
   stacked node-result publication differ. No verified net gain from
   a generic new frame helper.
9. **v397 bulk READ/WRITE:** common permission validation and
   provider arguments already exist (v376/v377); device dispatch,
   atomic APPEND and offset publication remain operation-specific.
10. **v398 FILE pathname wrappers:** common parent write-access check,
    one-argument cleanup and descriptor constructors are already in
    place. OPEN/FIFO error cleanup, RENAME two-parent validation and
    UNLINK/RMDIR type/busy checks have different ownership semantics.

**Conclusion:** none of these ten source reviews revealed a large
unimplemented consolidation supported by current evidence. The latest
measured linked baseline (v388) remains KCORE **9109**, KCORE+MRES
**15657**, PERMANENT_LAST **15704**; reaching KCORE <8192 needs
**918** further words. No new size build or test run is claimed,
because there is no production source change. Proposal C EXEC/RUN
is the next distinct roadmap phase. Proposal B can be closed after
any desired final dedicated regression/acceptance pass; avoid
continuing single-word instruction churn as a substitute for an
architectural change. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B direct READDIR namespace vnode comparison v388

Removed a redundant AC4 load before `CAME` in the resident
`vfs_readdir` `/MONITOR` special-case test: compare AC1 directly
against `monitorfs_names+7`, which already stores the canonical vnode.
The provider fallback, directory index bounds, special namespace
entries and ABI are unchanged; no persistent state or MRES movement.

Full linked GCC PDP-6 lowmem versus v387: KCORE **9110 -> 9109**
(-1), KCORE+MRES **15658 -> 15657** (-1), PERMANENT_LAST
**15705 -> 15704** (-1). All four native daimos-testkit checks
passed: `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
Global below-8192 goal still needs 918 words. Remaining Proposal B
work should prioritize larger FILE/VFS consolidation over repetitive
single-instruction changes; the original Proposal C addresses
EXEC/RUN separately.

## 2026-10-26 Proposal B VFS LOOKUP common namespace loop count v387

Both built-in MonitorFS namespace branches (`/` and `/MONITOR`)
search exactly three static four-word records. Moved their duplicated
`MOVEI 0,3` to `vfs_lookup_builtin_start`, where both paths converge;
all comparison fields, mount-root behavior and provider fallthrough
remain unchanged. No state, MRES or ABI changes.

Full linked GCC PDP-6 lowmem versus v386: KCORE **9111 -> 9110**
(-1), KCORE+MRES **15659 -> 15658** (-1), PERMANENT_LAST
**15706 -> 15705** (-1). All four daimos-testkit checks pass:
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
The global below-8192 goal still requires 919 words. The
remaining B work needs larger verified consolidation, not repeated
one-word tuning; Proposal C remains the separately defined EXEC/RUN
phase of the original A–F architectural roadmap.

## 2026-10-26 Proposal B VFS PARENT_NAME provider selection v386

Revisited the earlier invalid-assembler trial without retaining it.
The valid PDP-6 `CAIE 7,2` / `CAIN 7,3` skip pair replaces two
independent equal comparisons and jumps in the resident
`vfs_parent_name_dispatch`. Exactly providers 2 and 3 enter
`monitorfs_parent_name`; other providers continue to the existing
FS_MRES_OP_PARENT_NAME path. Both mount-root resolution and the
provider ABI stay unchanged. The rejected experimental `CAINE`
mnemonic is not used anywhere in this accepted edit.

Full linked GCC PDP-6 lowmem versus v385: KCORE **9112 -> 9111**
(-1), KCORE+MRES **15660 -> 15659** (-1), PERMANENT_LAST
**15707 -> 15706** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. The global below-8192 KCORE goal still needs 920 words;
Proposal B remains distinct from original Proposal C EXEC/RUN.

## 2026-10-26 Proposal B VFS READDIR builtin bounds branch v385

Reconfirmed the original 2026-10-24 A–F architectural roadmap:
Proposal B covers resident FILE/VFS/MonitorFS consolidation, while
Proposal C is distinct EXEC/RUN consolidation, preserving their
resident, distinct publication/replacement transactions. Therefore
v385 changes only resident VFS namespace READDIR, not EXEC/RUN.

In `vfs_readdir`, the post-provider namespace insertion index is
rejected if negative or at least three. Replaced `CAIGE 4,3; JRST
vfs_readdir_builtin; JRST vfs_readdir_zero` with `CAIL 4,3; JRST
vfs_readdir_zero`, falling through for valid indices 0..2. The
negative index rejection is unchanged; returned dirent records and
mount-root traversal are unchanged. No MRES, ABI or permanent state
changes.

Full linked GCC PDP-6 lowmem versus v384: KCORE **9113 -> 9112**
(-1), KCORE+MRES **15661 -> 15660** (-1), PERMANENT_LAST
**15708 -> 15707** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE target <8192 still requires 921 words; completion
of Proposal B is assessed by worthwhile structural savings rather
than that independent global target. Proposal C remains separate.

## 2026-10-26 Proposal B resident storage overlay candidate scan v384

Reviewed large explicit resident `.block` declarations, filesystem
workspace call sites, BCACHE slot ownership, MEMFS snapshot entry
points and MonitorFS swapped-process read dispatch. The only obvious
fixed 0200-octal transfer block is `fs_block_workspace` in `vfs.s`;
`bcache_pdp6.s` already overlays slot 0 there while slot 1 is a
**separately allocated 0201-word MM slab** that is needed to preserve
an old clean workspace block across cache misses. The second cache
slot is not an unused permanent block and cannot serve as the VFS
transfer destination without invalidating the cache's recovery model.

`memfs_runtime.s` calls `memfs_snapshot_mount` and
`memfs_snapshot_shutdown`; `monitorfs_runtime.s` dispatches
`mfsproc_swapped_read_words_v63`, which invokes `backstore_read`
with `fs_block_workspace` and invalidates the BCACHE workspace tag
first. These entry paths cannot be assumed to share the physical
provider's DTFS/TSFS/D6FS serialization merely because they operate
on the same scratch address. The audit did **not** establish a
mutually exclusive 128-word replacement allocation or safe buffer
lifetime, so **no storage overlay or memory saving is accepted**.
No code change was made; the latest measured complete linked image
remains KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST
**15708**. Next prioritize structural code savings with demonstrable
linked impact, or first prove an exact lock/call graph for the entire
workspace lifetime. Below-8192 KCORE still requires 922 words;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B BCACHE/workspace overlay audit v383

Continued the fixed 128-word `fs_block_workspace` lifetime analysis.
`system/kernel/storage/bcache_pdp6.s` **already aliases** BCACHE
slot 0 to that exact VFS scratch region. Its shared slot-0 tag takes
one fixed resident word. The separate MM-managed slot 1 is a
`0201`-word allocation (128 data plus tag), made lazily with
`mm_alloc_aligned_noreclaim`, not a second permanent KCORE buffer.
On workspace-backed cache misses, slot 0's live clean block is copied
to slot 1 before the workspace is overwritten. Reusing the MM slab as
scratch concurrently would destroy that protection; removing the
resident scratch and retaining another permanently allocated 128-word
buffer would merely relocate the RAM cost. The current sharing
already captures the obvious BCACHE overlay opportunity.

The MonitorFS swapped-process read explicitly invalidates the shared
workspace tag before `backstore_read` and uses a per-call stack source
record. The source comments say that swapping and scheduling preserve
executive syscall completion, but this does not prove safe arbitrary
overlap with every other filesystem/snapshot call. Physical provider
serialization through `fs_provider_reg_call` remains an independent
lifetime guarantee; MEMFS and snapshot paths require separate review.

No code change is accepted in v383. Linked v381/v382 baseline remains
KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST **15708**;
no additional size saving or runtime test claim. Before a storage
rewrite, require a complete call graph including boot-before-MM,
reentrant provider callbacks, MonitorFS swapped reads and snapshot
mount/shutdown. Search for a *different* existing mutually exclusive
allocation rather than overlaying BCACHE's required backup slot.
Strict 32K acceptance remains deferred, and 922 additional KCORE
words are required for the below-8192 target.

## 2026-10-26 Proposal B shared filesystem workspace lifetime audit v382

Inspected the 0200-octal (128-decimal)-word `.bss` `fs_block_workspace`
owned by resident `vfs.s`. Direct references extend beyond D6FS and
DTFS: TSFS, MEMFS snapshot, MonitorFS swap, the `fs_mres` zero-block
bridge and block-cache operations also depend on this fixed address.
`fs_provider_reg_call` serializes physical providers DTFS, D6FS and
TSFS across their full requests: those requests may sleep, and the
provider lock prevents cross-provider workspace corruption. The
MEMFS fast path deliberately bypasses this lock because it does not
sleep. The MonitorFS swap reader explicitly invalidates the shared
block-cache workspace tag before loading a swap block and keeps a
cached-block key local to its source.

Consequently an unconditional MM relocation is **not accepted**:
fixed-address references across assembler/C, calls in potentially
different contexts, and synchronization/caching assumptions must
first be audited. In particular we have not proved that every
MonitorFS swap or snapshot access is covered by provider locking, or
that any MM block can be released with no persistent replacement.
Merely moving the 128 words outside KCORE would not establish a net
permanent-RAM saving. No source changed and no size reduction claimed;
linked baseline remains KCORE **9113**, KCORE+MRES **15661**,
PERMANENT_LAST **15708**, as measured in v381. Next investigation:
map each user and caller to the lock domain, establish boot-before-MM
requirements, then identify an *existing* mutually exclusive 128-word
area before attempting a zero-extra-memory overlay. Strict 32K
acceptance stays deferred.

## 2026-10-26 Proposal B VFS dispatch / resident transfer workspace audit v381

Tested removal of the apparent redundant JRST immediately before
`vfs_read_words_provider` in resident `vfs.s`; full linked GCC PDP-6
lowmem image remained **KCORE 9113**, **KCORE+MRES 15661**,
**PERMANENT_LAST 15708**. Reverted the size-neutral experiment.
No accepted production code modification or new runtime acceptance.

Resident VFS also contains `fs_block_workspace`, a 0200-octal
(128-decimal)-word BSS transfer buffer, documented as shared between
filesystem providers. This is potentially much more significant than
single-instruction edits, but no reassignment is safe until all
provider references, boot-before-MM use, locking/sleep lifetimes,
concurrency and MM allocation policies are proven. In particular,
replacing it with another permanent 128-word reservation would not
improve total permanent RAM; lowering KCORE alone is not proof of a
net resident saving. Analyze provider calls and lifetimes before
editing, and retain the documented ABI. Strict 32K acceptance stays
deferred. Baseline KCORE requires 922 fewer words to reach 8191.

## 2026-10-26 Proposal B VFS READ_WORDS provider decode audit v380

Reviewed the resident READ_WORDS VFS dispatch and its fast paths for
PIPE and MonitorFS process data. After the packed provider/kind check,
experimentally replaced the explicit halfword shift that extracts the
provider from AC7 with `LDB 7,[POINT 6,1,5]`. The complete linked
GCC PDP-6 lowmem measurement was unchanged: KCORE **9113**,
KCORE+MRES **15661**, PERMANENT_LAST **15708** words. The size-neutral
change was reverted; no production code or MRES source changed.
These paths remain candidates only for larger semantic-preserving
consolidations, not ineffective register decode rewriting. No new
runtime acceptance is claimed. KCORE strictly below 8192 needs 922
additional words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE character I/O register-move audit v379

Examined resident `file_readchar` and `file_writechar` for redundant
AC1 descriptor-pointer reloads immediately after saving the same
pointer in AC10 and before `file_node_from_fd`. Removed one source
MOVE from each call path experimentally. The complete linked GCC
PDP-6 lowmem measurement remained unchanged at KCORE **9113**,
KCORE+MRES **15661** and PERMANENT_LAST **15708** words, equal to
v378. This strongly indicates existing assembly folding already
removes the moves. Reverted the size-neutral source experiment;
there is no accepted runtime modification or new test acceptance.
The KCORE below-8192 target still needs 922 words. Future work
should concentrate on demonstrably larger shared request and VFS
blocks, not textual edits already folded by the toolchain. Strict
32K acceptance remains deferred.

## 2026-10-26 Proposal B remove dead bulk FILE offset setup v378

In resident `file_runtime.s`, the shared READ/WRITE bulk entry had
loaded descriptor offset into AC2 and redundantly moved the descriptor
pointer from AC1 back into AC1 just before calling `file_node_from_fd`.
Inspection of the helper confirms that it requires only AC1 (already
holding the descriptor pointer), preserves AC2, and returns the vnode
in AC1. The subsequent shared request setup overwrites AC2 with the
transfer length; the descriptor offset is loaded at the VFS provider
boundary where needed. Both dead moves were removed. No provider
semantics, public ABI, or MRES source changes.

Full linked GCC PDP-6 lowmem against v377: KCORE **9115 -> 9113**
(-2), KCORE+MRES **15663 -> 15661** (-2), PERMANENT_LAST
**15710 -> 15708** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new concurrent physical
I/O acceptance is claimed. Below-8192 KCORE still requires 922
further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B shared bulk VFS provider arguments v377

Resident `file_runtime.s` bulk READ and WRITE previously duplicated
four register moves to prepare fallback VFS provider requests:
AC3 buffer, AC4 count, AC1 vnode, AC2 descriptor offset. A common
`file_words_provider_common_v377` block now prepares these once;
WRITE reaches it by fallthrough, READ jumps to it. The signed
READ/WRITE selector in AC5 then dispatches READ directly to
`vfs_read_words` while WRITE retains the original APPEND provider
EOF-lock decision and `vfs_write_words` path. Direct physical-device
shortcuts and shared descriptor-offset handling remain unchanged.
No ABI, MRES code or permanent state is added.

Full linked GCC PDP-6 lowmem vs v376: KCORE **9117 -> 9115**
(-2), KCORE+MRES **15665 -> 15663** (-2), PERMANENT_LAST
**15712 -> 15710** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` all PASS. Physical-provider
concurrency/APPEND end-to-end acceptance remains separate.
KCORE below 8192 needs 924 further words; strict 32K acceptance
remains deferred.

## 2026-10-26 Proposal B shared bulk READ/WRITE argument save v376

In resident `system/kernel/fs/file_runtime.s`, `file_read_words` and
`file_write_words` duplicated three entry PUSH instructions saving
AC10, AC2 buffer and AC3 transfer length. Moved those three identical
PUSHes into common `file_words_save_args_v376`, after the distinct
READ/WRITE permission selector in AC5 is initialized. READ branches
to the shared prologue and WRITE falls through into it. Both enter
`file_words_common` with identical stack layout, original AC1 fd,
arguments, and selector values; the existing result cleanup and
provider-side append semantics remain untouched. No MRES change.

Full linked GCC PDP-6 lowmem versus v375: KCORE 9120 -> **9117**
(-3 words), KCORE+MRES 15668 -> **15665** (-3), PERMANENT_LAST
15715 -> **15712** (-3). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new physical-device
concurrency acceptance is claimed. The below-8192 KCORE goal requires
926 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE OPEN directory flag branch v375

In resident `file_open`, replace the conditional branch around
`MOVEI 3,1` (directory argument to `file_new_fd`) with a conditional
`CAIN 1,1` followed by `MOVEI 3,1`: CAIN executes the MOVEI only
for `VFS_TYPE_DIR=1`, leaving zero for every other type. The first
experiment incorrectly used CAIE, which inverted the condition;
it was caught in review and immediately corrected **before testing
or committing**. Only the correct CAIN version is retained.

Full linked GCC PDP-6 lowmem against accepted v374: KCORE
9121 -> **9120** (-1), KCORE+MRES 15669 -> **15668** (-1),
PERMANENT_LAST 15716 -> **15715** (-1). daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS on the corrected source. Public FILE ABI, directory semantics
and provider behavior unchanged. No MRES code modified. KCORE below
8192 needs 929 more words; strict 32K acceptance deferred.

## 2026-10-26 Proposal B FILE OPEN access-mask consolidation v374

Consolidated the `file_open` READ/WRITE/TRUNC permission-mask builder
in resident `system/kernel/fs/file_runtime.s`. The prior branch tree
set bit 4 if READ and bit 2 if WRITE or TRUNC. Replaced it with two
conditional `TRNE` / `TRO` pairs, checking combined flag mask 022
for WRITE|TRUNC. The resulting mask for all 8 combinations of these
three input bits is identical, as are the subsequent STAT record
access check, CREATE path and failure handling. No public ABI, extra
persistent state or MRES modification.

Full linked GCC PDP-6 lowmem versus v373: KCORE 9125 -> **9121**
(-4 words), KCORE+MRES 15673 -> **15669** (-4), PERMANENT_LAST
15720 -> **15716** (-4). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. Full Open flag/error matrix
and physical-provider concurrency have not been separately accepted;
strict 32K acceptance remains deferred. Below-8192 KCORE requires
930 additional words. Continue resident file-runtime and VFS
structural analysis, prioritizing persistent RAM over speed.

## 2026-10-26 Proposal B KCORE FILE OPEN parent permission reuse v373

The resident FILE OPEN create-on-miss branch performed its parent
write/search access check with an inline `MOVEI 2,3` followed by
`PUSHJ file_check_access`, duplicating the contract already
centralized for SYMLINK/RENAME/MKDIR/UNLINK in the existing
`file_parent_write_access_v367` tail-call helper. Reused that helper
in `file_open` with unchanged parent vnode, permission mask and
failure semantics. No MRES changes, new state or new public ABI.

Full linked GCC PDP-6 lowmem versus v372: KCORE 9126 -> **9125**
(-1), KCORE+MRES 15674 -> **15673** (-1), PERMANENT_LAST 15721 ->
**15720** (-1). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. KCORE below 8192 requires
934 additional words. Continue larger structural FILE/VFS analysis;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE MOUNT vnode slot encoding v372

In the resident `vfs_mount_found` path, the public mount ID is the
validated zero-based slot plus one. Replaced `MOVE 6,7` followed by
`ADDI 6,1` with `MOVEI 6,1(7)` before the unchanged mount/kind
field shifts and bitwise packing. No state storage, ABI, root vnode
bit layout or provider interface changed.

Full linked GCC PDP-6 lowmem relative to v371: KCORE 9127 ->
**9126** (-1), KCORE+MRES 15675 -> **15674** (-1),
PERMANENT_LAST 15722 -> **15721** (-1). `daimos-testkit` targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
PASS. Multi-provider concurrent MOUNT/UNMOUNT acceptance remains
separate; strict 32K acceptance deferred. KCORE below 8192 requires
935 further words. Larger resident structural consolidations remain
the focus of Proposal B.

## 2026-10-26 Proposal B KCORE UNMOUNT mount-ID preparation v371

After restoring DevSpace2 connectivity, inspected the v370 checkout
and optimized two zero-based-slot-to-public-mount-ID preparations in
resident `system/kernel/fs/vfs.s` `vfs_unmount`:
`MOVE 4,2` + `ADDI 4,1` in the protected owner checks, and
`MOVE 1,2` + `ADDI 1,1` before `file_unlock_mount`, each replaced
by one PDP-6 indexed `MOVEI dest,1(2)`. AC2 remains the source slot,
with unchanged public owner tokens, error order and unmount lock
release. No new state and no MRES modifications.

Complete linked GCC lowmem image versus v370: KCORE 9129 ->
**9127** (-2), KCORE+MRES 15677 -> **15675** (-2),
PERMANENT_LAST 15724 -> **15722** (-2). Native daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. No concurrent physical-provider unmount acceptance is
claimed. Another 936 words are needed to reach below 8192; strict
32K acceptance remains deferred. Larger structural VFS/FILE work
remains the main Proposal B priority.

## 2026-10-26 Proposal B VFS unmount provider-field decode v370

In `system/kernel/fs/vfs.s`, after `vfs_sync` and before deciding
whether a provider requires PREPARE_UNMOUNT, replaced `MOVE 3,1` plus
`LSH 3,-036` with the direct `LDB 3,[POINT 6,1,5]` provider-field
extraction already used elsewhere in resident VFS. The existing
MEMFS/D6FS prepare-only dispatch and unmount completion order remain
unchanged; no MRES code or interface was modified.

Full linked GCC lowmem image against v369: KCORE 9130 -> **9129**,
KCORE+MRES 15678 -> **15677**, PERMANENT_LAST 15725 -> **15724**
(all -1 word). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. The test scope does not include
full multi-provider concurrent physical unmount acceptance. KCORE
strictly below 8192 still needs 938 words. Strict 32K boot acceptance
is deferred; further Proposal B work remains on resident MOUNT/UNMOUNT,
FILE OPEN and provider-boundary restructuring.

## 2026-10-26 Proposal B KCORE structural size census v369

Resumed KCORE-only optimization after v368. Inspected the live
`file_runtime.s` and `vfs.s` source and target DOBJ symbol sections,
using the current v368 GCC lowmem linked baseline. No MRES code,
package metadata, or exports were considered optimization targets.

**Pre-link section census, not a claim of additive linked savings:**
`file_runtime.dobj`: text 1127 words, data 0, BSS 0;
`vfs.dobj`: text 770, data 27, BSS 128. The VFS mount
base pointers and active slot-count form three data words. Several
other VFS counters/policies already alias otherwise unused resident
device-state words, and mount target/root tables are MM-backed;
blindly moving them or adding a second allocation would increase
permanent RAM or break boot publication. Thus a simple mount-table
relocation is not a justified candidate.

DOBJ symbol-entry spans (a useful hotspot ranking, **not** exact
function sizes because local routines and helper code may be included):

- FILE: `file_open` 116; `file_write_words` 117 (includes common bulk
  READ/WRITE dispatch); `file_getpath` 93; `file_lock` 79;
  `file_component` 51; `file_dup2` 43; `file_readchar` 43;
  `file_writechar` 43 words.
- VFS: `vfs_mount_prevalidated` 101 (includes mount setup),
  `vfs_readdir` 91; `vfs_unmount` 82; `vfs_parent_name` 81;
  `vfs_lookup` 64; `vfs_create_op` 41 words.

**Next investigation order:** (1) compare VFS MOUNT/UNMOUNT
request and publication phases, proving lifetime and error unwind
semantics before sharing any state; (2) review FILE OPEN and bulk
READ/WRITE dispatch for large mutually exclusive pathways without
changing device ABI, EOF/APPEND handling or offset publication;
(3) check VFS READDIR/PARENT_NAME for duplicated namespace-name
handling while preserving mount-root traversal. Retain the already
MM-backed mount arrays and serialized physical-provider gateway.
Build-native tests in `daimos-testkit`, including concurrency and
error paths, remain required for any accepted change. No production
edit or new saving is claimed in this census: full linked KCORE
remains **9130** words; below 8192 requires 939 more. Strict 32K
acceptance remains deferred.

## 2026-10-26 Proposal B parent/leaf frame helper experiment v368

Inspected resident parent/leaf request setup in `file_runtime.s` for
SYMLINK, MKDIR/MKFIFO, RENAME and UNLINK/RMDIR. Tested a new tail-call
helper deriving the contiguous leaf output pointer from the directory
output pointer, replacing duplicated `MOVEI AC3` setup at two call
sites. A full linked GCC lowmem image measured KCORE **9130**, all
MRES combined **15678**, PERMANENT_LAST **15725**, equal to v367.
The helper saved no RAM and was reverted immediately; no change to
filesystem semantics or production source is retained, and no new
runtime test is claimed for this rejected candidate. Continue
Proposal B with larger resident file-runtime/VFS request handling;
MRES optimization and strict 32K acceptance remain deferred.

## 2026-10-26 Proposal B KCORE parent-mutation access helper v367

Consolidated the repeated parent write-plus-search permission call
in resident `system/kernel/fs/file_runtime.s`. `file_symlink`,
`file_rename` (both parent directories), `file_make_node` and
`file_remove` now invoke the small internal
`file_parent_write_access_v367` leaf; it loads access mask 3 and
uses a tail JRST to `file_check_access` so the original caller's
return address, arguments and error semantics are preserved.
The common helper has no persistent state and no MRES changes.

Complete GCC PDP-6 lowmem linked image versus v366: KCORE
9133 -> **9130** (-3 words), KCORE+all MRES 15681 -> **15678**
(-3), PERMANENT_LAST 15728 -> **15725** (-3). Native
`daimos-testkit` targets `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This does not demonstrate
all failure branches of RENAME/SYMLINK or physical-provider
concurrency. Below-8192 KCORE target requires 939 additional
words. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE component clear BLT audit v366

Audited the resident `file_component` five-word output `vfs_name`
zero-initialization and its PDP-6 `BLT` source/destination pointer
setup. Experiment replaced `HRLI 0,(3)` with `HRL 0,3` after loading
`1(3)` into AC0; it compiles, but the complete linked GCC lowmem
image is unchanged: KCORE **9133**, KCORE+all MRES **15681**,
PERMANENT_LAST **15728** words. The experiment was reverted rather
than retain a size-neutral rephrasing. No MRES source changes and no
new runtime acceptance are claimed. Follow-on Proposal B work should
favor large resident file-runtime/VFS structural consolidation over
more zero-gain instruction substitutions. The below-8192 KCORE target
requires 942 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE indexed path byte-pointer offset v365

In `system/kernel/fs/file_runtime.s`, `file_component` constructs
its packed-SIXBIT input pointer after dividing the current character
position by six. Replaced `ADD 6,1` / `ADDI 6,1` with indexed
`ADDI 6,1(1)` to include both the path address and the one-word
counted-path header offset in one PDP-6 effective-address operation.
Both path pointers lie within the native lowmem address range; the
byte position remainder, SIXBIT pointer LH and path semantics remain
unchanged. No MRES packaging, exports or runtime was changed.

Full linked GCC lowmem KCORE 9134 -> **9133** (-1), all-MRES
combined size 15682 -> **15681** (-1), PERMANENT_LAST 15729 ->
**15728** (-1). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. A further 942 words are
needed to get strictly below 8192. Strict 32K acceptance deferred.

## 2026-10-26 Proposal B KCORE output SIXBIT pointer setup v364

`file_component` initialized its output SIXBIT byte pointer through a
literal pointer MOVE, a separate effective destination address load,
and HRR register merge. Replaced that three-instruction sequence with
`MOVEI 1,1(3)` followed by `HLL 1,[POINT 6,0]`, preserving the
right-half address and installing the same left-half byte-pointer
format. Source record parsing, component count, trailing separator
consumption and public VFS name semantics are unchanged. No MRES
source or export metadata was changed.

Full linked GCC PDP-6 lowmem KCORE 9135 -> **9134** (-1), combined
KCORE+all MRES 15683 -> **15682** (-1), PERMANENT_LAST 15730 ->
**15729** (-1). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is an accepted resident
KCORE-only size reduction. KCORE below 8192 requires 943 additional
words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE SIXBIT path pointer construction v363

In the resident `file_component` implementation, after `DIVI` and
addition of the path-data base, AC6 already contains the desired
right-half address for the input SIXBIT byte pointer. Replaced the
three-instruction MOVe literal / HRR / MOVe sequence with one
`HLL 6,[POINT 6,0]`, installing the byte-pointer left-half while
preserving the computed right-half address. No change to byte offset,
separator handling, component-size limit or caller ABI; no MRES work.

Complete GCC PDP-6 lowmem linked image compared with v362:
KCORE 9137 -> **9135** (-2), combined KCORE+MRES 15685 ->
**15683** (-2), PERMANENT_LAST 15732 -> **15730** (-2).
`daimos-testkit` `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE below 8192 requires 944 further words.
Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE file_getpath index multiply v362

The resident absolute-path formatter `file_getpath_common` uses a
five-word component array of up to sixteen path parts. Both the
upward parent-walk and reverse output walk computed `5 * depth` with
`LSH 4,2` plus `ADD 4,depth`. Replaced each two-word sequence with
single `IMULI 4,5`, preserving AC4, stack record indexing, operation
order, public ABI and pathname semantics. No MRES source was changed.

Full linked GCC PDP-6 lowmem baseline v361: KCORE 9139 -> **9137**
(-2 words), KCORE+all MRES 15687 -> **15685**, PERMANENT_LAST
15734 -> **15732**. daimos-testkit PDP-6 targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS. These are general file/VFS regression tests; a dedicated
native deep GETPATH/GETCWD path-component matrix has not yet been
run, and broader acceptance should include one. The reduction
is entirely resident KCORE. To get below 8192 from 9137 requires
946 additional words; strict 32K acceptance stays deferred.

## 2026-10-26 Proposal B shared descriptor clone v361

Consolidated duplicated resident `file_dup`/`file_dup2` descriptor
cloning in `system/kernel/fs/file_runtime.s` into the internal
`file_clone_descriptor_v361` leaf. The shared sequence copies both
contiguous FILE words, copies the descriptor mount-extension field,
reconstructs the canonical vnode and increments pipe references only
for the PIPE provider. `file_dup` preserves its returned fd across the
helper; `file_dup2` retains close-before-overwrite, equality, and error
semantics. Public FILE ABI and MRES sources remain untouched.

Full linked GCC PDP-6 lowmem disk-root measurement against v360:
KCORE **9148 -> 9139** (-9 words); KCORE+all MRES
15696 -> 15687 (-9); PERMANENT_LAST 15743 -> 15734 (-9).
`daimos-testkit` PDP-6 targets `target-file-pdp6`,
`target-file-walk-pdp6`, and checks `test-daimos-vfs-stat-frame`,
`test-daimos-pre-dsh-fs-closure` all PASS. Targeted native extracted
FILE mount-extension contract `daimos-file-mountext-v19.sh` PASS,
covering DUP/DUP2, 26-mount vnode extensions, locks, close and unmount.
This does not replace full concurrent-provider stress testing. KCORE
below 8192 needs another 948 words from the new baseline. Strict
32K acceptance remains deferred; no MRES optimization was performed.

## 2026-10-26 Proposal B shared bulk-I/O register preparation v360

Reduced permanent KCORE by three words in
`system/kernel/fs/file_runtime.s`. Resident `file_words_common` used
to build identical register arguments separately for READ and WRITE:
AC0 canonical vnode, AC1 user buffer, AC2 word count. These three
moves now execute once before the existing READ/WRITE branch. Device
selection, provider register ABI, append-at-EOF handling, lock and
sleep semantics, and descriptor offset updates remain unchanged.

Full linked GCC lowmem disk-root image versus v359: KCORE
9151 -> **9148**, KCORE+all MRES 15699 -> **15696**, and
PERMANENT_LAST 15746 -> **15743**. All MRES objects are unchanged;
the saving is entirely resident KCORE. PDP-6 `daimos-testkit`
`target-file-pdp6` and `target-file-walk-pdp6` PASS, along with
`test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is targeted regression
coverage, not a full physical-provider concurrency stress test.
The next KCORE milestone below 8192 requires 957 further words.
Strict 32K acceptance remains deferred. No MRES size optimization
is part of this change.

## 2026-10-26 Proposal B KCORE-only file-runtime entry audit v359

Resumed Proposal B strictly on KCORE (`file_runtime.s`, its VFS
boundary); no MRES packaging or metadata work. Inspected resident
readchar/writechar/readdir descriptor-to-vnode setup and the path,
permission and create/remove request frames. Three places redundantly
copied AC1 from a saved descriptor register back into AC1 immediately
after saving it. Tested removing one such move (v358) and then all
three (v359) against the complete GCC lowmem linked image, using
the current mandatory MonitorFS MRES build configuration. Both
experiments were **size neutral**: KCORE 9151, KCORE+MRES 15699,
PERMANENT_LAST 15746. Reverted all changes, including the three-move
experiment. No runtime test is claimed for rejected experiments.

This verifies that source-level micro-deletions are insufficient after
current assembler/link folding. Continue with actual structural
file-runtime/VFS opportunities: eliminate substantial duplicated
request preparation or persistent state only with ownership/lifetime
proofs. Keep provider sleeping/serialization contract intact. MonitorFS
MRES/export size work is explicitly out of scope in this KCORE phase.
KCORE goal remains below 8192 words; strict 32K acceptance deferred.

## 2026-10-25 MonitorFS MRES linked-overhead audit v357

Audited the always-loaded MonitorFS ABI trampoline file,
`mfsdev_install_minit` in `module_minit.c`, VFS process/domain name
callers, nine export declarations in the boot MRES link, and the
relocated `monitorfs-mres.map`. Nine fixed KCORE jump words correspond
to distinct ABI services, including `mfsproc_format_slot` called by
resident VFS; nine MRES exports have non-contiguous code offsets. Thus
neither dropping the last formatter entry nor assuming contiguous
function addresses is ABI-safe. A staged C simplification caching
`SIXBIT("MFS   ")` in a local name variable was compiled and measured
against the **complete linked lowmem image**; figures remained exactly
KCORE 9151, KCORE+MRES 15699, PERMANENT_LAST 15746. Because it saved
no permanent words, the experiment was reverted without commit.

No accepted production change from v356. The combined +17 words
compared with the historical resident default remain unresolved. A
future reduction should derive export/trampoline overhead from actual
linked package metadata and preserve independent ABI entry addresses,
then validate a native boot and filesystem semantic regression. The
full-profile permanent-address overflow and strict 32K acceptance
remain deferred.

## 2026-10-25 Mandatory MonitorFS MRES install cleanup v356

Removed the remaining compile-time `MONITORFS_MRES_ENABLED` branches
around the MonitorFS MRES package declarations and boot MINIT table
entry in `system/kernel/modules/module_table.s`, and around
`mfsdev_install_minit` in `module_minit.c`. MonitorFS now has an
unconditional MINIT installer in both configurations; the resident
MonitorFS implementation cannot be selected by disabling a build flag.
The kernel assembly copy selection is also unconditional and always
uses the ABI trampoline-only `fs/monitorfs_dispatch_v14.s`. The nine
resident ABI jump words remain necessary under the current interface.

Full-linked GCC 64K lowmem image metrics: KCORE 9151, KCORE+MRES
15699, PERMANENT_LAST 15746 words, **no RAM growth** from v355.
Native `daimos-testkit` `test-daimos-monitorfs-mres-domains-v346`
PASS with an isolated lowmem PDP-6 image, exercising process stat/read,
domain directory enumeration and numeric-domain lookup under the
forced always-on package. Earlier child/zombie test v354 passed under
the same forced package model in v355. This v356 cleanup does **not**
recover the remaining +17 combined words; do not remove ABI trampolines
without proving external callers and MINIT patch relocation remain
correct. Full-profile permanent-address overflow and strict 32K
acceptance remain deferred.

## 2026-10-25 MonitorFS always-loaded MRES, no resident implementation v355

MonitorFS is now **MRES-only by construction**. The PDP-6 boot build
includes the `monitorfs-mres` package unconditionally in both lowmem
and full MRES package sets, and both boot and kernel Makefiles force
`MONITORFS_MRES_ENABLED=1` even against old command-line requests to
disable it. Kernel build `monitorfs_runtime.s` is always assembled from
`fs/monitorfs_dispatch_v14.s` rather than the old full
`monitorfs_runtime.s` + `monitorfs_swap_v63.c` implementation.
Thus the MonitorFS device, process, domain, and swapped metadata runtime
is linked into the pinned, boot-installed MRES and **not resident as
full KCORE implementation**. The unavoidable nine one-word ABI/MINIT
jump slots remain in KCORE and are patched to the relocated package at
boot; VFS's shared generic provider routing stays resident. Do not
claim literally zero MonitorFS-related KCORE words.

Full-linked lowmem GCC default build measured KCORE=9151,
KCORE+MRES=15699, PERMANENT_LAST=15746. An explicit obsolete
`MONITORFS_MRES_ENABLED=0` argument still produces these figures,
proving resident fallback is disabled. Native daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS** under this forced default
(including RUN/WAIT, PROCINFO and decimal `/PROC/10/STATE`). Existing
native device, process, and domain MRES tests v341/v343/v344/v346
remain useful. No new permanent RAM change compared with prior
opt-in MRES, but compared with former normal non-MRES default,
KCORE is reduced by 1575 words while combined RAM is increased by 17.

Full-profile GCC size-only build yields KCORE=9151,
KCORE+MRES=18463, PERMANENT_LAST=18510 and fails its existing
`040000` permanent-address gate; full-profile boot acceptance is **not
claimed**. Keep the larger size restriction and strict 32K acceptance
on the deferred backlog; do not change this limit by workaround.
Future optimization may recover the 17 combined resident words and
remove unnecessary jump slots only if ABI-safe, with full linked
measurement before acceptance. Feature is no longer opt-in.

## 2026-10-25 Native zombie PROCINFO safety and RAM-neutral fix v354

Root cause of baseline v348-v351 lowmem child-completion halt found in
`system/kernel/proc/syscall_info.s`: `vm_space_destroy` publishes
`VM_SPACE_NONE` (`-1`) in the zombie's `vm_state`, while
`sys_procinfo_user_comm` previously checked only the nonzero RH
image base and could dereference the invalid `0777777` image address
while reconstructing COMM from argv[0]. A negative VM descriptor is
now treated as unavailable and selects the existing `USER` fallback.
The argv[0] offset is also added directly from memory to AC7,
eliminating one intermediate MOVe and offsetting the one-word safety
check without altering pointer interpretation. The first experimental
explicit zombie-state check would have cost +5 KCORE words and was
replaced; the narrower VM sentinel guard initially cost +1 word; the
final combined fix has **zero full-linked permanent RAM growth**.

Native 64K GCC lowmem regression in daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS**, including the previously
failing child `PROCINFO`, pipe EOF, WAIT and decimal `/PROC/10/STATE`
S6REC lookup checks. With the final source, full-linked default
KCORE 10726, combined 15682, PERMANENT_LAST 15729 words; optional
MonitorFS MRES KCORE 9151, combined 15699, PERMANENT_LAST 15746,
unchanged against v329/v333 respective baselines. Testkit commit
`c772b54`. Related v348-v351 HALTs are resolved in this tested
configuration, independent of MonitorFS extraction. The broader
MRES swapped-process metadata acceptance remains outstanding;
MonitorFS MRES stays opt-in, strict 32K gate deferred.

## 2026-10-25 Native lowmem SYS_PROCINFO immediately after RUN v351

Adapted the baseline native one-child RUN/PIPE test to call
`dsys_procinfo(pid,&info)` immediately after `SYS_RUN` returns,
before closing parent pipe descriptors or waiting for the child.
Using 64K GCC lowmem, `MONITORFS_MRES_ENABLED=0`, native PDP-6 trace
reached `D6FS LOADED`, then `<RSQT` before halting later at
`PC: 000000 (LUUO00 0,300)`. The `T` marker is emitted after the
first `PROCINFO(2)` returned zero; the fixture also checked that
`info.pid` equals 2. Hence initial child-slot PROCINFO dispatch and
record population work. The subsequent `wait_zombie_by_eof` path
still fails when querying PROCINFO after writer close, EOF read and
reader close, as previously shown in v348/v349. The defect is
associated with child completion/zombie state or with its preceding
lifecycle transition, not ordinary PROCINFO dispatch, and reproduces
without MonitorFS MRES.

The experimental fixture was withdrawn after the diagnostic run;
unrelated dirty testkit state was left untouched. No production
source or permanent RAM changes. Next isolate PROCINFO after each
individual teardown operation and study process slot zombie-layout
in `syscall_info.s`; reject any speculative workaround. MonitorFS
MRES native namespace acceptance v341/v343/v344/v346 remains PASS,
feature opt-in with KCORE=9151 and combined 15699 (+17 default),
strict 32K acceptance deferred.

## 2026-10-25 Native lowmem no-child SYS_PROCINFO acceptance v350

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-procinfo-current-v350` with `MONITORFS_MRES_ENABLED=0`.
User INIT calls `dsys_procinfo(1,&info)` without RUN, PIPE, WAIT or
other child lifecycle activity, verifies that the UUO returns zero and
that `info.pid == 1`, then emits the completion marker. Target run
**PASS**. Consequently, the native UUO 057 path is functional for
the initial process with a mapped caller result buffer.

Earlier v348-v349 tests consistently halted inside PROCINFO(2)
after completing child RUN, pipe-writer close, pipe EOF read and
pipe-reader close, regardless of the MonitorFS MRES switch. The
remaining fault is therefore related either to non-initial/zombie
process-table record handling or to state left by the preceding
lifecycle operations, not merely to invoking PROCINFO itself.
Check `sys_procinfo` handling of a zombie slot and its `COMM`
reconstruction, plus native syscall mapping/return invariants;
compare a direct PROCINFO(2) after RUN without pipe teardown.

The accepted testkit commit is `7dc7023`; the test touches no kernel
production source. Fixed KCORE baseline 10726, combined 15682;
optional MonitorFS MRES remains KCORE 9151 and combined 15699 (+17).
Feature stays opt-in, strict 32K acceptance deferred.

## 2026-10-25 Baseline lowmem SYS_PROCINFO comparison v349

Compared the one-child PIPE/RUN/EOF/PROCINFO probe against an isolated
64K GCC lowmem image with `MONITORFS_MRES_ENABLED=0`, matching v348
apart from disabling the extracted MonitorFS MRES. Both builds booted
through `D6FS LOADED`, and both emitted `<RSWabcd` before HALT at
`PC: 000000 (LUUO00 0,0)`. Thus the failure in `dsys_procinfo()`
also occurs in the default lowmem baseline. It is **not** evidence of
a defect introduced by MonitorFS MRES extraction. The probe did not
access MonitorFS namespace paths.

Source inspection locates the syscall ABI in `userland/libc/syscall.s`
(`dsys_procinfo: uuo 057,0(1)`), native dispatch in
`system/kernel/proc/syscall_dispatch_pdp6.s` (map result buffer, call
`sys_procinfo`), and the implementation in
`system/kernel/proc/syscall_info.s`. The root cause of the halt remains
unresolved; next step is a minimal zero-child SYS_PROCINFO probe and
trap/return tracing, with assembler/compiler defects prioritized.
The temporary failing baseline testkit fixture was withdrawn with
unrelated dirty testkit files intact. No production code or RAM change.

Prior accepted MRES tests v341/v343/v344/v346 remain PASS; optional
KCORE 9151 and combined 15699 (+17 versus default). Feature stays
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child SYS_PROCINFO trap localization v348

Created a temporary daimos-testkit native lowmem GCC 64K PDP-6
`MONITORFS_MRES_ENABLED=1` single-child RUN/PIPE lifecycle probe.
Instrumented the four distinct steps of `wait_zombie_by_eof` after
parent's RUN return: `a` before closing parent writer, `b` after
writer close, `c` after reading EOF, `d` after closing reader, `e`
only after returning from `dsys_procinfo`. PDP-6 trace reached
`D6FS LOADED`, emitted `<RSWabcd`, then halted at
`PC: 000000 (LUUO00 0,0)`. Thus writer close, EOF read, reader
close all completed successfully. The failure occurs within or while
returning from `dsys_procinfo(pid,&info)`, before `e` could print.
The libc entry uses `uuo 057,0(1)` in `userland/libc/syscall.s`.
No MonitorFS path is used by this probe. Diagnose the SYS_PROCINFO
UUO dispatch/return path and compare identical lowmem configuration
with `MONITORFS_MRES_ENABLED=0` before attributing to MRES.

The failed tracing fixture was withdrawn; all other dirty testkit
files were preserved. No production code or memory footprint changed.
Accepted native MRES namespaces v341/v343/v344/v346 remain passing.
Optional KCORE 9151, KCORE+MRES 15699 (+17 vs default), opt-in,
strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child RUN/WAIT isolation v347

Adapted the existing full-profile process decimal native fixture to an
isolated 64K lowmem GCC image with `MONITORFS_MRES_ENABLED=1`, exactly
one child rather than nine, and markers for pipe creation (`N`),
`SYS_RUN` entry (`R`), return from RUN (`S`), and entering the
zombie-by-pipe/PROCINFO check (`W`). The PDP-6 boot reached D6FS
LOADED; userspace emitted `<NRSW` and then halted at
`PC: 000000 (LUUO00 0,300)`. Thus the pipe creation and first RUN
returned to the parent successfully. The issue occurs inside the
completion routine, which closes the parent's pipe writer, consumes
pipe EOF, closes the reader, then checks `dsys_procinfo` for the zombie.
**No MonitorFS path was accessed** in this probe. The remaining
fault must be separated into those pipe and PROCINFO steps, and
lowmem baseline behavior should be compared before attributing to
MRES. The failed experimental testkit fixture was withdrawn; unrelated
dirty files remain untouched.

No production modification or RAM savings. Accepted v341/v343/v344/v346
MonitorFS MRES namespace tests retain PASS status. Permanent optional
KCORE 9151, combined 15699 (+17 over default). Feature remains
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain lookup acceptance v346

The failed v345 domain enumeration-to-STAT test was caused by a testkit
SIXBIT encoding error: literal `/` requires octal `017`, but v345
constructed the prefix with `057`. Corrected the prefix in a new
versioned daimos-testkit native test `test-daimos-monitorfs-mres-domains-v346`.
With `MONITORFS_MRES_ENABLED=1`, a 64K GCC lowmem PDP-6 image
successfully validates the v343 process and v344 domain-root checks,
reads a numeric domain ID from READDIR, constructs the corresponding
`/MONITOR/DOMAINS/<id>` SIXBIT path with correct separators, then
successfully STATs it. Native test **PASS**. The domain identity
mismatch from v345 was a fixture bug, not evidence of a kernel
namespace defect. Testkit commit `26c6404`.

No kernel production code or RAM changes. The optional MRES remains
KCORE 9151 and combined 15699, +17 words versus default v329.
Child RUN/WAIT and swapped-process metadata acceptance remain open;
strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain identity probe v345

Extended v344's successful 64K native MRES domain-root test to require
one `/MONITOR/DOMAINS` directory entry, validate a one-to-three-digit
SIXBIT domain ID, construct `/MONITOR/DOMAINS/<id>` from that returned
name, and call STAT on the resulting domain directory. The native PDP-6
image booted and passed v343's process checks and the v344 domain-root
checks, but reported `<ABCDE!c`: domain-entry identity was numeric,
yet STAT on the constructed path failed. This may be a fixture
path-encoding error or a real MonitorFS domain lookup inconsistency;
not established. The fixture's new code was withdrawn after failure,
and pre-existing dirty testkit files were preserved. Do not cite this
as native domain-leaf acceptance. Future investigation should compare
an independently encoded literal domain path, the returned directory
name and the VFS lookup chain before modifying the kernel.

No production code changes or RAM savings. The accepted native domain
root v344 remains PASS. Optional MonitorFS MRES is KCORE 9151 words,
KCORE+MRES 15699 (+17 versus default); 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain root access v344

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-monitorfs-mres-domains-v344` with
`MONITORFS_MRES_ENABLED=1`. Extends the proven v343 no-child
`/PROC/1/STATE` STAT/open/read/S6REC/close checks with
`STAT /MONITOR/DOMAINS`, open the domain directory, one READDIR
(no negative/error return allowed), and close. Native simulator
run **PASS**. This verifies service dispatch and basic domain-root
access; it does **not** yet verify returned entry identities,
child/decimal PID lifecycle, specific domain leaf data, or swapped
process metadata. Testkit commit `9ed13b0`.

No production code changes and no new RAM savings. Opt-in MonitorFS
MRES remains KCORE 9151, KCORE+MRES 15699 (+17 permanent words)
versus default KCORE 10726, KCORE+MRES 15682. Strict 32K acceptance
deferred. Next acceptance should validate specific domain entries
and metadata values, then isolate child process/RUN regressions.

## 2026-10-25 Proposal B native minimal MonitorFS MRES process access v343

Added `daimos-testkit` target `test-daimos-monitorfs-mres-proc-basic-v343`
with a dedicated GCC lowmem, 64K PDP-6 image and
`MONITORFS_MRES_ENABLED=1`. Unlike the failed full process lifecycle
probe v342, the native test does not RUN or create children. It checks
`STAT /PROC`, `STAT /PROC/1/STATE`, OPEN/READ/CLOSE of that state
leaf and a nonempty S6REC text header. Simulator **PASS**. This proves
basic MonitorFS process namespace services are callable through the
relocated MRES; child/RUN/WAIT, decimal PID 10, domain namespace,
and swapped-process argument metadata still require separate tests.

No production source or permanent RAM change. Optional MRES stays
KCORE 9151 and KCORE+MRES 15699 (+17 over default), remains opt-in;
strict 32K acceptance deferred. The native v343 testkit commit is
`3d82596`; unrelated testkit changes were not staged.

## 2026-10-25 Proposal B MonitorFS MRES process acceptance probe v342

Attempted a native process-namespace regression by adapting the
existing full-profile `daimos-monitorfs-process-decimal-v1` fixture
to a 64K GCC lowmem image with `MONITORFS_MRES_ENABLED=1`.
The fixture exercises child RUN/WAIT, two-digit decimal `/PROC/10/STATE`
lookup, and S6REC zombie-state payload. Image build and boot succeeded
and userspace emitted the initial `<` marker; then SIMH halted with
`PC: 000000 (LUUO00 0,300)` before any normal fixture error or final
`P` marker. Thus the test **did not pass**. The defect has not been
isolated to MonitorFS rather than the child/RUN or lowmem assumptions.
The versioned experimental fixture was removed rather than committed
as accepted test coverage. All pre-existing unrelated testkit changes
were preserved. No DAIMOS production change or new RAM saving.

The accepted native opt-in device namespace test v341 continues to
PASS. Further process acceptance should first isolate minimal
`/PROC` path STAT/open/read from the multi-child RUN/WAIT fixture,
then add child lifecycle and swapped metadata cases individually.
MonitorFS remains opt-in, KCORE=9151, KCORE+MRES=15699 (+17 over
v329 default); strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B native MonitorFS MRES devices acceptance v341

Added `daimos-testkit` native PDP-6 target
`test-daimos-monitorfs-mres-device-v341`, running an isolated GCC
lowmem `MONITORFS_MRES_ENABLED=1` image with matching 64K RAM.
The existing full-profile device-view test expected 0377 octal and
initially failed `!5`, as the lowmem profile omits PTR/OCNSLS/DRM.
The new fixture specifies the lowmem expected present set 0153 octal
(CTY0, CLK0, TTY0, DSK0, D6SET0), with absence checks inherited from
the source fixture. Native PDP-6 run **PASS**: MonitorFS device
namespace works through the relocated MRES entrypoints following the
v340 boot-source ordering correction. This is not yet a test of
process/domain leaves, swapped-process record reads, or module unloading.

No kernel production code changed; no new size saving is claimed.
Continue to use the accepted default v329 KCORE 10726 and combined
15682 words; the opt-in MonitorFS MRES is KCORE 9151 and combined
15699 (+17 words). The MRES mode remains opt-in pending broader
runtime acceptance; strict 32K acceptance remains deferred.

## 2026-10-25 MonitorFS MRES package-source ordering boot fix v340

Confirmed that v339's `REVERS` (six-character SIXBIT diagnostic)
actually appeared in the full simulator log, although the earlier
grep for `REVERSE` omitted it. The underlying fault was a mismatch
between MRES source layout and MINIT execution order: lowmem
`MRES_PACKAGE_OBJS` placed `dtfs-mres` **before** `blockset-mres`,
while `module_table.s` runs `blockset_minit` **before** `dtfs_minit`.
`module_run_minits` requires monotonically increasing nonzero package
source addresses to reclaim consumed boot MRES source intervals.

Fixed the lowmem package list in `system/boot/pdp6/Makefile` to link
BLOCKSET before DTFS, matching MINIT. This is the underlying ownership
ordering fix, not a relaxation of MM overlap checks. Fresh isolated
64K GCC lowmem MonitorFS MRES image built and booted in PDP-6 SIMH:
`BSET LOADED`, `D6FS LOADED`, `INIT V1`. Simulator ran until explicit
17-second timeout (exit 124); **no prior post-BSET `kinit_halt`**.
INIT reported `MEMFS NOT MOUNTED ON /TEMP`, not yet investigated.
Native MonitorFS namespace/swapped-process semantic acceptance remains
open, so the optional feature stays disabled by default.

Full linked optional lowmem `permanent-size` unchanged by reordering:
KCORE 021677 (9151), KCORE+MRES 036523 (15699),
PERMANENT_LAST 036602 (15746), +17 words combined versus default v329
(10726 / 15682 / 15729). The +17 cost is provisionally acceptable;
optimization of that overhead follows functional acceptance. Strict
32K gate deferred.

## 2026-10-25 MonitorFS MRES MINIT halt probes v338-v339

Investigated v337's post-BLOCKSET `kinit_halt` with two separate
temporary diagnostic builds, both using matching 64K PDP-6 SIMH
configuration. The first printed `GAPERR` only on failed
`mm_add_free(previous_package, package - previous_package)`; the
second additionally printed `REVERSE` for descending source-package
addresses. Neither marker appeared before the HALT, so neither branch
was conclusively identified as the caller. A failed debug-output
routine or different `kinit_halt` caller has not been excluded.
Both builds still halted after `BSET LOADED`. The instrumentation
was removed entirely; source is unchanged. The previous v337
inference that `mm_add_free` was specifically rejecting an interval
remains **unproven** and must not be used as the basis for an MM fix.
Next: capture the PDP-6 halted CPU's return chain or trace calls to
`kinit_halt` directly, then inspect the implicated MM/initializer
condition. MonitorFS MRES remains opt-in; v333 measured fixed KCORE
9151 words and combined KCORE+MRES 15699 versus default 10726/15682.
No runtime PASS or additional saving claimed; strict 32K deferred.

## 2026-10-25 MonitorFS opt-in MINIT source-reclaim localization v337

Built an isolated 64K lowmem MonitorFS-MRES image with temporary
SIXBIT `NEXT` and `DONE` trace markers around each call in
`module_run_minits` (not retained in production). The PDP-6 simulator
trace shows `BSET LOADED`, then `DONE`, followed by a HALT without the
next `NEXT`. The failure therefore follows successful return from the
BLOCKSET initializer and occurs before the next initializer call.
The intervening branch validates/free-publishes the previous package
source between two successive MRES package addresses. In the
instrumented linker map `blockset_mres_package=075633` and
`d6fs_mres_package=075672` octal, so the expected reclaimed gap is
037 octal words. Since the next `NEXT` marker follows the reclamation
branch and the addresses are increasing, `mm_add_free` rejecting this
source interval is the leading diagnosis; confirm the exact
allocation/arena overlap or MM return code before any fix. `mm_add_free`
explicitly rejects overlapping extents and arenas, and no bypass is
acceptable. The diagnostic KINIT edits were reverted immediately and
no kernel production source was changed. The option remains off by
default; +17 combined permanent words remains provisionally acceptable
if full runtime acceptance is achieved. Strict 32K gate deferred.

## 2026-10-25 MonitorFS MRES post-BSET boot localization v336

Using the existing optional `MONITORFS_MRES_ENABLED=1` image from
v333, compared unmodified boot media under SIMH configured for 64K
and 256K physical memory (versioned simulator INI copies). Both
executions reached the DAIMON KINIT banner and reported
`BSET LOADED`, then halted through `kinit_halt` at octal PC 063116.
This excludes the original 32K/64K configuration mismatch as the
sole reason for the *later* halt; simply increasing simulator RAM does
not resolve it. The module table places `d6fs_minit` after BLOCKSET,
so the next investigation should instrument the exact pre-D6FS/MRES
call boundary, plus early MM reservations, without asserting that D6FS
or MonitorFS caused the halt. A generic `kinit_halt` PC alone cannot
attribute the caller. No production code changed and the default
MonitorFS MRES option remains off. The +17 permanent-word difference
is provisionally acceptable contingent on real boot and service tests;
no additional RAM saving or runtime PASS is claimed.

## 2026-10-25 MonitorFS opt-in boot diagnosis v335

Examined the opt-in 64K MonitorFS-MRES image boot from v334. The
original `?B`, `HALT 000544` was produced with an inconsistent boot
configuration: the lowmem profile's generated `boot.ini` still set
`cpu 32k` while `BOOT_CORE_WORDS=0200000` built a 64K image.
An isolated boot-configuration copy using `set cpu 64k` proceeds past
Stage1 and emits the DAIMON V0.5 / PDP6 / MEM 64K KINIT banner and
MINIT statuses through `BSET LOADED`, but then halts at `kinit_halt`
(PC 063116). Thus there is still a later boot/installation failure;
no functional MonitorFS runtime acceptance is claimed. The boot failure
has not been attributed to the MRES itself. A matching CPU-size option
must accompany larger-image tests; do not weaken stack-reserve checks.

The opt-in MonitorFS MRES link fix from v333 is retained and remains
disabled by default. The proposed +17 combined permanent words is
provisionally acceptable if runtime validation succeeds, but should
be optimized afterward. Memory figures remain KCORE=9151,
KCORE+MRES=15699 (enabled) versus 10726/15682 (default).
The strict 32K gate remains deferred.

## 2026-10-25 MonitorFS MRES build and boot gate v334

The optional MonitorFS MRES full link (v333) saves 1575 KCORE words
but increases KCORE+MRES and PERMANENT_LAST by 17 words. The 17-word
cost is an acceptable provisional tradeoff if native behavior is correct;
seek to recover it only after the runtime acceptance gate, without
adding state or compromising module boundaries.

`BOOT_PROFILE=lowmem MONITORFS_MRES_ENABLED=1` with default 32K
`BOOT_CORE_WORDS` failed `mkimage` because KINIT left less than the
configured stack reserve. Without relaxing that protection, rebuilt
with isolated `BOOT_CORE_WORDS=0200000` (64K) and the image target
completed. A SIMH boot using its generated `boot.ini` then printed
`?B` and halted at PC `000544` (`HALT 544`). Consequently, successful
MRES linking/image creation is *not* successful runtime installation
or MonitorFS namespace acceptance. The failure needs diagnosis before
enabling MRES by default. The default v329 configuration is unchanged.
Strict 32K acceptance stays deferred.

## 2026-10-25 Proposal B MonitorFS MRES missing swap-helper build fix v333

Fixed the opt-in `MONITORFS_MRES_ENABLED=1` package link in
`system/boot/pdp6/Makefile`: compile the existing
`fs/monitorfs_swap_v63.c` with PDP-10 GCC into a distinct MRES object
and link it alongside the generated namespace service object. The
normal non-MRES KCORE build remains unchanged. This resolves the
previous `undefined symbol: mfsproc_swapped_read_words_v63` failure
without dropping functionality, stubbing a syscall, or adding KCORE
resident storage. A fresh isolated GCC lowmem `permanent-size` build
now **links successfully** with MonitorFS MRES enabled.

Measured default v329 versus experimental enabled profile:
KCORE 024746 (10726) -> 021677 (9151), a 1575-word reduction;
MonitorFS MRES adds 003070 (1592) words; KCORE+MRES 036502
(15682) -> 036523 (15699), an increase of 17 permanent words;
PERMANENT_LAST 036561 (15729) -> 036602 (15746), likewise +17.
This is a large **fixed-KCORE size reduction**, but **not a total
permanent occupancy reduction**. The feature stays opt-in, not the
accepted default permanent-RAM optimization baseline.

A successful link does not prove boot, MINIT export patching, pinned
module lifetime, swapped-process reads, or namespace semantics; those
are outstanding native runtime acceptance gates. Existing
`mfsdev_install_minit` already patches nine dispatch entries (contrary
to the earlier assumption that the installer was absent). Do not
mark MonitorFS extraction complete until those gates pass. Strict 32K
acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES configuration investigation v332

Re-examined the effective build rather than inferring residency from the
KCORE object names. `MONITORFS_MRES_ENABLED` defaults to 0; its normal
KCORE build concatenates `monitorfs_runtime.s` with the compiled
`monitorfs_swap_v63.c` implementation. With the option set to 1,
`system/kernel/Makefile` **instead substitutes**
`monitorfs_dispatch_v14.s` for the resident MonitorFS object, leaving
nine one-word `JRST kret_neg1` service entrypoints. Thus the KCORE
object list alone does not show which implementation actually resides
there. This corrects the earlier interpretation of the Makefile.

Attempted isolated GCC lowmem full-link build with
`MONITORFS_MRES_ENABLED=1` in a distinct TMPDIR build root. KCORE
assembly/link succeeded, but the optional MonitorFS package link failed
with `undefined symbol: mfsproc_swapped_read_words_v63`. The
`monitorfs-mres-probe-v15.py` generator rewrites the full assembler
source, which references that helper; the helper is supplied by
`monitorfs_swap_v63.c` in the normal resident build but was not included
in this package. The generator explicitly says its output is a
**non-installable probe**. `monitorfs_dispatch_v14.s` explicitly says
MINIT must still patch its nine JRST destinations after MRES relocation
and pin the MRES while MonitorFS is reachable. Neither link repair nor
runtime installation was attempted; the enabled configuration is NOT
a working MonitorFS extraction. No complete permanent-size comparison
or native enabled-mode acceptance can be claimed. No source changes
were made. Last accepted standard lowmem baseline remains v329:
KCORE 024746 (10726), KCORE+MRES 036502 (15682), PERMANENT_LAST
036561 (15729), octal values followed by decimal in parentheses.

Next B stage: supply the swapped-reader code/dependencies to the MRES
package properly, verify package relocation/link and resident entries,
implement and test MINIT patching and lifetime/pinning, measure both
profiles, and test MonitorFS device/process/domain namespace on the
PDP-6. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS device/namespace audit v331

Inspected the resident `monitorfs_runtime.s` device-name lookup,
READDIR, device-ID validation, special TTYDPY0 name handling,
leaf selection and IO type classification. The already shared
`mfsdev_name_length`, `mfsdev_validate_id`, `mfsleaf_lookup` and
`mfsdev_readdir_store` leaves account for the obvious overlapping
operations. The D6SET0 special leaf count and TTYDPY0 seventh character
must remain exceptional. No safe additional resident reduction was
established: no production code changed, and no RAM saving is claimed.

`daimos-testkit` GCC `test-daimos-monitorfs-devices-online` PASSed on
PDP-6 after its independent boot-image build (roughly 200 seconds).
This is a native MonitorFS device-visibility test, not a comprehensive
process-namespace or multi-process provider test. The attempted v330
UNLINK/RMDIR type-matrix fixture remains unaccepted; no false regression
PASS was recorded. Preserve the last measured v329 GCC lowmem full link:
KCORE 024746 (10726 decimal), KCORE+MRES 036502 (15682 decimal),
PERMANENT_LAST 036561 (15729 decimal). Strict 32K acceptance remains
deferred. Further B source work should target MonitorFS process/domain
namespace lifetime and file-path permission/lookup interactions, with
whole-image measurement before acceptance.

## 2026-10-25 Proposal B remove-type native fixture investigation v330

Attempted a dedicated PDP-6 UNLINK/RMDIR type-matrix fixture by extracting
v328's actual `file_remove` type check and iterating 16 STAT-type / action
cases. The initial fixture had undefined link symbols and incorrect test
expectation/loop instructions; those were corrected, but `p10run`
continued to time out (SIMH status 124) without readable test exit.
No native acceptance is claimed, and the testkit experiment was removed
rather than committing an unverified fixture. Existing unrelated testkit
edits were preserved. No production code changed; retain the last v329
full-image measurement (KCORE 024746, KCORE+MRES 036502,
PERMANENT_LAST 036561 octal). Dedicated native UNLINK/RMDIR semantics
remain unverified. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B CREATE/SYMLINK result publication v329

The shared `vfs_create_store_result` epilogue formerly loaded the
caller's output-node pointer from `(17)` into AC4 before storing the
provider-result vnode. PDP-6 indirect addressing can publish the
result directly with `MOVEM 3,@(17)`, eliminating the intermediary
register load. The preceding success check, mount-ID inheritance,
caller-owned pointer and three-word stack unwind remain unchanged;
CREATE/MKDIR/MKFIFO and SYMLINK retain the existing provider ABI.

Fresh full GCC lowmem disk-root linked image compared with v328:
KCORE 024747 (10727 decimal) -> 024746 (10726), KCORE+MRES
036503 (15683) -> 036502 (15682), PERMANENT_LAST 036562 (15730)
-> 036561 (15729). One permanent word saved, no added state.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. `test-daimos-d6fs-context-dispatch` and
`test-daimos-vfs-storage-reservation` also PASSed in this review.
The existing tests do not separately prove every CREATE/SYMLINK
failure-mode combination. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B UNLINK/RMDIR type normalization v328

Consolidated `file_remove`'s duplicated directory and non-directory
STAT result branches. Normalizing `VFS_TYPE_DIR` (1) to a boolean and
comparing it with the existing requested-directory flag preserves UNLINK
(reject directories) and RMDIR (accept directories only), without
changing the parent permission, backing-busy or provider removal paths.
No ABI, new storage or provider dispatch changes.

Full linked GCC lowmem disk-root image compared with accepted v324:
KCORE 024752 (10730 decimal) -> 024747 (10727), KCORE+MRES
036506 (15686) -> 036503 (15683), PERMANENT_LAST 036565 (15733)
-> 036562 (15730). Three permanent words saved, no MRES growth.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. A dedicated native UNLINK/RMDIR type matrix is still useful
for stronger semantic acceptance. The strict 32K acceptance gate
remains deferred.

## 2026-10-25 Proposal B expanded native provider gateway test v327

Expanded daimos-testkit's v326 PDP-6 gateway oracle into a distinct
`test-daimos-provider-lock-native-v327` target. Native simulator PASS
covers the v326 locked-wait/register-recovery and MEMFS bypass cases,
plus unlocked D6FS and TSFS table dispatch and rejection of provider
IDs outside the 4..7 range. This executes the extracted production
assembly gateway with test-controlled service jumps and wait/wakeup;
it is not an actual competing-process filesystem I/O test. The
multi-process physical-provider contention acceptance remains open.

No production source changed and no new RAM saving is claimed.
Last fully linked GCC lowmem v324 baseline remains KCORE 024752
(10730 decimal), KCORE+MRES 036506 (15686), PERMANENT_LAST
036565 (15733). No strict 32K acceptance work was performed.

## 2026-10-25 Proposal B PDP-6 provider lock native probe v326

Added `daimos-testkit` `test-daimos-provider-lock-native-v326` and ran it
successfully on PDP-6 via `p10run`. The fixture extracts the real
resident provider gateway and uses a controlled wait oracle that initially
holds the lock, releases it through `proc_wait_event`, and deliberately
clobbers request registers before retry. The production save/restore path
returns the original arguments to the provider, preserves its result across
wakeup and releases the lock. The MEMFS path bypasses serialization.
This is native execution of production gateway instructions with oracle
wait/wakeup services, **not** proof of true multi-process scheduler contention
or real physical-provider I/O. No production source changed. The accepted
v324 full linked baseline remains KCORE 024752 (10730 decimal),
KCORE+MRES 036506 (15686 decimal), PERMANENT_LAST 036565 (15733).
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider lock invariants gate v325

Audited the v324 resident provider dispatch and shared transfer-workspace
lock. The private 7-word request save survives the `proc_wait_event`
loop, the ready lock is acquired before registers are restored, and
provider completion releases the lock and wakes waiters while preserving
the returned AC1. MEMFS still bypasses serialization; DTFS/D6FS/TSFS
continue to serialize over potentially sleeping I/O. No production code
change or memory saving is claimed. The accepted full-linked baseline
remains v324: KCORE 024752 (10730 decimal), KCORE+MRES 036506
(15686 decimal), PERMANENT_LAST 036565 (15733 decimal).

Added `daimos-testkit` target `test-daimos-provider-lock-contract-v325`
(PASS). It enforces assembly source-level register/lock invariants only;
it is not a scheduler contention test. The testkit had unrelated local
edits which were preserved. Further reentrancy changes require actual
multi-process sleep/wakeup and concurrent-provider native acceptance.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider table index normalization v324

Normalized the provider number (4 MEMFS through 7 TSFS) to a 0..3
service-table index on entry to `fs_provider_reg_call`. The MEMFS
fast path remains direct and non-sleeping. The other providers retain
the seven-register process-private save, lock wait, dispatch and wakeup;
the index is now saved instead of the original provider number, removing
the redundant subtraction after a lock acquisition. Invalid provider
numbers still return `fs_mres_no_service`. No ABI or persistent storage
changes were made.

Full GCC lowmem disk-root linked image versus accepted v322:
KCORE 024755 (10733) -> 024752 (10730), KCORE+MRES 036511
(15689) -> 036506 (15686), PERMANENT_LAST 036570 (15736) ->
036565 (15733). Saving: three permanent words, unchanged MRES sizes.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. These tests do not exhaustively exercise concurrent lock waiters;
a targeted stress regression remains useful. Strict 32K acceptance
and further JRST folding remain deferred.

## 2026-10-25 Proposal B SETATTR unwind experiment v323 (reverted)

Re-tested reusing `vfs_mutate3_ro` for `vfs_setattr_ro` in the
current v322 GCC lowmem configuration. Although the source contains
one fewer inline cleanup instruction, the full linked image **grew**:
KCORE 024755 -> 024756 octal (10733 -> 10734 decimal),
KCORE+MRES 036511 -> 036512 octal (15689 -> 15690),
PERMANENT_LAST 036570 -> 036571 octal (15736 -> 15737).
This is a one-word permanent RAM regression. The experiment was
reverted immediately; no semantic acceptance claimed. Retain v322 as
the measured baseline. Continue Proposal B with structural
file-runtime/VFS/provider consolidation, not SETATTR cleanup sharing.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B VFS CREATE mode staging v322

The common CREATE/MKDIR/MKFIFO request staging in `vfs_create_common`
carried the umask-filtered mode through an unnecessary intermediate AC3
assignment before moving it into AC4. Passing the masked value directly
from AC6 into AC4 retains the fifth argument, original call conventions,
provider request order and mode semantics. No new state or dispatch.

Full GCC lowmem disk-root linked image against v321: KCORE 024756
(10734 decimal) to 024755 (10733); KCORE+MRES 036512 (15690) to
036511 (15689); PERMANENT_LAST 036571 (15737) to 036570 (15736).
Savings: one permanent word. Component MRES sizes are unchanged.

`daimos-testkit` GCC `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
all PASS. No dedicated native CREATE/mode matrix was added in this
step. Architectural follow-up should seek meaningful savings in
request frames and provider interactions rather than new dispatch
abstractions. Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-25 Proposal B file lock mount decode v321

Consolidated the identical two-bit mount-extension lookup in the
resident FLOCK conflict and update passes into a single internal leaf
helper, preserving AC4, the public descriptor/mount ABI, lock ownership,
and shared/exclusive/unlock semantics. This is not a new provider dispatch
or permanent storage allocation. The experiment was present as an
uncommitted workspace change before this audit; it was independently
measured against committed v318 source with full GCC lowmem disk-root
links. Baseline: KCORE 024764 (10740 decimal), KCORE+MRES 036520
(15696), PERMANENT_LAST 036577 (15743). Accepted candidate: KCORE
024756 (10734), KCORE+MRES 036512 (15690), PERMANENT_LAST 036571
(15737). Net saving: 6 permanent words; MRES component sizes unchanged.

GCC daimos-testkit target-file-pdp6, target-file-walk-pdp6,
test-daimos-vfs-stat-frame and test-daimos-pre-dsh-fs-closure all PASS.
The file-runtime native fixture exercises shared/exclusive locking,
conflict, unlock and descriptor duplicate behavior. An additional
explicit cross-mount high-extension collision scenario is still a
valuable follow-up; do not claim that narrow scenario was separately
tested here. The 32K acceptance gate and JRST folding remain deferred.

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

## 2026-10-25 v311 twenty-child native swap acceptance — PASS

The preserved v311-r2 native testkit run completed successfully:
`daimos-mm-swap-progress-v311: PASS (one post-INIT 20-child pressure cycle)`.
The 256K PDP-6 GCC full-profile guest using a 012000-octal-block
(5120 block) drum backing configuration printed
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFPGPHPIPJSRQ` and a normal
HALT. All twenty children were admitted; writer data integrity,
wait/reap and one full pressure cycle were checked by the guest.
The simulator-only timeout ceiling was 900 seconds; success must not
be read as proof that smaller swap capacity suffices or that every
64-slot simultaneous pressure scenario passes. This completes the
immediate high-concurrency swap correctness regression following the
production swap-in failure-path stack pop fix `3b41e87` and 12-/16-
child passes. No further kernel, KCORE or descriptor-policy change.
Resume the original permanent EXEC/RUN minimization/source sweep.
Further capacity/large concurrency tests remain optional backlog,
not a blocker for EXEC/RUN work.

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

## 2026-10-25 physical fault snapshot invariants v307

Reanalyzed the preserved v297 physical pre-swap-out/post-HALT trace
with versioned testkit tool `daimos-mm-swap-snapshot-invariants-v307.py`.
Of 317 common octal memory words, 30 changed: 27/64 sampled idle-stack
words and one of 83 process/MM metadata words. None of the four sampled
storage-map words changed. Both `mach_kernel_stack_base` and
`mach_kernel_sp` remained `040350` octal, and the first storage-mode
word remained `000005252520` octal. These diffs occurred across
continued execution and therefore do **not** prove stack overflow or
an illegal memory overlap. They also do not rule out unsampled memory
corruption. The known missing stack-local pop was separately repaired
in `3b41e87`; v306 revalidated 12 children. Current unresolved issue
is 16-child swap-space exhaustion, writer TERM handling, and the
higher-capacity v305 timeout. No production change in v307.

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

## 2026-10-25 physical swap-map overlap probe v292

A native 256K PDP-6 16-child failing guest was traced with SIMH
breakpoints examining the storage-state map and 28 MM descriptors.
At the unexpected-UUO aftermath the state-map pointer was physical
`041141` octal; its allocated MM extent was `[040750,041145)`,
`MM_TYPE_KERNEL_DYNAMIC=3`, owner `4` (swap metadata). The captured
neighboring extent boundaries were contiguous and did not overlap
this range. `proc_storage_runtime_v29.slots=0100` (64 slots).
The map's first word was `000005252520` octal, encoding slot 1 as
RESIDENT (0) and slot 2 as DISK (1). The earlier block-count check
matched `0320` octal; the swap-in storage-state expectation for slot 2
reached the good branch. Two failed mode checks observed **after**
the unexpected UUO do not establish the fault's original cause.
The trace covers only the descriptor range requested from SIMH, so
it does not establish a full-table global overlap invariant. It does
rule out a different visible MM extent overlapping this specific
storage-state allocation in the captured snapshot.

The next priority is to trace allocator pressure/compaction calls
nested in the first swap-in and the following swap-out, including
stack bounds and return paths; avoid attributing the UUO to the
storage-state map without new evidence. No production change.

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

## 2026-10-25 post-INIT concurrency escalation v282/v283 — UNRESOLVED

Starting from the passing 12-pressure-child v281 fixture, v282 raised
the guest's post-INIT pressure children to 20. In the 256K PDP-6 GCC
SIMH image the 17th requested pressure child (zero-based index 16)
failed admission (`pG!W`); the run eventually timed out at 360 seconds.
88 MM-insert samples showed maximum pre-insertion count 34, implying
an observed insertion upper bound 35/136. This is **not** proof of a
descriptor exhaustion error or an accepted pressure test.

v283 reduced the same test to 16 children. Instead of completing,
SIMH reported unexpected `UUO 000000777777 340000000003` and halted
at PC 000001. The transcript emitted `<w` but no normal failure or
completion marker. 72 MM-insert samples again gave maximum pre-count
34 and observed insertion upper bound 35/136. Treat this as a possible
runtime correctness fault with higher priority than further capacity
tuning. Determine the trap's origin and instruction corruption or
control-flow issue before declaring high-concurrency swap acceptance.
The v281 12-child PASS and v279 repeated-swap PASS remain valid.
No production code or 32K KCORE size change.

## 2026-10-25 post-INIT twelve-child live swap/high-water v281

The 256K PDP-6 GCC full-profile testkit workload now starts from one
INIT, activates swap and progressively launches twelve 24K-word
pressure children alongside a blocked pipe writer. A single cycle
PASSes writer restoration, pipe integrity, process cleanup and
MM insertion breakpoint sampling: 71 observations, maximum successful
post-insert descriptor occupancy 31 of 136 (105 spare). The initial
three-cycle version made progress through its second round but hit a
300-second SIMH runtime limit; it was not an acceptance pass.
This confirms post-INIT memory pressure at thirteen user processes,
not swapping with all 63 user slots occupied. The latter remains open.
No production code change or KCORE size growth.

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

## 2026-10-25 native 256K swap descriptor high-water v279

Testkit `daimos-mm-swap-highwater-v279.sh` completed the three
256K PDP-6 swap/reclaim rounds from v278 with a native SIMH
breakpoint on `mm_extent_insert`. From 101 pre-insertion count
samples, the greatest observed post-insertion occupancy was
27 of 136 descriptors (109 spare) in this particular workload.
The test passed all existing writer, pipe-integrity and process
reaping assertions. SIMH strips leading octal zeros from printed
addresses, so the trace parser explicitly normalizes addresses;
earlier unsound parser results of 1 were rejected. This does NOT
replace the 63-process workload's separate peak of 132 or prove
peak occupancy under simultaneous 63-process pressure. No
production code or KCORE size change.

## 2026-10-25 repeated 256K native swap cycles v278

The daimos-testkit native PDP-6 GCC full-profile 256K fixture
`daimos-mm-swap-cycles-v278.sh` passed three successive rounds
of blocked pipe writer plus eight sleeping pressure children.
Every round exercised the memory-pressure check, writer resume,
pipe-integrity assertions and reaping of all pressure children.
This validates repeated swap/reclaim and process-slot reuse beyond
the v277 single-cycle acceptance result. It does not yet provide
the 136-descriptor registry high-water under these cycles.
No production source change; the 32K GCC lowmem disk-root
KCORE size reference remains 10742 words, KCORE+MRES 13605.

## 2026-10-25 AUXSTORE/root mapping order fix v277

The v272 eight-pressure-child 256K native PDP-6 GCC workload now
PASSes after moving `auxstore_post_minits()` from before root mounting
to immediately after `kinit_boot()` in `system/kernel/boot/kinit.c`.
Root D6FS mounting updates the shared BLOCKSET direct-tail mapping;
publishing AUXSTORE before root mounting left its backing-store count
advertised while the root mount replaced the live direct-tail extent.
The corrected order permits all eight pressure children and restores
the blocked pipe writer with the fixture's data-integrity checks.
This is live swap-path acceptance, not comprehensive repeated swap
stress. No permanent KCORE data added. Mandatory 32K GCC lowmem
disk-root size: KCORE 10742 words, KCORE+MRES 13605 words,
PERMANENT_LAST 13652 (unchanged).

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

## 2026-10-25 256K eight-child swap-pressure diagnostic v272

A dedicated GCC full-profile 256K PDP-6 testkit guest uses a
24K-word BSS array (inside the DXR BSS limit) and attempts
eight pressure children behind a blocked pipe writer. The
first seven pressure children start and reach PROC_SLEEP;
the eighth `dsys_run` invocation (zero-based child index 7)
fails, emitting `<wp7!W`. This is a **failing diagnostic**,
not swap acceptance. The specific returned error and allocator
state have not yet been observed. Investigate process-create
admission, memory allocation and reclaim rather than raising
MM descriptor limits or declaring a swap bug prematurely.
No production source changed; KCORE measurement remains the
32K GCC lowmem disk-root configuration.

## 2026-10-25 PDP-6 256K executable BSS admission v271

The native exec loader in `system/kernel/proc/exec_load.s` masks
the DXR BSS field to 15 bits and rejects BSS exceeding `077777`
octal (32767) words. This explains v270's 56K-word BSS guest
failing before INIT startup. A separate 256K GCC full-profile
image with a 24K-word pressure array now reaches guest INIT and
prints `<ws!W`: startup succeeds, but the existing two-child
pressure test does not actually force swapping at 256K (`s`).
Testkit v271 records this as an explicitly expected negative
control, not a successful swap stress test; its static bound
check PASSes. The SIMH memory-size increment is 16K words,
while DAIMOS process/extent table policy retains 8K increments.
KCORE size reporting remains at the 32K GCC lowmem disk-root
configuration. No production code change.

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

## 2026-10-25 Native MM descriptor saturation v268

The daimos-testkit daimos-mm-pressure-v268.sh executes
production PDP-6 MM assembler insertion/deletion at the
256K-configured 136-descriptor ceiling. Starting with 132
entries (the measured v267 full-workload peak), four further
insertions reach 136; the 137th rejects with descriptor
exhaustion without mutating the count; a middle deletion
reduces occupancy to 135 and reinsertion restores 136 with
the expected shifted-table content. Native fixture PASS.
This tests allocator mechanics on a synthetic registry, not
live 63-process swap/reclaim transactions. No production
code or 32K KCORE size change. Live high-concurrency swap
pressure is the next acceptance gate.

## 2026-10-25 256K extent high-water acceptance v267

Fresh 63-process 256K GCC PDP-6 SIMH test with execution
breakpoint at the native `mm_extent_insert` entry PASS.
151 entry-point count samples showed pre-insert maximum 131.
Assembler audit verifies the only successful insertion count
increment is inside that routine; the independently observed
v266 count 132 establishes exact first-boot workload peak 132.
Capacity 136 leaves 4 free at peak. This is not acceptance of
additional allocations during high concurrency or process
swap/reclaim transitions. No production memory instrumentation
or KCORE changes; 32K GCC lowmem remains measurement reference.

## 2026-10-25 native MM insertion breakpoint trace v267

The daimos-testkit 256K full-profile GCC native PDP-6
`daimos-mm-256k-highwater-v267.sh` creates a fresh image with
63 guest processes and installs a SIMH execution breakpoint
on the mapped native `mm_extent_insert` entry point. Its
actions EXAMINE `mm_extent_count` and resume, requiring
no permanent kernel code or RAM. The first complete test
PASSed all process start/completion markers and collected
151 insertion-entry count samples. Maximum pre-insertion
count was 131 decimal; adding one gives an insertion-related
ceiling of 132, in agreement with the independent v266
snapshot of 132 allocated of 136 descriptors. A breakpoint
records an attempted insertion, not necessarily successful
insertion, and this run is not a formal global high-water
bound for all workloads or swap/reclaim sequences. The
existing 136-entry policy remains unchanged. Measure KCORE
using the 32K-word GCC low-memory disk-root profile.

## 2026-10-25 PDP-6 MM occupancy observations v266

Native 256K GCC full-profile 63-process first-boot test PASS,
using SIMH EXPECT/EXAMINE for non-production MM observations.
At process 63's start, 132 of 136 descriptors were allocated;
at its completion, 8 remained allocated. The 136 capacity
was independently read from the same guest image. These two
point samples do not establish the lifetime high-water or
adequate reserve for simultaneous swapping/allocations.
Testkit v266 observation parser explicitly supports examination
text appended to guest console marker lines. KCORE source and
the 32K GCC size reference remain unchanged. Next: test
allocation overlap / reclaim behavior and measure peak count.

## 2026-10-25 full 256K numbered process range v265

daimos-testkit `daimos-mm-256k-concurrency-v265.sh` creates
fresh GCC full-profile 256K PDP-6 images with 48 and 63
bootstrap guest processes. Both configurations PASSed:
all numbered start and completion markers appeared, with
private-state corruption reported as a separate error token.
The 64-entry process table reserves slot 0, thus all 63
usable numbered slots have now passed first-boot execution.
The marker protocol uses unambiguous hex slot tokens to
avoid collision with legitimate completions. This is not
an MM high-water, repeated fork/exit/reuse, or swap-pressure
acceptance result. No production code changed. The size
reference remains 32K GCC lowmem disk-root.

## 2026-10-24 PDP-6 256K thirty-two-process concurrency v264

Extended the fresh-image 256K GCC process-private state test
to 32 guest processes. Initial host acceptance accidentally
treated the legitimate slot-24 completion marker 'X' as a
failure marker; the fixture was corrected to reserve '@'
for private-state failure. Rebuilt and reran against new
filesystem media: PASS, all 32 start and completion markers
observed without process-private state corruption.
This does not measure maximum concurrent residency, peak
MM extents, or swap pressure. Capacity policy remains 64
table entries with slot zero reserved (63 numbered slots).
No DAIMOS production source modifications.

## 2026-10-24 PDP-6 256K sixteen-process concurrency v263

New daimos-testkit 256K full-profile GCC first-boot fixture
`daimos-mm-256k-concurrency-v263.sh` runs 16 independent
INIT images. The guest verifies process-private state across
CPU work; the host checks distinct start/completion markers.
Fresh native SIMH PASSed all 16 starts and completions.
This is stronger than v262's five-user acceptance, but is not
an extent high-water measurement or evidence of 32/63 process
operation. Process slot zero is reserved, so a 64-entry table
has at most 63 numbered slots. No production code changed.
The canonical size comparison remains 32K GCC lowmem.

## 2026-10-24 256K MM first-boot acceptance v262

The fresh-image PDP-6 full-profile 256K GCC scheduler fixture
`daimos-mm-256k-scheduler-v262.sh` PASSed timer preemption,
private process context, nice ABI and disk sleep/wakeup. The
configuration explicitly uses `BOOT_PROFILE=full`,
`BOOT_CORE_WORDS=01000000` and `SIMH_CPU_KWORDS=256`, selecting
64 process slots and 136 MM descriptors (272 descriptor words).
This validates basic 256K first-boot and five-user operation,
not simultaneous execution of 64 processes or a high-water
extent count. There is no new production code or claimed RAM
savings in v262. Continue measuring KCORE using the separate
32K-word GCC lowmem disk-root configuration.

## 2026-10-24 RAM-sized MM extent registry v261

Restored dynamic KINIT selection of MM descriptors based on
two per boot-selected process slot plus eight shared/transient
entries: 32K=24 extents, 64K=40, 128K=72, 256K=136.
KINIT reserves exactly `2 * mm_extent_capacity` words just
above the fixed KCORE symbol; the allocator/VM continue to
use the direct-address table. Native PDP-6 24/136 descriptor
boundary tests and the fresh five-process scheduler boot PASS.
The mandatory GCC 32K-profile link reports KCORE=10742 and
KCORE+MRES=13605 words; add the 48-word external table when
comparing real physical usage: 10790 and 13653 respectively,
both 15 words below the old fixed-table baseline. 256K full
workload/peak acceptance remains to be measured.

## 2026-10-24 KINIT external MM backing v260

First stage of restoring boot-sized MM extent descriptors:
`mm_extents` is a zero-sized symbol at `__kcore_low_end`,
with 64 backing words reserved and cleared by KINIT before
low free arenas are published. MRES begins after this region.
Native capacity comparisons use `mm_extent_capacity`, but it
is reset to 32 at boot; larger memory-dependent capacities
are **not yet enabled**. v249/v250 native PDP-6 extent tests
PASS; v260 map/boundary test PASS; lowmem GCC 32K-profile
image build PASS. KCORE link 024766 octal (10742 words),
but KCORE plus 64 descriptor words plus runtime-capacity word
totals 10806 words, one more than the previous 10805-word
fixed table. Full system boot and dynamic-capacity acceptance
are still pending. Do not equate KCORE label shrink with RAM
savings.

## 2026-10-24 measurement convention v259

All reported KCORE resident-size measurements must use the
32K-word GCC low-memory disk-root configuration explicitly:
`BOOT_PROFILE=lowmem ROOT=disk COMPILER=gcc BOOT_CORE_WORDS=0100000
SIMH_CPU_KWORDS=32`. This applies even while a complete 32K
runtime boot remains unsupported. Distinguish a successful
32K-profile link/size measurement from a passing 32K boot.
The v258 lowmem size report already used these 32K defaults:
KCORE 025065 octal (10805 words), KCORE+MRES 032544 octal
(13668 words), PERMANENT_LAST 032623 octal (13715 words).

## 2026-10-24 process-slot policy v258

Implemented the agreed 8K-granularity process-slot policy in
`proc_slots_for_core`: eight slots through 32K, plus two for
each completed additional 8K, capped at 64 from 256K upward.
`PROC_MIN_SLOTS` is now eight so packed/storage validators
accept valid small-system tables. Testkit
`daimos-proc-slot-policy-v258.sh` passes all 8K boundaries,
partial-bank rounding, and the upper cap. GCC lowmem disk
permanent-size link PASS: KCORE+MRES 032544 octal (13668 words),
PERMANENT_LAST 032623 octal (13715). No change to the
separate fixed 32-entry MM extent registry yet.

## 2026-10-24 native first-boot MM observation v257

Added daimos-testkit v257 first-boot PDP-6 scheduler fixture
(testkit commit 32dfb7e) with mapped MM extent-count
examination and standard scheduler acceptance checks.
Two fresh GCC-profile runs passed scheduler assertions.
The second explicitly reported six extents at one checkpoint
(octal count 000000000006 at address 024537).
This is a point sample, not a workload high-water or a safe
smaller-capacity finding. No DAIMOS production changes;
MM registry remains 32 entries/64 words.

## 2026-10-24 MM SIMH replay diagnostic correction v256

An MM replay helper erroneously chose a stale fixture via
lexicographic path sorting. Correct modification-time selection
of the latest October 24 image still produced `?RT`.
KINIT_ERR_RT identifies root mount/validation or missing
/SYSTEM/INIT, not failed scheduler preemption. The official
one-shot scheduler test passes, but reusing its executed media
does not establish MM occupancy. Future trace must attach to
first boot of a fresh isolated build. No production changes,
and the 32-entry extent limit remains unchanged.

## 2026-10-24 fresh-image MM insertion trace v255

Official five-user PDP-6 SIMH scheduler regression PASSes again
with GCC. A retained freshly linked image puts mm_extent_count
at octal 023543 and mm_extent_insert at 001167. Separate
one-shot-breakpoint manual replay of that image emitted `?RT`
before the breakpoint, indicating an unresolved difference
between manual and official simulator launch conditions.
No reliable MM peak measurement, production change, or registry
reduction. Maintain the 32-entry capacity.

## 2026-10-24 scheduler acceptance restored v254

Official `daimos-testkit test-daimos-scheduler-multiproc`
PASSed in native PDP-6 SIMH with `MAKEFLAGS=COMPILER=gcc`,
covering timer preemption, process context, nice ABI and disk
sleep/wakeup. The prior v253 ad hoc replay's `?RT` is not a
reproduced failure of the official acceptance path. Its root
cause remains unproven. Still no validated MM extent high-water;
retain all 32 descriptors. No production changes.

## 2026-10-24 unstepped scheduler validation v253

An unstepped replay of the retained GCC five-process SIMH
image booted but ended with guest output `?RT`, not the
complete scheduler markers required by the test. Thus the
v252 diagnostic run's failure is not solely an EXPECT-stepping
artifact. MM has only three unvalidated-for-peak observations
of 16 live extents; no high-water conclusion. Preserve 32
descriptors, current KCORE baseline and strict image limits.

## 2026-10-24 GCC-profile MM occupancy snapshots v252

The supported COMPILER=gcc lowmem profile cleared the
unchanged DSH 036000-octal gate and booted the isolated
five-process fixture. SIMH diagnostic examinations of
mm_extent_count (address 024537 octal) returned 020 octal
(16 decimal) at three scheduler checkpoints. The stepped
fixture subsequently failed its completion assertion, so
these are only point observations, not a validated high-water
measurement or passing scheduler test. Temporary fixture
removed, no production changes. Keep the MM registry at 32.

## 2026-10-24 runtime extent occupancy attempt v251

Non-invasive mm_extent_count observation using a versioned
five-process PDP-6 scheduler fixture did not reach SIMH:
the isolated image build stopped at the unchanged DSH EXEC size
gate, 036013 octal versus limit 036000. No runtime occupancy
or high-water measurements were obtained. The experimental
testkit script was discarded, pre-existing unrelated changes
preserved, and no DAIMOS production changes were made.
Keep the 32-entry/64-word registry until real measurements exist.

## 2026-10-24 MM extent shifting validation v250

Native daimos-testkit fixture v250 (testkit commit 580e978)
PASSes full 32-entry MM registry insertion/rejection and
middle-slot deletion/reinsertion with displaced descriptor
verification. No real workload extent high-water measurement
or capacity reduction is claimed; KCORE remains 025065 octal.

## 2026-10-24 native MM extent boundary test v249

daimos-testkit commit 11dc124 adds a native PDP-6 test based on
production MM insertion/deletion. PASS: 32 slots filled, 33rd
returns MM_ERR_DESCRIPTORS (-3), deletion enables refill. It is
not workload high-water evidence. No production code change,
and KCORE baseline remains 025065 octal (10805 decimal).

## 2026-10-24 MonitorFS lookup and classifier follow-up v231

Audited MonitorFS lookup, readdir and STAT after the successful v230
device-type sharing. An alternative character-default device classifier
saved only one post-link word (`025114` versus `025115`) but added common
character-device path work; it was rejected and reverted. A restored
GCC lowmem disk-root permanent-size build confirms KCORE=025115 octal
(10829 decimal), KCORE+MRES=13692. No production changes or new runtime
acceptance are claimed for v231; previous v230 device-view native pass
remains applicable. Continue proposal B at larger file/VFS boundaries.

## 2026-10-24 KCORE MonitorFS classification sharing v230

`mfsdev_stat` now reuses `mfsdev_io_type` already called by READDIR to
classify character, block and mount-source devices. A GCC lowmem disk-root
permanent-size link passed: KCORE `025115` octal (10829 decimal),
KCORE+MRES 13692 and PERMANENT_LAST 13739 decimal, net saving 10 words
versus v229. `daimos-testkit` native SIMH MonitorFS device view-online
test PASSed using `COMPILER=gcc` and the unchanged DSH limit. Dedicated
STAT/READDIR classification combinations remain to be checked. No extra
permanent data or provider indirection was introduced.

## 2026-10-24 KCORE A scan audit and B entry v229

Proposal A's process table scans were reviewed after the seven-word-growth
shared-cleanup experiment was rejected. Live-user, session/pgrp, trailing
FREE slot, and intrusive runnable-queue scans have different contracts;
no generic iterator or new resident state is justified. No production
change and no additional size saving in v229. Next source-level shrink
focus is the file/VFS/MonitorFS boundary (proposal B). Available compiled
object section measurements: file_runtime 1175 text; monitorfs_runtime
1565 text + 39 data; vfs 778 text + 27 data + 128 BSS (pre-fold; not
recoverable savings estimates). Native v227 process lifecycle and rollback
acceptance remains valid for the unchanged production source.

## 2026-10-24 process teardown factoring rejected v228

An experimental shared `proc_cleanup_after_vm` service for EXIT and RUN
discard was measured using the production GCC lowmem disk-root link.
KCORE grew from v226/v227 `025127` (10839 decimal) to `025136` (10846),
seven resident words; KCORE+MRES grew 13702 -> 13709. The change was
reverted rather than trading RAM for a source-level abstraction. No native
test was run for the rejected prototype. The supported v227 implementation
and its native lifecycle and rollback passes remain the accepted baseline.

## 2026-10-24 native GCC lifecycle acceptance v227

The recurring native process-fixture blocker was the default KCC-produced
DSH exceeding production `036000`, not a process-runtime failure. Using
the already supported `COMPILER=gcc` policy through `MAKEFLAGS=COMPILER=gcc`
without relaxing the gate, daimos-testkit native SIMH passes both:
`test-daimos-run-late-rollback` (16 post-VM descriptor-map failures, stable
process accounting and recovery) and `test-daimos-process-lifecycle`
(RUN, GETPID, WAIT/nohang, zombie reap, orphan adoption, explicit fd maps).
These exercise the new v226 discard implementation. No production source
changed at v227, no new KCORE saving is claimed; RT-owner fault injection
and full VM teardown fault coverage remain open.

## 2026-10-24 KCORE discard assembler prototype v226

Replaced KCC-generated `proc_slot_discard` in the PDP-6 KCORE build with
an ABI-compatible assembler implementation; the C fallback is guarded by
`PROC_SLOT_DISCARD_PDP6_ASM`. No change to normal EXIT publication, MM
ownership, u-area lifetime or on-error transition clearing was intended.
Lowmem disk-root permanent-size link PASS: KCORE 025127 octal (10839
decimal), KCORE+MRES 13702 and PERMANENT_LAST 13749 decimal; two words
saved against v223. No extra permanent data. Targeted native RUN late
rollback fixture could not start because DSH 036013 exceeds the unchanged
036000 EXEC limit. This implementation remains **native-unaccepted** until
rollback and process lifecycle tests pass on a valid fixture. See KCORE.MD.

## 2026-10-24 KCORE A teardown overlap audit v225

Identified a concrete source-level consolidation candidate: production
KCC-generated `proc_slot_discard` and handwritten `proc_finish_slot` both
perform RT ownership release, VM destruction, TTY/u-area release, run-queue
removal and slot trimming. They differ in FREE handling, session/HUP
semantics, child adoption and FREE versus ZOMB publication; their distinct
rollback/ownership contracts must survive any factoring. The inspected
generated discard body is about 50 instructions gross, not a predicted
post-fold saving. No production source changed and no new native run passed.
KCORE.MD records the gated assembler/common-teardown experiment.

## 2026-10-24 KCORE proposal A cleanup semantics v225

Compared normal/fatal `proc_finish_slot`, failed-creation
`proc_slot_discard` and remote fatal-event file closure. Their shared
VM/u-area/runq calls have different transaction requirements, so no
unproven shared cleanup wrapper was introduced. Existing daimos-testkit
host `proc-abort-transition-v40` PASSed with `CPATH` set to DAIMOS's
`tools/host` header directory, proving transition-flag hold, destroy
failure rollback, retry and slot reuse in the C path. PDP-6 native
assembler cleanup acceptance remains outstanding; there is no new size
measurement or production code change in this checkpoint.

## 2026-10-24 KCORE lifecycle/RT policy audit v224

Compared `proc_finish_slot` in PDP-6 assembler and the C fallback, plus
`proc_slot_discard` and fatal-event termination. Early RT-owner release
precedes VM teardown in both implementations; code review alone does not
establish a rollback bug. No production semantics or source were changed.
Attempted native `test-daimos-process-lifecycle` but the fixture build
stopped before execution because DSH `036013` exceeds EXEC gate `036000`.
Keep the production size gate unchanged and native acceptance open. See
KCORE.MD for the next source-level consolidation candidates.

## 2026-10-24 KCORE process-finish RT ownership v223

`proc_finish_slot` now uses `CAMN/SETZM` rather than
`MOVE/CAME/JRST/SETZM` for conditional RT owner cleanup. The GCC lowmem
disk-root permanent-size link passed: KCORE=025131 octal (10841 decimal),
KCORE+MRES=13704 decimal and PERMANENT_LAST=13751 decimal. This saves two
resident words relative to v222. No new permanent data or process ABI
change. Runtime termination, RT ownership and native hierarchy tests have
not been reaccepted at v223; v222's strict DSH fixture size blocker
remains relevant. Process lifecycle/TTY/teardown architectural audit
continues without collapsing distinct rollback and ownership contracts.

## 2026-10-24 KCORE process-adoption branch reduction v222

In `proc_adopt_children_pdp6` the INIT special case now uses one `CAIN`
branch, removing one unconditional branch word. A lowmem disk-root
`make permanent-size` build completed with KCORE `025133` octal (10843
decimal), a one-word reduction relative to the recorded `025134` profile;
KCORE+MRES = 13706 decimal words for this build. A targeted native
daimos-testkit process-hierarchy run was attempted but its fixture failed
before execution: DSH image `036013` exceeds production gate `036000`.
No hierarchy runtime pass is claimed and the gate was not changed.

## 2026-10-24 KCORE architectural reduction A inventory v221

KCORE.MD now records the ordered architectural shrinking candidates A-F.
Additional JRST folding is explicitly last-effort work, after structural
source reductions, still subject to the existing >20-net-word acceptance.
Proposal A has begun with a source/map inventory of proc_pdp6.s: it mixes
scheduler/PI/context-switch code with wait/reap, process-control, event,
session, and TTY handlers. Available pre-fold object sections are 2164
text + 3 data + 3 BSS words, not a fresh full-link measurement and not all
scheduler. No production code or test assertion changed; no shrink gain is
claimed. Next is an accurate external/indirect dependency graph and one
measured internal consolidation, preserving native process/TTY semantics.

## 2026-10-24 KCORE source/TODO reconciliation v220

Audited KCORE.MD's active backlog against the v219 DAIMOS tree. Verified
the three-word process ABI and boot slot tiers in proc/proc.h/proc_boot.c,
32 static MM extents in mm/mm.h/mm_pdp6.s, and eight-word pipe storage in
ipc/pipe.h. Confirmed EXEC/RUN's resident assembly implementations and that
vm_space_startup still writes to user VM even with empty argument lists.
MEMFS private XIP admission op 027 and VM release op 026 are implemented,
but production EXEC/RUN does not use XIP admission. The restricted raw XIP
linker producer is experimental; native fixtures do not constitute a full
scheduled-process XIP acceptance. Updated KCORE.MD and stale TODO.MD
checkboxes accordingly. No production source or ABI changes.

The most recent recorded GCC lowmem KCORE exclusive endpoint is 025134
octal (10844 words, v213); strict below-020000 would require at least
2653 words of reduction at that profile. The separate full EXEC/RUN profile
reported 025657 octal (11183 words, v205). Neither endpoint was remeasured
at v219 during this documentation-only audit. The strict 32K acceptance
remains deferred and unpassed. The outstanding packed-process, D6 codec,
folding, and size-profiling tasks remain open, not silently promoted.

## 2026-10-23 movable MEMFS XIP admission gateway v219

The existing MEMFS provider register dispatch now accepts private op 027:
AC1=vnode, AC2=&base, AC3=&entry. It calls `memfs_xip_admit` wholly inside
MRES, which validates the raw PURE header and owns pin failure/rollback.
This is the symmetric counterpart of VM XIP release op 026. No new kernel
entry vector, process state or permanent data is required. Native PDP-6
SIMH and AAP MEMFS/XIP lifecycle tests call the real dispatch and PASS.
Normal EXEC/RUN has not yet been changed to request this service, so raw
XIP remains unlaunchable by ordinary users. Privileged, argument-free
admission and no-write startup are still needed before runtime enablement.

## 2026-10-23 experimental linker XIP producer v218

Host `dlink --xip --pure -b 020` now creates a distinct 04000-word RAW
MEMFS image using the existing text-link layout and relocation engine.
Word 0 holds XIP magic, word 1 holds `PURE,,020`, 2..017 are zero, text
begins at logical 020 and the remainder is zero-padded. The extra padding
forces an aligned dedicated MEMFS extent under its existing allocator.
For this experimental generator, accepted images have **no DATA or BSS**
and instruction text is restricted to unindexed, non-indirect `MOVEI`
instructions followed by one `SYS_EXIT` (monitor UUO 040). All other
instructions/syscalls are rejected; this is stricter than the normal DXR
PURE heuristic, which alone cannot prove absence of user stores. Normal
DXR/PURE output is unchanged. Negative/format host tests live in
daimos-testkit. Production EXEC/RUN **does not accept raw XIP yet**:
`vm_space_startup` still writes into the user image. No KCORE change.

## 2026-10-23 reuse DXR PURE for raw MEMFS XIP admission v217

Simplified the experimental XIP image contract to reuse the existing DXR
`PURE` flag (octal 0200000) rather than invent a new immutable-image bit.
The raw image remains a separately laid-out format: word 0 is XIP magic,
and word 1 is `PURE_FLAG,,entry` (LH flag, RH logical entry). MEMFS admission
requires exactly this flag in addition to the existing execute/no-write
file modes and physical alignment. A missing PURE flag must reject without
retaining a pin. This is software metadata only, NOT hardware protection.
Ordinary DXR PURE images remain unchanged and are not automatically XIP.
No EXEC/RUN/VM/KCORE change was made. A privileged loader path with genuine
user-mode no-write constraints remains an open integration task.

## 2026-10-23 MEMFS non-writing XIP user lifecycle v216

daimos-testkit adds `target-memfs-xip-user-lifecycle-v216`. A distinct
non-writable/executable MEMFS raw image is admitted and pinned, then entered
at logical `020` under the PDP-6 APR. The user image executes only `MOVEI`
and a monitor UUO (`040`), without stack or other user-memory stores.
The hardware UUO invokes an executive-only handler, which checks both user
instructions are unchanged and calls the unchanged production
`vm_space_destroy` body. The test requires VM state to be cleared, MM pin
released, and the original MEMFS extent to become reclaimable.

Native SIMH and AAP PDP-6 both PASS. The testkit uses scheduler/storage
stubs and is **not** a fully scheduled process or production EXEC/RUN.
For SIMH, the dedicated runner omits p10run's unconditional `break 20`,
which otherwise intercepts this legitimate user entry before relocation.
This changes no DAIMOS production code or per-process RAM; the last measured
permanent KCORE remains 025134 octal (10844 words).

## 2026-10-23 production VM destroy native XIP rollback v215

daimos-testkit now extracts the unchanged `vm_space_destroy` body from
`system/kernel/proc/vm_pdp6.s` and links it into the native MEMFS fixture.
The fixture uses a real two-half VM descriptor and injects `MM_ERR_BUSY` on
the final XIP unpin. First destruction fails with VM state unchanged and
the pinned MEMFS extent still owned; retry succeeds and clears VM state.
Both SIMH and AAP PDP-6 PASS. Scheduler/storage mode expectations and swap
detach are focused stubs, not a full native process exit. There were no
DAIMOS production changes or KCORE growth in v215. Real process EXEC/exit
and non-writing startup remain open.

## 2026-10-23 XIP release failure injection v214

The daimos-testkit native MEMFS regression now fault-injects `MM_ERR_BUSY`
from `mm_unpin` during private MEMFS XIP release op 026. It verifies the
dispatch reports failure (-1), leaves the MEMFS pin/reference intact, and
permits normal successful release after the injected failure is removed.
Both SIMH and AAP PDP-6 tests PASS. No DAIMOS production source or KCORE
change was necessary. A full process-VM exit/EXEC rollback test remains open.

## 2026-10-23 MEMFS XIP VM teardown dispatch v213

`vm_space_destroy` now probes the existing MEMFS MRES register gateway with
private operation 026, passing the resident physical VM base. The MEMFS
provider uses the existing XIP base provenance and reverse-unpin functions:
result 1 means released; result 0 means ordinary/non-owned VM; -1 means
provider unavailable or release failure. In the latter two cases only the
normal `mm_free(MM_TYPE_PROCESS, owner)` is tried, which cannot free a
MEMFS-owned dynamic extent. No new per-process storage or provider vector.
An MRES-dispatch native test checks unknown, valid, and repeated releases on
SIMH and PDP6-AAP. GCC lowmem image PASS; linked KCORE is now 025134
octal (10844 words), a 9-word increase over v212's 025123.

The existing swap-out path checks `mm_is_pinned`; `vm_space_brk_current`
requires MM_TYPE_PROCESS for resizing. The separately trusted non-writing
startup and true process-XIP launch remain disabled; this commit does not
make arbitrary DXR images safe to map from MEMFS.

## 2026-10-23 MEMFS XIP base provenance v212

MEMFS MRES now exposes `memfs_xip_owns_base(base)` using its existing four
pinned-node reference words. It recognizes exact physical bases only while
an XIP reference is live, requires no extra per-process or persistent MEMFS
data, and becomes false after final unpin. The corresponding native tests
PASS on SIMH and PDP6-AAP. This supplies a prospective VM provenance query,
but **does not yet connect VM teardown or swap to MEMFS MRES**. EXEC/RUN
and resident VM remain unchanged and production XIP launches remain disabled.

## 2026-10-23 MEMFS XIP reverse ownership release v211

Added `memfs_xip_unpin_base(base)` inside MEMFS MRES. It finds the exact
MEMFS-owned physical base using the existing four-entry pinned-node table,
decrements a shared file reference, and releases the underlying MM pin only
on the last reference. Unknown bases return failure without changing pins.
Native SIMH and PDP6-AAP MEMFS target tests PASS. No new persistent MEMFS
reference-table words, no KCORE changes, and no production EXEC/RUN XIP path.
This is the primitive needed before the VM teardown path can distinguish
MEMFS-backed mappings from ordinary MM_TYPE_PROCESS allocations.

## 2026-10-23 MEMFS XIP admission hardening v210

`memfs_xip_admit` now checks mode bits before taking an image pin: at least
one execute bit must be present and all write bits must be absent. A valid
magic/header without safe file modes is rejected without pinning. The native
testkit confirms that both writable and non-executable modes are rejected;
mode 0555 is admitted. SIMH MEMFS and PDP6-AAP MEMFS/user-APR tests PASS.
This is only a file-admission restriction, not hardware write protection:
the PDP-6 can still store into admitted MEMFS physical memory in user mode.
No VM ownership, swap, startup or EXEC/RUN XIP path has been enabled.

## 2026-10-23 MEMFS user-mode APR XIP hardware test v209

daimos-testkit now provides `target-memfs-xip-user-apr-v209`. The test first
admits and pins a separately marked raw MEMFS XIP file; it then programs the
PDP-6 APR relocation base and enters user mode at logical 020 directly into
that file's words. The test user code intentionally executes `MOVEM 2,030`,
and AAP confirms the marker at the corresponding MEMFS physical word and
the correct pinned file base. **AAP PASS.** The native PDP-6 image is built
using p10run, but a completed SIMH user-mode run is not claimed.

This is intentionally a test-only *writable* example. It demonstrates that
the PDP-6 APR cannot prevent writes to mapped MEMFS backing; it does not
prove that ordinary user applications are safe to share or that process
teardown, swapping, and non-writing startup are integrated. EXEC/RUN, KCORE,
and production MEMFS source are unchanged by v209.

## 2026-10-23 MEMFS XIP native execute probe v208

The daimos-testkit MEMFS fixture now uses executive-mode PDP-6 `XCT` to
execute a single instruction at `admitted MEMFS physical base + entry` while
holding the file's pin. Both SIMH and PDP6-AAP PASS and observe the expected
AC result. This verifies native instruction fetch directly from MEMFS-owned
backing without a copied process extent. It does **not** exercise user APR
relocation, startup, process scheduling, swapping, or EXEC/RUN integration.
No production KCORE or EXEC/RUN code changed for v208.

## 2026-10-23 XIP MRES image admission v207

Implemented `memfs_xip_admit` in the separately loadable MEMFS MRES. It
recognizes an explicit raw-image header and entry address, pins backing while
validating, and unpins on rejection; ordinary DXR is never auto-admitted.
Native MEMFS target tests PASS on SIMH and AAP PDP-6. No XIP process launch
is implemented; `vm_space_startup` and MM_TYPE_PROCESS teardown still make
transparent substitution unsafe. No EXEC/RUN or KCORE change in this patch.

## 2026-10-23 post-EXEC-optimization XIP integration audit v206

The optimized `exec_load_process` calls `vm_space_create` followed by
`vm_space_load_file`, `proc_swap_attach`, and the RUN/EXEC commit path calls
`vm_space_startup`. That routine unconditionally writes argc/envc metadata
through the translated VM base (`movem 3,(11)`); it is not safe to replace
the process's MM_TYPE_PROCESS extent with immutable/shared MEMFS backing.
`vm_space_destroy` still unconditionally frees a resident base under
MM_TYPE_PROCESS ownership, and normal swapping assumes resident process
storage. Therefore **no XIP executable is admitted** yet, despite the
completed MEMFS alignment/pinning support. Do not add an unconditional
MEMFS-base substitution to `vm_space_load_file`.

Post-optimization GCC lowmem disk `permanent-size` check PASS:
KCORE 025123 octal (10835 words); KCORE+MRES 032602 octal (13698 words);
PERMANENT_LAST 032661 octal (13745 words). No XIP loader code was added.

The supported next implementation is a privilege-gated, separately marked
non-writing executable ABI, a mapping/teardown provenance mechanism and an
XIP-specific startup path that does not write into backing storage, followed
by testkit native execution on SIMH and AAP. The public EXEC/RUN syscall ABI
need not change, but VM/startup/swap behavior must change before launch.

## 2026-10-23 EXEC UNCOMPRESSED STAGED-VM OOM REGRESSION V205

The outstanding v205 testkit pressure experiment now PASSes natively
on the 256K PDP-6. The test builds an uncompressed, valid DXR of
0100000 octal image words plus 077000 BSS words. The current PID 1
image first grows to 0600000 words, so staging both resident VMs
cannot fit in physical core. EXEC succeeds through the existing
destructive low-memory retry, and the replacement checks PID 1 and
continues execution. The initial failing run before the v201 resident
36-bit ENOMEM comparison correction returned `<H!x`; a corrected
kernel had already passed an early v205 run. A later test-only failure
was caused by expecting `dxrcheck` to print a leading zero in the
octal image word count; the output is `image=100000 bss=077000`.
The v205 fixture now accepts the tool's actual octal formatting while
checking both image and BSS size fields.

Fresh native proof: `$TMPDIR/daimos-exec-create-oom-v205-accepted.log`:
`PASS (staged VM cannot fit physically; low-memory EXEC replaced PID 1)`.
`__kcore_low_end=025657` octal. No additional DAIMOS kernel change,
permanent state, or stack growth. The 32K gate stays deferred, and
the independent full-profile permanent-memory acceptance limit remains
open.

## 2026-10-23 EXEC LOW-MEMORY RETRY CORRECTION V201

The native 256K PDP-6 v201 EXEC pressure regression identified a real
full-word error-comparison bug in resident `exec_replace_current`:
`CAIE 1,EXEC_LOAD_NOMEM` compares against an 18-bit immediate and
cannot recognize the 36-bit `-2` returned by `exec_load_process` when
staged VM allocation fails.  Replace it with `CAME 1,[-2]` so the
existing destructive low-memory EXEC retry is actually invoked.

The previously failing compressed/padded large executable now replaces
PID 1 with physical pressure forcing the staged allocation to fail;
native v201 PASS (`$TMPDIR/daimos-exec-pressure-v201-fullcompare-test.log`).
The same large executable also PASSes without pressure
(`$TMPDIR/daimos-exec-pressure-control-v201-afterfix.log`).  The
independent v200 eight-case PDP-6 instruction-level loader/commit
rollback fixture PASSes after this correction
(`$TMPDIR/daimos-exec-commit-rollback-v201-retest.log`).
No persistent kernel data or added stack words. A fresh full-profile
v201 link map confirms `__kcore_low_end=025657` octal, unchanged from
the pre-fix full-profile KCC/p10fold baseline. This is *not* an
acceptance of the separate 16K permanent-memory, default DSH image
size or 15-second runtime gates. The 32K-specific gate remains
deferred by explicit direction.

## 2026-10-23 MEMFS XIP v204: aligned allocation failure contract

The native testkit MEMFS fixture now injects `MM_ERR_FRAGMENTED` for an
aligned large allocation and confirms there is no fallback to an unaligned
extent, published data pointer, logical quota change, or stale XIP pin.
The subsequent aligned retry succeeds. SIMH and AAP PDP-6 both PASS.
This is a deterministic failure-injection test, not a realistic MM stress
measurement. The production XIP process launch remains disabled.

## 2026-10-23 MEMFS XIP aligned backing v203

Dedicated large MEMFS allocations use 02000-word alignment through MM.
Both SIMH and AAP MEMFS pinning regressions PASS. No XIP process launch
is enabled; executable admission and VM provenance remain open.

## 2026-10-23 MEMFS XIP protection milestone v202

An explicit MEMFS XIP reservation is now supported in the MEMFS MRES, without
enabling any XIP launch path. Four packed (slot+1),,reference-count words
represent the maximum four simultaneously dedicated MEMFS chunks; MM pinning
guards the underlying physical extents. The new API only pins regular files
on large dedicated 02000-aligned resident extents. Pinned file writes,
resizes/truncations, chunk evictions and unmount teardown are denied. Final
unpin releases the MM reference. This avoids additional permanent KCORE data;
MRES size effect has not been measured in a comparable post-fold baseline.
GCC lowmem image build and focused SIMH MEMFS/FIFO native tests PASS.
The identical MEMFS fixture PASSes on AAP PDP-6 as well. This is not
general XIP, and no native XIP process launch has been claimed.

## 2026-10-23 EXEC STAGED COMMIT FAILURE INJECTION V200

The new daimos-testkit `test-daimos-exec-commit-rollback` PASSes eight
instruction-level scenarios against the **actual resident**
`exec_replace_current` implementation extracted from
`system/kernel/proc/exec_lowmem_pdp6.s`; only its external
`exec_load_process`, `vm_space_startup` and `vm_space_destroy` call
boundaries are replaced by deterministic test doubles. Covered cases:
failed image loading, startup creation failure, failed destruction of
the old VM, successful PURE/backing commit with either fifth mount bit,
rollback of a newly claimed RT owner after startup failure, preservation
of an existing RT owner after startup failure, and releasing an existing
owner when a non-RT image commits. The old-VM-destroy failure also
asserts restoration of the user mapping hold. Each failed transaction
must restore the previous backing record/credential bits and return -1.

Evidence: `$TMPDIR/daimos-exec-commit-rollback-v200-final.log`;
testkit commit `de17627`. This is a PDP-6 instruction-level emulator
test of the production transaction with injected call results, **not**
a native full-kernel failure or a real failed `mm_free` invocation.
Successful RT-to-RT replacement is not in the eight-case matrix; its
experimental ninth scenario remained unaccepted and was excluded from
the committed regression. Native v191-v193 owner and decode failure
tests remain the independent real-kernel checks. The 32K gate is
deferred. No kernel code or resident RAM change was made; KCORE
exclusive end remains `025657` octal.

## 2026-10-23 NATIVE RUN CHILD-VM CREATION OUT-OF-MEMORY V199

The new daimos-testkit `test-daimos-run-create-oom` PASSed on a fresh
256K PDP-6 image. The fixture first verifies that a large, valid child
DXR can be RUN and reaped normally. The boot INIT/parent remains the
known-working v197 image; `/SYSTEM/EXEC/TSFSPROBE` is replaced only
in this test image by a separate executable with 077000 octal BSS
words (below the DXR loader's 077777 BSS limit). The child's complete
text/BSS/stack requires at least 0100000 physical words.

The parent then grows its break via bounded 02000-quantum search until
the remaining 256K core cannot accommodate the large child, checking
that the parent process-word allocation plus minimum child requirement
exceeds all installed core. Four otherwise VALID RUN V2 requests for
the already-proven child return -1. After each rejection, the parent's
break and writable sentinel, its live process slots and aggregate
process-word accounting remain unchanged. Shrinking the parent back to
the original break restores the original process accounting, and the
EXACT SAME child RUN/WAIT succeeds again. The fixture then also passes
v197's 16 late descriptor-map rollback failures with three blocked
children, normal reap and final valid RUN/WAIT recovery.

This specifically tests insufficient physical core at RUN's
`exec_load_process -> vm_space_create` stage, without adding kernel
fault switches or reducing memory limits. Early fixture trials failed
for test reasons: an oversized parent image stopped during INIT boot,
and a 0100000-octal BSS child exceeded the documented DXR 077777 limit.
The corrected separate-child fixture PASSed. Evidence:
`$TMPDIR/daimos-run-create-oom-v199-native-c.log`.

All v197/v198/v199 test sources compile with PDP-6 GCC/C99, and runner
shell syntax/diff checks pass. No DAIMOS code, kernel stack, KCORE or
MRES changes; linked KCORE end stays `025657` octal. A forced EXEC
replacement allocation failure and late-commit fault injection are
still separate open scenarios. Strict 32K remains deferred by request.

## 2026-10-23 PDP-6 VM ALLOCATION-FAILURE AND RUN RECOVERY V198

The versioned daimos-testkit v198 regression extends the v197 native
RUN pressure fixture, keeping three children alive while issuing four
valid 18-bit `BRK` requests for `0776000` words each. This is below
the 18-bit logical address ceiling, but the requested resident process
extent cannot fit alongside the kernel and running processes in 256K
physical memory. All four requests returned `-1` through the actual
VM growth/MM allocation failure path rather than a malformed-address
check. After each failure the original process break, own logical VM
word count, existing sentinel data and occupied slot count remained
intact. Resident process-word accounting never increased; it may
decrease when the reclaim path swaps a sleeping process. The fixture
then completed the existing 16 late RUN descriptor-mapping failures,
reaped the three children and launched/reaped one valid recovery child.

The native fresh-image PDP-6 test `test-daimos-run-vm-oom` PASSed;
evidence: `$TMPDIR/daimos-run-vm-oom-v198-native.log`. The baseline
v197 C99 fixture still target-compiles without the v198 extension.
No DAIMOS kernel changes or permanent RAM additions were needed;
post-p10fold `__kcore_low_end=025657` octal. This does *not* yet
force `vm_space_create` to return ENOMEM from an actual RUN or EXEC
loader, nor does it individually audit every transient MM descriptor.
The 32K gate stays deferred; strict full-profile residency and
production image-size/timing requirements are unchanged.

## 2026-10-23 RUN LATE-FDMAP ROLLBACK UNDER LIVE PRESSURE V197

The new native PDP-6 testkit `test-daimos-run-late-rollback` PASSed on
a fresh normal 256K image. While three children remained alive and
blocked on a pipe, sixteen RUN V2 requests passed launch validation,
executable loading, child VM creation, startup construction and u-area
allocation, then failed at the deliberately unavailable parent FD 15
in the final explicit descriptor map. All returned `-1`; each subsequent
`SYS_MEMINFO` sample retained the same process slot count and process
extent accounting as before the failures. After closing the pipe writer,
all three children exited and were reaped; process accounting returned
to the initial baseline. A further valid RUN and WAIT also succeeded.
This tests a late child-creation rollback route and recovery, not a
deliberately exhausted MM allocator or a complete dynamic-MM leak audit.

Testkit fixture: `tests/validation/daimos-run-late-rollback-v197.c`;
runner: `tests/validation/daimos-run-late-rollback-v197.sh`.
Evidence: `$TMPDIR/daimos-run-late-rollback-v197-native-b.log`.
The first diagnostic run failed on an overly strict initial
`process_slots_used == 1` fixture assumption; the corrected fixture
accepts the runtime's actual nonzero baseline and the fresh native
rerun PASSed. DAIMOS source and size are unchanged:
`__kcore_low_end=025657` octal. The 32K gate stays deferred.

## 2026-10-23 RUN PROCESS LIFECYCLE AND ID REUSE RETEST V196

Two existing daimos-testkit native PDP-6 tests PASSed on fresh 256K
images, independent of the deferred 32K boot/image constraints:

* `test-daimos-process-lifecycle`: RUN, GETPID, WAIT/nohang,
  zombie reaping, orphan adoption and explicit descriptor mapping.
* `test-daimos-process-id-lifetime`: process-group, session and domain
  IDs remain protected while referenced after the creator slot is reused.

Versioned logs: `$TMPDIR/daimos-run-lifecycle-v196-native.log` and
`$TMPDIR/daimos-run-id-lifetime-v196-native.log`. DSH functional-only
`DSH_IMAGE_LIMIT=037000` was exported without changing the production
default. No kernel modifications were required. Linked resident KCORE
exclusive end remains `025657` octal. The 32K-specific failures remain
deferred by request as documented separately in v196, not resolved.

## 2026-10-23 EXEC/RUN SCOPE: 32K GATE DEFERRED V196

By explicit project direction, pause **all strict 32K-specific sizing,
configuration-error and native boot acceptance work** while DSH remains
too large for that profile. Do not treat a 32K boot, LOGIN/DSH EXEC,
KINIT reserve or 8K KCORE gate failure as a current EXEC/RUN blocker;
do not relax the architectural limits or consume time engineering
32K-only workarounds. Continue checking resident RAM growth in normal
builds, and continue EXEC/RUN correctness on the 256K PDP-6 profile.

The last exploratory GCC lowmem comparisons (not acceptance) showed
ROOT=auto KINIT end `102552`, above the `076000` stack-safe bound by
`04552` octal (2410 words). ROOT=disk/BOOT=dsk built an image with
KINIT end `074435`, but its KCORE end `025203` still exceeded the
separate `020000` strict size target, and the native disk-root login
reported `LOGIN: EXEC FAILED` while boot, INIT and LOGIN ran. These
observations are *deferred*, not fixed or accepted. Logs:
`$TMPDIR/daimos-lowmem-v195-audit-build.log`,
`$TMPDIR/daimos-lowmem-v195-disk-build.log`, and
`$TMPDIR/daimos-lowmem-v195-disk-native.log`.

The full-profile KCORE/MRES permanent-memory limit is a separate
project-wide size issue. Do not mistake the lowmem deferral for a
claim that the full-profile strict memory gate has passed. The
non-32K EXEC/RUN functional regression program continues below.

## 2026-10-23 RT REQUIRED CHILD CROSS-REGRESSION V194

Following the v193 post-VM EXEC compressed decode failure checks,
the existing `make test-daimos-rt-required` testkit target PASSed with
a fresh native PDP-6 image. It covers compressed RT_REQUIRED child
admission, competing-owner rejection, and eventual release/reacquire.
Evidence: `$TMPDIR/daimos-rt-required-v193-regression.log`.
This verifies the RUN-created child path separately from the v193
same-process EXEC RT rollback tests. Kernel source, KCORE exclusive
end (`025657` octal), and full residency were unchanged. The image
was built with the existing functional `DSH_IMAGE_LIMIT=037000`; the
strict permanent-memory, executable-size and timing gates remain open.

## 2026-10-23 EXEC POST-VM-CREATION RT ROLLBACK V193

The native PDP-6 testkit real-time-owner fixture now additionally
constructs a validly framed compressed DXR2 executable with the
RT_REQUIRED bit, one output image word, a one-word D6LZ payload, and
one relocation word. Its zero control word requires an absent literal,
causing `d6lz36_decode_vfs` to fail after `vm_space_create` on the
normal 256K simulation profile. The test checks both the release of a
newly claimed RT reservation and preservation of an existing owner.
Both checks PASS; the earlier header-failure rollback, RT-required
successful EXEC, and subsequent ordinary EXEC release also PASS.
The stage markers are `<abcdRN` followed by HALT. Testkit commit
`dcaca8c`; native log `$TMPDIR/daimos-exec-rt-postvm-v193-native.log`.

No kernel changes were required after the v191 scratch-overlap fix.
The linked KCORE exclusive end remains `025657` octal. This is
functional-only validation (`DAIMOS_EXEC_OBSERVE_SECONDS=35`,
`DSH_IMAGE_LIMIT=037000`); strict 15-second and `036000` image gates,
native 32K acceptance, and full 16K permanent-memory accounting remain
open. Fault injection after successful startup/VM commit is not part
of this test.

## 2026-10-23 EXEC RT FIX FOLLOW-UP NEGATIVE REGRESSIONS V192

The v191 RT-owner scratch-lifetime correction also passed the native
v190 malformed-launch/ordinary-EXEC fixture when rebuilt against DAIMOS
commit `e2248c0`. All 28 malformed RUN and EXEC requests returned the
expected error; the original process then performed normal EXEC and
preserved its expected state. Evidence:
`$TMPDIR/daimos-exec-v192-malformed-retest.log`.

The tested RT-owner regression separately passed all four transaction
paths and subsequent re-acquisition in v191. No code, stack, KCORE or
MRES growth followed the defect correction (`__kcore_low_end=025657`
octal). Functional-only observation at 35 seconds and the `037000`
DSH cap were used. The strict 15-second timing, `036000` DSH cap,
16K fixed memory, and further post-VM-creation failure injection remain
open; none of those gates has been weakened.

## 2026-10-23 EXEC RT OWNER FLAG STACK OVERLAP FIX V191

Native testkit v191 discovered and reproduced an EXEC loader RT-owner
rollback defect: `exec_load_process` stored its new-RT-claim flag in
`-003(17)`, which is also the seventh/last word (`mtime`) of the
`struct vfs_stat` output at `-011(17)` through `-003(17)`.
Consequently, after a failed RT_REQUIRED EXEC, a process that owned the
reservation before EXEC could lose it despite never claiming a new one.
The v191 PDP-6 fixture verified RT ownership immediately before EXEC
and reproduced the erroneous release after a two-word valid-header,
incomplete-payload DXR failure.

`exec_load.s` now reuses the vnode output scratch at `-012(17)` for the
RT-claim flag **after** copying that vnode to AC12. The early null/path
lookup failures return directly without testing a not-yet-initialized
flag. Seven-word `vfs_stat` output no longer aliases the flag. No stack
growth or persistent allocation is required. KCC/p10fold link PASS:
`__kcore_low_end` remains `025657` octal; full KCORE 11135 words and
KCORE+MRES 18670 words are unchanged. Strict 16K gate still FAILs.

Testkit `test-daimos-exec-rt-owner` native fresh-image PDP-6 PASS:
new RT claim released on failing EXEC; existing owner preserved on a
failing EXEC; RT_REQUIRED successful EXEC acquires ownership; ordinary
replacement releases ownership; subsequent acquisition/release works.
Evidence: `$TMPDIR/daimos-exec-rt-owner-v191-fixed-native.log`.
This uses diagnostic `DAIMOS_EXEC_OBSERVE_SECONDS=35` and functional
`DSH_IMAGE_LIMIT=037000`; default timing and image-size gates remain
unmodified. Additional failure injection after VM creation and native
32K acceptance are still open.

## 2026-10-23 EXEC/RUN MALFORMED LAUNCH REJECTION V190

The native PDP-6 EXEC replacement test in daimos-testkit commit
`9bc72d6` now runs 28 explicit malformed EXEC V1 and RUN V2 launch
probes before its successful replacement path. All 28 invalid calls
returned `-1`, the original process stayed alive, and the subsequent
EXEC replacement passed with preserved PID, credentials, cwd, session,
process group, TTY and descriptors. Cases include incorrect header
version/word count, nonzero upper halves and excessive scalar counts,
invalid counted-SIXBIT path/argv lengths, truncated records and
inconsistent block endings. The RUN probes never submit a valid child
request, avoiding incidental process creation.

The GCC cross-compiled test source passed; the fresh-image PDP-6 native
runner also PASSed. Evidence:
`$TMPDIR/daimos-exec-malformed-v190-native.log`. This is a functional
diagnostic with `DAIMOS_EXEC_OBSERVE_SECONDS=35` and
`DSH_IMAGE_LIMIT=037000`, not strict 15-second/`036000` acceptance.
Dedicated RT owner admission/release/rollback and native 32K testing
remain open. No DAIMOS kernel source or resident-memory change was
needed; linked KCORE exclusive endpoint remains `025657` octal.

## 2026-10-23 EXEC V188 NATIVE REGRESSIONS V189

Both supported fresh-image PDP-6 EXEC testkit targets PASS on the
v188 source revision `6b9ce2a`: current-image replacement preserving
PID, credentials, cwd, session, process group, TTY and descriptors;
and session-leader child EXEC with successful parent reap. Logs:
`$TMPDIR/daimos-exec-v188-replace.log` and
`$TMPDIR/daimos-exec-v188-child.log`.

The same explicit diagnostic settings were used as v186:
`DAIMOS_EXEC_OBSERVE_SECONDS=35`, `DSH_IMAGE_LIMIT=037000`.
This validates ordinary EXEC and RUN/child-EXEC flows after the v188
record-start comparison and low-memory RT owner check. It is not
acceptance of the strict 15-second deadline or `036000` image cap;
malformed argument blocks and real-time-owner rollback remain untested.
Resident KCORE exclusive end stays `025657` octal; permanent-size
figures remain as recorded in v188.

## 2026-10-23 KCORE EXEC SIZE AND FULL PERMANENT ACCOUNTING V188

KCC/p10fold linked resident EXEC v188 saves two further KCORE words:
`__kcore_low_end` `025661` -> `025657` octal. The latest full
`make -C system/boot/pdp6 permanent-size` run (default full/auto
KCC configuration) reports KCORE 11135 words, KCORE+MRES 18670 words,
PERMANENT_LAST `044435`, MULTI_LAST `044476`. Both the single-root
and multi-root strict `040000` permanent-address tests FAIL (report
distances -2333 and -2366 addresses respectively). This is a current
verified size failure, not a successful 16K target. The full report
is `$TMPDIR/daimos-permanent-v188b.log`.

The EXEC record-start comparison and low-memory RT cleanup changes
need native replacement, child-session and malformed-record regression
coverage. Previously validated v186 native tests are not v188 tests.

## 2026-10-23 EXEC/RUN POST-OPTIMIZATION REGRESSIONS V187

Current resident EXEC/RUN optimizations through v186 have KCC/p10fold
link acceptance and two passing fresh-image native PDP-6 diagnostics:
EXEC replacement (preserved PID, credentials, cwd, session, pgrp, TTY,
descriptors) and session-leader child EXEC with successful parent reap.
The preceding v185 EXEC replacement run also passed. Logs:
`$TMPDIR/daimos-exec-v185-replace.log`,
`$TMPDIR/daimos-exec-v186-replace.log`, and
`$TMPDIR/daimos-exec-v186-child.log`.

These tests used `DAIMOS_EXEC_OBSERVE_SECONDS=35` and the existing
functional-only `DSH_IMAGE_LIMIT=037000` without changing the default
15-second or `036000` production gates. The v186 post-p10fold linked
KCORE exclusive end is `025661` octal (28 words below `025715` at v173).
Strict acceptance, malformed-input stress, and targeted real-time-owner
handoff/rollback testing remain open.

## 2026-10-23 EXEC/RUN NATIVE DIAGNOSTIC ACCEPTANCE V183

After the v172-v181 resident EXEC/RUN instruction reductions, the
existing native PDP-6 EXEC-replacement and child-session EXEC tests both
PASS on fresh images using an explicit 35-second observation window and
the functional-only `DSH_IMAGE_LIMIT=037000`. The ABI smoke test also
PASSes after daimos-testkit v182 corrected its stale `proc/exec.c`
reference to the live assembler `exec_replace_current` and
`exec_load_process` entry points. Testkit v183 adds an opt-in
`DAIMOS_EXEC_OBSERVE_SECONDS` parameter; its default remains 15 seconds.
Evidence: `$TMPDIR/daimos-exec-v183-observe.log` and
`$TMPDIR/daimos-exec-child-v183-observe.log`.

This is **functional-only runtime acceptance**, not the strict production
EXEC image-size or 15-second timing gate. The default `036000` image
limit still rejects an oversized generated executable (`036013`), and
the default 15-second simulation test timed out. No production limits
were changed. Current same-profile KCORE exclusive endpoint after v181:
`025671` octal. Further low-memory and malformed-input coverage remains open.

## 2026-10-22 DECtape/TSFS GROUP 027 RETEST V169

Group 027's supported boot-root-only test inventory has seven
independently passing checks on the installed PDP-6 toolchain:
counted DECtape block-stream I/O, userspace-created TSFS images
read through the production Type-551 driver, eight-member scanning
and KINIT boot selection, cross-member extent reads, D6LZ restart
handling and real reverse-direction DECtape transfer. Testkit v169
uses installed `mktsfs` instead of the retired external `make build`
target. The result logs are `$TMPDIR/daimos-group027-v168-*.log`
plus the stream and scanner v167 logs.

The `dtc-online` test originally failed at the deferred DSH image
size cap 036000. Versioned testkit runner v169 runs with the
functional-only cap 037000 and reaches the PDP-6 guest. The first
format/mount cycle runs, but persisted DECtape media fails to mount
on the second boot with `MOUNT.DTFS:` diagnostic output. Group 027
is therefore still open; this is a DTFS compatibility mount test,
not a reason to restore the obsolete `MOUNT.TSFS` interface.
Atomic KCC/GCC matrix retesting has not yet passed.

## 2026-10-22 TSFS BOOT-ROOT-ONLY CONTRACT / TEST REMOVAL V165

By design TSFS is usable only as the **boot filesystem**. Mounting
another TSFS set later is not supported. TSFS filesystem generation
and writing to DECtape belong in userspace tooling and `dd`. Do not
change the root-selected TSFS MRES install policy to satisfy obsolete
secondary-mount tests.

The testkit removed the post-boot `MOUNT.TSFS` runtime tests
(`daimos-tsfs-mount-v1.sh` and `daimos-tsfs-dir-v2.sh`), and
removed their registration from both regression matrices. The
obsolete v1 host scan fixture, which tested the retired mount handoff,
is deleted; the retained eight-member v2 scanner validates member
discovery/ninth-member rejection and passes.

The DAIMOS userspace build and production disk root no longer compile,
package, document or expose `MOUNT.TSFS`. Its source and manual,
userspace syscall stub and declaration, extension dispatch entry,
mount-handoff builder and its unused metadata validation are removed.
The old syscall selector is directed to the existing unsupported
extension handler. KINIT's separate boot-root MRES mount operation
(`tsfs_boot_mount_root` -> `FS_MRES_OP_MOUNT_UNIT` ->
`tsfs_mount_set`) remains intact, as required for TSFS boot.

Group 027 remains active for DTC I/O, TSFS scanning, image generation,
compressed extents and tape-root boot, under KCC and GCC. Strict
32K-size tests remain independent.

## 2026-10-22 GE0 / TYPE-340 TOOLCHAIN ROOT CAUSE V164

GE0 LOGIN failure root-caused to an obsolete *installed*
`$PDP10_PREFIX/bin/p10fold` binary. On `ge_io.dobj` the installed
binary emitted a zero-cost two-word CONSO/JRST fold between distinct
GTYO readiness loops, redirecting a status skip into an unrelated
continuation. The current `tools/host/p10fold.c` source has the
skip-sensitive I/O checks and emits only `P10FOLD1` for the same
object. The source was compiled and installed to `$HOME/cross/bin`.
No P10FOLD source fix was needed in this turn.

On the repaired toolchain, the complete CTY/DCS0/GE0 multi-TTY
runtime v145 PASS, including respawn, routing, resource checks and
STOP isolation. The full native Type-340 DPY login and primary/shifted
refresh test v150 also PASS. Testkit v164 now checks the *installed*
P10FOLD against the real GE assembly source to prevent stale binaries
from silently reintroducing the defect. Logs:
`$TMPDIR/daimos-ge-correct-p10fold-multitty-v161.log`,
`$TMPDIR/daimos-ge-correct-p10fold-dpy-v163.log`.

KCC standalone paths are green; GCC and the full atomic group 030
are not accepted yet. Group 027 DECtape/TSFS is active.

## 2026-10-22 TYPE-340 NATIVE REFRESH V150 BLOCKED BY GE0 LOGIN

The corrected-SIMH, fresh-256K KCC testkit Type-340 native-refresh
runtime v150 boots DAIMOS, connects DCS0 and GE0 and receives DCS0
LOGIN, but receives no GE0 LOGIN prompt; FAIL `remote LOGIN missing`
before running Type-340 refresh assertions. This reproduces the GE0
startup symptom of standalone multi-TTY v144 and does not yet
identify a display-driver bug. Testkit `e1d49ab`; log
`$TMPDIR/daimos-dpy-native-v150.log`. Root-cause the common GE0
LOGIN/session/transport issue with pristine media and unique ports.

## 2026-10-22 GROUP 027 REACTIVATED V149

Group 027 (DECtape/TSFS) is back in the active testkit TODO at the
user's request; it is no longer deferred. Revalidate its ten member,
mount, compressed extent, eight-member boot, and reverse-operation
targets under KCC/GCC. Existing root-selected DTFS/TSFS MRES policy
must remain intact: fix test boot-class assumptions instead of
unconditionally loading resident filesystem code. Strict 32K size
acceptance remains separately deferred. Continue group 030 first,
especially multi-TTY, EXEC timing and Type-340 refresh.

## 2026-10-22 GROUP 030 EXEC / DTFS / MULTI-TTY INVESTIGATION V148

The independently accepted 66-probe KCC/GCC runtime suite and CTY
READCHAR accounting are unchanged. Group 030 as a whole is still open.

Two custom INIT targets, EXEC replacement and child-session EXEC,
complete on corrected SIMH with expected `<E` and `<xEP` HALT
markers. Fresh-image simulation-only timings are 18 and 19 seconds,
respectively, versus the existing 15-second wall-clock acceptance
timeouts. No acceptance timeout or EXEC assertion was weakened.
Versioned testkit diagnostics v137/v140 permit an explicit longer
observation window but default to the original 15 seconds.

DTFS-native testkit v143 boots D6FS-root and fails to mount DECtape.
That is consistent with the intentional root-selected MRES policy:
`dtfs_minits` installs the DTFS/TSFS package only for
`KINIT_ROOT_DTC`. Successful file probes after failed MOUNT.DTFS
target the underlying mountpoint, not DTFS. This test must be
adapted to a supported DECtape-root configuration before any
DTFS acceptance can be claimed. Group 027 is still deferred by
request; this investigation did not modify the root/provider policy.

With corrected SIMH, standalone multi-TTY v144 obtains DCS0 LOGIN
but not GE0 LOGIN. Process-inventory diagnostic v145 could not
distinguish session spawn from transport, because one attempt
reused a dirty disk (HALT `?RT`) and another had a DCS0 TCP
connection timeout. No kernel TTY change is yet justified.

Evidence: `$TMPDIR/daimos-exec-simh-timed-v148.log`,
`$TMPDIR/daimos-exec-child-diagnostic-v141.log`,
`$TMPDIR/daimos-dtfs-group030-v143.log`,
`$TMPDIR/daimos-userspace-multitty-v144.log`, and
`$TMPDIR/daimos-userspace-multitty-pristine-v146.log`.
Strict 32K and full serialized matrix acceptance remain deferred/open.

## 2026-10-21 CTY COUNTER DEFECT / GROUP 030 PARTIAL V136

DAIMOS commit `08625cf` fixes missing completed-CTY I/O counters:
`cty_putchar` now increments the existing MonitorFS CTY0 output slot
only after PI4 confirms transmission, while `cty_getchar` increments
the CTY0 input slot only after actually returning a character.
The driver has no new BSS, buffer, or queue allocation. Before the fix,
the independent CTY READCHAR test read the three correctly formatted
octal STAT lines but returned `<C!4` because output accounting never
advanced. After the fix it passed under KCC and GCC. Evidence:
`$TMPDIR/daimos-monitorfs-cty-kcc-v133.log`,
`$TMPDIR/daimos-monitorfs-cty-gcc-v134.log`.

The original runtime probe suite used obsolete STAT/DF output formats
and attempted to read MonitorFS CTY0/STATS with CAT, which consumes
S6REC word records rather than the READCHAR character API. Testkit
commit `77f7d89` provides versioned `daimos-runtime-probes-v132.txt`
and the separate `test-daimos-monitorfs-cty-stats-v132` target.
All **66/66 runtime probes pass KCC and GCC** on fresh 256K disks:
`$TMPDIR/daimos-group030-runtime-kcc-v135.log` and
`$TMPDIR/daimos-group030-runtime-gcc-v136.log`.

This independently closes only the runtime-build/MonitorFS portion
of regression group 030. Exec, DTFS, display and multi-TTY acceptance
remain OPEN; group 027 is deferred by request, and strict 32K gates
remain deferred.

## 2026-10-21 D6FS REGRESSION GROUP 029 V131 ACCEPTED

The 256K non-size **group 029** D6FS HDD, multimember and recovery
tests now pass independently under both KCC and GCC.

The multimember test fixture had two unrelated defects: its generated
SIMH command files reattached obsolete DSK1-3 media, and its synthetic
D6FS roots omitted production MOUNTALL/configuration and the external
executables required by the 16 file-operation probes. Testkit commit
`552428c` selects versioned `run-multimember-v126.sh`, which seeds
correctly populated roots and attaches only the intended test members.
KCC v126 and GCC v127 each pass all 16 operations on both two-member
and four-member roots, including both rejected invalid-member layouts
and LOGSTORE post-checks. No DAIMOS kernel or D6FS code changed.

The ordinary HDD runtime passed 16 probes and the corrected four-member
LOGSTORE badmap check under KCC v128 and GCC v119. The separate
dual-superblock recovery fixture also passed 16 probes, bootstream
preservation, LOGSTORE inspection and dirty-reboot rejection under
KCC v130 and GCC v129.

Evidence logs are `$TMPDIR/daimos-d6fs-multi-kcc-v126.log`,
`$TMPDIR/daimos-d6fs-multi-gcc-v127.log`,
`$TMPDIR/daimos-d6fs-hdd-runtime-kcc-v128.log`,
`$TMPDIR/daimos-d6fs-recovery-gcc-v129.log` and
`$TMPDIR/daimos-d6fs-recovery-kcc-v130.log`, plus the earlier GCC
`$TMPDIR/daimos-d6fs-logstore-v119/d6fs-hdd-runtime-v119.log`.

Groups **022, 028, 029** are independently accepted; **027** is deferred
by request. **030** and atomic full-matrix acceptance remain open.
Strict 32K and size gates remain deferred.

## 2026-10-21 KERNEL/STANDALONE REGRESSION GROUP 028 V123 ACCEPTED

The independent non-size **group 028** test now passes KCC and GCC on
fresh 256K production PDP-6 disk images. Its previous three-second CTY
watcher incorrectly required an interactive `LOGIN:` prompt on CTY;
the actual interactive LOGIN/DSH transport is DCS0. The versioned testkit
v121 replacement requires the DAIMON version banner and BSET initialization
on CTY, ROOT login through DCS0, a fresh DSH prompt, and successful
external ECHO and PWD commands. Both compiler builds passed both probes.

The standalone PDP-6 bootloader suite was rerun independently and passed
**26/26** DSK/PTR/MTC/DTC cases, with zero failures. This was a test-harness
contract correction; no DAIMOS kernel source changed. The functional
DSH ceiling was explicitly set to `037000`. Strict 32K/size acceptance
remains deferred.

Evidence is `$TMPDIR/daimos-kernel-group028-kcc-v121.log`,
`$TMPDIR/daimos-kernel-group028-gcc-v122.log`, and
`$TMPDIR/daimos-stand-group028-v122.txt`. **Group 027** (DECtape/TSFS)
is deferred by user request. Groups **029/030** and the complete
non-size matrix remain open.

## 2026-10-21 PDP-6 SIMH CONSOLE PC / TRAP GROUP 022 V120 ACCEPTED

The independent 256K non-size group-022 trap-frequency retest now passes
under **both KCC and GCC**, with unchanged syscall-count ceilings. KCC
measured startup/echo/memstat/ls/ps/cat128 as 111/16/19/33/23/18;
GCC measured 90/16/19/33/25/18. Both passed ceilings of
128/35/70/70/110/45, respectively. The GCC run used the isolated
corrected PDP-6 compiler and the same corrected SIMH CPU as KCC.

The root cause was a host ELF symbol collision: the SIMH KX10 CPU's global
`PC` overlapped terminal-library padding `PC`, which interactive readline
overwrote. The corrected symbol `kx10_pc` (sims `1a41f45`) passed direct
script-to-console PC preservation, PDP-6 breakpoint/step execution, and
register sanity for PDP-6, KA10, KI10, KL10, and KS10. The standalone
testkit regression is `test-daimos-simh-console-pc-v115` (commit `58150fd`).
KA10/KI10 also received a separate default `-lm` link correction
(sims `5f6d21b`).

Testkit `0f6ad07` explicitly selects diagnostic `DSH_IMAGE_LIMIT=037000`
for this *functional* trap test; the original default 036000 limit rejected
the current 036021 image before boot. Strict 32K boot and permanent size
acceptance remain deferred, not silently passed. Runtime evidence:
`$TMPDIR/daimos-trap-runtime-fixed-pc-v117.log` and
`$TMPDIR/daimos-trap-gcc-fixed-pc-v118.log`.

This verifies group 022 independently; **full non-size matrix acceptance
is still open**. Groups 027, 028, 029, and 030 require separate repair and
retakes, and the complete matrix has not been rerun atomically. The v111
investigation below is retained as historical evidence, superseded for
the console-PC root cause and group-022 outcome.

## 2026-10-21 D6FS FOUR-MEMBER LOGSTORE RUNTIME V119

The non-size 256K GCC D6FS HDD runtime's `invalid badmap` post-check was
caused by the **testkit** invoking host `logstore --dump -n 1` against the
standard **four-member** D6FS disk set.  The incorrect logical-to-physical
mapping read a non-BADMAP block.  Direct comparison against the same disk
demonstrated `-n 1` FAIL (`invalid badmap`) and `-n 4` PASS (valid metadata).
Testkit commit `16ccd39` corrects the geometry to `-n 4`, without changing
any kernel or filesystem source.  A fresh GCC/256K `d6fs-hdd-runtime` run
then passed **16/16 guest probes** and its host LOGSTORE check (exit 0),
at `$TMPDIR/daimos-d6fs-logstore-v119/d6fs-hdd-runtime-v119.log`.
This resolves that specific group-029 subtest, **not** the complete group
(multi-member and recovery acceptance remain separate).  All dedicated
size and strict 32K tests remain **deferred**.

## 2026-10-21 PDP-6 SIMH STOP/CONTINUE REGRESSION V111 — OPEN

The checkpointed non-size matrix v75 has finished under **both GCC and
KCC**.  Groups 022 (trap), 027 (DECtape/TSFS), 028 (kernel runtime),
029 (D6FS), and 030 (userspace runtime) remain FAIL under both compilers.
They must be repaired independently; neither compiler has achieved full
non-size functional acceptance.  Evidence:
`$TMPDIR/daimos-nonsize-acceptance-v75/{gcc,kcc}-v75.state`.

Trap group 022 fails *before* it measures syscall rates.  The original
test reported `LOGIN INCORRECT` following an asynchronous SIMH Ctrl-E
stop/resume.  The isolated v98 experiment moved the first measurement
until after DSH login: ROOT authentication then succeeded, but the first
stop/resume still caused an invalid UUO and a HALT near PC 000001.
Independently, with **no syscall breakpoint installed**, a tape-root
SIMH session reproduced the stop/resume problem (v103).  In the v108
single-step diagnostic SIMH reported stopping at PC 024162 but a
subsequent `EXAMINE PC` returned 024000; a step ran from 024000 and
the machine halted.  This establishes a simulator console/PC-state
discrepancy, but its exact cause remains unresolved.  Temporarily
restoring the reported PC (v110) did **not** restore a completed DSH
command, so it is not an accepted workaround or fix.

An experimental PDP-6 CPU asynchronous-stop deferral also failed its
v105/v107 runtime controls.  It was removed in sims commit `3ae21bb`,
preserving the independent Type-166 floating-point implementation in
commit `828459c`.  Testkit commit `3ccaaa2` preserves CTY/DCS error
transcripts on request (`DAIMOS_TRAP_KEEP_WORK=1`), without altering the
original syscall-count thresholds.  Diagnostic transcripts:
`$TMPDIR/daimos-trap-frequency-v1-24813/cty.log`,
`$TMPDIR/daimos-simh-resume-v108/cty.log`, and
`$TMPDIR/daimos-simh-resume-v110/driver-v110.log`.

Next: isolate the simulator console's saved PC across a stop and
`CONTINUE` and add a target-executed test to `daimos-testkit`; do not
weaken the trap-frequency oracle.  **All 32K and dedicated size tests
remain deferred** by user direction.

## 2026-10-20 SECONDARY TSFS MOUNT ON TAPE ROOT V96

The independent 256K PDP-6 SIMH tape-root experiment passed **3/3**
secondary-volume probes: `MOUNT.TSFS /DEV/DTC4 /MOUNT/DT0`,
`STAT /MOUNT/DT0` (read-only directory, `TYPE 1`, `MODE 0555`), and
`LS /MOUNT/DT0` (entries `README`, `DOC`).  The bootable TSFS root
was attached on DTC0, and separate three-member TSFS media on DTC4–6.
The kernel's existing tape-root-only provider was used; there were no
additional resident TSFS modules on disk-root systems or kernel changes.
Evidence: `$TMPDIR/daimos-tsfs-tape-root-v96/report-v96.md` and
`pdp10-doc/STATUS/DAIMOS-TSFS-SECONDARY-MOUNT-TAPE-ROOT-20261020-V96.MD`.
This validates the three specific probes only.  The original group-027
disk-root TSFS mount fixtures require adaptation and full retesting.
The 32K and permanent-size gates remain **deferred**.

## 2026-10-20 AUTO ROOT DRM FOUR-MEMBER FIXTURE V91

The v74 non-size GCC AUTO-root run passed DISK and TAPE, but initially
failed DRUM.  Inspection of the live `d6fs_boot_scan_unit` return showed
that the Type-236 read succeeded and the blockset root marker advertised a
four-member D6FS set (membership mask 017).  The testkit fixture had copied
only drum member zero into the AUTO image, leaving its other three drum
volumes from an unrelated build.  Root discovery correctly rejected that
incomplete set; `kinit_boot_handoff` remained unset and mount capacity was
zero.  The testkit now copies **all four** drum image members in commit
`68d5664`.  A focused 256K GCC/PDP-6 SIMH boot with fresh, complete drum
media reached `D6FS LOADED`, `INIT V1`, ROOT login, DSH, and passed
`CAT /CONFIG/SYSTEM` with `ROOTCLASS DRUM` (one of one probes, v91).
No production KINIT or D6FS change was required.  DISK/TAPE from v74 and
DRUM from v91 are separate verified runs, not yet a single full atomic
three-case acceptance.  Evidence:
`$TMPDIR/daimos-root-four-member-v91/report-v91.md` and
`$TMPDIR/daimos-nonsize-acceptance-v74/006.log`.  The 32K boot and all
dedicated size tests remain **deferred** at the user's direction.

## 2026-10-20 MONITORFS SWAP READER CHECKPOINT V71

The experimental MonitorFS swapped-process startup-record reader, its
resident assembler dispatch, and its kernel Makefile integration were
committed as DAIMOS `dc1f96f` at the user's request.  This is a source
checkpoint, **not functional acceptance**.  Previous KCC/GCC source builds
and a 256K KCC image link passed, but the runtime test did not return the
expected swapped-process NAME/CMDLINE/ENVIRONMENT metadata.  Repair and
rerun the unchanged acceptance oracle before declaring the reader ready;
the strict 32K residency/boot gate is also open.  This commit was made
locally, without pushing.

## 2026-10-20 AUTO ROOT THREE-CLASS ACCEPTANCE V70

The corrected `daimos-auto-root-priority-v1.sh` fixture is committed as
daimos-testkit `ab730fe`.  Under a rebuilt Type-551 SIMH with a fresh 256K
KCC AUTO image, independent probes verified the full desired preference:
DSK present selects `ROOTCLASS DISK`, all DSK units absent selects
`ROOTCLASS TAPE`, and both DSK and DTC classes absent selects
`ROOTCLASS DRUM`.  All three root-class assertions passed and reached ROOT
login and DSH.  The initial 120-second per-command TSFS fallback allowance
timed out with the shell already running; the repeat using 360 seconds
completed `CAT /CONFIG/SYSTEM` and produced `ROOTCLASS TAPE`.

Evidence is preserved under `$TMPDIR/daimos-auto-root-priority-v1-26755/`:
`all.md`, `no-dsk-long-v66.md`, and `drm-only-v69.md`.  The drum test used
regenerated clean test media after a diagnostic run left its previous
instance unclean.  These are independently verified cases, not an atomic
end-to-end rerun of the final test script, and they establish neither a
strict 32K boot nor MonitorFS swapped-process metadata correctness.  The
three experimental MonitorFS source/build changes were subsequently
checkpointed in `dc1f96f` without runtime acceptance (see V71 above).

## 2026-10-20 UNCOMMITTED-WORK TRIAGE V64

The pending D6FS host maintenance changes have been independently tested and
committed: `daimos-tools` c79752c and `pdp10-tools` 74ef3aa.  `mkd6fs` now
accepts an empty auxiliary root; `d6fsck` verifies that result as CLEAN.
The `d6maint` helper correctly resolves `d6fsck` through PATH when invoked
as `packfs` without an explicit directory.  The host tests for directory
creation, consistency checking, recovery and PATH-based compaction passed.
The GE lost-carrier output-completion fix was built with a forced SIMH PDP-6
rebuild and its register sanity check passed (SIMH 220f839).

The experimental MonitorFS swapped-process startup-metadata reader
(`system/kernel/fs/monitorfs_swap_v63.c`, the runtime assembler change and
the kernel Makefile integration) compiles with KCC and GCC, and a fresh
256K KCC disk boot image links.  It is **not accepted**: the live MonitorFS
test reaches ROOT/DSH but both relative `CAT 2/NAME` and absolute
`CAT /MONITOR/PROCESSES/2/NAME` return without the expected DSH process
name.  The v64 test re-run failed its existing read oracle.  These three
kernel files were uncommitted at the time of this V64 triage and were later
committed as the unaccepted V71 checkpoint; compilation alone is not
functional acceptance.  Diagnostic
log: `$TMPDIR/daimos-uncommitted-20261020-v64/monitorfs-live-retest-v64.log`.
The testkit diagnostic-preservation change is committed as b3c458a; the
separate automatic-root-priority test remains unverified in this sweep.

The first DECtape retest used an outdated installed SIMH PDP-6 binary and
timed out after DTC startup.  That timeout is invalid as an assessment of
the PI3 kernel experiment, because it did not run the repaired Type-551
simulator.  The repeat with freshly rebuilt `sims/BIN/pdp6`, the same tape
image, and the two PI3 kernel changes passed **both external commands**:
`ECHO RETESTED_OK_V24` produced its output and a prompt, and `PWD` produced
`/` and a prompt.  The v34 harness returned exit zero (2/2 PASS).  Evidence:
`$TMPDIR/daimos-uncommitted-20261020-v64/simh-tape-sims-fixed-v64/report-v34.md`.
The two PI3 files were committed separately as `2c85325`; project-state
acceptance is commit `c1f693a`. The three MonitorFS files were later
checkpointed, without acceptance, as `dc1f96f`.  These checks are
256K runtime acceptance, not strict 32K size/boot acceptance.

## 2026-10-20 GCC STORAGE GROUP 023 VERIFIED V53

The complete non-size, 256K GCC storage-integration group 023 now PASSes
all ten tests, including MonitorFS online controller state, independent
D6FS mount contexts, drum backing, BCACHE, remount/recovery, reservation,
and BADMAP root recovery.  This is the first atomic rerun of the group
since the testkit fixture repairs `2804838` and `81b2610`.  The compiler
is the isolated GCC backend `df27247`; there are no new DAIMOS kernel
changes.  Evidence: `$TMPDIR/daimos-gcc-v53/group023-v53.log` (exit 0),
and pdp10-doc `STATUS/DAIMOS-GCC-STORAGE-INTEGRATION-20261020-V53.MD`.
Other non-size GCC failures are still under investigation; neither full
functional-suite nor 32K size acceptance is implied.

## 2026-10-19 GCC DISK FUNCTIONAL RETEST V52

The focused 256K disk-suite GCC regressions now PASS group 004 (five boot
origins, DSK root discovery, Stage1/KINIT, SIXBIT lifetime and PDP-6
codegen), group 031 (115/115 DSH probes), and group 032 (all host-tools
scripts). Group 023's three D6FS mount oracles and its previously failing
MonitorFS device-state test all pass independently; group 023 has not
yet been rerun atomically.  Testkit commit 81b2610 detaches all four DTC
units before disabling the SIMH controller.  Testkit commit 11feecb
updates the host D6FS repair crash fixture to verify the actual three
durable writes: DIRTY, changed summary, CLEAN; cut 4 never triggers.
The prior fourth-write expectation was stale, not a host checker bug.

The GCC scheduler multiprocess case (017) still fails under patched GCC.
Some group 027 tape mount fixtures request a DTFS/TSFS package in a D6FS
root profile where it is deliberately absent; neither a kernel feature
nor a test skip has been accepted.  Other original matrix failures remain
open.  The corrected compiler has not been installed systemwide.  All
retakes here are 256K, DSH_IMAGE_LIMIT=037000; no 32K claims.  Detailed
evidence: pdp10-doc/STATUS/DAIMOS-GCC-FUNCTIONAL-REGRESSION-REPAIRS-
20261019-V52.MD.

## 2026-10-19 GCC PDP-6 NINE-BIT ARRAY COMPILER REPAIR V46

The GCC backend had a target-executed, reproducible compiler bug at -O1,
-O2 and -Os: a 9-bit char array on the stack returned the wrong character
at a 36-bit word boundary.  A frame-relative 36-bit word access already
selected by the RTL was incorrectly treated as a byte displacement during
final assembly output.  GCC commit df27247 repairs that output path;
the direct PDP-6 testkit regression (commit e24488e) now passes at -O0,
-O1, -O2 and -Os.  The neighboring multidimensional-char regression
passes -O0/-O1/-O2.  No DAIMOS production-code change is involved.

An isolated rebuild with the repaired GCC booted a fresh 256K DAIMOS disk
image and passed **all 115 DSH conformance probes**, including HELP ECHO
and HELP DAS, which were the only two failing GCC probes before the fix.
Reproduction: $TMPDIR/daimos-gcc-v46/dsh-conformance-v46.log, result v46
exit 0.  The patched GCC was tested with an isolated toolchain wrapper;
the installed compiler was not overwritten.  Dedicated 32K and image-size
tests remain explicitly excluded.

The GCC multiprocess scheduler preemption test still fails under the
repaired compiler (v47); its clock/quantum behavior must be diagnosed
independently rather than weakening the test.  The disk regression groups
also retain separate VFS/MonitorFS, storage, DECtape and D6FS failures.
The original 30-group matrix has not been rerun in full after this repair;
do not claim full testkit acceptance.  See
pdp10-doc/TOOLCHAIN/GCC-PDP6-LOCAL-CHAR-ARRAY-20261019-V45.MD.

## 2026-10-19 AAP AND SIMH DECTAPE COMMAND ACCEPTANCE V37

Both supported simulator tape-root paths now reach INIT, ROOT, and DSH
and execute external ECHO and PWD successfully at 256K with the diagnostic
DSH image ceiling 037000.  The repaired SIMH Type-551 selected-transport
case passes 2/2 command probes.  A fresh AAP headless PDP-6 run passes
ECHO and PWD too: DSH was entered after 103.4 seconds, ECHO finished
after 300.2 seconds, and PWD finished after 548.2 seconds total.
The former 240-second AAP test timeout was insufficient for the observed
DECtape seeks and does not establish a permanent command-execution stall.
No new DAIMOS or AAP production change was required for this retest.

The separate full-device SIMH disk/D6FS functional regression sweep is
still under way with 32K and dedicated size tests excluded.  The 12-way
independent boot/root cross-matrix has passed.  The automatic-root-priority
testkit had inconsistent INIT/login expectations; its corrected fixture
is being independently retested.  No full disk-testkit acceptance is yet
claimed, and strict 32K boot and persistence remain separate gates.

Reproductions: daimos-testkit/tests/validation/daimos-aap-boot-basic-v37.py,
daimos-simh-dtc-selected-v34.sh, and
daimos-regression-disk-nosize-v35.tsv.  Details are in
pdp10-doc/STATUS/PDP6-SIMH-TYPE551-REGRESSION-20261019-V34.MD.

## 2026-10-19 SIMH TYPE-551 MULTI-UNIT BOOT REPAIR V34

The high-priority V24 AAP-like SIMH DECtape-root stall is fixed in the
SIMH source tree (local commit `8361359`).  The SIMH Type-551 event routine
was updating a shared `DTSB` from nonselected moving drives; an inactive
drive's end-of-tape could stop the selected drive with a Type-136 read still
in progress (V24 observed 119/128 words, IOWD -9).  A new selected-unit
owner gates status, Type-136 data, and interrupt operations while retaining
physical motion for unselected drives.  The same SIMH commit restores CPU
`PC` as the first register, which is necessary for `GO 020` in rebuilt SIMH.

On the **unchanged experimental DAIMOS PI3** tape driver, AAP-like SIMH
DECtape/TSFS root now reaches INIT, ROOT, DSH and successfully executes
external ECHO and PWD (2/2 tests).  Freshly rebuilt full-device SIMH with
DSK/D6FS root also reaches ROOT/DSH and passes ECHO/PWD (2/2).  A disk-root
`?RT` on reused media remains a distinct unresolved persistence regression;
it did not reproduce after regenerating the image.  All runs here are 256K
with the diagnostic `DSH_IMAGE_LIMIT=037000`, **not** 32K acceptance.

The AAP post-DSH ECHO timeout from V24 has **not** been retested or fixed.
The DAIMOS DECtape PI3 work in `tape_io.s` and `storage_router.s` remains
uncommitted, unchanged by the SIMH repair.  Test reproduction is in
`daimos-testkit/tests/validation/daimos-simh-dtc-selected-v34.sh` and
`pdp10-doc/STATUS/PDP6-SIMH-TYPE551-REGRESSION-20261019-V34.MD`.

## 2026-10-18 CROSS-EMULATOR BOOT RETEST V24

Fresh 256K images from HEAD bc22c27 plus the uncommitted DECtape PI3
driver/router changes were exercised on AAP, comparable-hardware
SIMH with DTC/TSFS root, and full-device SIMH with DSK/D6FS root.
The diagnostic DSH size override 037000 remains necessary.

AAP reaches INIT, ROOT and DSH but ECHO RETESTED_OK_V24 never produces
output or a new prompt within 240 seconds. SIMH with non-AAP disk,
drum, MTC and terminal peripherals disabled reaches DTC OK but does
not reach INIT. A second AAP run, using the headless emulator forcibly
rebuilt from cleaned revision ac603e8, reproduces the same ECHO timeout.
Debugging the SIMH tape run found an active 128-word Type-136 DECtape
read with IOWD count -9 (119 words transferred), no progress on
resumption. This is an open cross-simulator DECtape regression; the
source of the discrepancy has not been assigned to DAIMOS or SIMH.

Full-device SIMH disk boot reaches ROOT/DSH on DCS0 and runs external
ECHO and PWD. A separate fresh-media CTY boot passes the same tests.
The first DCS0 test also logged INIT: RUN FAILED, and reusing its disk
media led to a subsequent ?RT root-mount halt; regenerating the
media made the CTY test pass. Startup consistency and disk persistence
still require investigation.

Reproduction probes are in daimos-testkit. Detailed evidence is in
pdp10-doc STATUS/EMULATOR-BOOT-RETEST-20261018-V24.MD.
No production code or emulator fix was made during the retest.

## 2026-10-18 AAP DEFAULT-INITTAB BOOT RECOVERY V23C

INIT no longer launches LOGIN against missing physical terminal controllers.
The default INITTAB still lists TTY0 (CTY), TTY1 (DCS0) and TTY17 (GE0),
but `init_tty_input_present_v23c` checks the corresponding MINIT-published
MonitorFS `/DEV/CTY0`, `/DEV/DCS0`, or `/DEV/GE0` endpoint before admitting
each entry.  A successfully probed controller permits all its logical lines
even without carrier; unprobed/absent controllers do not allocate a LOGIN
process or enter a failed respawn cycle.  No kernel ABI or resident RAM change.

With the previous v23b packed CTY FIFO and still-**uncommitted** experimental
Type-551 PI3 tape driver, a fresh AAP root-tape boot **using the unchanged
three-entry INITTAB** reaches INIT, ROOT authentication and `DSH V1`.
Evidence: `$HOME/tmp/aap-dtc-boot-v41/aap-boot-console-pi3-v41.log`,
validated by the new testkit v23c trace verifier: 234 forward plus 21 reverse
128-word DECtape audits, zero word errors, no respawn/session/auth messages.
Host C99 tests of the exact production check cover TTY 0/1/16/17/20,
missing hardware, invalid status, and syscall failures.  A previous failed
AAP boot capture is correctly rejected by the verifier.

The transient INIT code text grew from 983 to 1050 words (+67), BSS remains
177 words; KCORE/MRES residency is unchanged. The boot still uses a
diagnostic-only expanded DSH image ceiling, not strict 32K acceptance.

**Separate open issue:** a subsequent AAP DSH `ECHO AAPBOOTOK` request is
delivered and echoed as a command line but does not produce the command's
result or a fresh prompt within the longer v43 experiment. Correct DECtape
word content does not establish provider lookup/search completion. This does
not invalidate the completed default-INITTAB INIT/LOGIN/DSH boot fix.

## 2026-10-18 CTY TYPE-626 INPUT PRESERVATION V23B

The CTY PI4 driver previously used a one-character mailbox which each
incoming DATAI unconditionally overwrote.  Under DECtape I/O latency this
lost queued input: a controlled AAP headless run received all five letters
of `ROOT` and CR, but echoed only `RO`.  V23B replaces the mailbox with a
bounded ten-character, two-word packed seven-bit FIFO and a one-word count.
If full, PI4 masks its input priority without acknowledging the hardware
byte; the foreground dequeue releases a slot and re-enables PI4.  The
Type-626 can then deliver its pending byte.  The producer/consumer use no
unbounded loops and no dynamic allocation; the implementation adds two
resident data words relative to the old one-byte mailbox.  CTY MRES grows
from 58 to 112 words (+54 linked resident words including instructions).

Native PDP-6 exact-assembly FIFO tests pass (NUL, ten-byte capacity, full
rejection, cross-word FIFO order, reuse).  Existing TTY routing tests pass.
An AAP PI3 DECtape + CTY FIFO diagnostic boot with a **CTY-only test
INITTAB** successfully reached ROOT login and DSH.  The original three-
session INITTAB does not: extra LOGIN process/respawn attempts trigger
`INIT: RESPAWN FAILED` and `LOGIN INCORRECT` despite physically correct
DECtape word transfers.  Only the CTY FIFO is accepted here; the PI3
DECtape search changes and the multi-session startup investigation remain
uncommitted.  The diagnostic INITTAB is not the shipped configuration.

The repaired installed `pdp10-objdump` correctly reports MRES resident size
using the current source, packaged as versioned `local.tar.gz` and installed
to `$PDP10_PREFIX`.  The disk permanent gate passes with 170 free words
and tape permanent gate with 568 free words for this *experimental PI3*
working tree; they are not strict 32K KCORE acceptance.

## 2026-10-18 UNCOMMITTED-CODE TRIAGE V23A

The native FILE FLOCK traversal once again computes the owning descriptor's
packed-node difference after extracting the per-fd high mount bits. This
prevents the candidate table displacement from being mistaken for the lock
family comparison. `proc_swap_boot.c` invalidates the shared filesystem
block-cache workspace before a direct persistent BACKSTORE read. The GCC
process swap-out and swap-in paths now publish RESIDENT/DISK state changes
through the production storage map, matching the native assembly path.

The PDP-6 FILE mount-extension and VFS 26-slot policy regressions pass. The
Termux host `mm-core` fixture links the existing storage-map implementation
and supplies an inert block-cache invalidation stub; its full MM suite passes.
These fixes do not enable experimental AAP bootstrap diagnostics or CTY FIFO
changes. The DECtape PI3 search rewrite is reviewed and tested separately.

## 2026-10-16 BOOT-SELECTED MM VFS CAPACITY V22

KINIT now derives its mount slot count from validated physical root-member
masks and the MINIT service directory. D6FS handoff locators expose physical
disk/drum units (including nonconsecutive drives); TSFS selection stores a
transient mask of its validated DECtape units. The installed DSK/DRM/DTC
controllers contribute supported transport slots, not media occupancy.
Controllers without their installed filesystem provider do not consume
mount-table space (D6FS excludes TSFS/DTFS MRES and vice versa in the
current boot profile). MEMFS contributes one slot only in full builds.

KINIT calls the existing 26-slot `vfs_mount_capacity_v1` planner, allocates
and zeroes the exact six-array MM slab, moves the boot reader, publishes all
six table bases and **then** publishes the `vfs_mount_limit`. There is no
fixed-four fallback on inconsistent inventory. A full D6FS/DSK+DRM boot
with one root member allocates nine slots/54 MM words; a detected DSK-only
full boot uses five slots/30 MM words, and a minimal disk-only profile uses
four/24. A full TSFS/DTC one-member root also uses nine slots. The packed
record and policy upper bounds remain 26; current bootable MRES combinations
require at most nine rather than reserving phantom controller/provider slots.

Native PDP-6 full/reduced inventory tests and the existing MM/policy/provider
regressions pass. Disk-root diagnostic ROOT login/ECHO uses the documented
test-only DSH size override. Documentation:
`design/vfs-boot-capacity-20261016-v22.md`.

V22 disk/tape permanent last words remain 037355/036537 octal (disk multi
037416). Static 32K KCORE end 024142 fails the 020000 gate by 2146 words.
The KINIT disk/tape image ends are 073277/074277, leaving 1345/833 words
after reserved bootstrap stacks; these are **not** 32K boot acceptance.

## 2026-10-16 VFS 26-SLOT PROVIDER BOUNDS V21

Replaced the last explicit 4-slot D6FS MRES dispatch and DTFS SPACE bounds
with `vfs_mount_limit`, protecting MM-backed table indexing. Optional
TENEX/ITS DTFS personality lookup no longer uses a four-word self-modifying
XCT table: it reads the existing MM-backed `dtfs_media_base` entry and
validates its slot. Foreign support remains disabled in the shipped build,
but the enabled assembly branch builds and its exact production leaves pass
native PDP-6 tests at mounts 10 and 26, including invalid mount 27.
The separate staged VFS access/parent helpers now also admit 26 slots.

No added resident/process RAM. Disk single/multi/tape permanent last remain
037355/037416/036537 octal; 32K lowmem KCORE end remains 024142 octal,
2146 decimal words above its strict gate. Diagnostic disk-root ROOT login
and ECHO pass under the documented test-only DSH size override.

**Capacity remains four in production.** `vfs_mount_capacity_v1` is a
staged inventory planner, not wired to KINIT. Boot must obtain validated
controller unit counts and root membership before it can publish an MM slab
capacity up to 26. See `design/vfs-provider-26-20261016-v21.md`.

## 2026-10-16 PURE BACKING 26-MOUNT EXTENSION V20

The one-word resident PURE swap record still contains its complete 18-bit
vnode index, four low mount-slot bits, and 14-bit text length. The fifth
mount-slot bit (public IDs 17..26) lives in the existing stable u-area
credential word 066, bit 27, with **no additional process words**. For RUN,
which attaches the executable before allocating the u-area, the bit is staged
in otherwise-unused SIDL scheduler RH bit 0 and published after inheriting
credentials but before setting SRUN. Detach clears the attached/staged bit.
This fixes an actual early RUN boot regression encountered when attachment
was initially made to require an already-allocated u-area.

PURE swap-out/in, raw/compressed reload, optional PURE verifier, vnode and
mount-busy guards all preserve five slot bits. Native unpack validates
`vfs_mount_limit` before indexing the MM-backed root array, and busy guards
skip slots lacking a valid u-area. Ordinary and destructive EXEC preserve
the lifecycle; transactional EXEC snapshots both the old and new extension
in transient stack words and restores the correct bit on failure/commit.

PDP-6 native tests pass for mount IDs 10/26, maximal index/text, corruption
bounds, full raw/compressed PURE swap and verifier, attachment and SIDL
staging, RUN inherited-credential isolation, and EXEC load/startup/VM
rollback and successful commits. Diagnostic disk-root SIMH ROOT login/ECHO
passes after fixing RUN staging. See
`design/pure-backing-mount26-20261016-v20.md`.

Final v20 linked permanent ends (octal): disk single `037355`, disk multi
`037416`, tape single `036537`. This is +116 decimal permanent words over
v19, with no extra per-process memory. GCC lowmem 32K KCORE end `024142`
still fails its strict `020000` limit by 2146 decimal words. Disk/tape KINIT
ends `073042`/`074001`, leaving 1502/1023 words after reserved boot stack.
Diagnostic disk-root PDP-6 ROOT login/ECHO is verified with the existing
test-only DSH size override. No production 32K boot claim is made.

Production still publishes four mounts; remaining 26-mount blockers are
provider-local bounds, optional TENEX/ITS DTFS personality dispatch, and
KINIT's capacity selection. The 32K strict KCORE gate is not yet met.

## 2026-10-16 TWO-WORD FILE DESCRIPTOR 26-MOUNT EXTENSION V19

Active native FILE descriptors now preserve mount bits 3..4 (canonical vnode
bits 27..28) in the existing per-process two-bit-per-fd extension bitmap
at u-area word 067. The packed two-word descriptors, full 36-bit offset,
and `0410`-octal u-area remain unchanged. FILE open/close, canonical-vnode
I/O dispatch, FIFO substitution, DUP/DUP2, FLOCK, unmount lock cleanup and
RUN mapped-descriptor inheritance now maintain the extension. Pipe/TSFS
provider-7 special dispatch was hardened: a TSFS root on mount 8 must not
be mistaken for a PIPE endpoint when duplicated, closed or inherited.
Native PDP-6 tests cover mount IDs 2 versus 26, high-bit preservation, lock
independence, duplicate/close/unmount cleanup, RUN fd remapping, and TSFS
versus PIPE reference ownership; diagnostic disk-root ROOT login/ECHO passes.
Full PDP-6 FIFO and word-FIFO simulator regressions also pass with the
existing diagnostic-only DSH `037000` size override. Without that override
the standard `036000` DSH gate still fails; production acceptance is not
claimed.

Memory costs: +138 linked permanent words versus v18, no per-process data
increase. Disk-root single permanent last 037171, multi 037232, tape root
036353 (octal). GCC 32K lowmem KCORE end 024006 FAILS strict 020000 by
2054 decimal words; KINIT disk/tape ends 072706/073645 leaving 1594/1115
words after reserved stack. Production mount capacity is still **four**.
PURE backing's fifth mount bit, optional TENEX/ITS provider table/bounds,
and KINIT capacity selection remain pending. Details in
`design/file-mount-packing-20261016-v18.md`.

## 2026-10-16 COMPACT CREDENTIAL / FILE EXTENSION RESERVATION V18

UID9, GID9 and UMASK9 now share u-area credential word 066, leaving the
existing word 067 completely available for 16 two-bit FILE mount extensions
(32 of 36 bits). The native GETUID/GETGID/SETUID/SETGID, file credential
checks, PROC_UMASK macro, UMASK syscall and RUN credential inheritance use
the new layout. PDP-6 native tests for packed identity/umask preservation and
the unchanged 0410-octal u-area layout pass. The FILE two-word descriptors
and full 36-bit offsets are unchanged. **Descriptor extension storage is
reserved but not yet used**; KINIT still publishes only four mounts.

Next update all FILE vnode readers/writers and FD lifecycle paths atomically,
including RUN fd-map inheritance, then solve the separate overfull PURE-swap
record. See `design/file-mount-packing-20261016-v18.md`.

An unused high credential bit is a possible home for PURE backing's fifth
mount-slot bit, keeping its 36-bit primary record; attachment/restore/release
lifetimes have not been migrated yet. The repacking adds nine permanent
words over v17: disk/tape last 036757/036141 octal. GCC lowmem 32K KCORE
end 023574 still exceeds 020000 by 1916 decimal words. Boot-image ends
disk/tape 072474/073433, leaving 1732/1253 words after stack reservation.

## 2026-10-16 VFS 26-SLOT POLICY AND SLAB PLANNER V17

The active 36-bit `vfs_mount_policy` word now uses 26 read-only bits (0..25),
five SWAP-owner bits (26..30), and five LOGSTORE-owner bits (31..35). Public
owner IDs 1..26 fit in five bits. Native PDP-6 tests exercise mount ID 26,
its RO flag, exclusive ownership/release, and unmount retirement; the prior
mount-16 regression and mount/storage/D6FS tests pass. No additional
resident words are required; disk/tape permanent last remain 036746/036130.
The six-array KINIT slab planner accepts up to 26 slots (156 dynamic words).
The staged inventory calculator no longer discards the possible seventeenth
device, although its currently described controllers provide at most 17
potential volumes. Full KINIT still allocates exactly four slots and publishes
`vfs_mount_limit=4`, intentionally: production support above four is NOT
enabled or boot-accepted yet.

Two compact runtime formats must be solved before raising the limit:
(1) FILE descriptors overlay metadata on the top three bits of the six-bit
vnode mount field, retaining only THREE mount-ID bits (`file.h` and
`file_unlock_mount`), and (2) resident PURE swap backing packs a full
18-bit index, four-bit mount slot, and 14-bit text length into 36 bits.
Keeping all three full ranges with five mount bits requires 37 bits and is
mathematically impossible in the present one-word swap format. Provider
limits including optional TENEX/ITS personality indexing also remain.
Do not enable >4 production mount slots until these are correctly migrated.

GCC lowmem 32K measurements are unchanged: KCORE ends 023563, failing the
020000 gate by 1907 decimal words; disk/tape KINIT ends 072463/073422,
leaving 1741/1262 words after reserved boot stack. No shell is required for
these size tests, and no 32K boot acceptance is claimed.

## 2026-10-16 COMPACT 16-SLOT VFS POLICY V16

The native VFS policy no longer uses four-slot RO/SWAP/LOGSTORE bit groups.
It retains **one** resident 36-bit word, aliased with the unused MonitorFS
TTY0 input counter: bits 0..15 are per-slot RO flags, bits 16..20 contain
the public SWAP owner mount ID (0=none), and bits 21..25 contain the public
LOGSTORE owner ID (0=none). The owner representation exploits the existing
exclusive reservation rule and avoids allocating three separate bitmaps.
All active users are converted: VFS publish/read/release/unmount/shutdown,
D6FS CLEAN/RW toggling, and FSINFO. The unlinked three-bitmap v2 policy
prototype is not the resident implementation.

Native PDP-6 tests pass for reservation lifetime, atomic rejection of
invalid release, remount CLEAN policy, slot reuse, a reduced runtime bound,
and a synthetic 16-slot table exercising public mount ID 16 (RO and both
exclusive owners, wrong-owner release, correct release, retirement).
Diagnostic disk-root SIMH ROOT login and ECHO also pass under the existing
test-only DSH size override. Production VFS capacity remains four until
the separate provider bounds and hardware-derived KINIT sizing are ready.

The more general owner checks increase permanent instructions by 25 words
over v15, despite adding zero policy data words: disk single last 036746,
multi last 037007, tape last 036130 (octal). GCC 32K KCORE end 023563
FAILS the 020000 gate by 1907 decimal words. KINIT disk/tape ends are
072463/073422, leaving 1741/1262 decimal words after the reserved stack.
Native 32K boot remains unaccepted. Preserve the compact one-word data
representation while optimizing the instruction paths in a later RAM sweep.

## 2026-10-16 MM-BACKED DTFS/TSFS MEDIA TABLES V15

KINIT now publishes all six MM-slab table bases: VFS target/root, D6FS
reader, shared DTFS/TSFS media, TSFS file shape, and TSFS extent locator.
The former `dtfs_media[4]` and TSFS file-shape/extent fixed arrays are no
longer linked. Active tape C/native assembly accesses the MM-backed tables;
PDP-6 TSFS multi-extent, D6LZ, nonzero-slot, and DTFS NATIVE/TENEX/ITS
chain-walk fixtures PASS. Tape-root without D6FS/BLOCKSET/BADMAP links,
and diagnostic disk-root ROOT login and ECHO PASS (DSH size override only).

RAM accounting is not a net saving yet: the new absolute base pointers and
access instructions increase disk-root permanent last from 036712 to 036715
(+3 decimal words), and tape-root last from 036060 to 036077 (+15 decimal).
GCC 32K lowmem KCORE end 023532 still FAILS the strict 020000 gate by 1882
decimal words. Boot streams build without shell: KINIT ends disk 072432,
tape 073371, leaving 1766/1287 decimal words after the 02000-octal KINIT
stack reserve. No 32K runtime acceptance is claimed. The active mount count
remains four until the live policy representation, provider bounds, boot
inventory and optional foreign-format personality indexing are migrated.

## 2026-10-16 KCORE LEGACY TABLE REMOVAL V14

Removed obsolete four-word VFS target/root arrays (eight words) and one
duplicate D6FS boot-reader slot. KINIT already publishes MM-backed VFS
tables and now transfers the D6FS initial reader directly from its active
reader word. Disk/tape size gates and disk-root diagnostic PDP-6 boot pass.
Nine 36-bit permanent words saved over v13: disk permanent last 036712,
tape 036060; lowmem KCORE end 023527, still FAILING the independent 020000
32K gate by 1879 decimal words. Other compatibility-looking paths remain
live, are unlinked, or cost zero words; details in
`design/kcore-legacy-audit-20261016-v14.md`. VFS remains limited to four
mounts until provider metadata and mount-policy migration completes.

## 2026-10-16 VFS D6FS READER SLAB AND 32K MEASUREMENTS V13

D6FS now uses the MM-backed slab reader array via `d6fs_reader_slots_base`.
MINIT stores its initial reader in a single fallback word; KINIT transfers
that pointer into the MM slab before publishing the new base. Native PDP-6
dispatch, multi-mount, remount, and fallback-poisoning tests pass. Diagnostic
disk-root SIMH ROOT login and ECHO pass under the existing test-only DSH size
override. DTFS/TSFS provider metadata and mount policy are not yet converted;
the active VFS mount limit is still four (planned maximum sixteen).

Normal permanent-size last addresses (octal): disk single 036723, disk multi
036764, tape 036071. Fresh GCC lowmem 32K builds without shell produce KINIT
ends 072422 (disk) and 073327 (tape), leaving 1774 and 1321 words after the
02000-octal stack reserve. KCORE end is 023540, failing the independent
020000 KCORE gate by 1888 decimal words in both profiles. KCORE+MRES is
12837/12705 decimal words (disk/tape). These are build/layout measurements,
NOT 32K boot acceptance. See daimos-testkit's versioned 32K footprint test.

## 2026-10-15 VFS LIVE MM TARGET/ROOT TABLES V12

KINIT now allocates the 24-word four-slot VFS slab from MM after MRES
installation and publishes target/root bases before root mount. Production
VFS, FSINFO, and PURE backing use the MM-backed target/root arrays; provider
metadata is still fixed and the live mount capacity remains four (maximum
planned sixteen). The source and complete KINIT image link succeed for the
dedicated disk-root profile. Fixed-symbol test regressions and a new PDP-6
test switching table bases to separate arrays pass. The disk-root permanent
last is 036721, multi-root 036762, and tape-root 036073, octal. Permanent
text grew 35 decimal words compared to v11; MM adds 24 slab words for four
slots, partly unused until provider migration. Remove redundant fixed tables
and optimize pointer hot paths in the final cutover, not by unsafe capacity
changes now. The module-table conditional DTFS import and explicit KINIT
object lists were corrected to make the real image link work.

Diagnostic disk-root PDP-6 SIMH boot reached ROOT login and ran an ECHO
command successfully. The documented DSH_IMAGE_LIMIT=037000 diagnostic
override was necessary because the unmodified production 036000 DSH gate
rejects the 036021 DSH image. This is NOT production-size acceptance.

## 2026-10-15 VFS SHARED RUNTIME SLOT LIMIT V11

The live four-slot VFS now reads one resident `vfs_mount_limit` word instead
of duplicating immediate four-slot bounds in native VFS and FSINFO assembly.
PURE swap attachment also validates against that limit. It starts at four
and cannot be raised until KINIT publishes MM-backed tables for all providers.
PDP-6 execution tests pass with a test-only reduced bound of two and confirm
overflow rejection, storage-reservation bounds, and mount-slot reuse.
Dedicated disk/tape permanent-size gates pass (disk single last 036656,
multi 036717, tape 036030, octal). The change adds three permanent words
over the v10 mount-width fix. Dynamic 16-slot KINIT activation is pending.

## 2026-10-15 SWAP BACKING MOUNT-WIDTH CORRECTION V10

Preparatory VFS work targets boot-configured mount capacity <=16. The active
one-word executable swap/PURE backing record previously encoded only four
mount slots. Its two provider bits have been reassigned to the mount field,
giving index:18, mount:4, PURE-text:14 while retaining exactly one word per
process. Native restore recovers the provider from the mounted root; swapped
PURE processes keep their backing mount busy. PDP-6 tests for raw/compressed
PURE swap and mount-16 reconstruction pass. The disk and tape dedicated-root
permanent-size gates pass; linked permanent footprint is four words smaller
than the preceding VFS mount-mutation v9 build. This does NOT activate VFS
mounts above four: dynamic slab callers and KINIT connection remain pending.

## 2026-10-14 LIBC STRING ASM/C SPLIT V1

DAIMOS libc now uses PDP-6 assembler for `memset`, `memcmp`, `strlen`,
`strcmp`, and `strncmp`, matching the existing KCC freestanding runtime
implementations.  Existing `memcpy` and `memmove` remain assembly.  Only
`strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, `strrchr`, `strtok`, and
`strcspn` remain in `userland/libc/string.c`.  The libc archive and the
standalone userland MAKE executable both build and link successfully with
this split.  This is a build/link validation, not a new SIMH functional
regression result.  The standalone semantic test for `strchr` still uses
C intentionally; no assembly `strchr` implementation exists.

## 2026-10-13 NEW D6 CODEC NATIVE ABI STAGING V055

Imported the native v053 D6 compressor/decompressor implementation and
the v054 all-producer cutover checklist from the completed codec branch.
Added an inactive AC10..AC16-preserving kernel calling adapter. Its
native PDP-6 test uses the independently recorded v051 darc golden stream
and verifies exact bitstream parity, decode and register preservation:
PASS. The old `000060` resumable decoder, stage-1 installation, image
builders, VFS/EXEC/TSFS consumers and kpack identity API are unchanged.
The v053 complete-buffer decoder cannot simply replace the resumable
fixed core without breaking the old API; a coordinated single-format
image cutover is still required before the user-requested production
activation. See `system/kernel/core/D6CODEC-KCORE-ABI-v055.md` and
`D6CODEC-INTEGRATION-TODO-v054.md`. This step adds no active KCORE words.

## 2026-10-13 REGISTRY-AWARE PACKED RESTORE V46

Staged `proc_packed_restore_transaction_v46` consumes the installed v38
dense/sparse descriptor layout by reconstructing the two-word source from
the MM-authoritative v37 lookup. While lifecycle TRANSITION is held it
pins the packed source, allocates/pins a new aligned resident extent,
zeroes omitted PURE words, reloads executable backing before decoding the
writable suffix, and releases packed MM only after all fallible I/O succeeds.
MM release is the commit point; the matching registry entry and PACKED mode
are retired before VM base publication and TRANSITION release. On a failed
decoder, backing mismatch or MM source-free refusal, the registry, packed
ownership and storage mode remain intact and the destination is freed.

PDP-6 GCC/DAS standalone object assembles at 209 pre-fold text words and
zero BSS. `daimos-testkit/tests/system/packed-restore-v46` passes dense
and sparse success and failure coverage, including PURE-before-writable
ordering. Existing v31 restoration and v45 eviction regressions pass.
The production VM/scheduler and KINIT link remain unchanged. Next test a
complete v45->v46 cycle against one registry with allocation/pin/free fault
injection, then evaluate scheduler entry points and complete linked size.
Do not remove disk swapping or integrate the production codec prematurely.

## 2026-10-13 REGISTRY-AWARE EVICTION TRANSACTION V45

Added inactive `proc_packed_evict_transaction_v45` (separate from the
existing v32 API to preserve previous test coverage). Under TRANSITION
it preflights the configured dense/sparse registry, process state, owned
resident MM extent, and absence of a pre-existing packed extent. It then
pins the resident allocation, allocates and pins packed storage without
reclaim recursion, invokes a caller-provided encoder, and rejects a
nonshrinking or incorrect encoded length. After source MM free, it sets
PACKED mode and zero VM base, publishes the MM-authoritative registry
entry via v44 while TRANSITION is still held, and only then unpins packed
storage and clears TRANSITION. An error after source release is fatal and
leaves TRANSITION set; no scheduler may run such a process. Precommit
failures leave resident ownership and mode intact.

PDP-6 GCC/DAS standalone object: 251 pre-fold text words, no BSS.
daimos-testkit `packed-evict-v45` passes dense/sparse transaction and
Stage 43 release, synthetic lossless pair compression, encoder failure,
nonshrinking output refusal, sparse registry exhaustion, and repeated
slot cleanup. This is a host test, NOT a SIMH scheduler acceptance test.
The original v31 restore still consumes a private descriptor rather than
the selected registry, so scheduler integration and active KCORE linkage
remain disabled. Codec integration and removal of disk swapping remain
deferred. The last measured active KCORE endpoint remains 021541 octal.

## 2026-10-13 MM-AUTHORITATIVE PACKED PUBLICATION V44

The inactive `proc_packed_publish_v44.c/.h` prototype complements compact
v43 teardown using the v38 boot-selected dense/sparse layout. It requires
PACKED mode, a zero resident VM base, lifecycle TRANSITION, a valid prepared
layout, and exactly one MM_TYPE_PACKED_PROCESS extent with matching owner,
base and length. It refuses duplicates and sparse capacity exhaustion
without mutating the registry. Once validation succeeds, it commits the
layout word and (for sparse) its slot index; these are serialized operations
with no remaining fallible steps. The descriptor span stays MM-authoritative.

PDP-6 GCC/DAS standalone compilation passed at 137 text words before
folding (0 BSS). daimos-testkit `packed-publish-v44` passes composed boot,
publish, lookup, release, failure rollback, and slot reuse against both
layout formats. Its companion v43 release is 166 text words standalone;
these are NOT full-image savings and do not include codec, restore, or
KINIT linkage. Active KCORE and boot behavior are unchanged.

IMPORTANT integration mismatch: staged v32 eviction clears TRANSITION on
success before the v44 publication API could run; staged v31 restoration
consumes/clears a private two-word descriptor rather than the selected
registry layout. Before scheduler integration, refactor the transaction
commit boundaries so the packed registry is published while TRANSITION is
held and the restore cleanup retires the same registry entry atomically.
Do not weaken the ownership checks or publish PACKED mode without an
installed descriptor. Disk swap remains operational; production codec
integration remains deferred. The previous live KCORE endpoint was 021541
octal and has not been remeasured by this inactive stage.

## 2026-10-13 COMPACT MM-AUTHORITATIVE PACKED RELEASE V43

The inactive `proc_packed_release_v43.c/.h` prototype performs the packed
teardown commit directly against one MM_TYPE_PACKED_PROCESS extent for the
slot, plus the KINIT-selected v38 dense/sparse layout. It does not construct
or retain an independent packed span descriptor. The routine validates the
explicit PACKED mode, TRANSITION, absent resident VM base, nonempty and
valid writable layout, exact single MM owner, and unique sparse slot entry.
Only after a successful MM free does it clear the selected layout and the
two-bit storage mode. Any MM pin/ownership failure preserves both.

PDP-6 GCC/DAS standalone build: 166 text words before folding, no BSS.
The prior v37 shared owner object was 197 words and v39 guard was 49 words;
these figures are NOT a whole-image saving, since v43 still requires the
boot registry, mode-map, layout and publication machinery. The new testkit
`packed-release-v43` passes both dense and sparse boot-to-teardown lifecycles,
including duplicate sparse entry, ambiguous MM owner, missing TRANSITION,
nonzero VM base, pinned free refusal, MM free failure, and slot reuse.

No live KINIT registry allocation or new KCORE linkage is enabled. The
Stage 41 fail-closed destructor remains active; its last measured endpoint
is 021541 octal. The next implementation decision must compare complete
linked KCORE and total dynamic RAM for an integrated release/publication
path, not merely the isolated object sizes. Production codec and packed
eviction remain deferred.

## 2026-10-13 PACKED BOOT/TEARDOWN INTEGRATION TEST V42

Added a combined host regression in daimos-testkit
`tests/system/packed-boot-teardown-v42`. It runs the existing real
v36/v37/v38/v39 C code in one process, with a controlled MM mock: allocation
and boot-publication of a dense or sparse registry, failed boot allocation,
boot abort and reinitialization, publication and lookup of an MM-owned
packed extent, rejection of wrong ownership or missing lifecycle exclusion,
refusal to release pinned or ambiguous extents, successful packed teardown,
slot reuse, and rejection of registry abort while an entry is live. A failed
registry MM free retains the runtime pointer; a successful abort clears it.
Both 24-slot configurations pass: dense at 24 descriptor words and sparse
with four entries at eight descriptor words. This test establishes the
staged interfaces can be composed without integrating a production codec.

The live GCC 32K image is NOT modified by v42: the Stage 41 VM destructor
continues to fail closed on PACKED rather than treating it as disk-backed.
No additional KCORE or runtime allocation is linked. Connecting the
registered descriptor path to production VM teardown, process restoration,
and KINIT remains blocked on measurable permanent-memory cost and complete
runtime failure-injection coverage. Production codec and scheduler eviction
remain deferred. The most recent measured active KCORE endpoint is 021541
octal (v41), above the required 020000 boundary.

## 2026-10-13 FAIL-CLOSED LIVE VM DESTROY V41

The live PDP-6 `vm_space_destroy` now calls the existing resident
`proc_storage_expect_v30` before any MM free or disk-swap detach. It accepts
RESIDENT and DISK storage only; DISK additionally requires a zero VM base.
PACKED, invalid, or missing map modes fail before changing MM ownership,
the swap record, or the process VM descriptor. This is a minimal safety
boundary while packed-registry activation remains postponed. The corrected
v40 failed-creation path and the exit path both hold TRANSITION during VM
destroy. The future packed teardown handler must be installed together
with boot registration before enabling any PACKED state publication.

The low-memory GCC three-member TSFS image built successfully and booted
in SIMH to DTFS LOADED / INIT V1 without observed HALT or ?RT. On the
current working tree the linked KCORE endpoint moved from v40's 021525
to 021541 octal: **12 decimal resident words** of growth. This does not
solve the hard KCORE shrink target; it is a fail-closed correctness guard.
Existing daimos-testkit packed-teardown-v39, packed-owner-v37 and
proc-abort-transition-v40 host tests still pass. Runtime failure injection
for a deliberately PACKED process is not yet tested; boot smoke covers only
the normal RESIDENT/DISK paths. Full dense/sparse registry code is not
linked into KCORE because that would add substantial permanent code before
packed eviction exists. Production compressor/decompressor integration
remains deferred.

## 2026-10-13 PROCESS ABORT LIFECYCLE TRANSITION V40

The live C `proc_slot_discard()` used to enter `vm_space_destroy()` without
holding `PROC_F_TRANSITION`, unlike the live assembler `proc_finish_slot`
exit path. Since packed teardown requires lifecycle exclusion, that would
make aborted process creation incorrectly reject PACKED cleanup. The abort
path now holds TRANSITION across VM destruction and clears it if VM release
fails; on success the slot is still cleared through its existing teardown.
The live packed registry and teardown hook are NOT activated by this patch.

Testkit `proc-abort-transition-v40` compiles the real `proc.c` against an
instrumented VM destructor and verifies transition-held destruction,
failure rollback, successful cleanup, and no duplicate destruction of a
FREE slot. PDP-6 GCC/DAS compile passed; lowmem GCC three-member TSFS
image built and booted in SIMH to DTFS LOADED / INIT V1 without a reported
HALT or ?RT. The measured KCORE endpoint for this working tree is 021525
octal, versus the previous 021520 snapshot; the tree also contains other
agents' uncommitted boot changes, so do not attribute all five words to
this change without an identical-tree A/B comparison. The 32K target is
still unmet. Codec, packed eviction and KINIT packed registry activation
remain deferred. The next step is paired boot registration and VM destructor
wiring, with a size gate that avoids excessive permanent KCORE growth.

## 2026-10-13 GUARDED PACKED PROCESS TEARDOWN V39

Added an inactive `proc_packed_teardown_guard_v39` interface for the common
VM destruction boundary used by normal process exit and failed-creation
cleanup. It requires process lifecycle TRANSITION, examines the explicit
two-bit per-slot storage mode, and leaves RESIDENT or DISK teardown to the
existing VM code. PACKED requires an installed, matching descriptor layout;
the guard invokes the common v37 MM-authoritative release and returns an
error if ownership, MM pins, or metadata are inconsistent. In that case
the caller MUST NOT clear the VM or reuse the process slot. An invalid
storage mode also fails closed. There is no compressor dependency.

PDP-6 GCC/DAS standalone build passed (49 text words before folding).
Testkit `packed-teardown-v39` passed for dense and sparse layouts, checking
PACKED cleanup and reuse, failed pinned release, absent registration, and
RESIDENT/DISK/invalid discriminators. The guard is NOT currently called
from `vm_space_destroy` or linked into production KCORE: KINIT has not yet
installed a persistent packed registry, and installing the hook alone would
leave an unusable PACKED path. No active KCORE or resident RAM savings
are claimed. Codec integration remains deferred.

## 2026-10-13 PACKED LAYOUT BOOT PREPARATION V38

Added an inactive transient-boot allocator for v36 dense/sparse descriptor
layouts (`proc_packed_boot_v38.c/.h`). It accepts configured slots, maximum
simultaneously packed processes and a descriptor-word budget supplied after
essential allocations, chooses the smaller feasible representation using
the existing v36 planner, then allocates exactly one zero-initialized
MM_TYPE_KERNEL_DYNAMIC extent. The four-word runtime descriptor is only
published once initialization succeeds. The boot preparation is explicitly
opt-in: production KINIT and process exit remain unchanged until complete
packed ownership, rollback and teardown paths are installed. Abort rejects
occupied slots and preserves ownership if MM refuses the free.

PDP-6 GCC/DAS standalone object compiles and assembles (100 text words,
4 BSS words before folding); the host `packed-boot-v38` test covers budget
and allocation failures, sparse and dense selection, duplicate setup,
occupied-table abort refusal, MM free refusal and successful reinitialization.
The v36 layout and v37 ownership tests also pass. These are isolated host
tests; no boot/runtime KCORE size or packed process acceptance is claimed.
Next: connect teardown, aborted creation and slot reuse consistently with
the enabled registry, then add production boot activation without making
the 32K permanent KCORE larger merely to support larger systems. Production
compression and decompression integration remain explicitly deferred.

## 2026-10-13 SHARED DENSE/SPARSE PACKED OWNERSHIP V37

Added an inactive single MM-authoritative ownership engine for either v36
descriptor layout (`proc_packed_owner_v37.c/.h`). It reconstructs packed
physical base/length from exactly one MM_TYPE_PACKED_PROCESS extent owned
by the process slot, validates process transition and PACKED discriminator,
and publishes only the writable layout. Teardown rejects pinned extents,
invalid ownership and an inconsistent mode without freeing the descriptor;
successful MM release clears the selected layout and resets the slot's
storage mode to RESIDENT. Neither operation allocates memory or calls a
compressor. This removes the need for separate duplicate MM validation
engines when selecting dense versus sparse in a future boot policy.

PDP-6 GCC/DAS standalone compilation passed (197 text words before folding
for the ownership engine, excluding v36 layout and other dependencies).
The new testkit `packed-owner-v37` passes for both layout formats, covering
transition/ownership checks, duplicate publication, pinned extent rollback,
ambiguous MM extent rejection and slot reuse. This is still inactive:
KINIT does not allocate a descriptor layout, and process exit/abort paths
do not call these functions. Activating a teardown hook before a supported
registry exists would be unsafe. No compressor integration, active KCORE
growth or saving is claimed. Next: define an explicit inactive-to-active
boot registration contract, then wire all teardown paths at the same time
as activating storage publication and checked restore descriptor consumption.

## 2026-10-13 BOOT-SELECTABLE PACKED LAYOUT PRIMITIVE V36

Added an inactive common descriptor-layout primitive for the dense/sparse
choice. `proc_packed_layout_plan_v36` chooses the smaller feasible data
representation given the configured process-slot count, expected maximum
packed concurrency, and caller-provided descriptor-word RAM budget. Dense
uses one layout word per slot; sparse uses two words per reserved entry
(slot + layout). Tie goes to constant-time dense lookup. The layout routines
implement bounded reservation, lookup, clearing, and slot reuse with no
dynamic allocation, MM mutation, or compiler/compression dependency.

PDP-6 GCC/DAS standalone object: 232 text words before folding. Testkit
`packed-layout-v36` passes layout selection, invalid/budget cases, sparse
exhaustion, duplicate reservation, clearing and slot reuse in both modes.
This is not yet the single shared ownership/teardown engine, and KINIT does
not allocate or select it in the production image. v35 remains a separate
MM-authoritative ownership prototype. Do not claim a 32K resident saving or
turn on packed eviction before lifecycle publication/teardown is unified
and the codec passes target execution tests.

## 2026-10-13 MM-AUTHORITATIVE PACKED DESCRIPTORS V35

The inactive `proc_packed_mm_registry_v35` prototype stores only the two
pieces absent from MM: a slot ID and the retained writable layout. Physical
base and encoded extent length are recovered from exactly one
MM_TYPE_PACKED_PROCESS extent belonging to that slot. Lookup rejects a
duplicate MM owner or duplicate registry slot rather than guessing which
descriptor is authoritative. Publication and discard require lifecycle
exclusion, PACKED mode, matching physical extent ownership, and the
process transition bit. MM release failure leaves the descriptor and mode
unchanged; successful release clears the two-word entry and storage mode.

The PDP-6 GCC/DAS standalone object assembles to 301 text words before
folding; the prior v34 sparse design used 277. Each v35 sparse entry uses
two runtime data words rather than v34's three, but the extra code means
there is no proven overall permanent RAM win. A dense layout-only table
would require one word per process slot but has not been implemented.
The daimos-testkit v35 host tests cover publication, duplicate entries,
ambiguous MM owner, wrong owner, pinned free rejection, teardown, and slot
reuse. No packed codec integration, KINIT allocation, process exit hook,
or KCORE link is enabled. The last measured live KCORE endpoint remains
021520 octal, pending a fresh whole-image build after other agents' work.

## 2026-10-13 SPARSE PACKED DESCRIPTOR REGISTRY V34

An inactive alternative to the dense v33 registry is now available as
`proc_packed_sparse_v34.c/.h`. Its caller-supplied bounded table holds only
three words per reserved packed-process entry (slot ID and the existing
two-word descriptor), not two words for every configured process slot.
Each entry is stable until explicit teardown. Publication validates MM
ownership and rejects duplicate slot claims; disposal checks ownership and
pin state before freeing the extent, clearing PACKED mode and making the
entry reusable. A full table returns MM_ERR_DESCRIPTORS. Calls are permitted
only under process-lifecycle exclusion; no allocations occur in the path.

GCC/DAS PDP-6 standalone assembly passed: 277 text words versus the v33
dense-registry object's 192, prior to folding. For a 24-slot machine the
dense table requires 48 descriptor words, while an eight-entry sparse table
would require 24 words. The sparse table is not necessarily superior: it
adds code, linear lookup, and a configured maximum simultaneous packed count.
Neither implementation is linked into KCORE or allocated by boot yet.
daimos-testkit `packed-sparse-v34` exercises publication, duplicates, MM
ownership mismatch, pinned release refusal, successful cleanup and reuse.
Do not activate packed eviction or integrate the under-development codec.

## 2026-10-13 PACKED DESCRIPTOR LIFETIME REGISTRY V33

Added a separate, inactive slot-indexed packed descriptor registry. Each
descriptor holds two 36-bit words. The owner checked at registration and
teardown is the MM extent type, owner slot, exact physical base and length;
registration rejects already-filled slots and requires PACKED mode plus the
existing process transition exclusion. Teardown checks the same ownership,
refuses pinned extents and preserves the descriptor and PACKED mode on MM
release failure; successful release clears both descriptor words and resets
the mode to RESIDENT for safe reuse of the slot. The registry consumes no
permanent RAM until enabled and allocated for the runtime slot count.

PDP-6 GCC/DAS standalone compilation passed. daimos-testkit
`packed-lifetime-v33` tests duplicate publication, pinned-release refusal,
successful cleanup, and rejection of MM ownership mismatches; the test
passes. These APIs are not yet linked into active KCORE or attached to
vm_space_destroy / process exit, because scheduler PACKED processes and the
production codec are not enabled. Before enabling packed eviction, allocate
the registry at boot, bind the existing v32/v31 descriptor argument to its
slot entry, and ensure all process destroy/abort/slot reuse paths invoke
checked descriptor teardown. No KCORE saving is claimed.

## 2026-10-13 PACKED PROCESS EVICTION TRANSACTION V32

Staged the complement to v31 restore: `proc_packed_evict_transaction_v32`
requires lifecycle exclusion plus PROC_F_TRANSITION, an exactly owned
MM_TYPE_PROCESS extent, RESIDENT mode, and an unpublished descriptor. It
allocates packed MM storage without recursive reclaim, pins the live source
and encoded destination while encoding, and refuses encoded capacities that
do not actually reduce the retained writable suffix. For a successful
fixed-capacity lossless encoder the reported encoded length must exactly
match the allocated span; failure leaves the live VM base, storage mode,
and packed descriptor unchanged. After the source extent is freed, the
descriptor and PACKED mode are published under the transition flag.

The testkit `packed-evict-v32` exercises encoder failure rollback,
non-shrinking rejection, and a synthetic lossless pair-encoding
eviction/restoration round trip with PURE reload. The v31 restore test
continues to pass. The v32 PDP-6 compiler/assembler build succeeded;
the complete inactive packed-process object has 608 pre-fold text words.
Neither eviction nor restoration is linked into active KCORE. The test
codec is only a controlled fixture, NOT a production compressor or a
general-purpose way to pack arbitrary process state. Scheduler integration,
full PURE ownership/pinning, descriptor lifetime across process exit,
real codec benchmarks and SIMH runtime tests remain prerequisites to
removing disk swap. No resident KCORE saving is claimed.

## 2026-10-13 PACKED RESTORE PURE TRANSACTION V31

An inactive `proc_packed_restore_transaction_v31` now performs a complete
prepare/reload/decode/commit sequence under caller-held process lifecycle
exclusion and PROC_F_TRANSITION. It validates the PACKED storage discriminator,
exact source MM ownership, and executable backing, then pins the packed source
and the new APR-aligned destination. It zeroes the omitted prefix, reloads
PURE backing (including compressed DXR that may overwrite the entire image),
then decodes the saved writable suffix. It does not publish process residency
or free packed storage on ordinary decode/reload failure. After successfully
freeing packed storage it publishes VM base and RESIDENT mode before dropping
the destination pin and transition flag. Fatal late MM failures retain the
transition flag, rather than treating a partial commit as runnable.

PDP-6 GCC/DAS standalone build succeeded (438 total text words for the entire
unlinked packed-process object). Host testkit `packed-restore-v31` checks
decode rollback, incorrect executable backing rejection, and PURE-reload-first
ordering when the reloader overwrites the entire destination image. The host
test uses explicit PDP-6 word-address to native byte-address translation.
No KCORE size change or active RAM saving is claimed: the v31 object remains
unlinked. Packed eviction, descriptor lifetime/discriminator integration,
scheduler transactions and PDP-6 SIMH runtime failure tests are not complete.
Do not remove disk swap or enable PACKED process scheduling yet.

## 2026-10-13 DISK SWAP STORAGE-MODE TRANSITIONS V30

The existing PDP-6 assembler `proc_swap_out` and `proc_swap_in` now check
the live two-bit storage map before beginning disk swap transactions, then
publish RESIDENT->DISK or DISK->RESIDENT after successful MM/storage metadata
updates and before clearing PROC_TRANSITION. A late impossible mismatch halts
with the transition still held rather than releasing an incoherent process.
`proc_swap_detach` also clears DISK mode on nonresident-process destruction.
The transition helpers use short PDP-6 assembly with no added process frames
or allocations. No PACKED state is published; no packed eviction is enabled.

The GCC lowmem ROOT=tape BOOT=dtc three-member image builds and reaches
`DTFS LOADED` / `INIT V1` in SIMH with no observed ?RT or HALT. KCORE endpoint
is 021520 octal versus 021412 in the preceding v29 build; these correctness
guards add 70 words after folding. The 100-word planning reserve is a budget,
not permission to treat growth as optimization. Dedicated disk-backed
swap-out/in/detach runtime transitions and failure injection remain untested;
TSFS boot smoke does not exercise them. Leave the packed-process transition
disabled until PURE reload, mode publication and MM release are atomic.

## 2026-10-13 LIVE STORAGE MAP BOOT ALLOCATION V29

The existing proc_swap_boot_init() dynamic swap-record MM extent now also
contains a compact 2-bit-per-slot storage-state map after the backstore
bitmap. It is zero-initialized with the allocation, published via the
resident two-word proc_storage_runtime_v29 descriptor, and requires no
additional MM extent or fixed 256-slot table. At 24 slots, this consumes
two extra dynamic words and two KCORE words for the pointer and slot count.
The GCC lowmem ROOT=tape BOOT=dtc three-member TSFS image built and booted
to `DTFS LOADED` and `INIT V1`. Linked KCORE endpoint is 021412 octal,
versus 021410 in the prior same-tree measurement. The map remains
read-only RESIDENT state in production until transitions can be integrated
with the scheduler and disk swap consistently. No packed eviction is enabled;
do not describe this as an optimization saving or passed login test.

## 2026-10-13 PACKED PROCESS STORAGE DISCRIMINATOR V28

Added an inactive 2-bit-per-process storage-state map with exact slot-count
capacity: 24 lowmem slots need two 36-bit words; 256 slots need 15 words.
States are RESIDENT, DISK, PACKED, and one invalid encoding. This avoids
overloading the legacy swap backing record or the two fully occupied process
metadata flag bits. The API supports caller-owned zero-initialized storage,
bounded lookup, and compare-and-change with validation. Callers must hold the
process lifecycle lock/exclusion: the map itself is not an atomic primitive.
PDP-6 GCC/DAS standalone compilation passed (93 text words). It is not yet
allocated at boot or wired into the scheduler; therefore no KCORE or process
RAM saving is claimed. The descriptor and complete PURE reload/commit
transaction, as well as tests in sibling daimos-testkit, remain outstanding.

## 2026-10-13 PACKED RESTORE MM OWNERSHIP CHECK V27

The inactive packed-process restore preparation now validates that the
compressed source extent has the descriptor's exact physical base and word
length, MM_TYPE_PACKED_PROCESS, and the requested slot as MM owner before
pinning/decoding it. `mm_pin` alone checks existence, not type or ownership;
without this validation a stale or foreign descriptor could read another
MM allocation. The source remains protected by its pin while preparing the
aligned destination. PDP-6 GCC and DAS compile the isolated object (245
text words), but it remains unlinked from KCORE. This is a correctness
prerequisite, not a KCORE saving; full failure-injection tests in testkit
and an explicit lifecycle storage discriminator are still required.

## 2026-10-13 PACKED PROCESS RESTORE PREPARATION V26

An inactive `proc_packed_prepare_restore_v26()` API now allocates a fresh
APR-aligned process extent without invoking recursive reclaim, pins the
packed source across allocation/decoding, and decodes its retained writable
suffix through a caller-provided codec. Decode failures release the new
extent and leave the packed descriptor unchanged. The prepared destination
stays MM-pinned because the unpublished VM base cannot safely be discovered
by compaction. `proc_packed_abort_restore_v26()` unpins and frees it.

This is *not* a complete process restoration transaction: the PURE prefix
still requires executable backing reload, and commit must atomically install
the VM base/storage discriminator and release the packed source and destination
pin. The identity codec is not used for eviction. PDP-6 GCC/DAS standalone
assembly succeeds (206 text words for the whole inactive proc_packed object).
No new KCORE code is linked and no runtime RAM or size savings are claimed.
The planning target continues to reserve 100 extra KCORE words above the
current measured state. Follow-on behavioral and failure tests belong in
daimos-testkit before integration into the live lifecycle.

## 2026-10-13 GCC KCORE SWAP DETACH CONSOLIDATION V24

Consolidated the resident executable-backing clear and legacy nonresident
disk-swap release into the existing `proc_swap_detach()` implementation.
The conditions and release order are preserved; no packed-process eviction
is enabled, and the lifecycle and scheduler remain resident. A paired GCC
lowmem ROOT=tape comparison with identical other working-tree inputs
measured 021416 -> 021410 octal KCORE, six words saved after P10fold.
The complete three-member TSFS image build also succeeds. Runtime swap
detach/restore regression tests and interactive DTFS/TSFS boot acceptance
remain outstanding; do not treat a successful link as runtime acceptance.

## 2026-10-13 GCC 32K THREE-MEMBER TSFS MEDIA V21

The PDP-6 boot image builder now accepts TSFS_ROOT_MEMBERS=1,2,3. A
versioned media-layout stamp regenerates TSFS metadata when the member
count changes. In the BOOT=dtc configuration, Stage1 boots from DTC0 and
the system TSFS members occupy DTC1..DTC3. The SIMH boot template exposes
all four DECtape drives. A fresh GCC lowmem ROOT=tape BOOT=dtc
TSFS_ROOT_MEMBERS=3 MONITORFS_MRES_ENABLED=1 image loads under 32K SIMH,
installs DTFS/TSFS, and reaches INIT V1. KCORE endpoint is still 021375.

Important: this is a **three-member TSFS system boot smoke**, not the
complete requested physical four-drive steady-state arrangement. DTC0
currently holds the separate bootstrap image, so a fourth writable DTFS
`/home` volume cannot occupy a drive simultaneously. A correct solution
needs an in-place bootstrap format coexisting with a filesystem tape (or
another explicitly approved bootstrap medium). Avoid automatic media swaps
or overwriting the user's writable DTFS tape. LOGIN and /home mount tests
remain outstanding. Do not claim the 3+1 acceptance complete.

## 2026-10-13 GCC 32K TSFS ROOT BOOT RESTORATION V20

Decoupled TSFS boot and DTFS/TSFS MRES installation from KINIT_FULL using
KINIT_TAPE_BOOT (derived from TAPE_FS_ENABLED). GCC lowmem ROOT=tape now
assembles the TSFS discovery/mount path, DTC/MTC MINIT entries and the
filesystem MRES package without enabling unrelated full-profile modules.
The DTC Stage1 loader accepts a zero compressed-word count and loads the
raw lowmem KINIT stream directly at 030000, avoiding an image-plus-staging
copy exceeding 32K. BOOT=auto selects DTC for ROOT=tape; lowmem permits
BOOT=dtc. The lowmem MRES source link order matches the MINIT traversal
(TAPE then DSK then DTFS); an earlier reversed order halted at the
module_run_minits package monotonicity check.

Fresh GCC lowmem ROOT=tape BOOT=dtc builds with resident MonitorFS and with
MONITORFS_MRES_ENABLED=1 both boot a single-member TSFS root successfully
through the `DTFS LOADED` message to `INIT V1` under PDP-6 SIMH 32K.
Both variants pass the new daimos-testkit TSFS v20 INIT boot smoke test.
The GCC MRES configuration reports 12612 decimal words of permanent
KCORE+MRES (end 030504 octal) and `__kcore_low_end=021375` octal.
The MonitorFS-MRES KCORE endpoint remains 021375 octal (resident MonitorFS:
023360). LOGIN/MonitorFS VFS acceptance is not yet established. The image
builder currently creates a one-member TSFS root on DTC1, with a separate
DTC0 bootstrap tape. It does NOT yet satisfy the desired four-drive layout
(three TSFS system tapes plus a separate DTFS /home tape). That media/boot
layout and interactive login tests remain required before declaring the
32K DECtape configuration complete.

## 2026-10-13 LOWMEM LOGIN DIAGNOSTIC V19

The MonitorFS v17 testkit runner omitted DCS transport arguments although
the lowmem inittab starts LOGIN on DCS line 0, not CTY. The runner is being
corrected to request TCP relay and DCS port 1101. A manual baseline rerun
with DCS attached established a DCS connection but halted with `?RT` at
PC 062054 before LOGIN. Therefore the prior CTY-only timeout was a harness
error; the new early-boot fatal is a separate unresolved blocker. Do not
claim runtime MonitorFS acceptance. Both root/media state and KINIT boot
failure handling must be checked before attributing this to the MRES change.

## 2026-10-13 MONITORFS GCC BASELINE BOOT COMPARISON V18

A fresh GCC lowmem image was rebuilt with MONITORFS_MRES_ENABLED=0
(original fixed resident MonitorFS); KCORE endpoint is 023360 octal.
The existing daimos-testkit MonitorFS v17 SIMH harness reports the SAME
pre-LOGIN stall as the experimental MONITORFS_MRES_ENABLED=1 image: both
reach INIT V1 and print `INIT: WARNING: MEMFS NOT MOUNTED ON /TEMP`, but
neither reaches LOGIN within the harness timeout. This isolates the LOGIN
blocker from the optional MonitorFS relocation: the observed failure is
not unique to the MRES configuration. It does NOT prove that MonitorFS
MRES runtime VFS behavior works; those post-login tests remain blocked.
Do not enable MonitorFS MRES by default until the baseline LOGIN issue
is diagnosed and the full acceptance probes pass.

## 2026-10-13 MONITORFS MRES BOOT-INSTALL FIX V17

The experimental GCC lowmem MonitorFS MRES configuration now builds a
complete boot image and reaches `INIT V1` on a fresh 32K PDP-6 simulator.
It remains opt-in (`MONITORFS_MRES_ENABLED=1`) until the MonitorFS namespace
probes pass after user login. The 32K and diagnostic 64K lowmem harnesses
both reached INIT but did not obtain a LOGIN prompt within their timeout;
therefore live MonitorFS VFS semantics are NOT yet accepted.

The runtime failure in v16 was diagnosed in two stages: the MRES package
source must appear in ascending MINIT traversal order, and a D6FS MINIT
dynamic 14-word MM allocation prevents a later MRES package from occupying
the required next contiguous address. The correct solution separates early
`mfsdev_install_minit` (after CLK, before dynamic allocations) from the
original late `mfsdev_minit` name publication (after driver discovery).
The MRES package itself appears immediately after CLK in the boot image.

The initial C symbol aliases for writable jump words generated invalid
GCC assembly operands; dedicated ordinary external assembly labels now
provide exact patch addresses. An unnecessary static pointer table that
emitted an unsupported `.long` directive was also eliminated. Build-mode
stamps ensure switching MONITORFS_MRES_ENABLED between 0 and 1 rebuilds
the correct KINIT and KCORE outputs in the same build directory.

GCC lowmem KCORE link is still 021375 octal (versus 023360 resident),
saving 01763 octal (1011 decimal) low-core words. MonitorFS MRES occupies
1030 words, so *net permanent RAM increases by about 19 words*; the
020000 octal KCORE hard target still requires 01376 octal more saving.
The revised size report handles the selected lowmem MRES set. The full
image was built after installing authoritative KCC/DAIMOS toolchain
components under PDP10_PREFIX. Five versioned MonitorFS acceptance probes
and a fresh-build test target are in sibling daimos-testkit; probe syntax
checks pass, but behavioral tests remain blocked before LOGIN.

## 2026-10-13 MONITORFS GCC MRES INTEGRATION V16 (BOOT PENDING)

The opt-in MONITORFS_MRES_ENABLED=1 configuration substitutes nine KCORE
dispatch words for the original MonitorFS implementation. An MRES package
is installed at the final MINIT entry and each VFS-facing jump is patched
to its relocated export. The package comes from the original assembly source.

GCC KCORE low end: 023360 -> 021375 octal, a reduction of 01763 octal
(1011 decimal) words. The installed MonitorFS MRES requires 1030 words,
so total permanent RAM increases by roughly 19 words; this is not net
memory recovery. KINIT image linking succeeds. The profile-aware
permanent-size report gives KCORE+MRES 030704 octal, PERMANENT_LAST
030763 octal. The KCORE 020000 limit still needs 01376 octal more savings.

Full image generation and SIMH testing remain BLOCKED: the external DAS
native self-build manual-check requires sixmd-check, which is absent from
the configured tool prefix. The MRES configuration is opt-in until those
tests validate boot and MonitorFS semantics.

## 2026-10-13 MONITORFS MRES LINK PROBE V15

The standalone MonitorFS MRES link probe succeeds under DAS/DLINK against
the current GCC KCORE map. tools/host/monitorfs-mres-probe-v15.py derives
the probe from the sole authoritative assembler source and renames public
entries for collision-free linking with resident MonitorFS. The resulting
MRES contains 1030 initialized words and zero BSS. This does NOT install
the module or change KCORE residency; MINIT and VFS dispatch integration,
real boot validation, and total permanent-memory accounting remain open.

## 2026-10-13 MONITORFS MRES DISPATCH STAGING V14

Added an inactive nine-entry VFS-to-MonitorFS jump interface in
fs/monitorfs_dispatch_v14.s. Each one-word entry fails safely until MINIT
installs its relocated destination. This is staging source only: it is NOT
linked into KCORE and does not replace the working resident MonitorFS.
Next wire the pinned MRES package, exports and MINIT patching as one atomic
change, then boot-test VFS before removing the resident implementation.

## 2026-10-13 MONITORFS MRES DEPENDENCY ANALYSIS V13

See design/monitorfs-mres-extraction-v13.md. The 1030-word GCC MonitorFS
object is referenced directly from VFS; its MINIT table entry currently has
no MRES package. Moving it needs resident dispatch and pinned service
lifetime. No resident code is removed in this stage, no boot regression is
claimed, and the GCC KCORE endpoint remains 023360 octal.

## 2026-10-13 GCC KCORE REPEATABLE SIZE AUDIT V12

Added tools/host/kcore-size-audit-v1.sh to report a selected fixed-KCORE
object size breakdown from a GCC build independently of the folded link
endpoint. Current endpoint remains 023360 octal. Large savings require
structural work in process control, file/MonitorFS runtime, and disk-backed
swap; the DSK/DRM interface leaf objects are only 37 decimal words combined
before folding. This stage does not change kernel code or claim any savings.

## 2026-10-13 GCC MONITORFS FALLTHROUGH CLEANUP V11

Removed four unconditional jumps to immediately following local labels in
monitorfs_runtime.s. The folded GCC lowmem KCORE link moves from 023361
to 023360 octal: a verified saving of one permanent word after folding.
This is intentionally a small semantics-preserving assembly cleanup; no
filesystem functionality was disabled. Full link passes. Runtime tests
remain outstanding. Next savings must come from larger structural splits.

## 2026-10-13 STRICT GCC 32K KCORE ACCEPTANCE GATE V10

Run make kcore-32k-gate BOOT_PROFILE=lowmem COMPILER=gcc with a clean
GCC-specific build directory. The dedicated check rejects missing/invalid
link-end symbols and requires __kcore_low_end strictly below 020000 octal.
Ordinary development builds intentionally remain possible above the limit.
The gate tests KCORE only; MRES, process availability and actual boot require
separate acceptance tests. The active GCC baseline remains 023361 octal.
The GCC object-level triage is in design/kcore-gcc-size-audit-v10.md;
proc_pdp6, file_runtime and monitorfs_runtime are the largest candidates.

## 2026-10-13 32K GCC KERNEL BUILD POLICY V9

The 32K lowmem boot profile now defaults to GCC. KCC remains selectable
for larger configurations and explicit legacy comparisons. All future 32K
KCORE/KINIT measurements use COMPILER=gcc and a clean compiler-specific
build tree; the previous KCC link endpoint 023617 is historical only.
The strict 32K KCORE goal is a low-core end address below 020000 octal.
The clean GCC lowmem link succeeded with __kcore_low_end=023361 octal,
compared with historical KCC 023617. GCC saves 0236 octal words at this
checkpoint; a further 03362 octal words must be removed to get below 020000.
This is a link result, not a boot/runtime validation.

## 2026-10-13 PACKED MM ALLOCATION HELPERS V8

Staged proc_packed_reserve_v1/proc_packed_release_v1 helpers allocate
unaligned high-core MM type-4 extents with slot ownership, validation,
no reclaim recursion, and release-only descriptor clearing. They do not
publish packed process state or modify legacy swap records. In a trial link
they increased KCORE by 0145 octal words, so they are deliberately NOT linked
in the production resident kernel until the feature is activated. Eviction
and restoration remain disabled pending atomic transitions and rollback.

## 2026-10-13 PACKED PROCESS DESCRIPTOR V7

The inactive proc_packed_v1 two-word descriptor now defines the physical
encoded span and contiguous logical writable suffix, together with distinct
RESIDENT/DISK/PACKED lifecycle tag values. No per-process storage has been
added, no runtime path uses the descriptor, and packed eviction remains off.
Before integration, state tags need a concrete home, atomic transition rules
and MM owner validation; skip the unused middle of memory only with ABI proof.

## 2026-10-13 PACKED PROCESS MM OWNERSHIP RESERVATION V6

MM type 4 is reserved for packed writable process state, distinct from
ordinary aligned process images (type 1), modules (type 2) and dynamic kernel
allocations (type 3). The packed contract is in proc_packed_v1.h. No packed
state is allocated, no ambiguous disk-swap record is reinterpreted, and MM
compaction deliberately does not move the new type without an owner-update
transaction. Runtime behavior is unchanged; no eviction is enabled.

## 2026-10-13 PROCESS BACKING CLEANUP BOUNDARY V5

proc_swap_detach() now separates the legacy backstore-span release from
the executable backing cleanup in named helpers. The existing three-way
ownership decision still depends on VM residency: there is NO new packed
state tag, and compressed eviction remains disabled. KCC and full KCORE
link pass. Next implementation must introduce explicit, validated packed
state ownership before sharing cleanup with compressed in-core processes.

## 2026-10-13 PURE BACKING GUARDS SOURCE SEPARATION V4

Executable vnode mutation and filesystem-unmount busy checks now reside in
proc_pure_backing_pdp6.s with executable unpack/reconstruction.  Existing
entry names and packed backing metadata remain unchanged.  KCC generation and
the PDP-6 DAS proc_swap object pass.  proc_swap_detach still releases disk
backstore for nonresident processes: it must not be reused for compressed
resident pack allocations until a distinct state tag and transaction-safe
cleanup are introduced.  No packed eviction is activated by this patch.

## 2026-10-13 PURE BACKING ATTACH SOURCE SEPARATION V3

The provider-neutral executable backing record encoder proc_swap_attach()
now resides in proc_pure_attach.c. KCC includes it through the existing
vm_pdp6_swap.c translation unit; emitting two independent KCC .s files into
one object caused colliding local labels (%L3), so no fragile label renaming
was accepted. Existing process-swap detach and disk transactions stay put.
The PDP-6 DAS proc_swap object assembles after the change. This does not yet
remove swap from KCORE or enable compressed process packing.

## 2026-10-13 PURE BACKING SOURCE SEPARATION V2

PURE executable vnode unpacking and raw/compressed DXR2 text reconstruction
have moved from proc_swap_pdp6.s into proc_pure_backing_pdp6.s.  The build
still concatenates these leaves into the existing resident proc_swap object,
preserving its entry symbols and swap contract.  The dedicated DAS object
build passes.  This is a source boundary, not yet an MRES extraction or a
measured KCORE reduction.  Next separate the process-state lifecycle from
backstore transactions and ensure compressed resident packing can use the
PURE reload path without disk swap.

## 2026-10-13 DSH V25 OPTIONAL HISTORY FOR 32K

User approved disabling shell history during the current shrink phase,
provided the implementation remains available behind a compile-time
ifdef.  `DSH_ENABLE_HISTORY` defaults to 0; define it as 1 to enable
the existing Ctrl-P and HISTFILE behavior.  The opt-in history cache
is lazily allocated via the shared DSH heap, not static BSS or bare BRK.
GCC image/BSS octal baseline v24: 032104/013742; history-off v25:
031321/012751; history-on v25: 032120/012763.  The history-disabled
default passes full SIMH conformance 115/115; history-on behavior still
requires dedicated regression acceptance.
Strict below-030000 image gap is now 01322 octal in the default mode.

## 2026-10-13 KCORE PACKED PROCESS PREPARATION V1

The first staged interface for future compressed-process eviction is in
system/kernel/core/kpack_codec_v1.[ch].  V1 is an identity codec, is not linked
into the running kernel, and does not reclaim RAM.  No-swap PURE eviction must
NOT be activated until packed-process MM transactions and an actual compressor
are implemented and verified.  Design and next stages are recorded in
design/kcore-packed-process-v1.md.  Existing swap remains unchanged.

Minimum hardware profile revised to three TSFS tapes plus one native DTFS
writable /home tape. CLK is mandatory; the PCLK wall-clock reader should be
split out later. Operator civil-date entry before login is deferred, and
no uninitialized clock substitute may replace the current PCLK call path.

## 2026-10-13 DSH V24 SCRIPT WORKSPACE CLEARING

The 32-token DSH script workspace is now cleared in one word-wise pass
instead of calling `dsh_token_clear` for every element.  All fields are
PDP-6 word-sized, DSH_T_EOF is zero, and required zero-initialization
semantics are preserved.  GCC DXR image shrinks from 032117 to 032104
octal words, BSS remains 013742.  The dedicated nested SOURCE suite
passes 4/4 and full GCC SIMH DSH conformance passes 115/115.  The
strict below-030000 text gate still needs 02105 octal words removed;
physical RAM/BSS/stack/heap work remains open.  No overlays.

## 2026-10-12 DSH V23 STORED-FUNCTION WALKER SHRINK

The AST function-node preflight count and copy use one bounded walker,
with an optional destination.  Preflight remains read-only and occurs
before any existing definition is removed.  GCC executable image drops
from 032147 to 032117 octal words, saving 030; BSS stays 013742.
The full GCC target SIMH conformance passed 115/115; no overlay changes.

## 2026-10-12 DSH V22 PARSER NODE INITIALIZATION SHRINK

An experimental simplification of `dsh_node_clear` replaces repeated
`dsh_s6_clear` calls with one word-wise zeroing pass, then restores the
three DSH_NONE links.  GCC DXR image changes from 032177 to 032147
octal, a reduction of 030 octal words.  BSS remains 013742.  Direct
host parser test and the seven-case live shared-heap SIMH suite pass.
Full GCC SIMH conformance passed 115/115 after the change.  This
source-level reduction is accepted on the experimental branch, but the
strict 030000 text target and physical 32K resident limit remain open.

## 2026-10-12 DSH V21 STRICT IMAGE/BSS/STACK SHRINK PLAN

The accepted optimization target is now strictly below 030000 octal GCC
DXR image words, with further reductions of BSS and verified peak stack
and heap use.  Current shared-heap GCC image 032177, BSS 013742; at
least 02200 octal image words must be eliminated.  The existing 036000
image gate is no longer sufficient as the optimization goal.  GCC `-Oz`
is unsupported (cc1 rejects it); baseline remains GCC `-Os`.  The
prioritized, no-overlay measurement and shrink plan is in
`userland/dsh/TODO.MD`.  No new code optimization is accepted in V21.

## 2026-10-12 DSH V20 GCC-REQUIRED 32K BUILD POLICY

The corrected experimental shared-heap DSH links to image 032177 and
BSS 013742 octal words under GCC, versus image 037000/BSS 013742 under
KCC.  GCC therefore saves 04601 octal executable words and satisfies
the production 036000 DXR gate.  The full 115/115 DSH conformance and
7/7 dedicated shared-heap SIMH regressions pass with COMPILER=gcc.
The agreed configuration policy is that 32K DAIMOS must use GCC; KCC
remains supported for larger memory configurations.  This is not yet
proof of successful 32K DSH launch: INIT residency, process allocation
rounding, fragmentation and actual heap high-water remain open.
See design/dsh-compiler-size-comparison-20261012-v20.md.

## 2026-10-12 DSH V17 SHARED-HEAP DOUBLE-FREE FIX AND ACCEPTANCE

The experimental two-word DSH variable descriptor plus shared heap now
correctly distinguishes occupied variable slots from freed slots.  A new
SET into a slot previously UNSET must not release that slot's stale
payload pointer a second time.  UNSET now clears the pointer, and SET
checks occupancy before releasing an old payload.  Without this fix,
a seven-case target heap stress regression timed out in its final case;
after the fix all 7/7 pass under SIMH.  Final measured DXR image 037000,
BSS 013742 octal, compared to clean-main 036523/015241.  Image exactly
meets the temporary 037000 gate but still fails production 036000.
The corrected implementation also passed the full 115/115 SIMH conformance
suite.  The separate argument-frame suite passed 9/9.  The testkit's
resource-contract fixture was corrected from the obsolete four-entry
history assumption to the implemented sixteen-entry history limit and
passes all eleven fixed-limit checks.  It remains on the experimental branch pending heap high-water
accounting and production image shrink.  No overlays.

## 2026-10-12 DSH VARIABLE EXPERIMENT V17 (NOT ACCEPTED)

An isolated assembly variable-table rewrite plus shared C word heap
compiled and linked with image 036773, BSS 013742 octal against clean
current-main 036523/015241.  Combined image+BSS improves by 01027 octal
words; this is not a demonstrated reduction in physical APR allocations.
The temporary 037000 executable gate passes, while the production 036000
gate does not.  Full target regression is blocked before execution by a
missing native KCC bootstrap at `$HOME/cross/lib/kcc/bootstrap/KCC.dxr`.
V17 must remain experimental until bootstrap, target regression, and peak
memory acceptance are completed.  No overlay code introduced.

## 2026-10-12 DSH VARIABLE ALLOCATOR SIZE EXPERIMENT V16

Clean current-main PDP-6 KCC DSH baseline: DXR image 036523 and BSS 015241
octal.  A shared dynamic allocator for packed variable payloads and the
temporary 20-argument expansion compiled and linked with image 037213 and
BSS 013742.  Combined image+BSS fell by 0607 octal words, but the image
exceeded both the temporary 037000 and production 036000 limits.  No runtime
acceptance was run.  All prototype code was reverted; current production
DSH is unchanged.  The valid shared-lifetime concept requires smaller
assembly implementation and full regression, not the earlier unsafe direct
BRK-restore approach.  Detailed analysis is in
`design/dsh-variable-allocator-shrink-20261012-v16.md`.

## 2026-10-12 P10FOLD PDP-6 I/O SKIP SAFETY AND MULTI-TTY

- Fixed an unsafe zero-cost fold of GE driver CONSO/JRST polling prefixes.
  The CONSO skips the JRST, so identical two-word prefixes with different
  fall-through continuations are not equivalent. P10FOLD now recognizes
  BLKI, BLKO, CONSZ and CONSO I/O skips (encoded in the function field),
  and conservatively treats XCT as capable of propagating a skip.
- The GE-MRES fold plan no longer merges the two unsafe polling loops.
  The full CTY/DCS0/GE0 multi-TTY functional regression passes under both
  GCC and KCC, including terminal routing, LOGIN respawn, memory accounting,
  and STOP isolation. The testkit now discounts the temporary PS process
  from its persistent-process count, while requiring exactly one PS entry.
- The separate GCC/KCC kernel-and-userland compiler-selection regression
  also passes under both selections. The corrected P10FOLD is installed
  under PDP10_PREFIX using a versioned local.tar.gz distribution.
- DSH image-size acceptance is unchanged and remains deferred.

## 2026-10-12 FULL GCC/KCC BUILD SELECTION FOLLOW-UP

- `COMPILER=gcc|kcc` now selects the compiler through boot KINIT, KCORE,
  and native userland translation units, including compiler-specific build
  prerequisites. The previous userland KCC-only recipe path is eliminated.
- GCC native executables link the target libgcc archive. GCC's C-entry
  `__main` ABI has a dedicated one-word startup stub only in GCC builds;
  KCC builds do not include it.
- The versioned testkit compiler-selection probe checks actual compiler
  invocations for kernel and full userland, detects mixed-compiler builds,
  and verifies representative linked DXRs. GCC and KCC both PASS.
- A complete GCC image build passed with diagnostic
  `DSH_IMAGE_LIMIT=037000`; the production `036000` gate is unchanged and
  remains a known independent failure. No DSH-size acceptance was claimed.
- GE0 multi-TTY login is still under investigation; its standalone carrier
  test is not green. This compiler-selection change does not claim to fix it.

## 2026-10-12 TESTKIT COMPILER SELECTION

- `daimos-testkit` now accepts `COMPILER=gcc` and `COMPILER=kcc` (default)
  and forwards the selected compiler through its regression runner.
- `system/kernel/Makefile` honors the exported `COMPILER` while preserving
  explicit command-line `CC` overrides.  The GCC and KCC kernel compile
  commands were verified independently.
- Testkit build roots and regression checkpoints are compiler-specific.
  Both host `check` variants passed, as did representative GCC and KCC
  PDP-6 target probes.  The complete dual-compiler regression suite has
  not yet passed; DSH size acceptance remains excluded from that objective.

## D6FS LIVE PDP-6 APPEND INTERRUPTION AND REBOOT ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT `9CAF59F` + `91B19FD` IMPLEMENT A TEST-ONLY GENERATED
BACKING-WRITE HALT AFTER A SUCCESSFUL WRITTEN-DATA MARKER. LIVE PDP-6
SIMH RAN AN APPEND TO PRESEEDED `/CONFIG/FCBSEED`, HALTED INSIDE THE
BACKING CALLBACK AT OCTAL PC 032342 BEFORE THE WRITE RETURNED, AND LEFT
THE SELECTED SUPERBLOCK DIRTY (0671, SEQUENCE 2). A FOUR-MEMBER CLONE
WAS REPAIRED OFFLINE WITH D6FSCK -R TO A CLEAN SUPERBLOCK (0670, SEQ 3),
179196 BLOCKS AND 368 FCBS. A FRESH SIMH BOOT FROM THE REPAIRED CLONE
PASSED THE C99 ORIGINAL-WORD/ONE-WORD-EOF READBACK, STAT CONFIRMED
WORDS 1, AND DSH COMPLETED A NORMAL HALT. SOURCE DISK MEMBERS REMAINED
UNCHANGED THROUGH REPAIR AND REBOOT. NO PRODUCTION KERNEL OR DISK FORMAT
WAS ALTERED.

THIS IS ONE LIVE FILE-DATA-WRITE INTERRUPTION BOUNDARY AFTER A SIMULATOR
BACKING CALLBACK, NOT HARDWARE POWER-CUT DURABILITY PROOF. DIRECTORY/FCB
AND EXTENT-ALLOCATION CRASH CUTS, BOOT REPLAY, AND CRASH-ATOMIC HISTFILE
WITH CONCURRENT WRITERS REMAIN OPEN. DO NOT GENERALIZE ONE-WORD EOF
PRESERVATION TO COMPLETE FILE TRANSACTIONS.

## D6FS OFFLINE ORPHAN RECOVERY AND CHANGED-BLOCK REPAIR (2026-10-12)

DAIMOS-TESTKIT `092A67A` EXECUTES REAL FOUR-MEMBER DISK-IMAGE REPAIR,
NOT AN ABSTRACT REPLAY MODEL. THE ORIGINAL HOST D6FSCK RECOVERED FROM
ALL 48 FSYNC-COMPLETE WRITE INTERRUPTIONS WHILE RECLAIMING TWO
ZERO-EXTENT ORPHANS (0451/0452). THE HOST D6FSCK NOW COMPARES STAGED
FCB, FREEMAP AND SUMMARY BLOCKS AGAINST BACKING MEDIA AND WRITES ONLY
CHANGED BLOCKS. THE SAME REPAIR NEEDS THREE DURABLE WRITES (DIRTY
SUPERBLOCK, ONE MODIFIED FCB BLOCK, CLEAN SUPERBLOCK), AND ALL THREE
INTERRUPTION CUTS RECOVER CLEANLY AND IDEMPOTENTLY. NO KERNEL RAM OR
ON-DISK FORMAT CHANGE.

SEPARATE CLONED-IMAGE OWNERSHIP TESTS PASS: A SINGLE-OWNER ORPHAN FCB
WITH AN EIGHT-BLOCK EXTENT IS RECLAIMED AND ITS ALLOCATION BITS REPAIRED;
AN ORPHAN SHARING EXTENTS WITH A REACHABLE FILE IS REPORTED AMBIGUOUS
AND REPAIR WRITES NOTHING. ALL SOURCE IMAGES REMAIN UNMODIFIED.
THESE ARE OFFLINE HOST-TOOL RECOVERY RESULTS AFTER SYNTHETIC FSYNC
INTERRUPTIONS, NOT PROOF OF POWER-LOSS ATOMICITY. BOOT REPLAY,
CRASH-ATOMIC DSH HISTFILE AND CONCURRENT-WRITER TESTS REMAIN OPEN.

## D6FS ACTUAL DIRECTORY-ENTRY IDENTITY AUDIT (2026-10-12)

DAIMOS-TESTKIT `92A9622` INSPECTS THE PRESERVED FAULTED AND CONTROL
DISK IMAGES USING THE EXISTING D6FSCK DIRECTORY WALK. ON THE FAULT
IMAGE FCBS 0451 AND 0452 ARE TYPE-1 ZERO-LENGTH FILES WITH ZERO
DIRECTORY REFERENCES AND ARE UNREACHABLE. ON THE CLEAN CONTROL FCB
0451 HAS SIX WORDS AND ONE REFERENCE: ITS ENTRY IS IN PARENT FCB 0,
SLOT 7, CACHED TYPE 1, WITH NAME WORDS 164463507750:516364576271:
000000000000:000000000000 (OCTAL). FCB 0452 IS FREE. THE CONTROL
CHECKS CLEAN; THE FAULTED IMAGE DOES NOT. THIS DIRECTLY SUPPORTS
MISSING DIRECTORY PUBLICATION UNDER AMBIGUOUS CREATE COMPLETION.
IT DOES NOT YET PROVE PRECISE FAULT HIT COUNT OR CRASH-SAFE REPLAY.
NO KERNEL OR ON-DISK FORMAT CHANGE; PHASE ONE REMAINS OPEN.

## D6FS RECOVERY INTENT IDENTITY CHECKS (2026-10-12)

TESTKIT `7CB07F3` ADDS A HOST-ONLY C99 MODEL COMPARING CHILD FCB INDEX,
PARENT FCB, DIRECTORY SLOT, OBJECT TYPE AND FOUR SIXBIT NAME WORDS.
18 STATE COMBINATIONS PASS; 15 REQUIRE FURTHER INVESTIGATION, ONE
FULLY MATCHING/VISIBLE PAIR IS ALREADY PUBLISHED, AND TWO ALL-ABSENT
CASES ARE ONLY LOCALLY SAFE (ALLOCATIONS REQUIRE SEPARATE CHECKS).
THIS DOES NOT READ A LIVE D6FS IMAGE OR IMPLEMENT BOOT REPLAY, JOURNAL
DURABILITY, POWER-CUT RECOVERY OR CRASH-ATOMIC HISTFILE. PHASE ONE OPEN.

## D6FS CONSERVATIVE REPLAY-DECISION MATRIX (2026-10-12)

DAIMOS-TESTKIT `3759636` ADDS A HOST-ONLY C99 MODEL OF 96 COMBINATIONS
OF INTENT STATE (EMPTY/PREPARE/COMMIT) AND FIVE DURABLE-METADATA FLAGS.
87 REQUIRE FURTHER IDENTITY/CONSISTENCY INVESTIGATION; ONLY A COMPLETE
COMMIT CASE IS ELIGIBLE FOR ROLL-FORWARD UNDER ADDITIONAL IDENTITY
CHECKS. THE EIGHT NO-OP CASES ARE ABSTRACT EMPTY-INTENT CONDITIONS.
THIS IS NOT AN AUTOMATIC RECOVERY ALGORITHM OR DISK-DURABILITY PROOF.
METADATA IDENTITY, REPLAY/UNDO ORDER, POWER-CUT TESTS, AND REAL
CRASH-ATOMIC DSH HISTFILE ARE STILL OPEN. NO KERNEL CHANGE.

## D6FS TWO-COPY INTENT STATE TRANSITION TEST (2026-10-12)

TESTKIT `1B09FF2` MODELS SIX ALTERNATING PREPARE/COMMIT/EMPTY
TRANSITIONS AND ALL 774 WORD-PREFIX INTERRUPTED OVERWRITES. THE FIRST
VERSION FAILED WHEN IT OVERWROTE THE ONLY VALID RECORD; CORRECT
ALTERNATION PRESERVES THE INTACT COPY AND PASSES. STRICT HOST C99
COMPILATION PASS. THIS IS A SELECTOR MODEL WITH WEAK PROVISIONAL
CHECKSUM, NOT PERSISTENT JOURNALING, BOOT REPLAY, POWER-CUT RECOVERY,
OR CRASH-ATOMIC HISTFILE. NO PRODUCTION FORMAT OR KERNEL CODE CHANGED.

## D6FS INTENT RECORD SELECTOR PROTOTYPE (2026-10-12)

TESTKIT `AC9495D` ADDS A C99 HOST-ONLY TWO-COPY INTENT RECORD SELECTOR,
WHICH PASSES 129 WORD-PREFIX OVERWRITE CUTS AND EQUAL-SEQUENCE CONFLICT
REJECTION. THE MODEL USES NO DYNAMIC MEMORY BUT HAS ONLY A PROVISIONAL
36-BIT XOR-ROTATE INTEGRITY DIGEST; IT IS NOT A DEPLOYABLE ON-DISK
FORMAT. NO JOURNAL BLOCKS, BOOT REPLAY, CRASH-ATOMIC CREATE/HISTFILE,
OR KERNEL CODE WERE IMPLEMENTED. POWER-LOSS TESTS REMAIN OPEN.

## D6FS CREATION WRITE-ORDER CRASH MATRIX (2026-10-12)

DAIMOS-TESTKIT `ADF1E65` ADDS AN EXECUTABLE HOST-SIDE MODEL OF CHILD FCB,
OPTIONAL SYMLINK DATA, OPTIONAL PARENT GROWTH, DIRECTORY ENTRY, AND
PARENT FCB PUBLICATION. IT IDENTIFIES ORPHAN FCB WINDOWS AND A POSSIBLY
INVISIBLE DIRECTORY ENTRY BEFORE PARENT EOF PUBLICATION. THIS IS AN
ABSTRACT CRASH-PREFIX AUDIT, NOT A DURABILITY OR RECOVERY TEST. THE
D6FS CREATE RECOVERY DESIGN INCLUDES THE AUDIT RESULTS. IN-PLACE
DIRECTORY AND BITMAP TORN WRITES, COMMIT MARKER AND BOOT REPLAY REMAIN
OPEN. NO PRODUCTION KERNEL PROTOCOL CHANGE WAS MADE.

## D6FS CREATE TRANSACTION RECOVERY DESIGN GATE (2026-10-12)

`SYSTEM/KERNEL/FS/D6FS-CREATE-RECOVERY-DESIGN-20261012-V1.MD` AUDITS THE
EXISTING CREATE PATH AND DEFINES A RECOVERABLE FCB/DIRECTORY PUBLICATION
CONTRACT. IT IS DESIGN ONLY: NO JOURNAL, COW FORMAT, POWER-CUT RECOVERY,
OR PRODUCTION FIX IS IMPLEMENTED. CURRENT BCACHE INVALIDATION PREVENTS
FALSE CLEAN CACHE REUSE AFTER FAILED WRITE, BUT DOES NOT PROVIDE A
CRASH-ATOMIC COMMIT POINT. THE FCB 0451/0452 ORPHAN REGRESSION REMAINS
OPEN. IMPLEMENT ONLY AFTER WRITE-ORDER AUDIT AND TESTKIT FAULT MATRIX;
PRESERVE A SINGLE 128-WORD TRANSFER WORKSPACE AND AVOID PER-OPEN RAM.

## AMBIGUOUS FCB CREATION: DISK FORENSICS (2026-10-12)

TESTKIT `78C094A` ADDS A READ-ONLY D6FSCK-BASED DUMP OF FCBS 0451/0452.
ON THE PRESERVED POST-CALLBACK-ERROR IMAGE BOTH ARE TYPE-1 REGULAR,
SIZE ZERO, ZERO EXTENTS, UNREACHABLE. ON THE CLEAN CONTROL IMAGE FCB
0451 IS A SIX-WORD, ONE-EXTENT REGULAR FILE AND FCB 0452 IS FREE.
THE FAULT IMAGE IS CLEAN-MARKED DESPITE UNREACHABLE FCBS. THIS
STRENGTHENS THE EVIDENCE FOR THE CREATE-OBJECT PATH PUBLISHING THE
FCB BEFORE ITS DIRENT, THEN EXITING ON AN AMBIGUOUS RETURN.
THIS IS NOT YET A KERNEL FIX OR A POWER-LOSS TEST. CORRECT RECOVERY
MUST RESOLVE FCB AND DIRECTORY PUBLICATION TOGETHER, WITHOUT FREEING
POSSIBLY LIVE BLOCKS OR MERELY MASKING RETURN CODES. PHASE ONE OPEN.

## D6FS AMBIGUOUS FCB CREATE ERROR PATH IDENTIFIED (2026-10-12)

DAIMOS-TESTKIT `081DC33` CONFIRMS THAT D6FSCK REPORTS FCB INDICES IN
OCTAL: ORPHANS 451 AND 452 ARE DECIMAL 297 AND 298, BOTH IN LOGICAL
FCB BLOCK 479 (SLOTS 1 AND 2). `AE40D39` ADDS A SOURCE AUDIT:
`D6FS_PROVIDER_CREATE_OBJECT` WRITES THE NEW FCB BEFORE THE DIRECTORY
ENTRY, AND AN ERROR FROM `D6FS_READER_PUT_FCB` BRANCHES DIRECTLY TO
`D6FS_CREATE_FAIL`. A WRITE THAT REACHES BACKING STORAGE BUT RETURNS AN
ERROR CAN THEREFORE LEAVE AN ALLOCATED, UNREFERENCED FCB. THIS IS A
CONCRETE SOURCE PATH CONSISTENT WITH THE FAULT TEST; TARGET HIT COUNT
AND FULL CAUSAL ATTRIBUTION STILL REQUIRE DIRECT OBSERVATION. DO NOT
FREE THE FCB BLINDLY ON ERROR: DURABILITY IS AMBIGUOUS. RECONCILE
RECOVERABLE FCB AND DIRECTORY-ENTRY PUBLICATION AS A CORRECT PROTOCOL
BEFORE CLOSING PHASE ONE. POWER-LOSS/ATOMIC HISTFILE REMAIN OPEN.

## AMBIGUOUS FCB CONTROL IS CLEAN (2026-10-12)

TESTKIT `83A0E6E` ADDED THE MISSING SUCCESS-EXPECTING CONTROL. THE
SAME PRESEEDED FILE APPEND, WITH SYNTHETIC ERROR TARGET SET TO AN
UNREACHABLE LOGICAL BLOCK, PASSED APPEND/READBACK, HALT, AND D6FSCK
(CLEAN, SUPERBLOCK 670 SEQ 3, 179196 BLOCKS, 368 FCBS). BY CONTRAST
INJECTION TARGETING FCB LOGICAL BLOCK 479 REPRODUCED UNREACHABLE FCBS
451 AND 452. THIS CONFIRMS A FAULT-DEPENDENT REGRESSION, BUT TARGET
FCB IDENTITY AND THE EXACT INJECTED WRITE COUNT ARE NOT VERIFIED.
DO NOT ASCRIBE A ROOT CAUSE BEFORE INSTRUMENTING THE TARGET AND HIT
COUNT. POWER-LOSS AND CRASH-ATOMIC RECORD COMMIT ARE STILL OPEN.

## AMBIGUOUS FCB RETURN — NARROWED REPRODUCTION (2026-10-12)

DAIMOS-TESTKIT `E8E66DB` NARROWED THE SYNTHETIC POST-WRITE ERROR
SELECTOR TO LOGICAL FCB-TABLE BLOCK 479. THE APPEND REPORTED AN ERROR,
THE NEW DATA WAS READABLE, AND HALT PASSED. D6FSCK STILL REPORTED
UNREACHABLE FCBS 451 AND 452. THIS NARROWING DOES NOT ELIMINATE THE
REGRESSION. TARGET-FILE FCB IDENTITY AND THE INJECTOR HIT COUNT HAVE
NOT YET BEEN PROVEN. AN UNREACHABLE-BLOCK CONTROL FAILED THE FIXTURE'S
EXPECTED-ERROR PROBE AND DID NOT REACH HALT/FSCK: IT IS INCONCLUSIVE.
BEFORE ROOT-CAUSE CHANGES, BUILD A CLEAN END-TO-END NO-FAULT CONTROL
WITH A SUCCESS-EXPECTING FIXTURE AND PRECISELY TRACE THE TARGET FCB.
CRASH-ATOMIC HISTFILE AND POWER-CUT TESTS REMAIN OPEN.

## D6FS AMBIGUOUS FCB COMPLETION: OPEN FSCK REGRESSION (2026-10-12)

DAIMOS-TESTKIT `2255B02` INTRODUCES A TEST-ONLY POST-BACKING-CALLBACK
ERROR RETURN FOR FCB TABLE WRITES. IN PDP-6 SIMH THE WRITER RECEIVED
AN ERROR WHILE THE SECOND WORD OF A PRESEEDED FILE WAS COMMITTED AND
READ BACK SUCCESSFULLY. HALT PASSED. HOWEVER, D6FSCK FAILED: TWO
UNREACHABLE FCBS (451, 452), EACH WITH ZERO REFERENCES, ON A CLEAN-MARKED
VOLUME. THE INJECTION AFFECTS ALL FCB BLOCKS THROUGHOUT BOOT; THUS
UNRELATED OPERATIONS MAY HAVE CAUSED THE ORPHANS. THE RESULT IS A
REAL FAILING TEST, NOT PROOF OF A SPECIFIC STAGED-GROWTH KERNEL DEFECT.
ISOLATE TARGET PUBLICATION AND TRACE HIT COUNT BEFORE DETERMINING THE
ROOT CAUSE. THIS IS A SYNTHETIC AMBIGUOUS RETURN, NOT A DURABLE WRITE
OR POWER-LOSS ATOMICITY GUARANTEE. KEEP PHASE ONE OPEN.

## D6FS FCB PUBLICATION REJECTION TEST (2026-10-12)

DAIMOS-TESTKIT `C2E8708` TESTED A REJECTED FCB TABLE WRITE ON PDP-6
SIMH. TEST-ONLY BACKING INJECTION TARGETED FCB LOGICAL BLOCKS 442..486,
DETERMINED BY D6FSCK. THE TARGET FILE WAS PRESEEDED IN THE DISK IMAGE;
APPEND RETURNED AN ERROR, AND THE ORIGINAL ONE-WORD CONTENT AND EOF
WERE STILL READABLE AFTER REOPEN. ORDERLY HALT PASSED AND D6FSCK WAS
CLEAN (SUPERBLOCK 670, SEQ 3, 179196 BLOCKS, 368 FCBS). THIS IS A
DEFINITELY REJECTED FCB WRITE, **NOT** AN AMBIGUOUS OR PARTIALLY DURABLE
COMMIT. POWER-LOSS AND CRASH-ATOMIC RECORD COMMIT REMAIN OPEN.

## D6FS ALLOCATION BITMAP RANGE FAULT (2026-10-12)

TESTKIT `8BC8872` EXTENDS THE ISOLATED BACKING-WRITE INJECTOR TO
REJECT A CONTIGUOUS LOGICAL RANGE. ON THE TEST IMAGE (179196 BLOCKS)
THE FREEMAP STARTS AT DECIMAL 487; REJECTING 487..525 CAUSED A DISPOSABLE
ONE-WORD FILE GROWTH TO RETURN AN ERROR, RETAIN ZERO-WORD EOF, AND HALT
NORMALLY. SIMH PASSED BOTH PROBES; D6FSCK REPORTED CLEAN, SUPERBLOCK
670 SEQUENCE 3 AND 360 FCBS. EARLIER REJECTION OF BLOCK 487 ALONE DID
NOT TRIGGER THE ERROR. THIS GEOMETRY IS IMAGE-SPECIFIC. A WRITE-ATTEMPT
COUNTER HAS NOT YET BEEN ADDED, SO TARGETED BITMAP WRITE REJECTION IS
SUPPORTED BY ISOLATION TO THE BITMAP RANGE, NOT BY A RECORDED HIT COUNT.
AMBIGUOUS FCB PUBLICATION, CRASH RECOVERY, AND ATOMIC HISTFILE ARE OPEN.

## D6FS LIVE SECOND-BLOCK FAILURE ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT `94A4A6B` ADDS A PDP-6 C99 TEST THAT ATTEMPTS TO WRITE
256 WORDS TO A NEW FILE, WITH A TEST-ONLY BACKING DISPATCHER REJECTING
THE SECOND 128-WORD DATA BLOCK VIA ITS FIRST-WORD MARKER. THE WRITE
FAILS, REOPENING REPORTS THE OLD ZERO-WORD EOF, AND ORDERLY HALT PLUS
D6FSCK REPORT CLEAN (179196 BLOCKS, 360 FCBS, SUPERBLOCK 670 SEQ 3).
THIS VALIDATES VISIBLE EOF ROLLBACK AFTER A SECOND-BLOCK FAILURE, NOT
INDEPENDENT PHYSICAL DURABILITY OF THE FIRST BLOCK. ALLOCATION-MAP,
FCB PUBLICATION, POWER CUT, AND ATOMIC HISTFILE REMAIN OPEN.

## D6FS LIVE PAYLOAD-WRITE FAILURE ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT COMMIT `3D7C045` PROVIDES A TEST-ONLY BACKING-WRITE
INJECTOR SELECTING A DISTINCTIVE FIRST PAYLOAD WORD, AND A NATIVE PDP-6
C99 TEST THAT CREATES A DISPOSABLE D6FS FILE, REQUIRES THE MARKED WRITE
TO FAIL, AND REOPENS THE FILE TO VERIFY THE ORIGINAL ZERO-WORD EOF.
LIVE SIMH ACCEPTANCE PASSED BOTH THE INJECTED-WRITE AND ORDERLY-HALT
PROBES. D6FSCK REPORTED SUPERBLOCK 670, SEQUENCE 3, CLEAN, 179196 BLOCKS
AND 360 FCBS. THE FIRST HARNESS ATTEMPT TIMED OUT AT LOGIN; THE SAME
IMAGE PASSED WITH A LONGER BOOT ALLOWANCE. THE REPEATABLE TEST RUNNER
NOW USES A 120-SECOND BOOT LIMIT.

THIS VALIDATES THE NORMAL ERROR RETURN AND OLD EOF AFTER ONE FAILED
DATA-BLOCK WRITE. ALLOCATION-MAP FAILURE, MULTI-BLOCK PARTIAL WRITES,
AMBIGUOUS FCB COMMIT, POWER-LOSS RECOVERY, AND CRASH-ATOMIC HISTFILE
RECORD COMMITS REMAIN OPEN. THE INJECTOR LIVES ONLY IN DAIMOS-TESTKIT;
PRODUCTION DAIMOS KERNEL SOURCES AND DISK FORMAT ARE UNCHANGED.

## D6FS STAGED GROWTH FAILURE-PATH TESTING STATUS (2026-10-11)

DAIMOS-TESTKIT NOW HAS AN EIGHT-ASSERTION STATIC SOURCE CONTRACT FOR THE
STAGED GROWTH / ABORT FLOW: PAYLOAD BEFORE FCB PUBLICATION, COMPLETE
TRANSFER REQUIRED, ABORT ON WRITE FAILURE, RE-READ OLD DISK FCB, PRESERVE
ITS ALLOCATED EXTENT CAPACITY, RESTORE FCB/INFO, AND PROPAGATE CLEANUP
ERRORS. THE CONTRACT PASSES. THE EXISTING HOST D6FSCK FAULT ABORT HOOK
ONLY EXERCISES FSCK REPAIR, NOT THE LIVE FS_BACKING_WRITE PATH; IT CANNOT
BE USED AS EVIDENCE FOR LIVE STAGED-GROWTH ROLLBACK. SEE THE TESTKIT
`DAIMOS-D6FS-STAGED-WRITE-FAULT-PLAN-20261011-V1.MD` FOR THE BACKING-I/O
FAILURE MATRIX. NO DEVICE-LEVEL FAULT INJECTION HAS BEEN EXECUTED YET.
KEEP FAILURE-PATH VALIDATION AND CRASH-ATOMIC HISTFILE RECORDS OPEN.

## D6FS V2 STAGED REGULAR-FILE GROWTH (2026-10-11, TESTED WRITE PATH)

THE D6FS REGULAR-FILE WRITE PROVIDER NOW STAGES EXPANDED EXTENTS IN A
PROCESS-PRIVATE FCB WITHOUT PUBLISHING THE NEW READABLE SIZE, WRITES THE
PAYLOAD, THEN PUBLISHES THE FINAL FCB. `D6FS_PROVIDER_RESIZE_FCB` USES AN
INTERNAL SIGN-BIT STAGING MODE; ALL EXISTING NON-STAGED CALLERS RETAIN THEIR
PRIOR BEHAVIOR. A FAILED DATA WRITE CALLS THE ASSEMBLER ROLLBACK HELPER,
WHICH RE-READS THE OLD DURABLE FCB, RETAINS ITS FULL ALLOCATED CAPACITY
(RATHER THAN ITS SHORTER LOGICAL EOF), RELEASES ONLY NEW BLOCKS, AND
RESTORES THE OLD IN-MEMORY FCB/INFO. AMBIGUOUS FINAL FCB-COMMIT FAILURE
IS REPORTED WITHOUT FREEING POSSIBLY LIVE BLOCKS. NO EXTRA PER-FILE
RESIDENT RAM OR DISK FORMAT CHANGE IS INTRODUCED.

PDP-6 SIMH PASS: NATIVE D6FS TWO-PREOPENED APPEND (2/2), NATIVE 444-WORD
CROSS-BLOCK GROWTH/READBACK (2/2), BACKGROUND DSH TWO-WRITER APPEND (6/6),
HISTFILE WRITE/HALT (3/3), REBOOT RELOAD (1/1), CLEAN D6FS CHECKS.
SOURCE CONTRACT AUDITS NOW ASSERT STAGED PUBLICATION, NOT THE OLD DEFECT.
BACKING-WRITE FAULT INJECTION AND POWER-CUT RECOVERY REMAIN OPEN; THIS IS
AN ORDERED-GROWTH REPAIR, **NOT** A CRASH-ATOMIC RECORD TRANSACTION.

## D6FS V2 WRITE-GROWTH STAGING SAFETY AUDIT (2026-10-11)

THE EXISTING RESIZE HELPER SERVES TRUNCATE, DIRECTORY, SYMLINK AND REGULAR
FILE PATHS. GLOBAL REORDERING WOULD ALTER THEM ALL. REGULAR WRITES CAN
STAGE EXTENTS IN MEMORY (D6FS_FILE_BLOCK USES EXTENTS, NOT FILE SIZE), BUT
MUST KEEP THE OLD RESERVED EXTENT CAPACITY FOR ERROR ROLLBACK. EARLY
FAILURES MUST NOT RECLAIM OLD CAPACITY; AMBIGUOUS FCB COMMIT MUST NOT FREE
POSSIBLY LIVE BLOCKS. SEE `design/d6fs-v2-growth-staging-20261011-v1.md`.
THE KERNEL IMPLEMENTATION IS STILL UNCHANGED; THE SOURCE-ORDER DEFECT IS
OPEN. NO CRASH-ATOMICITY GUARANTEE IS CLAIMED.

## D6FS V2 ORDERED-WRITE CONTRADICTION AND RECORD-COMMIT DECISION (2026-10-11)

THE D6FS V2 FORMAT DOCUMENT EXPLICITLY EXCLUDES JOURNALS/CHECKSUMS AND
REQUIRES COMPLETE DATA BEFORE NEW FCB EXTENT/SIZE PUBLICATION. THE CURRENT
`D6FS_PROVIDER_WRITE_WORDS`/`D6FS_PROVIDER_RESIZE_FCB` PATH PUBLISHES THE
FCB FIRST. THIS IS AN INDEPENDENT V2 ORDERING BUG AND MUST BE CORRECTED
BEFORE A FORMAT CHANGE. RESTORING ORDER DOES NOT ALONE GIVE CRASH-ATOMIC
MULTIWORD HISTFILE RECORDS; THOSE REQUIRE A SEPARATE VERSION-GATED COMMIT
PROTOCOL OR AN EXPLICITLY WEAKER DURABILITY CONTRACT. SEE
`design/d6fs-record-atomicity-20261011-v1.md`. NO KERNEL FIX IS CLAIMED.

## D6FS RECORD-COMMIT WRITE-ORDER AUDIT (2026-10-11)

THE PROVIDER-LEVEL `O_APPEND` LOCK RESERVES CURRENT EOF BUT DOES NOT
GIVE CRASH-ATOMIC RECORD COMMIT. `D6FS_PROVIDER_WRITE_WORDS` CALLS
`D6FS_PROVIDER_RESIZE_FCB` BEFORE COPYING NEW PAYLOAD. THE RESIZE PATH
UPDATES EXTENTS AND FCB SIZE THROUGH `D6FS_READER_PUT_FCB`; THEN
`D6FS_READER_WRITE_WORDS` WRITES EACH 128-WORD BLOCK SEPARATELY THROUGH
`D6FS_READER_COMMIT_CACHE`. A CRASH CAN THEREFORE EXPOSE A NEW FCB SIZE
WITH MISSING OR PARTIAL PAYLOAD. FINAL MTIME/FCB PUBLICATION IS ANOTHER
FAILURE POINT. A/B CLEAN/DIRTY SUPERBLOCK STATE DOES NOT JOURNAL RECORDS.

DAIMOS-TESTKIT `DAIMOS-D6FS-RECORD-COMMIT-ORDER-AUDIT-20261011-V1.SH`
NOW DETECTS THIS UNSAFE ORDERING AND REPORTS `KNOWN NONATOMIC`; THIS
IS A SOURCE CONTRACT CHECK, NOT A PASSED CRASH-SAFETY TEST.

BEFORE IMPLEMENTATION, SPECIFY AN ON-DISK ATOMIC COMMIT POINT AND
RECOVERY FOR DATA, FCB/EXTENT METADATA AND ALLOCATION MAP; THEN INJECT
I/O FAILURES AT EVERY PUBLICATION STEP. DO NOT CLAIM ATOMICITY BY MOVING
THE EXISTING SIZE WRITE OR BY RELYING ON THE IN-MEMORY PROVIDER LOCK.
NO KERNEL CODE OR DISK FORMAT WAS CHANGED BY THIS AUDIT.

## DSH HISTFILE CORRUPTED-TAIL SIMH ACCEPTANCE (2026-10-11)

THE DAIMOS-TESTKIT NATIVE C99 FIXTURE WRITES A VALID `ECHO SAFE` S6REC
FOLLOWED BY A TEXT HEADER WHOSE PAYLOAD WORDS ARE MISSING. AFTER CLEAN
HALT AND REBOOT OF THE SAME D6FS DISKS, INTERACTIVE DSH EMITS
`HISTFILE DAMAGED; VALID PREFIX LOADED`, AND CTRL-P RECALLS `ECHO SAFE`.
THE SECOND HALT LEAVES SUPERBLOCK 706, SEQUENCE 5, CLEAN. THE TWO-BOOT
SIMH REGRESSION PASSES FOUR PROBES. EXISTING LOADER CODE REQUIRED NO
MODIFICATION. RECOVERING RECORDS AFTER A DAMAGED TAIL OR AUTOMATIC
TRUNCATION REMAINS DISABLED UNTIL CROSS-PROCESS-SAFE RECORD COMMIT/REPAIR
SEMANTICS EXIST. PHASE-ONE IMAGE SIZE OVERRIDE REMAINS TEMPORARY.

## DSH PHASE-ONE HISTFILE CONCURRENT WRITER / DAMAGED TAIL (2026-10-11)

AN ISOLATED PDP-6 SIMH TEST LAUNCHED TWO BACKGROUND DSH GROUPS THAT
APPENDED ALPHA AND BETA TO A D6FS FILE WHILE THE INTERACTIVE PARENT
REMAINED RESPONSIVE. BOTH WORKERS COMPLETED; HEADER, ALPHA, AND BETA
SURVIVED READBACK; CLEAN HALT SUCCEEDED (6/6 PROBES). THIS IS LIVE
MULTIPROCESS ACCEPTANCE BUT NOT DETERMINISTIC EVERY-INTERLEAVING PROOF.

ON HISTFILE LOAD, A MALFORMED OR TRUNCATED S6REC TAIL NOW REPORTS
`DSH: HISTFILE DAMAGED; VALID PREFIX LOADED`, WHILE THE VALID HISTORY
PREFIX REMAINS USABLE. NO AUTOMATIC TRUNCATION IS ATTEMPTED: THE CURRENT
FLOCK VALIDATION IS LOCAL TO A PROCESS FILE TABLE AND CANNOT SAFELY
SERIALIZE REPAIR AGAINST OTHER PROCESSES. NORMAL HISTORY/HALT 3/3 AND
SAME-DISK REBOOT 1/1 PASS; LIVE MALFORMED-TAIL FAULT INJECTION AND
RECOVERY/REPAIR POLICY ARE STILL OPEN. DSH IMAGE 036523 OCTAL WORDS
USES THE TEMPORARY PHASE-ONE BUILD GATE OF 037000.

## DSH PHASE-ONE CONFORMANCE RETEST (2026-10-11)

THE 115-CASE PDP-6 SIMH DSH CONFORMANCE SUITE PASSES 115/115 WITH THE
BOUNDED `$()` S6REC WORKER, HISTFILE WRITE RETRIES AND INTERACTIVE-ONLY
FOREGROUND TTY HANDOFF. A FIRST RETEST RETURNED 106/115 BECAUSE THE
TESTKIT HARNESS MISTOOK THE `# ` SUBSTRING IN ECHOED COMMAND TEXT FOR A
FRESH DSH PROMPT, SUBMITTING LATER COMMANDS TOO EARLY. THE TESTKIT NOW
REQUIRES THE PROMPT TO START A LOGICAL TERMINAL LINE; THE FULL SUITE
PASSES. THE DSH IMAGE IS 036475 OCTAL WORDS IN THE ACCEPTED TEMPORARY
037000 PHASE-ONE BUILD WINDOW. THE DEFAULT 036000 GATE REMAINS IN FORCE
FOR NORMAL BUILDS.

## DSH PHASE-ONE BACKGROUND SESSION REPAIR (2026-10-11)

THE RETURN-TO-LOGIN REGRESSION AFTER `BEGIN ECHO ... END &` WAS CAUSED BY
NONINTERACTIVE DSH WORKERS WITH AN INHERITED CONTROLLING TTY CALLING
`TTY_SETFG` WHILE STARTING THEIR OWN EXTERNAL COMMANDS. THE INTERACTIVE
PARENT REMAINED IN PGRP 2 BUT TTY FG MOVED TO A WORKER CHILD PGRP (5),
SO THE NEXT PARENT `READCHAR` RETURNED -1 AND DSH EXITED. THE FIX REQUIRES
BOTH `st->interactive` AND `st->tty_attached` FOR FOREGROUND HANDOFF AND
RESTORATION IN THE EXTERNAL-COMMAND AND PIPELINE PATHS. NO KERNEL TEARDOWN
CHANGES OR WORKAROUNDS WERE RETAINED.

PDP-6 SIMH: BACKGROUND GROUP LAUNCH, PARENT COMMAND, WAIT, BACKGROUND
GROUP PIPELINE, PARENT COMMAND, WAIT, NORMAL FOREGROUND PIPELINE,
BOUNDED `$()` AFTER THE WORKER AND ORDERLY HALT ALL PASS (9/9 PROBES).
THE DEFAULT DSH IMAGE GATE IS STILL 036000 OCTAL WORDS, WITH USER-APPROVED
TEMPORARY OVERRIDE `DSH_IMAGE_LIMIT=037000` DURING PHASE ONE.

## DSH BOUNDED COMMAND SUBSTITUTION (2026-10-11, PHASE ONE)

THE FIRST BOUNDED `$()` IMPLEMENTATION ACCEPTS ONE STANDALONE SIMPLE
SUBSTITUTION ARGUMENT, WITH AN UNQUOTED SIMPLE COMMAND INSIDE. A SEPARATE
DSH `-S` WORKER EMITS NATIVE S6REC VIA STDOUT, AND THE PARENT DRAINS THE
PIPE WHILE THE WORKER RUNS. IT RETAINS AT MOST 36 SIXBIT CHARACTERS,
DISCARDS TRAILING RECORD NEWLINES, AND REJECTS OUTPUT OVERFLOW AND WORKER
FAILURE. AST SUBSTITUTION POSITION IS PACKED INTO SIMPLE NODE FLAGS;
`extra` REMAINS THE AST CHILD LINK. DIRECT PARSER REGRESSIONS AND SIMH
SUCCESS/FAILURE/OVERFLOW/SYNTAX/SHUTDOWN PROBES PASS.

THE DSH IMAGE IS CURRENTLY 036444 OCTAL WORDS IN THIS EXPERIMENT.
AT USER REQUEST THE BUILD ACCEPTS `DSH_IMAGE_LIMIT=037000` TEMPORARILY
DURING PHASE ONE; ITS DEFAULT 036000 LIMIT IS UNCHANGED. SHRINKING DSH
SOURCE/IMAGE IS DEFERRED UNTIL ALL PHASE-ONE WORK IS COMPLETE.

## MEMFS O_APPEND WRITE-TIME EOF RESOLUTION (2026-10-11)

MEMFS NOW ACCEPTS THE SAME DESCRIPTOR O_APPEND SENTINEL AS D6FS, RESOLVING
THE CURRENT MEMFS SLOT SIZE DURING WRITE_WORDS RATHER THAN AT OPEN TIME.
THE SHARED FILE LAYER DISPATCHES APPEND SENTINELS FOR PROVIDERS 4 AND 6
AND REFRESHES THEIR CURSORS AFTER A SUCCESSFUL WRITE. NO EXTRA DESCRIPTOR
WORDS OR PER-FILE RESIDENT LOCKS WERE ADDED.

THE NATIVE PDP-6 TWO-PREOPENED-DESCRIPTOR MEMFS TEST ON /TEMP PASSES,
INCLUDING SEEK(0), DUPLICATE-APPEND, FOUR-WORD RAW READBACK AND CURSORS.
THE SAME TEST FAILS ON THE ORIGINAL FILE IMPLEMENTATION (SECOND CURSOR 1).
THE NATIVE D6FS TEST STILL PASSES AFTER THE SHARED FILE-LAYER CHANGE.
TRUE SIMULTANEOUS TWO-PROCESS APPENDS AND SHORT-WRITE S6REC INTEGRITY
REMAIN OPEN; A TWO-DESCRIPTOR TEST IS NOT A SCHEDULER CONCURRENCY TEST.

## D6FS ATOMIC O_APPEND IMPLEMENTATION AND SIMH VALIDATION (2026-10-11)

D6FS WRITE_WORDS NOW RESOLVES O_APPEND END-OF-FILE INSIDE THE EXISTING
SERIALIZED PROVIDER CALL, AFTER READING THE CURRENT FCB. THE FILE DESCRIPTOR
KEEPS APPEND INTENT WITHOUT GROWING ITS TWO-WORD FORMAT: THE PACKED DIR BIT
IS REUSED FOR REGULAR FILES WITH NONZERO REGULAR/LOCK STATE. DIRECTORY CHECKS
DISTINGUISH THESE DISJOINT CASES. AN INVALID EXPERIMENTAL LH 000004 FLAG
WAS REJECTED BECAUSE IT CORRUPTED THE CANONICAL VNODE; THE FINAL ENCODING
PASSES PDP-6 SIMH. THE APPEND DESCRIPTOR CURSOR IS REFRESHED AFTER WRITE.

A DETERMINISTIC NATIVE PDP-6 TEST ON /CONFIG (D6FS) OPENS TWO APPEND
DESCRIPTORS BEFORE EITHER WRITE, SEEKS THE SECOND TO ZERO, THEN WRITES
THROUGH BOTH AND A DUPLICATE. ALL FOUR WORDS AND DESCRIPTOR CURSORS PASS;
THE UNMODIFIED BASELINE FAILS (SECOND CURSOR 1 INSTEAD OF 3). THE FIRST
POST-WRITE CURSOR IMPLEMENTATION DECODED AN UNMASKED PROVIDER FIELD AND
WAS FIXED TO MASK METADATA BEFORE LOOKUP. THE TEST ALSO CONFIRMED CLEAN
HALT, SUPERBLOCK 705 SEQUENCE 3. OLDER SEQUENTIAL /TEMP APPEND PROBES
ACTUALLY EXERCISED MEMFS; THEY WERE NOT D6FS VALIDATION. DIRECTORY/HISTFILE
5/5 AND SAME-DISK REBOOT 2/2 PASS, INCLUDING HISTORY ON THE D6FS ROOT.
A SIMULTANEOUS TWO-PROCESS TEST REMAINS OPEN. THE DSH BACKGROUND-GROUP
TEST RETURNS TO LOGIN EVEN ON THE UNMODIFIED BASELINE. MEMFS APPEND
SEMANTICS AND SHORT-WRITE RECORD INTEGRITY ALSO REMAIN OPEN.

## NATIVE MAKE V0.9 FEATURE FREEZE AND SIXBIT SHRINK (2026-10-11)

NATIVE MAKE IS FEATURE-COMPLETE FOR V0.9, WITH UPPERCASE SIXBIT-ONLY
INPUT, SERIAL EXECUTION AND A DUMMY -J OPTION. DOUBLE-COLON RULES AND
PROPAGATION OF COMMAND-LINE VARIABLE OVERRIDES ARE OUT OF SCOPE FOR V0.9.
THE LAST SOURCE AUDIT FOUND NO REMAINING LOWERCASE OR ASCII-COMPATIBILITY
BRANCHES. MAKE STILL NEEDS UNPACKED INTERNAL TEXT FOR S6REC MAKEFILE
PARSING AND COMMAND EXPANSION; REMOVING THAT REPRESENTATION WOULD BE A
MAJOR REWRITE, NOT A SAFE SHRINK. THE RUN V2 MAKEFLAGS ENVIRONMENT FILTER
NOW MATCHES THE SIXBIT RECORD DIRECTLY, ELIMINATING ITS 257-BYTE LOCAL
DECODE BUFFER. KCC BUILD/LINK AND SIXBIT MANUAL VALIDATION PASS.

## NATIVE MAKE MINIMAL MAKEFLAGS (2026-10-11)

NATIVE MAKE NOW IMPORTS UPPERCASE SIXBIT MAKEFLAGS FROM ENVIRONMENT,
MERGES SUPPORTED SERIAL SWITCHES WITH COMMAND-LINE OPTIONS, AND FORWARDS
ONE REPLACEMENT MAKEFLAGS RECORD THROUGH BOTH RUN V2 RECIPE PATHS.
THE NO-OP -J JOB COUNT IS NOT EXPORTED. LITERAL RECURSIVE MAKE COMMANDS
ARE RUN UNDER -N TO ALLOW CHILD DRY-RUN OUTPUT, BUT NOT UNDER -Q.
NATIVE PDP-6 PARENT/CHILD -I AND -N REGRESSIONS PASS (15/15 TOTAL).
NO ASCII ALIASES, PARALLELISM, OR GENERAL ENVIRONMENT REPLACEMENT LAYER
WERE ADDED. COMMAND-LINE VARIABLE OVERRIDES REMAIN NON-PROPAGATING.

## NATIVE MAKE PRECIOUS REGRESSION (2026-10-11)

THE UPPERCASE SIXBIT NATIVE MAKE SIMH SUITE NOW PASSES 13/13 PROBES.
THE NEW .PRECIOUS TEST CREATES A FILE, TRIGGERS A FAILING RECIPE WITH
-B, AND CONFIRMS THE FILE IS RETAINED. THIS IS CONSISTENT WITH THE
EXISTING POLICY OF NEVER REMOVING FAILED OUTPUTS, REGARDLESS OF WHETHER
.PRECIOUS WAS DECLARED. THERE IS NO NEED FOR A NEW DELETION MECHANISM.
DOUBLE-COLON RULES AND AUTOMATIC RECURSIVE MAKEFLAGS PROPAGATION REMAIN
UNIMPLEMENTED. BOTH NEED A SEPARATE SEMANTIC DESIGN BEFORE MODIFICATION.

## D6FS ORDERLY SHUTDOWN AND DSH HISTFILE REBOOT (2026-10-11)

THE PRIVILEGED HALT PATH NOW SYNCS AND TRANSITIONS A WRITABLE D6FS
NAMESPACE ROOT TO CLEAN THROUGH THE EXISTING SERIALIZED VFS REMOUNT.
ALREADY READ-ONLY ROOTS ARE NOT TOGGLED; A FAILED CLEAN PUBLICATION
REFUSES HALT. MEMFS SNAPSHOT SHUTDOWN STILL PRECEDES D6FS PUBLICATION.
AN ISOLATED PDP-6 SIMH TEST PASSED 3/3 INITIAL HISTORY/HALT PROBES.
CURRENT-SOURCE D6FSCK REPORTED CLEAN SUPERBLOCK 705, SEQUENCE 3. THE
SECOND BOOT OF THE SAME DISK PASSED HISTFILE RELOAD 1/1. FAILURE
INJECTION AND CONCURRENT HISTORY APPENDS REMAIN TO BE TESTED.


## NATIVE MAKE UPPERCASE SIMH FOLLOW-UP (2026-10-11)

THE STRICT-UPPERCASE SIXBIT NATIVE MAKE REGRESSION PASSED 7/7 PDP-6 SIMH
PROBES AFTER LOWERCASE ALIASES WERE REMOVED. THE TEST WAS THEN EXPANDED
WITH TARGET-SPECIFIC .SILENT AND .IGNORE AND GLOBAL -I FAILURE HANDLING;
ALL 10/10 PROBES PASS. -J REMAINS A SERIAL NO-OP. .PRECIOUS IS RECOGNIZED
BUT REQUIRES SEPARATE BEHAVIORAL TESTING. PORTABLE DOUBLE-COLON RULES AND
RECURSIVE MAKEFLAGS PROPAGATION REMAIN OUTSTANDING.

## NATIVE MAKE UPPERCASE SIXBIT ONLY (2026-10-11)

NATIVE MAKE NOW ACCEPTS ONLY UPPERCASE SIXBIT OPTION LETTERS AND INCLUDE
DIRECTIVES. LOWERCASE OPTION AND INCLUDE ALIASES HAVE BEEN REMOVED. THIS
REVERSES THE PREVIOUS HOST-STYLE LOWERCASE ACCEPTANCE: THE NATIVE USER ABI
IS SIXBIT, NOT ASCII. THE SERIAL -J OPTION IS RETAINED AS A NO-OP.

## NATIVE MAKE TARGET SIMH ACCEPTANCE (2026-10-11)

THE NEW COMMON-OPTIONS NATIVE MAKE TESTKIT PROBE PASSES ALL SEVEN PDP-6
SIMH CASES: -J WITH SEPARATE/ATTACHED COUNTS, ENABLED/CLEARED .SUFFIXES,
-R IMPLICIT-RULE SUPPRESSION, -T WITHOUT RUNNING A RECIPE, AND STAT OF
THE TOUCHED OUTPUT. DAIMOS SIXBIT COMMAND ARGUMENTS ARRIVE UPPERCASED,
SO MAKE NOW ACCEPTS UPPERCASE -I, -R, -T AS WELL AS LOWERCASE SPELLINGS.
THE JOB-COUNT OPTIONS REMAIN NO-OPS; ALL EXECUTION IS SERIAL.

THE INITIAL IMAGE CHECK FAILURE WAS CAUSED BY MIXING OLDER INSTALLED
MKD6FS/D6FSCK BINARIES WITH THE NEWER CHECKOUT-BUILT TOOLS. THE TEST
NOW BUILDS THE HOST TOOLS AND EXPLICITLY SELECTS THE MATCHING MKD6FS
AND ITS ADJACENT D6FSCK. NO FILESYSTEM CODE WAS CHANGED IN THIS PASS.
ADDITIONAL .SILENT, .IGNORE, .PRECIOUS, -I, AND RECURSIVE MAKEFLAGS
BEHAVIOR REMAINS TO BE COVERED BY NATIVE EXECUTION TESTS.

## NATIVE MAKE SIMH REGRESSION ATTEMPT (2026-10-11)

A NEW DAIMOS-TESTKIT TARGET PROBE COVERS SERIAL -j/-J OPTIONS, ACTIVE AND
CLEARED SUFFIX LISTS, -r, AND -t. NATIVE MAKE HAD PREVIOUSLY COMPILED AND
LINKED, BUT THIS SIMH RUN HAS NOT REACHED TARGET EXECUTION. IMAGE BUILD
FIRST EXPOSED A SIXBIT MANUAL TYPO (FIXED), THEN AN INCORRECT TEST HARNESS
BOOTSTRAP PATH (FIXED); THE CURRENT BLOCKER IS THE EXISTING MKD6FS
POST-FORMAT D6FSCK FAILURE. NO MAKE FEATURE HAS BEEN DECLARED TARGET-VERIFIED.

## NATIVE MAKE SHARED OPTIONS (2026-10-11)

NATIVE MAKE NOW RECOGNIZES .SUFFIXES LISTS AND RESET, TARGET-SCOPED .SILENT
AND .IGNORE, .PRECIOUS DECLARATIONS, AND GLOBAL -i, -r, AND -t. -j AND -J
ACCEPT AN OPTIONAL JOB COUNT WITHOUT ENABLING PARALLEL EXECUTION; MAKE
REMAINS STRICTLY SERIAL. .PRECIOUS REQUIRES NO CLEANUP POLICY CHANGE BECAUSE
NATIVE MAKE DOES NOT DELETE FAILED TARGET OUTPUTS. THE MANUAL NOW DESCRIBES
THE EXISTING INCLUDE HANDLING, $^ EXPANSION, AND DYNAMIC GRAPH STORAGE.

HOST KCC COMPILATION, DAS ASSEMBLY, AND NEW TESTKIT SOURCE CHECKS PASS.
NATIVE PDP-6 SIMH EXECUTION OF THESE NEW PATHS IS NOT YET VERIFIED.
DOUBLE-COLON RULES, AUTOMATIC RECURSIVE MAKEFLAGS PROPAGATION, AND FULL
SUFFIX-PRECEDENCE CONFORMANCE REMAIN OPEN. JOB PARALLELISM IS EXPLICITLY
OUT OF SCOPE.

## DSH SOURCE/RECURSION STACK FRAMES AND FULL TARGET CONFORMANCE (2026-10-10)

THE SOURCE POSITIONAL-RESTORATION AND FUNCTION-RECURSION REGRESSIONS WERE
CAUSED BY NESTED SIMPLE-COMMAND EXECUTION EXHAUSTING THE PDP-6 USER STACK:
EVERY CALL PREVIOUSLY RESERVED TWENTY SEVEN-WORD ARGUMENT RECORDS. DSH NOW
USES A FOUR-RECORD COMMON PATH AND RESERVES THE FULL TWENTY-RECORD SCRATCH
EXTENT THROUGH SYS_BRK ONLY WHEN REQUIRED BY ARITY, ALIAS, OR GLOBBING.
THE SCRATCH EXTENT IS RELEASED IN LIFO ORDER AFTER EXECUTION. NO NEW
PERMANENT BSS OR GENERAL LIBC ALLOCATOR IS REQUIRED; THE TWENTY-WORD
SHELL PARSER LIMIT AND SIXTEEN-ARGUMENT SYS_RUN LIMIT REMAIN DISTINCT.
PIPEDBG DEVELOPMENT OUTPUT IS REMOVED.

THE TESTKIT SIMH HARNESS ALSO NOW MATCHES THE DSH '# ' PROMPT ONLY AT A
TERMINAL LINE START, RATHER THAN MISTAKING AN ECHOED LITERAL '# ' FOR A
PROMPT. WITH BOTH CHANGES, ALL 115 DSH CONFORMANCE PROBES PASS ON PDP-6
SIMH, INCLUDING SOURCE, RECURSION, EIGHT-STAGE PIPELINES, AND FINAL
QUOTE/COMMENT CASES. THE INDEPENDENT REAL-DCS FOREGROUND/TTY TEST PASSES.
SAME-BASE DXR COMPARISON: ORIGINAL IMAGE 035071 WORDS, CORRECTED 035273
(+0202 OCTAL WORDS), BOTH WITH 014445 BSS WORDS. THE EXEC IMAGE REMAINS
BELOW THE 036000-WORD LIMIT. BROADER RESOURCE-LIMIT FEATURE COVERAGE IS
STILL OPEN AND IS TRACKED IN USERLAND/DSH/TODO.MD.

## DSH PIPELINE DISK-PI AND WAKEUP REGISTER FIX (2026-10-10)

PDP-6 TYPE 270 DISK READS FAILED WITH DRL WHILE A PRECEDING PIPELINE CHILD
EXITED: PROC_EXIT_CURRENT HELD THE GLOBAL PI GATE DISABLED THROUGH RESOURCE
TEARDOWN. IT NOW MASKS THE PI6 SCHEDULER CLOCK DURING CLEANUP WHILE ALLOWING
PI3/PI5 STORAGE TRANSFERS. THE PREVIOUS PI6 ENABLE STATE IS RESTORED ON
RETURN TO THE IDLE SCHEDULER, AND THE FINAL-PROCESS HALT DISABLES GLOBAL PI.
THE PI-SAFE PROC_WAKEUP_EVENT ROUTINE ALSO NOW PRESERVES AC7, WHICH IS
MODIFIED BY NESTED PROC_RUNQ_ADD; WITHOUT THAT SAVE, LONG PIPELINES COULD
HALT AT PC 000001. BOTH ERRORS WERE CONFIRMED THROUGH SIMH ISOLATION.

FIVE ISOLATED RUN/PIPE TESTS, 5/6/8-STAGE PIPELINE TESTS, AND REAL-DCS
FOREGROUND/TTY RESTORATION PASS WITH THE CORRECTION. IN THE FULL DSH
CONFORMANCE RUN, 73 PROBES PASSED AND THE TWO INDEPENDENT SOURCE POSITIONAL
RESTORATION AND FUNCTION RECURSION-LIMIT REGRESSIONS REMAIN OPEN. DO NOT
DECLARE FULL DSH CONFORMANCE COMPLETE UNTIL THESE ARE FIXED AND RETESTED.

## NATIVE MAKE DIRECT COMMAND PATH LOOKUP (2026-10-10)

FOLLOW-UP HOST AND NATIVE MAKEFILE CLEANUP: TEN ADDITIONAL DAIMOS
MAKEFILES NOW DEFAULT TO EXECUTABLE NAMES RESOLVED THROUGH PATH INSTEAD
OF THE PREFIX/BIN DIRECTORY OR /OPTION/BASE/EXEC. EXPLICIT TOOL VARIABLE
OVERRIDES REMAIN AVAILABLE. LIBC CHECKS COMMAND AVAILABILITY USING
COMMAND -V; PREFIX-BASED HEADER/LIBRARY PATHS AND ABSOLUTE BOOT-IMAGE
INSTALLATION DESTINATIONS ARE UNCHANGED. TESTKIT VERIFIES 47 DEFAULTS;
GNU MAKE KERNEL AND LIBC DRY RUNS PASS. NATIVE SIMH EXECUTION HAS NOT
BEEN VERIFIED FOR THIS FOLLOW-UP.

NATIVE MAKE DIRECT-RUN RECIPES NOW ACCEPT BARE COMMAND NAMES AND SEARCH THE
IMPORTED PATH VARIABLE FOR REGULAR EXECUTABLE FILES. PREVIOUSLY THESE
RECIPES REQUIRED ABSOLUTE EXECUTABLE PATHS. ABSOLUTE AND EXPLICIT RELATIVE
PATHS REMAIN SUPPORTED. KCC/DAIMOS.MK USES KCC, DAS, DARC, DLINK, INSTALL,
AND RM AS PATH-RESOLVED COMMANDS; THE INSTALLATION DESTINATIONS REMAIN
ABSOLUTE. HOST KCC COMPILATION AND TESTKIT COMMAND-COMPARISON CHECKS PASS.
NATIVE SIMH EXECUTION REMAINS TO BE VERIFIED.

## NATIVE KCC CLEANUP AND RECURSIVE RM (2026-10-10)

THE SYSTEM C IMPLEMENTATION OF RM NOW SUPPORTS BOUNDED RECURSIVE REMOVAL
(-R), OPTIONAL ERROR SUPPRESSION (-F), AND FINAL-COMPONENT SYMLINK HANDLING
THROUGH PARENT DIRECTORY ENTRY LOOKUP. ROOT AND DOT/DOT-DOT PATH COMPONENTS
ARE REFUSED FOR RECURSION. THE OBSOLETE OPTIONAL ASSEMBLER RM IS REMOVED
FROM SOURCE/INSTALL RULES, INCLUDING REMOVAL OF A PREVIOUSLY INSTALLED
ASSEMBLER RM. KCC/DAIMOS.MK NOW USES A COMMON .S.DOBJ SUFFIX RULE AND
CLEAN INVOKES RM -R -F BUILD. THE CURRENT HOST ENTRY POINT USES CROSS.MK,
COMMON.MK AND NATIVE.MK; UNIX.MK WAS REMOVED.

HOST KCC COMPILATION AND TESTKIT MOCK-VFS RECURSION, SUFFIX-INFERENCE,
AND STRUCTURAL TESTS PASS. NATIVE SIMH EXECUTION OF THE NEW SYSTEM RM AND
ON-TARGET MAKE SUFFIX RULE REMAINS UNVERIFIED.

## KCC CANONICAL CROSS/NATIVE TARGETS (2026-10-10)

THE SHARED KCC MAKEFILE STILL SELECTS UNIX.MK ON HOSTS AND
DAIMOS.MK AFTER NATIVE SIXBIT STAGING. UNIX `make` DEFAULTS TO
`all: cross`; `make cross` BUILDS THE UNIX-HOSTED KCC CROSS-
COMPILER AND HOST RUNTIME. `make native` BUILDS THE DAIMOS
COMPILER PHASES/DRIVER, RUNTIME AND LIBC USING THE CROSS TOOLCHAIN.
LEGACY PUBLIC ALIASES HOST-BUILT, NATIVE-BUILT, KCC, ASM,
SELF-ASM, AND NATIVE-* CONVENIENCE TARGETS WERE REMOVED;
INTERNAL ARTIFACT TARGETS AND MAINTENANCE INSTALL/CLEAN TARGETS
REMAIN. UNDER DAIMOS, NATIVE IS THE FIRST BUILD TARGET AND
THE ONLY PUBLIC BUILD TARGET; PHASE ALIASES AND ALL WERE REMOVED.
GNU MAKE AND ISOLATED BSD BMAKE `-n` VERIFIED UNIX DEFAULT=CROSS,
NATIVE TARGET SELECTION, AND REJECTION OF OLD ALIASES. THE
NATIVE PLATFORM GRAPH WAS CHECKED STRUCTURALLY. NO COMPILER
EXECUTION WAS PERFORMED; THIS DOES NOT ESTABLISH THAT KCC WORKS.
REGRESSIONS ARE IN DAIMOS-TESTKIT.

## KCC GNU/BSD MAKE ASCII COMPATIBILITY (2026-10-10)

KCC/DAIMOS.MK IS ALREADY ASCII IN THE REPOSITORY; ONLY THE STAGED
DAIMOS.MK IS CONVERTED TO SIXBIT RECORDS. THE ASCII DAIMOS FILE
PASSES GNU MAKE AND ISOLATED BSD BMAKE CLEAN DRY-RUN PARSING.
IT IS NOT A DIRECT HOST BUILD: NATIVE RECIPES REFER TO SIXBIT
STAGED UPPERCASE SOURCE NAMES AND /OPTION/BASE/EXEC/KCC PATHS.
THE UNIX.MK FILE IS THE HOST/CROSS BUILD CONTRACT. TEN ADJACENT
+= ASSIGNMENT GROUPS WERE WRAPPED WITH PORTABLE BACKSLASHES;
LINES FELL FROM 561 TO 492 WITH NO CHANGE TO VARIABLE INVENTORIES.
GNU MAKE AND BMAKE EXPAND SIX REPRESENTATIVE HOST/NATIVE MODULE
INVENTORIES TO IDENTICAL LISTS AND ACCEPT CLEAN DRY RUNS. CLEAN
COMMAND OUTPUT IS UNCHANGED BEFORE/AFTER THE FORMATTING CHANGE.
BMAKE WAS OBTAINED AS AN EXTRACTED PACKAGE IN A TEMP DIRECTORY;
NO SYSTEM INSTALLATION WAS REQUIRED. THE PORTABILITY CHECKER IS
IN DAIMOS-TESTKIT. NEITHER BACKEND RAN THE BROKEN NATIVE KCC;
NO NEW COMPILER EXECUTABLE WAS PRODUCED OR CLAIMED.

## NATIVE KCC MAKEFILE DRY-RUN VALIDATION BLOCKED (2026-10-10)

A SAFE NATIVE SIMH PREFLIGHT WAS PREPARED WITH MAKE -Q / MAKE -N
ONLY: NO KCC, DAS, LINKER, OR CLEAN RECIPE EXECUTION IS ALLOWED.
IT USES THE NORMALIZED KCC DAIMOS.MK WITH PORTABLE $@/$< DAS
RECIPES. THE HOST SEMANTIC CHECK STILL SHOWS 267 EXPANDED RECIPE
COMMANDS AND 140 RULE TARGETS IDENTICAL TO THE PRECHANGE INPUT.
GNU MAKE -N CLEAN PARSES SUCCESSFULLY. THE NATIVE PREFLIGHT
COULD NOT START SIMH: HOST BOOT IMAGE GENERATION FAILED DURING
MKD6FS POST-FORMAT D6FSCK, REPORTING ALLOCATED LEAKED BLOCKS
14213..14217 AND ABORTING ON DSK0.DSK. THIS IS A SEPARATE BOOT
IMAGE VALIDATION FAILURE, NOT EVIDENCE OF A MAKEFILE REGRESSION.
DO NOT CLAIM A PASS FOR NATIVE -N / -Q UNTIL A VALID IMAGE CAN BE
BUILT; DO NOT INVOKE THE STILL-BROKEN NATIVE KCC COMPILER.
THE NO-RECIPE-EXECUTION PREFLIGHT IS PRESERVED IN DAIMOS-TESTKIT.

## NATIVE KCC PORTABLE AUTOMATIC-VARIABLE RECIPES (2026-10-10)

THE NATIVE MAKE PARSER ALREADY SUPPORTS THE PORTABLE AUTOMATIC
VARIABLES $@ (TARGET), $< (FIRST PREREQUISITE), $? (NEWER
PREREQUISITES), AND $* (STEM) IN RECIPES. NO PARSER CHANGE WAS
NEEDED. THE KCC DAIMOS.MK FILE NOW USES $@ $< IN 53 ONE-INPUT
DAS ASSEMBLER RECIPES IN PLACE OF REPEATED OUTPUT/INPUT NAMES.
THE DAIMOS-TESTKIT HOST CHECK EXPANDS BOTH SIDES AND VERIFIES
THAT ALL 267 RECIPE COMMANDS AND 140 TARGET DECLARATIONS ARE
UNCHANGED. GNU MAKE ACCEPTS THE SOURCE (A FULL NATIVE SIMH
INCREMENTAL BUILD ON THE REVISED SOURCE REMAINS TO BE RUN).
COMPILER AND LINKER RECIPES REMAIN EXPLICIT BECAUSE THEIR
PHASE-SPECIFIC FLAGS AND MULTIPLE INPUTS MAKE THEM DIFFERENT.
ALL FURTHER MAKE WORK SHOULD SERVE THE ACTUAL KCC MAKEFILE; DO
NOT EXTEND THE PARSER WITHOUT A DEMONSTRABLE PORTABLE REQUIREMENT.

## NATIVE KCC MAKEFILE PORTABLE CONTINUATIONS (2026-10-10)

THE KCC DAIMOS.MK FILE NOW COMBINES ADJACENT TARGET DEPENDENCIES
INTO READABLE PHYSICAL LINES (USUALLY <= 96 CHARACTERS), USING
STANDARD BACKSLASH CONTINUATIONS WHEN A TARGET NEEDS MORE THAN ONE
PHYSICAL LINE. NATIVE MAKE ALREADY IMPLEMENTS THIS CONTINUATION
SYNTAX, SO NO MAKE PARSER FEATURE CHANGE WAS NEEDED.
COMPLETED NORMALIZATION REDUCED THE FILE FROM 1038 TO 460 LINES
INCLUDING NEW COMMENTS; THE LONGEST JOINED LOGICAL LINE IS 233
CHARACTERS (NATIVE LIMIT 256). DAIMOS-TESTKIT VALIDATION PRESERVES
ALL ORDERED PREREQUISITES, RECIPE LINES, AND DIRECTIVES FROM THE
ORIGINAL FILE. GNU MAKE -N CLEAN ACCEPTS THE NORMALIZED FILE.
THE TARGET/DEPENDENCY GRAPH IS UNCHANGED, SO A PEAK-RAM REDUCTION
IS NOT CLAIMED. NATIVE SIMH BUILD INVALIDATION REMAINS TO BE
TESTED. CHEAP PORTABLE FEATURES SHOULD BE EVALUATED ONLY WHEN A
REAL MAKEFILE REQUIREMENT EXISTS; .PHONY, .SUFFIXES, AND BASIC
BACKSLASH CONTINUATIONS ARE ALREADY IMPLEMENTED. DO NOT ADD
GNU-ONLY OR BSD-ONLY SYNTAX TO THE SHARED BUILD CONTRACT.

## NATIVE MAKE END-TO-END PEAK BRK VERIFIED (2026-10-10)

MATCHED NATIVE PDP-6 SIMH BEFORE/AFTER RUNS, 13E85C6 VS 5B0E768,
INTERCEPTED ALL MAKE-LINKED LIBC DSYS_BRK SITES, LOGGED HIGHEST
SUCCESSFUL BREAK, AND EMITTED AN ADDITIONAL SNAPSHOT IMMEDIATELY
BEFORE MAKE MAIN RETURNED. ALL FIVE PROBES PASS FOR BOTH VERSIONS.
LARGE-GRAPH MAXIMUM BREAK WORD ADDRESSES WERE 43051 BASELINE AND
37302 OPTIMIZED, DIFFERENCE -5749 WORDS. THE EXIT BREAK AND PEAK
MATCHED IN BOTH VERSIONS; THE EXECUTION PHASE DID NOT INCREASE THE
BREAK ABOVE THE POST-PARSE LEVEL. EXIT FREE-LIST WORDS WERE
10847/5096; LARGEST BLOCKS 7218/2752. BOTH VERSIONS HAVE TWO
ARENA RELOCATIONS; THE LATER RELOCATION IS EARLIER WITH THE
SEPARATE SCRATCH STACK (20000->25000 VS 31250->39062 CHARS).
THESE ARE PROCESS BREAK ADDRESSES, NOT NET ALLOCATED WORDS, AND
PROFILE BUILD LAYOUT CAN SHIFT ABSOLUTE VALUES. THE DIFFERENCE
IS THE MATCHED-END-TO-END MAXIMUM BREAK REDUCTION IN THIS FIXTURE.
NO PRODUCTION PROFILING INSTRUMENTATION IS RETAINED.

## NATIVE MAKE COMPLETE LIBC BREAK-EXTENSION PROFILE (2026-10-10)

AN ISOLATED PDP-6 SIMH BEFORE/AFTER PROFILE INTERCEPTED EVERY
DSYS_BRK CALL SITE IN THE MAKE-LINKED LIBC, INCLUDING THE MALLOC,
REALLOC, BRK AND SBRK PATHS. ALL FIVE NATIVE MAKE TESTS PASSED FOR
EACH BUILD. AT THE LARGE-GRAPH POST-PARSE CHECKPOINT, THE MAXIMUM
BREAK OBSERVED SO FAR WAS 43051 (BASELINE 13E85C6) AND 37302
(SEPARATE SCRATCH 5B0E768), A REDUCTION OF 5749 PDP-6 WORDS.
CURRENT BREAKS EQUALED THOSE MAXIMA AT THE CHECKPOINT. FREE-LIST
TOTALS WERE 10879/5128 WORDS; LARGEST FREE BLOCKS 7218/2752.
ARENA REALLOCATIONS REMAIN TWO IN EACH BUILD. BASELINE RELOCATIONS
OCCUR AT 8192->10240 AND 31250->39062 CHARACTERS; OPTIMIZED
RELOCATIONS AT 8192->10240 AND 20000->25000 CHARACTERS.
THE TEST RECORDS 76/77 LIBC BREAK CALLS AND 18/21 SUCCESSFUL NONZERO
BREAK SET REQUESTS (WHICH ARE NOT NECESSARILY ALL INCREASES).
THE RESULT IS THE PEAK THROUGH THE POST-PARSE CHECKPOINT, NOT A
PROVEN END-TO-END MAXIMUM: LATER DEPENDENCY EXECUTION MAY ALLOCATE.
ABSOLUTE BREAK ADDRESSES ARE NOT TOTAL PROCESS ALLOCATION SIZES.
THE DIFFERENT 37302 VALUE FROM THE EARLIER 36278 SNAPSHOT IS A
PROFILING-BUILD LAYOUT EFFECT; COMPARE MATCHED INSTRUMENTED RUNS,
NOT ABSOLUTE ADDRESSES ACROSS DIFFERENT INSTRUMENTATIONS.
PRODUCTION MAKE AND LIBC REMAIN UNINSTRUMENTED. THE HOST PARSER IN
DAIMOS-TESTKIT VALIDATES THE COMPARISON.

## NATIVE MAKE SCRATCH-SEPARATION HEAP A/B (2026-10-10)

ISOLATED PDP-6 SIMH LARGE-GRAPH PROBES WITH TEMPORARY LIBC HEAP
SNAPSHOTS COMPARE 13E85C6 AGAINST 5B0E768. ALL FIVE NATIVE MAKE
PROBES PASS FOR BOTH BUILDS. AT THE SAME POST-PARSE CHECKPOINT,
PROCESS BREAK WORD ADDRESS FALLS FROM 43051 TO 36278 (6773 WORDS),
FREE-LIST TOTAL FROM 10879 TO 5128 (5751 WORDS), AND LARGEST FREE
BLOCK FROM 7218 TO 2752 (4466 WORDS). THE STORED STRING COUNT IS
IDENTICAL AT 35647 NINE-BIT CHARACTERS, WHILE TRIMMED ARENA
CAPACITY FALLS FROM 42644 TO 35648 CHARACTERS. THIS CONFIRMS A
SUBSTANTIALLY LOWER POST-PARSE HEAP BREAK AND LESS FREE-LIST
FRAGMENTATION, NOT MERELY SMALLER NOMINAL ARENA CAPACITY.
THE BREAK VALUES ARE ABSOLUTE WORD ADDRESSES; THE DIFFERENCE IS
NOT THE TOTAL PROCESS MEMORY OR A DIRECT MEASURE OF ITS HIGH-WATER
MARK. THE TEMPORARY HIGH COUNTER DOES NOT TRACK ALL BRK EXTENSIONS
THROUGH REALLOC, SO IT MUST NOT BE CLAIMED AS AN EXECUTION-WIDE
MAXIMUM. PRODUCTION LIBC AND MAKE WERE NOT INSTRUMENTED.
DAIMOS-TESTKIT HAS A HOST PARSER FOR THESE MATCHED SNAPSHOTS.

## NATIVE MAKE SCRATCH SEPARATION A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B EXCLUDED BUILD/BOOT/LOGIN AND
COMPARED BASELINE 13E85C6 VS SEPARATE IMPLICIT SCRATCH 5B0E768.
ALL FIVE PROBES PASSED. FRAGMENT 1.131/1.029 SECONDS, INCLUDE
1.130/1.130, SUFFIX 1.028/1.130, LARGE GRAPH 107.879/106.338
(-1.541 S, -1.43%), INDEX ORDER 1.642/1.539. THESE SMALL SPEED
DIFFERENCES ARE FROM ONE MATCHED PAIR, NOT STATISTICALLY PROVEN.
THE FIXED 33-LEVEL IMPLICIT SCRATCH ARRAY IS ALLOCATED ONCE AFTER
PARSING; IT NO LONGER INFLATES EACH STRING-ARENA GROWTH REQUIREMENT.
NO PER-RECURSION MALLOC IS INTRODUCED. ACTUAL PROCESS HIGH-WATER
BREAK AND FREE-LIST FRAGMENTATION MUST STILL BE PROFILED; LOWER
ARENA CAPACITY ALONE IS NOT PROOF OF LOWER TOTAL PEAK HEAP.
THE REPRODUCER IS IN DAIMOS-TESTKIT TESTS/VALIDATION.

## NATIVE MAKE SEPARATE IMPLICIT SCRATCH STACK (2026-10-10)

THE FIXED (MAKE_MAX_DEPTH+1) IMPLICIT RULE SCRATCH ARRAY IS NOW
ALLOCATED ONCE, AFTER PARSING, RATHER THAN RESERVED AT THE TOP OF
THE GROWING STRING ARENA. MAKE_STORE GROWS FOR STORED STRINGS ONLY;
POST-PARSE ARENA SHRINK RETAINS WORD ALIGNMENT AND ALL STRING
OFFSETS. MAKE_BUILD ADDRESSES A DEDICATED PREALLOCATED SCRATCH
ARRAY BY RECURSION DEPTH; NO PER-RECURSION ALLOCATION IS USED.
A FAILED SCRATCH ALLOCATION REPORTS A NORMAL MAKE STORAGE ERROR.
KCC/DAS COMPILATION AND ALL FIVE NATIVE PDP-6 SIMH MAKE REGRESSIONS
PASS. EFFECT ON PEAK HEAP AND RUNTIME REQUIRES ISOLATED A/B
BENCHMARKING; THE CHANGE IS A TESTED CANDIDATE, NOT PROOF OF A
REDUCED PROCESS HIGH-WATER MARK.

## NATIVE MAKE RELOCATION BLOCK OWNER IDENTIFICATION (2026-10-10)

A TEMPORARY DETACHED WORKTREE WITH WORD-ADDRESS ALLOCATION TRACE
LOGGED EVERY MAKE_GROW AND MAKE_GROW_GRAPH REALLOC RESULT. ALL FIVE
NATIVE PDP-6 SIMH MAKE REGRESSIONS PASS. MATCHING THE ARENA BLOCK
END AGAINST THE START OF THE NEXT LIVE ALLOCATION SHOWS:
AT ARENA CAPACITY 8192 CHARS, END WORD 14338 IS FOLLOWED BY
THE VARIABLE TABLE (INITIAL=16, RECORD SIZE=12 CHARACTERS,
CAPACITY=16), OCCUPYING 50 WORDS INCLUDING MALLOC HEADER.
AT ARENA CAPACITY 31250 CHARS, END WORD 22505 IS FOLLOWED BY
THE DEPENDENCY-LINK TABLE (INITIAL=64, RECORD SIZE=4 CHARACTERS,
CAPACITY=324), OCCUPYING 326 WORDS INCLUDING HEADER.
THIS IDENTIFIES THE OWNERS OF BOTH BLOCKING LIVE ALLOCATIONS.
THE GENERAL LIBC IN-PLACE REALLOC PATH IS FUNCTIONING; THE
FOLLOWING MEMORY IS LIVE, NOT FREE. A STORE-FIRST MAKE EXPERIMENT
PREVIOUSLY LEFT BOTH RELOCATIONS UNCHANGED. FUTURE OPTIONS MUST
BE EVALUATED AGAINST TOTAL PEAK HEAP AND COPYING WORK; SIMPLY
REORDERING AN INSERTION OR INCREASING ALL INITIAL CAPACITIES IS
NOT A PROVEN WIN. TEMPORARY INSTRUMENTATION IS NOT PRODUCTION CODE.

## NATIVE MAKE ARENA NEIGHBOR BLOCK PROFILE (2026-10-10)

DETACHED TEMPORARY LIBC INSTRUMENTATION REPORTED THE ARENA BLOCK'S
ENDING PDP-6 WORD ADDRESS, WHETHER THE PHYSICALLY ADJACENT BLOCK
WAS FREE, ITS FREE-BLOCK SIZE, AND THE CURRENT PROCESS BREAK.
ALL FIVE NATIVE SIMH MAKE PROBES PASSED. LARGE-GRAPH GROWTH AT
8192 CHARS: ARENA END=14338, BREAK=14690, NO ADJACENT FREE BLOCK;
AT 31250 CHARS: END=22505, BREAK=25926, NO ADJACENT FREE BLOCK.
THUS BOTH OBSERVED ARENA RELOCATIONS ARE NECESSARY UNDER THE
EXISTING ALLOCATION LAYOUT: NEITHER DIRECT BREAK EXPANSION NOR
ADJACENT-FREE-BLOCK EXPANSION IS AVAILABLE. IN-PLACE GROWTH AT
10240, 12800, 16000, 20000, 25000, AND 39062 CHARS OCCURRED
WHEN THE ARENA REACHED THE BREAK. THE GENERAL LIBC ALLOCATOR IS
NOT SHOWN DEFECTIVE BY THIS PROFILE; NO LIBC OR PRODUCTION MAKE
MODIFICATIONS WERE JUSTIFIED. FURTHER IMPROVEMENT REQUIRES
IDENTIFYING THE OWNER/LIFETIME OF THE FOLLOWING ALLOCATED BLOCK
AND TESTING A SPECIFIC LAYOUT CHANGE. DAIMOS-TESTKIT HAS A HOST
PARSER FOR THE TEMPORARY NATIVE TRACE.

## NATIVE MAKE STORE-FIRST ALLOCATION EXPERIMENT REJECTED (2026-10-10)

AN ISOLATED EXPERIMENT MOVED MAKE_STORE/MAKE_DEP_INTERN AHEAD OF
GRAPH TABLE CAPACITY GROWTH IN MAKE_GET_RULE AND MAKE_ADD_DEP.
KCC/DAS AND ALL FIVE NATIVE PDP-6 SIMH REGRESSION PROBES PASSED.
A REPEATED INSTRUMENTED LARGE-GRAPH ARENA GROWTH PROFILE HAD THE
SAME TWO RELOCATIONS AS BASELINE: 8192->10240 AND 31250->39062
NINE-BIT CHARACTERS. ALL OTHER ARENA GROWTH WAS IN PLACE.
THERE IS NO CONFIRMED FRAGMENTATION BENEFIT, SO THE SOURCE EXPERIMENT
WAS REVERTED WITHOUT A PRODUCTION CODE CHANGE. FURTHER WORK SHOULD
IDENTIFY THE ACTUAL BLOCK DIRECTLY ADJACENT TO THE ARENA USING A
WORD-ADDRESS-AWARE HEAP TRACE, RATHER THAN INFER THE BLOCKER FROM
NEARBY MAKE-GROW EVENTS. LIBC REALLOC ALREADY SUPPORTS FREE-NEIGHBOR
AND BREAK-ADJACENT IN-PLACE EXPANSION.

## NATIVE MAKE ADDRESS-LEVEL ARENA GROWTH CHECK (2026-10-10)

AN INDEPENDENT DETACHED WORKTREE TRACED RAW ALLOCATOR POINTER VALUES
FOR MAKE_GROW (THE STRING ARENA) ON THE NATIVE PDP-6 SIMH LARGE-GRAPH
FIXTURE. ALL FIVE NATIVE MAKE REGRESSIONS PASS. NINE ALLOCATION
EVENTS OCCUR: ONE INITIAL ALLOCATION; TWO MOVING REALLOCS AT
8192->10240 AND 31250->39062 CHARACTERS; SIX IN-PLACE REALLOCS.
POINTERS ARE KCC BYTE POINTER VALUES, NOT DIRECT WORD ADDRESSES.
THIS TRACE CONFIRMS ARENA MOVEMENT BUT DOES NOT IDENTIFY WHICH
INTERLEAVED RULE/DEPENDENCY/HASH ALLOCATION BLOCKED CONTIGUOUS
GROWTH. DO NOT CHANGE GENERAL LIBC REALLOC OR MAKE'S ALLOCATION
ORDER WITHOUT IDENTIFYING THE INTERVENING BLOCK AND BENCHMARKING
THE RESULT. PRODUCTION SOURCE WAS NOT MODIFIED. TESTKIT CONTAINS
A REUSABLE TRACE PARSER.

## NATIVE MAKE ARENA REALLOC MOVEMENT TRACE (2026-10-10)

TEMPORARY MAKE_GROW INSTRUMENTATION IN A DETACHED WORKTREE LOGGED
EVERY ARENA REALLOC'S OLD CAPACITY, NEW CAPACITY, AND WHETHER THE
ALLOCATED POINTER MOVED. ALL FIVE NATIVE PDP-6 SIMH MAKE PROBES PASS.
LARGE-GRAPH ARENA CHARS: INITIAL 0->8192; RELOCATION 8192->10240;
IN-PLACE 10240->12800->16000->20000->25000->31250;
RELOCATION 31250->39062; IN-PLACE 39062->48827.
TWO OF EIGHT REALLOCATIONS OF AN EXISTING ARENA RELOCATED IT.
THE SECOND RELOCATION FREES AN OLD 31250-CHAR (~7813-WORD) ARENA;
THE PREVIOUSLY REPORTED LARGEST FREE BLOCK WAS 7218 WORDS, SO THE
TWO OBSERVATIONS ARE RELATED CANDIDATES, NOT A PROVEN IDENTICAL
BLOCK. ITS EXACT ORIGIN NEEDS FREE-LIST ADDRESS AND ALLOCATION-
SITE TRACING. DO NOT ALTER THE GENERAL LIBC ALLOCATOR BASED ONLY
ON THIS MAKE-SPECIFIC SNAPSHOT. PRODUCTION MAKE/LIBC ARE UNCHANGED;
DAIMOS-TESTKIT CONTAINS THE MOVEMENT-TRACE PARSER.

## NATIVE MAKE ALLOCATOR FREE-LIST SNAPSHOTS (2026-10-10)

A DETACHED INSTRUMENTED WORKTREE ADDED A TEMPORARY LIBC HEAP-FREE-
LIST PROBE, AND NATIVE MAKE PRINTED BREAK, FREE WORDS, AND LARGEST
FREE BLOCK BEFORE AND AFTER POST-PARSE ARENA SHRINK. ALL FIVE PDP-6
SIMH MAKE PROBES PASS. THE LARGE GRAPH REPORTS BREAK=43051 BOTH
TIMES, FREE WORDS=9333/10879 AND LARGEST FREE BLOCK=7218/7218.
THUS THE TRIM RETURNED 1546 ADDITIONAL 36-BIT WORDS TO THE FREE
LIST; THE PROCESS BREAK DID NOT FALL. THE PRE-TRIM ARENA CAPACITY
WAS 48827 NINE-BIT CHARACTERS AND USED LENGTH 35647. THIS IS
ALREADY A 5/4-GROWTH BUILD. ABSOLUTE BREAK IS AN ADDRESS, NOT A
MEASURE OF HEAP FOOTPRINT, AND THIS SNAPSHOT DOES NOT MEASURE THE
FULL EXECUTION HIGH-WATER MARK OR NUMBER OF REALLOC COPYING EVENTS.
OTHER PROBES RECOVERED 259, 204, 204, AND 389 FREE WORDS FROM
POST-PARSE TRIMMING. THE 7218-WORD LARGEST FREE BLOCK IN THE LARGE
GRAPH REMAINED UNCHANGED; INVESTIGATE TEMPORARY ALLOCATION MOVEMENT
AND FRAGMENTATION BEFORE FURTHER ARENA GROWTH CHANGES. PRODUCTION
MAKE/LIBC WERE NOT INSTRUMENTED. HOST PROFILE PARSER IS IN TESTKIT.

## NATIVE MAKE FIVE-QUARTERS ARENA GROWTH A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B MEASURED DSH COMMAND TO PROMPT, NOT
BOOT OR IMAGE BUILD: BASELINE C54ACA1 VS 5/4 ARENA GROWTH 0B24B62.
ALL FIVE PROBES PASS. FRAGMENT 1.028/1.027, INCLUDE 1.131/1.131,
SUFFIX 1.028/1.028, LARGE GRAPH 108.181/107.563 (-0.618 S,
-0.57%), INDEX ORDER 1.641/1.641 SECONDS. THE SMALL SPEED
DIFFERENCE FROM ONE PAIR IS NOT STATISTICALLY ESTABLISHED.
FOR THE PREVIOUSLY MEASURED 42644-CHARACTER ARENA REQUIREMENT,
5/4 GROWTH FROM 8192 REACHES 48827 CHARACTERS RATHER THAN 65536,
A PROJECTED CAPACITY REDUCTION OF 16709 NINE-BIT CHARS OR 4177
36-BIT WORDS (ROUNDED DOWN). THIS IS NOT MEASURED PEAK PROCESS
RAM; HEAP REALLOC FRAGMENTATION IS STILL TO BE CHECKED. THE
ALIGNED POST-PARSE ARENA TRIM REMAINS. SEE DAIMOS-TESTKIT'S NATIVE
MAKE ARENA-125 A/B REPRODUCER.

## NATIVE MAKE FIVE-QUARTERS ARENA GROWTH (2026-10-10)

STRING ARENA GEOMETRIC GROWTH IS REDUCED FROM 2X TO 5/4X TO LOWER
PEAK ALLOCATED ARENA RAM DURING PARSING; GRAPH ARRAYS RETAIN 3/2X.
THE MEASURED LARGE-GRAPH SHRINK TARGET FROM THE PREVIOUS PROFILE IS
42644 NINE-BIT CHARS. STARTING FROM 8192, THE NEW GROWTH SERIES
REACHES 48827 CHARS RATHER THAN 65536: A CAPACITY DIFFERENCE OF
16709 CHARS, OR APPROXIMATELY 4177 PDP-6 WORDS. THIS IS A
CAPACITY PROJECTION FROM THE RECORDED WORKLOAD, NOT A HEAP-BREAK OR
PEAK-PROCESS-RAM MEASUREMENT; ALLOCATOR REALLOC WORK MAY INCREASE.
THE POST-PARSE WORD-ALIGNED ARENA TRIM REMAINS ACTIVE. KCC/DAS AND
ALL FIVE NATIVE SIMH MAKE PROBES PASS. ISOLATED A/B TIMING IS NEXT.

## NATIVE MAKE ACTUAL ARENA CAPACITY PROFILE (2026-10-10)

TEMPORARY INSTRUMENTATION IN A DETACHED DAIMOS WORKTREE PRINTED
ARENA CHARACTERS USED, ALLOCATED BEFORE SHRINK AND ALIGNED TARGET
CAPACITY. THE FIVE NATIVE PDP-6 SIMH PROBES ALL PASS. LARGE GRAPH:
USED=35647, BEFORE CAPACITY=65536, TARGET=42644 9-BIT CHARACTERS.
THE POST-PARSE SHRINK RETURNS 22892 CHARACTERS = 5723 36-BIT WORDS
OF ALLOCATED ARENA PAYLOAD (SUBJECT TO LIBC ALLOCATOR TAIL-SPLITTING).
OTHER PROBES: FRAGMENT 8192->7156 (259 WORDS); INCLUDE AND SUFFIX
8192->7376 (204 WORDS EACH); INDEX-ORDER 16384->8684 (1925 WORDS).
THE TARGET PRESERVES THE FULL 33-LEVEL IMPLICIT SCRATCH RESERVATION;
THIS IS NOT AN INCREASE IN THE HIGH-WATER-MARK BREAK OR AN ACTUAL
PEAK PROCESS-RAM MEASUREMENT. THE ARENA'S PEAK STILL OCCURS WHILE
PARSING. INSTRUMENTATION WAS NOT ADDED TO PRODUCTION MAKE SOURCE.
TESTKIT HOST PARSER: DAIMOS-NATIVE-MAKE-ARENA-ACCOUNTING-20261010-V1.PY.

## NATIVE MAKE POST-PARSE ARENA TRIM A/B (2026-10-10)

MATCHED PDP-6 SIMH A/B (DSH COMMAND TO PROMPT; IMAGE BUILD/BOOT/LOGIN
EXCLUDED) COMPARED BASELINE 937D01F AGAINST POST-PARSE WORD-ALIGNED
ARENA TRIM 9F76395. ALL FIVE PROBES PASS. FRAGMENT 1.029/1.028 S,
INCLUDE 1.028/1.028, SUFFIX 1.028/1.028, LARGE GRAPH
109.534/108.419 (-1.115 S, -1.02%), INDEX 1.540/1.540 S.
A SINGLE MATCHED PAIR IS INSUFFICIENT TO ESTABLISH A RELIABLE SPEEDUP.
THE TRIM RETURNS UNUSED GEOMETRIC ARENA CAPACITY AFTER PARSING WHILE
RETAINING ALL STRINGS AND THE ALIGNED 33-SLOT IMPLICIT SCRATCH STACK.
THE PREVIOUS PROFILE REPORTED 35647 STORED ARENA CHARACTERS, BUT THIS
A/B DID NOT DIRECTLY MEASURE PRE- AND POST-TRIM ALLOCATIONS OR PEAK
PROCESS RAM. NO QUANTIFIED RESIDENT-WORD SAVING IS CLAIMED YET.
THE REPRODUCER IS IN DAIMOS-TESTKIT TESTS/VALIDATION.

## NATIVE MAKE POST-PARSE ARENA TRIM (2026-10-10)

AFTER MAKEFILE PARSING, MAKE RETURNS SPARE GEOMETRIC STRING-ARENA
CAPACITY TO THE DAIMOS LIBC ALLOCATOR WITH REALLOC SHRINK-IN-PLACE.
THE RETAINED ARENA SIZE IS WORD-ALIGNED AND INCLUDES ALL STORED
STRINGS AND THE ENTIRE (MAKE_MAX_DEPTH+1) IMPLICIT-RULE SCRATCH
STACK. NO STRING POINTERS OR GRAPH INDICES ARE INVALIDATED;
SHRINK FAILURE LEAVES THE ORIGINAL ARENA VALID. THE DAIMOS LIBC
REALLOC IMPLEMENTATION RETURNS A FREE TAIL WHEN LARGE ENOUGH.
KCC/DAS AND ALL FIVE NATIVE PDP-6 SIMH MAKE TESTS PASS ON THE FINAL
WORD-ALIGNED IMPLEMENTATION. SAVINGS DEPEND ON LAST GEOMETRIC GROWTH;
ACTUAL PEAK PROCESS RAM AND WALL-TIME A/B REMAIN TO BE MEASURED.

## NATIVE MAKE THREE-HALVES GRAPH GROWTH A/B (2026-10-10)

MATCHED NATIVE SIMH RUN EXCLUDING IMAGE BUILD, BOOT AND LOGIN:
BASELINE 34EF931 VS GRAPH 3/2-GROWTH 594232C; ALL FIVE PROBES PASS.
FRAGMENT 1.028/1.132 S, INCLUDE 1.131/1.028, SUFFIX 1.028/1.028,
LARGE GRAPH 110.553/111.475 (+0.922 S, +0.83%), INDEX 1.643/1.641.
ONE A/B PAIR CANNOT ESTABLISH A PRECISE PERFORMANCE REGRESSION.
RETAIN 3/2 GRAPH GROWTH FOR AN ESTIMATED 2390 FEWER 36-BIT WORDS
OF RULE, ORDER AND DEPENDENCY CAPACITY ON THIS FIXTURE. ACTUAL
PEAK PROCESS RAM AND ALLOCATOR FRAGMENTATION REMAIN UNMEASURED.
THE BYTE STRING ARENA STILL DOUBLES. TESTKIT VALIDATION CONTAINS
THE REPRODUCIBLE MATCHED-NATIVE A/B DRIVER.

## NATIVE MAKE THREE-HALVES GRAPH GROWTH (2026-10-10)

GRAPH TABLES (RULES, SORTED RULE ORDER, DEPENDENCIES, RECIPES,
VARIABLES) NOW GROW BY APPROXIMATELY 3/2 RATHER THAN DOUBLING.
THE STRING ARENA KEEPS DOUBLING TO LIMIT DATA COPYING. FOR THE
681-LHS/1080-LINK LARGE-GRAPH FIXTURE, CAPACITIES CHANGE AS FOLLOWS:
RULES 1024 TO 819 (SIX WORDS EACH, SAVING 1230 WORDS), RULE ORDER
1024 TO 819 (205 WORDS), DEPENDENCIES 2048 TO 1093 (955 WORDS).
TOTAL ESTIMATED ALLOCATED GRAPH-TABLE SAVINGS ARE 2390 36-BIT WORDS
BEFORE RECIPE/VARIABLE TABLES AND ALLOCATOR HEADERS. THESE ARE
CAPACITY ESTIMATES, NOT PEAK USER-RAM MEASUREMENTS. KCC/DAS AND
ALL FIVE NATIVE SIMH MAKE PROBES PASS. ISOLATED A/B TIME IS PENDING.

## NATIVE MAKE ONE-WORD LINK A/B — RAM-FIRST RETAINED (2026-10-10)

MATCHED NATIVE SIMH A/B EXCLUDING IMAGE BUILD/BOOT/LOGIN: BASELINE
48AAB5B VS ONE-WORD DEPENDENCY/RECIPE LINKS EE1F377. ALL FIVE NATIVE
PROBES PASSED IN BOTH. FRAGMENT 1.028/1.030 S; INCLUDE 1.132/1.131;
SUFFIX 1.031/1.030; LARGE GRAPH 113.112/115.187 (+2.075 S, +1.83%);
INDEX ORDER 1.644/1.642 S. ONE A/B RUN CANNOT ESTABLISH A PRECISE
RUNTIME REGRESSION. KEEP THE PACKED LINKS FOR RAM SAVINGS: AT 2048
ALLOCATED DEPENDENCY LINKS, SAVES 2048 36-BIT WORDS; EACH ALLOCATED
RECIPE LINK SAVES ONE FURTHER WORD. THIS IS TABLE CAPACITY, NOT A
MEASURE OF PEAK PROCESS MEMORY OR FULL EXECUTABLE SIZE. BENCHMARK
REPRODUCER IS IN DAIMOS-TESTKIT UNDER TESTS/VALIDATION.

## NATIVE MAKE ONE-WORD DEPENDENCY/RECIPE LINKS (2026-10-10)

MAKE_LINK STORES THE LOW 18-BIT STRING ARENA OFFSET AND HIGH 18-BIT
NEXT-LINK INDEX IN ONE 36-BIT WORD. LINK-INDEX END MARKER REMAINS
0777777 AND THE ORIGINAL OFFSET/RANGE LIMITS ARE UNCHANGED. BOTH
DEPENDENCY AND RECIPE TABLES SHRINK FROM TWO WORDS PER ALLOCATED LINK
TO ONE. THE LARGE-GRAPH 2048-LINK CAPACITY THEREFORE SAVES 2048
WORDS OF DEPENDENCY-TABLE STORAGE, PLUS ANY RECIPE-TABLE CAPACITY.
NO EXTRA INDEX IS USED; ACCESSORS MASK/SHIFT ON TRAVERSAL. KCC/DAS
AND ALL FIVE NATIVE SIMH MAKE TESTS PASSED. THE ISOLATED NATIVE A/B
RUNTIME COST AND PEAK PROCESS RAM ARE STILL TO BE MEASURED.

## NATIVE MAKE SIX-WORD RULE ISOLATED A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B TIMED DSH COMMAND TO PROMPT, EXCLUDING
BUILD, BOOT AND LOGIN: BASELINE 91B15D5 VS PACKED 9BB6DC3.
FRAGMENT 1.033/1.030 S, INCLUDE 1.139/1.030 S, SUFFIX 1.136/1.133 S,
LARGE GRAPH 119.344/114.807 S (-4.537 S, -3.8%), AND SORTED INDEX
1.652/1.746 S. ALL FIVE PROBES PASSED. SINGLE PAIR DOES NOT ESTABLISH
A RELIABLE SPEEDUP BUT SHOWS NO LARGE OVERHEAD. THE PRIMARY BENEFIT
IS DEFINITE 7-TO-6-WORD RULE RECORD SHRINKAGE: 1024 WORDS LESS AT
1024-RULE CAPACITY (DOES NOT INCLUDE POSSIBLE EXECUTABLE CODE GROWTH).
PEAK PROCESS RAM REMAINS UNMEASURED. TESTKIT REPRODUCER:
DAIMOS-NATIVE-MAKE-RULE-PACK-AB-20261010-V1.SH.

## NATIVE MAKE SIX-WORD RULE STRUCTURE (2026-10-10)

RULE FLAGS AND FOUR EXECUTION STATES NOW SHARE ONE 36-BIT WORD.
FLAGS USE BITS 0-2 AND STATE USES BITS 3-4. STRUCT MAKE_RULE
SHRINKS FROM SEVEN TO SIX WORDS, WITH NO EXTRA INDEX OR CACHE.
ON THE 1024-CAPACITY LARGE GRAPH THIS SAVES 1024 ALLOCATED RULE-TABLE
WORDS (RAM CAPACITY ESTIMATE, NOT PEAK-RSS MEASUREMENT).
KCC/DAS BUILD AND ALL FIVE NATIVE PDP-6 SIMH MAKE REGRESSIONS PASS.
ISOLATED A/B TIMING IS REQUIRED BEFORE RETAINING THIS AS A RUNTIME
OPTIMIZATION; RAM REDUCTION IS THE PRIMARY PURPOSE.

## NATIVE MAKE POINTER COMPARISON A/B — REVERTED (2026-10-10)

THE POINTER-ITERATION MAKE_RULE_COMPARE FROM C18F3C9 ELIMINATED
%ADJBPH HELPER CALLS FROM ITS GENERATED PDP-6 COMPARISON LOOP,
BUT DID NOT IMPROVE THE MATCHED ISOLATED SIMH COMMAND BENCHMARK.
LARGE GRAPH: 116.873 S AT BASELINE 676B07A VS 118.866 S AT C18F3C9
(+1.993 S, +1.7%). FRAGMENT 1.031/1.033, INCLUDE 1.131/1.137,
SUFFIX 1.032/1.035, INDEX ORDER 1.750/1.644 S. ALL FIVE NATIVE
REGRESSIONS PASSED FOR BOTH IMAGES. THIS IS ONE A/B PAIR, NOT A
STATISTICALLY ESTABLISHED SLOWDOWN, BUT NO SPEED BENEFIT WAS SHOWN.
THE INDEXED IMPLEMENTATION IS RESTORED. DO NOT INFER THAT FEWER
%ADJBPH CALLS ALONE IMPLY FASTER PDP-6 EXECUTION.

## NATIVE MAKE POINTER-BASED RULE COMPARISON (2026-10-10)

MAKE_RULE_COMPARE USES ADVANCING BYTE POINTERS RATHER THAN INDEXED
CHARACTER ACCESS. KCC -O NOW EMITS IBP/LDB IN THE COMPARISON LOOP,
REMOVING EXPENSIVE %ADJBPH CALLS FROM THAT LOOP. SIGNEDNESS AND
LEXICAL COMPARISON ORDER ARE PRESERVED BY EXPLICIT UNSIGNED CHAR
VALUES. NO ADDITIONAL GRAPH RAM IS USED. KCC/DAS AND ALL FIVE
NATIVE PDP-6 SIMH MAKE PROBES PASS. AN ISOLATED A/B PERFORMANCE
CHECK AGAINST 676B07A IS REQUIRED TO ESTABLISH SPEEDUP.

## NATIVE MAKE STRUCTURE AND CODE GENERATION AUDIT (2026-10-09)

SOURCE AND GENERATED PDP-6 ASSEMBLY AUDIT (NO PRODUCTION CODE CHANGE):
MAKE_RULE CONTAINS SEVEN 36-BIT WORD FIELDS; MAKE_LINK TWO; MAKE_VAR
THREE. THE LARGE-GRAPH FIXTURE HAS APPROXIMATELY 681 DISTINCT LHS
LABELS AND 1080 DEPENDENCY LINKS. AT DOUBLING ALLOCATION CAPACITIES
OF 1024 RULES AND 2048 DEPENDENCIES, THE RULE TABLE ALONE COSTS
7168 WORDS, LINKS 4096 WORDS, AND SORTED RULE INDEX 1024 WORDS.
COMBINING FLAGS AND STATE INTO ONE RULE WORD WOULD THEORETICALLY
SAVE 1024 ALLOCATED WORDS ON THIS FIXTURE (681 LIVE RECORD WORDS).
THIS IS CAPACITY-BASED ESTIMATION, NOT MEASURED PEAK PROCESS RAM.
MAKE STRINGS ARE 9-BIT C CHARS PACKED IN 36-BIT WORDS, NOT ONE
36-BIT WORD PER CHARACTER; EARLIER ARENA CHARACTER COUNTS MUST NOT
BE INTERPRETED AS WORD COUNTS.

KCC -O PDP-6 OUTPUT FOR MAKE_RULE_COMPARE PERFORMS EXPENSIVE 9-BIT
INDEXED BYTE-POINTER ADJUSTMENT CALLS TO %ADJBPH AND LDB OPERATIONS
FOR CHARACTER ACCESS. MAKE_STREQ AND MAKE_STRLEN SHOW SIMILAR CODE;
MAKE_DEP_HASH USES DIRECT LDB/IBP AND DOES NOT SHOW THE SAME INDEXED
ADJUSTMENT PATTERN. A HAND-VERIFIED POINTER-WALKING COMPARE (OR
COMPILER OPTIMIZATION) IS A STRONG SPEED CANDIDATE, BUT MUST BE
BENCHMARKED WITHOUT AFFECTING CORRECT UNSIGNED CHARACTER ORDER.
THE NEXT RAM CANDIDATE IS PACKING RULE FLAGS AND STATE INTO ONE WORD;
MEASURE GENERATED CODE GROWTH AND EXECUTION TIME BEFORE RETAINING IT.
TESTKIT HOST AUDIT: DAIMOS-NATIVE-MAKE-STORAGE-AUDIT-20261009-V1.PY.

## NATIVE MAKE PACKED RULE CACHE REJECTED AFTER A/B (2026-10-09)

A MATCHED NATIVE SIMH A/B MEASURED DSH COMMAND EXECUTION (EXCLUDING
BUILD, BOOT AND LOGIN) AT 93FD5EB BASELINE VS C720F2D PACKED-LINK
RULE CACHE. ALL FIVE PROBES PASSED IN BOTH BUILDS. LARGE GRAPH:
112.115 S BASELINE VS 114.322 S CACHED (+2.207 S, +1.97%). OTHER
PROBES: FRAGMENT 1.028/1.031; INCLUDE 1.131/1.131; SUFFIX 1.028/1.030;
INDEX ORDER 1.645/1.641 S. ONE RUN DOES NOT ESTABLISH A PRECISE
REGRESSION MAGNITUDE, BUT THERE IS NO EVIDENCE OF IMPROVEMENT.
A SINGLE MAKE TRAVERSAL USUALLY VISITS EACH DEPENDENCY LINK ONLY ONCE;
LAZY CACHING THEREFORE ADDS MASK/SHIFT/BRANCH COSTS WITHOUT REUSING
THE REFERENCE. GENERATED PDP-6 ASSEMBLY CONFIRMS EXTRA PACK/UNPACK
INSTRUCTIONS AROUND THE LOOKUP. THE SOURCE IS RETURNED TO THE
PRE-CACHE ALGORITHM; DO NOT ADD A PER-LINK CACHE WITHOUT EVIDENCE OF
MULTIPLE LINK TRAVERSALS. THE ORIGINAL TWO-WORD DEPENDENCY LINK AND
SEVEN-WORD RULE STRUCTURES REMAIN UNCHANGED. PEAK PROCESS RAM AND
CODE-WORD DIFFERENCE WERE NOT MEASURED. SEE DAIMOS-TESTKIT VALIDATION
DAIMOS-NATIVE-MAKE-DEPREF-AB-20261009-V1.SH FOR THE REPRODUCER.

## NATIVE MAKE PACKED DEPENDENCY RULE REFERENCES (2026-10-09)

DEP LINK NAME REFERENCES NOW LAZILY CACHE RESOLVED RULE INDEX+1 IN THE
UPPER 18 BITS OF THE EXISTING 36-BIT TEXT WORD. THE LOWER 18 BITS
RETAIN THE ORIGINAL ARENA TEXT OFFSET; ZERO IN THE UPPER HALF MEANS
UNRESOLVED. MAKE_BUILD ACCEPTS A POINTER TO THAT WORD FROM EXPLICIT
DEPENDENCIES AND USES THE RULE INDEX ON LATER TRAVERSALS. IMPLICIT
SOURCE RULES AND COMMAND-LINE GOALS STILL USE NORMAL LOOKUP. $?-LIST
CONSTRUCTION AND DEPENDENCY DIAGNOSTICS MASK TO THE LOW 18-BIT NAME
OFFSET. NO ADDITIONAL PER-RULE OR PER-DEPENDENCY WORDS ARE NEEDED;
THE EXISTING LINK REMAINS TWO 36-BIT WORDS, RULE REMAINS SEVEN WORDS.
THE LAZY CACHE DOES NOT RESOLVE ALL DEPENDENCIES BEFORE BUILDING AND
THEREFORE DOES NOT ADD A WHOLE-GRAPH PREPASS. NATIVE KCC AND DAS
PASSED; ALL FIVE SIMH MAKE-INCLUDE TEST PROBES PASSED. AN ISOLATED
RUNTIME A/B AND MEASURED PEAK PROCESS RAM ARE STILL OPEN; MEMORY
NEUTRALITY HERE REFERS TO THE LINK FIELD LAYOUT, NOT CODE SIZE.

## NATIVE MAKE ISOLATED COMMAND A/B (2026-10-09)

A TESTKIT HARNESS TIMED DSH COMMAND INVOCATION THROUGH RETURN TO DSH
PROMPT, EXCLUDING IMAGE CONSTRUCTION, BOOT AND LOGIN. ON OTHERWISE
MATCHING PDP-6 SIMH IMAGES, NATIVE MAKE BEFORE INTERN PROBE-FIRST
(42A227C) VERSUS AFTER (873A024) RETURNED THESE WALL TIMES (SECONDS):
FRAGMENT 1.029/1.028; INCLUDE 1.132/1.028; SUFFIX SUBSTITUTION
1.031/1.030; LARGE GRAPH 115.724/115.068; SORTED INDEX 1.644/1.644.
ALL FIVE PROBES PASSED IN BOTH RUNS. THE LARGE-GRAPH DIFFERENCE IS
0.656 S (0.57%); ONE RUN IS INSUFFICIENT TO ATTRIBUTE THIS SMALL
DIFFERENCE TO THE OPTIMIZATION. EACH COMMAND INCLUDES DSH DISPATCH,
NATIVE MAKE PARSING AND TRAVERSAL, AND PROMPT-POLLING OVERHEAD;
IT EXCLUDES SIMH BOOT AND BUILD. NATIVE PHASE CYCLE COUNTS, ACTUAL
HASH COLLISION STATISTICS AND PEAK PDP-6 PROCESS RAM WERE NOT MEASURED.
THE OPTIMIZATION IS RETAINED FOR ITS CORRECTNESS AND AVOIDANCE OF
PREMATURE REHASH, BUT NO STRONG SPEEDUP CLAIM IS MADE.

## NATIVE MAKE DEPENDENCY INTERN PROBE BEFORE GROWTH (2026-10-09)

MAKE_DEP_INTERN NOW CHECKS THE EXISTING HASH TABLE FOR DUPLICATES
BEFORE ASSESSING WHETHER INSERTION REQUIRES GROWTH. REPEATED
DEPENDENCY NAMES NO LONGER TRIGGER A PREMATURE REHASH AT THE LOAD
THRESHOLD. ABSENT NAMES STILL GROW THE TABLE BEFORE INSERTION AS
NEEDED; OPEN ADDRESSING AND STRING ARENA OWNERSHIP ARE UNCHANGED.
THE CHANGE ADDS NO PERSISTENT RAM. THE NATIVE PDP-6 KCC/DAS OBJECT
BUILD AND ALL FIVE SIMH MAKE-INCLUDE PROBES PASS. THE FULL HARNESS
TOOK ABOUT 145 S INCLUDING BOOT AND IMAGE CONSTRUCTION; NO ISOLATED
SPEEDUP IS CLAIMED. THE NEXT BENCHMARK SHOULD TIME ONLY NATIVE MAKE
COMMANDS, EXCLUDING IMAGE CONSTRUCTION AND BOOT.

## NATIVE MAKE RULE INSERTION ONE-SEARCH FAST PATH (2026-10-09)

MAKE_GET_RULE PREVIOUSLY DID A BINARY SEARCH THROUGH MAKE_FIND_RULE
AND REPEATED MAKE_RULE_LOWER_BOUND FOR NEW RULES. IT NOW REUSES THE
INITIAL LOWER BOUND FOR BOTH DUPLICATE DETECTION AND INSERTION. THIS
REMOVES ONE O(LOG N) SEARCH PER NEW RULE WITHOUT ANY NEW MEMORY OR
CHANGES TO RULE ORDER. THE 2026-10-09 PARSER PROFILE HAD 2323 RULE
REQUESTS; THIS CHANGE AFFECTS NEW RULES ONLY. KCC/DAS BUILD AND ALL
FIVE NATIVE PDP-6 SIMH MAKE-INCLUDE REGRESSION PROBES PASS. NO
ISOLATED RUNTIME SPEEDUP OR PHASE CYCLE COUNT HAS BEEN MEASURED.

## NATIVE MAKE PARSER OPERATION PROFILE (2026-10-09)

AN INSTRUMENTED COPY OF COMMIT ACEB654, BUILT AND EXECUTED WITH THE
DAIMOS-TESTKIT NATIVE PDP-6 SIMH HARNESS, PASSED ALL FIVE PROBES.
FOR THE 1080-DEPENDENCY LARGE-GRAPH FIXTURE, PARSING PERFORMED:
2042 LITERAL RULE-PARSE CALLS, 0 VARIABLE RULE-PARSE CALLS,
0 MAKE_EXPAND CALLS, 2323 MAKE_GET_RULE CALLS, 1080 MAKE_ADD_DEP CALLS,
AND 32 CAPACITY-GROWTH EVENTS ACROSS THE GRAPH ARRAYS AND ARENA.
STRING ARENA USED WAS 35647 CHARACTERS. THESE ARE COUNTS, NOT
ELAPSED CYCLE ATTRIBUTION. THEY INDICATE THAT VARIABLE EXPANSION IS
NOT THE BOTTLENECK FOR THIS ALL-LITERAL GRAPH, AND ARRAY GROWTH IS
UNLIKELY TO DOMINATE. RULE RESOLUTION AND DEPENDENCY-RECORD INSERTION
ARE THE NEXT ALGORITHMS TO MEASURE, WITH ISOLATED TIMING BEFORE ANY
FURTHER CHANGES. NO PRODUCTION MAKE SOURCE CHANGE WAS JUSTIFIED BY
THIS PROFILE. THE TEMPORARY PROFILER LIVES OUTSIDE THE PRODUCTION
TREE. EXISTING WORKTREE EDITS WERE PRESERVED.

## NATIVE MAKE LITERAL DEPENDENCY PARSING (2026-10-09)

MAKE_PARSE_RULE NOW USES ITS INPUT LINE DIRECTLY WHEN THE RULE CONTAINS
NO DOLLAR SIGN. RULES CONTAINING VARIABLE REFERENCES RETAIN THE
ORIGINAL MAKE_EXPAND PATH. THIS AVOIDS REDUNDANT EXPANSION SCANNING
AND STRING COPYING ON LITERAL DEPENDENCY RULES. THE PDP-6 KCC/DAS
OBJECT COMPILES AND ASSEMBLES; ALL FIVE NATIVE SIMH MAKE-INCLUDE
TESTS PASS. THE FULL RUN TOOK APPROXIMATELY 155 S INCLUDING IMAGE
BUILD AND BOOT; AN ISOLATED A/B SPEEDUP HAS NOT BEEN MEASURED.
PREEXISTING WORKTREE EDITS TO TODO.MD AND MACH_USER_PDP6.S ARE NOT PART
OF THIS CHANGE.

## TEMPORARY INVALID-UUO DIAGNOSTIC (2026-10-09)

THE KCORE MACH_SYSCALL_INVALID EXIT PATH NOW SAVES LOW-CORE 040 (TRAP
INSTRUCTION) AND MACH_SYSCALL_SAVE (JSR RETURN WORD) TO TWO EXPORTED
DIAGNOSTIC WORDS AND PRINTS BOTH AS TWELVE-DIGIT OCTAL VALUES ON THE
PHYSICAL PDP-6 CTY, PREFIXED "UUO ", BEFORE RETURNING STATUS 126.
THE CONSOLE ROUTINE IS SELF-CONTAINED IN KCORE TO AVOID A DEPENDENCY
ON THE CTY MRES. THIS IS TEMPORARY DIAGNOSTIC CODE, TO REMOVE AFTER
IDENTIFYING THE FAULT. DAS AND KCORE LINK/CONVERT PASS. A FRESH BOOT
IMAGE DID NOT COMPLETE: MKD6FS POST-FORMAT D6FSCK REPORTED ALLOCATED
LEAKED BLOCKS 14235-14241; NO SIMH RUNTIME DIAGNOSTIC PASS IS CLAIMED.

## NATIVE MAKE RULE INSERTION / INTERNING A-B PROFILE (2026-10-09)

TWO SEPARATE INSTRUMENTED PDP-6 SIMH BOOT IMAGES FROM 1178F14 (BEFORE)
AND 78FBC43 (AFTER) COMPLETED THE SAME FIVE NATIVE MAKE TEST PROBES.
FOR THE 1080-LINK LARGE GRAPH, BOTH HAD 682 SORTED RULE-INDEX SHIFTS;
DEPENDENCY NAME INTERNING CUT STRING ARENA USE FROM 39647 TO 35647
CHARACTERS (4000, OR 10.1%). THE OPEN-ADDRESSED INTERN INDEX HAS ITS
OWN MEMORY COST AND NO PEAK-RAM MEASUREMENT WAS MADE. ELAPSED TIME FOR
THE COMPLETE BUILD/BOOT/HARNESS SEQUENCES WAS ABOUT 167 S BEFORE AND
162 S AFTER; THIS IS NOT AN ISOLATED MAKE COMMAND TIMING, SO NO
RELIABLE EXECUTION SPEEDUP CAN BE CLAIMED. ONLY 682 RULE SHIFTS WERE
COUNTED; REPLACING THE EXISTING SORTED RULE INDEX IS NOT JUSTIFIED
BY THIS WORKLOAD. RETAIN CURRENT ALGORITHM. NEXT PROFILE SHOULD TARGET
RULE PARSING AND STRING EXPANSION WITH ISOLATED PHASE CLOCKS.

## NATIVE MAKE PHASE PROFILE AND DEPENDENCY INTERNING (2026-10-09)

A FRESH NATIVE SIMH BUILD WITH TEMPORARY PARSE/BUILD MARKERS PASSED ALL
FIVE MAKE-INCLUDE PROBES. AN EXTERNAL WATCHER OBSERVED PARSE DONE AT
T+50.55 S AND BUILD DONE AT T+88.08 S, A 37.53 S TRAVERSAL INTERVAL.
THE WATCHER STARTED AFTER MAKE HAD BEGUN, SO 50.55 S IS ONLY A LOWER
BOUND ON PARSE DURATION, NOT AN ACCURATE FULL PHASE TIME.

THE LARGE-GRAPH FIXTURE HAS 1080 EXPLICIT DEPENDENCY LINKS BUT ONLY
280 DISTINCT NAMES: 5400 CHARACTERS INCLUDING TERMINATORS WOULD BE
STORED AS INDIVIDUAL COPIES, VERSUS 1400 UNIQUE NAME CHARACTERS.
MAKE NOW INTERNS DEPENDENCY NAMES IN A GROWING OPEN-ADDRESSED TABLE
OF ARENA OFFSETS (ONE WORD PER SLOT). IT FREES OLD INDEX TABLES ON
GROWTH AND PRESERVES DEPENDENCY LINK ORDER AND ALL GRAPH SEMANTICS.
THE TABLE ITSELF CONSUMES WORDS, SO THE NET RAM BENEFIT DEPENDS ON
NAME REPETITION. NATIVE KCC/DAS BUILD AND ALL FIVE SIMH MAKE-INCLUDE
PROBES PASS. PRECISE NEW-VERSUS-OLD PHASE TIMING AND PEAK-RAM
BENCHMARKS ARE STILL OPEN.

## NATIVE MAKE LARGE-GRAPH COUNTER PROFILE / PHONY METADATA (2026-10-09)

A TEMPORARY INSTRUMENTED MAKE, BUILT IN AN ISOLATED WORKTREE AND RUN
UNDER THE NATIVE SIMH TESTKIT, COMPLETED ALL FIVE PROBES. THE 280-
TARGET / 800+-LINK LARGE-GRAPH PROBE RECORDED 1081 BUILD INVOCATIONS,
1082 MAKE_STAT CALLS, 3404 RULE LOOKUPS AND 2555 STRING STORES.
THESE ARE CALL COUNTS, NOT CPU TIME OR ATTRIBUTED COSTS. MOST OF THE
GRAPH IS EXPLICITLY PHONY, SO A FOLLOW-UP FAST PATH SKIPS INITIAL
AND POST-RECIPE STAT FOR PHONY TARGETS WHILE PRESERVING ALWAYS-CHANGED
SEMANTICS. THE OPTIMIZED PDP-6 OBJECT COMPILES AND ASSEMBLES; ALL FIVE
NATIVE SIMH MAKE-INCLUDE PROBES PASS. TOTAL HARNESS ELAPSED TIME WAS
ABOUT 162 S (INCLUDING IMAGE BUILD AND BOOT), SO NO END-TO-END SPEEDUP
IS ESTABLISHED FOR THE FAST PATH. PARSING/ALLOCATION PHASE TIMING AND
ACTUAL FILE-DEPENDENCY INVALIDATION REMAIN TO BE MEASURED.

## NATIVE MAKE SIMH A/B BENCHMARK (2026-10-09)

A CONTROLLED NATIVE SIMH RUN COMPARED THE OLD MAKE.C FROM COMMIT
31AC5B3 WITH THE OPTIMIZED MAKE.C FROM 12749AD. BOTH USED THE SAME
CURRENT DAIMOS SOURCE TREE, HOST TOOLCHAIN AND INSTALLED KCC BOOTSTRAP;
ONLY MAKE.C WAS REVERTED IN A DETACHED BASELINE WORKTREE. EACH IMAGE
BOOTED A PDP-6 AND EXECUTED THE SAME MAKE TESTKIT PROBES.

THE FRAGMENT, INCLUDE, AND SUFFIX-SUBSTITUTION PROBES PASSED IN 1 S
EACH ON BOTH BUILDS (WHOLE-SECOND TIMING RESOLUTION). THE 280-TARGET,
800+-DEPENDENCY LARGE-GRAPH PROBE TIMED OUT AT THE 300 S LIMIT IN
THE BASELINE (307 S INCLUDING HARNESS TIMEOUT HANDLING); OPTIMIZED
MAKE PASSED IN 129 S. THE OPTIMIZED BUILD ALSO PASSED THE SORTED-INDEX
PROBE IN 2 S. LARGE-GRAPH TIME IS AT LEAST 2.33 TIMES FASTER BY THE
TIME LIMIT (NOT AN EXACT SPEEDUP: BASELINE DID NOT COMPLETE). MEASURED
TIMES INCLUDE SIMH AND HOST HARNESS OVERHEAD. NO-OP/INCREMENTAL REBUILD
AND SEPARATE MEMORY/STAT-COUNT BENCHMARKS HAVE NOT YET BEEN MEASURED.

## NATIVE MAKE SUFFIX-RULE INDEX (2026-10-09)

IMPLICIT-RULE SEARCH NOW ITERATES ONLY OVER THE SUFFIX RECIPES, USING
ONE INDEX WORD PER SUFFIX RULE IN SOURCE ORDER. ORDINARY TARGET RULES
ARE NO LONGER SCANNED FOR EACH IMPLICIT RESOLUTION. PDP-6 KCC/DAS
COMPILATION AND STRUCTURAL TEST PASS. SIMH RUNTIME AND TIMING TESTS
ARE STILL REQUIRED. COMPLETED-RULE TIMESTAMP CACHING IS DEFERRED:
ANOTHER RECIPE CAN CHANGE A PREVIOUSLY EXAMINED FILE, SO A CORRECT
CACHE MUST ADDRESS INVALIDATION WITHOUT HIDING RECIPE SIDE EFFECTS.

## NATIVE MAKE PACKED DEPENDENCY METADATA (2026-10-09)

THE FIRST DFS PASS NOW STORES EACH EXPLICIT DEPENDENCY'S NEWER STATUS
IN A PACKED 36-BIT BITMAP, ALLOCATED ONCE AFTER PARSING (ONE BIT PER
DEPENDENCY). IMPLICIT SOURCE STATUS USES A LOCAL FLAG. `$?` CONSTRUCTION
NO LONGER RE-STATS PREREQUISITES. PDP-6 KCC/DAS BUILD AND STRUCTURAL
TEST PASS; SIMH INCREMENTAL INVALIDATION AND PERFORMANCE MEASUREMENTS
REMAIN REQUIRED. THE USER'S EXISTING TODO.MD EDITS WERE NOT CHANGED.

## DAIMOS LIBC REALLOC FRAGMENTATION FIX (2026-10-09)

LIBC REALLOC NOW GROWS ALLOCATIONS IN PLACE USING AN ADJACENT ADDRESS-ORDERED
FREE BLOCK, OR EXTENDS THE PROCESS BREAK WHEN THE ALLOCATION (WITH OR
WITHOUT A FREE NEIGHBOR) REACHES THE HEAP END. IT SPLITS EXCESS SPACE
WHEN SHRINKING OR PARTIALLY CONSUMING A FREE BLOCK AND RETURNS SHRUNK
SPACE TO THE COALESCING FREE LIST. THE COPY-AND-FREE FALLBACK IS USED
ONLY WHEN NEITHER IN-PLACE STRATEGY SUCCEEDS; FAILED GROWTH LEAVES THE
ORIGINAL BLOCK VALID. NO ADDITIONAL ALLOCATOR METADATA IS REQUIRED.
LIBC BUILDS WITH /USR/LOCAL/BIN/KCC, AND THE DAIMOS-TESTKIT HOST MOCK
TEST COVERS ADJACENT GROWTH, TOP-OF-HEAP GROWTH, SHRINKING, COALESCING,
AND MOVING GROWTH. KIR'S 16384-TYPE FAILURE IS NOT YET RETESTED ON THE
NATIVE SYSTEM. THE NEW LIBC MUST BE INCORPORATED INTO THE KCC NATIVE
BOOTSTRAP AND BOOT IMAGE BEFORE REPEATING KCC COMPILATION.

## NATIVE MAKE NEWER-PREREQUISITE FAST PATH (2026-10-09)

MAKE NOW SKIPS THE SECOND DEPENDENCY-METADATA WALK FOR TARGETS WITH
NO RECIPE TO EXECUTE, UP-TO-DATE TARGETS, AND RECIPES NOT USING `$?`.
A CONSERVATIVE VARIABLE-VALUE FLAG PRESERVES INDIRECT `$?` EXPANSION.
THE NATIVE MAKE OBJECT COMPILES AND ASSEMBLES. RUNTIME TIMING AND FULL
INCREMENTAL INVALIDATION REGRESSIONS REMAIN PENDING; SOURCE STAT CACHING
AND SUFFIX-INDEXING ARE NOT YET IMPLEMENTED.

## KCC SOURCE RESTAGING READ-ONLY FILE FIX (2026-10-09)

THE KCC SOURCE STAGER PREVIOUSLY USED CP ON INSTALLED BOOTSTRAP INPUTS
WITH READ-ONLY MODE 0444. SECOND AND LATER IMAGE BUILDS FAILED WITH
PERMISSION DENIED WHEN REPLACING EXISTING BOOT/CRT0.DOBJ. IT NOW USES
INSTALL -M 0644 TO CREATE WRITABLE STAGED COPIES. RESTAGING TWICE IN THE
EXISTING BUILD DIRECTORY PASSES, AS DOES THE DAIMOS-TESTKIT HOST REGRESSION
THAT MAKES ALL FIVE STAGED BOOTSTRAP INPUTS READ-ONLY BEFORE RESTAGING.

## NATIVE MAKE SIXBIT INCLUDE SPELLING (2026-10-09)

NATIVE MAKE NOW RECOGNIZES OPTIONAL `-INCLUDE` IN ITS SIXBIT
UPPERCASE MAKEFILES. THE HOST-STAGED SOURCES MAY USE LOWERCASE
`include` AND `-include`, BUT THE NATIVE SIXBIT FILE MUST BE
UPPERCASED BEFORE S6TEXT CONVERSION. THE PDP-6 OBJECT BUILT WITH
THE INSTALLED KCC COMPILER. THE FOCUSED SIMH REGRESSION IS PENDING.

## INSTALLED KCC BOOTSTRAP LOCATION (2026-10-09)

THE HOST KCC AND ALL FIVE DAIMOS BOOTSTRAP DXRS WERE REBUILT FROM
THE KCC DEVSPACE CHECKOUT AND INSTALLED INTO $PDP10_PREFIX.
KCC INSTALL-NATIVE STORES THE BOOTSTRAP UNDER
$PDP10_PREFIX/LIB/KCC/BOOTSTRAP, INCLUDING THE NATIVE RUNTIME AND
LIBC OBJECTS USED BY BOOT SOURCE STAGING. SYSTEM/BOOT/PDP6/MAKEFILE
USES THAT INSTALLED PATH AS ITS DEFAULT, NOT KCC/BUILD-NATIVE.
HOST-ONLY BOOT MAKE DRY-RUN CHECKS PASSED. LONG NATIVE KCC SELF-BUILDS
REMAIN USER-OPERATED AND WERE NOT RUN.

## NATIVE MAKE INCLUDE SYNTAX EXTENSION (2026-10-09)

NATIVE MAKE ACCEPTS THE EXISTING `INCLUDE` SYNTAX AS WELL AS GNU-STYLE
`include` AND OPTIONAL `-include`. AN OPTIONAL INCLUDE IS SKIPPED ONLY
WHEN FILE STATUS CONFIRMS IT IS ABSENT; AN EXISTING FILE WITH A PARSE
ERROR STILL FAILS. THE MODIFIED PDP-6 MAKE OBJECT COMPILES AND ASSEMBLES.
DAIMOS-TESTKIT HAS NEW INCLUDE SYNTAX CASES, BUT SIMH RUNTIME VERIFICATION
IS BLOCKED BY THE MISSING KCC HOST BOOTSTRAP AND NATIVE PHASE DXRS IN
THIS DEVSPACE CHECKOUT. NO LONG NATIVE KCC RUN WAS STARTED.

## NATIVE MAKE RULE LOOKUP OPTIMIZATION (2026-10-09)

RULE LOOKUPS NOW USE A SORTED SINGLE-WORD-PER-RULE INDEX WITH BINARY
SEARCH. RULE OBJECTS RETAIN STABLE IDS FOR DEPENDENCY LINKS, AND THE
SORTED INDEX IS UPDATED ONLY WHEN NEW RULES ARE PARSED. THE PDP-6
OBJECT COMPILES, BUT NATIVE RUNTIME REGRESSION AND LARGE-GRAPH TIMING
ARE STILL REQUIRED. THE USER OPERATES LONG NATIVE KCC BUILDS.

## NATIVE MAKE DYNAMIC GRAPH STORAGE (2026-10-09)

NATIVE MAKE NOW ALLOCATES RULES, DEPENDENCIES, RECIPES, VARIABLES, AND ITS
STRING ARENA ON DEMAND. THE VARIABLE TABLE IS NAME-SORTED FOR BINARY LOOKUPS
WITHOUT A SEPARATE HASH INDEX. THE `$?` PREREQUISITE LIST GROWS ONLY WHEN
NEEDED, AND PHONY RULES SKIP IMPOSSIBLE IMPLICIT-RULE SEARCHES. MAKE NOW
LINKS THE REAL DAIMOS LIBC ALLOCATOR, STRING ROUTINES, AND MEMCPY. THE
DAIMOS-TESTKIT INCLUDE/SUBSTITUTION/CAPACITY REGRESSION PASSED WITH 280
RULES AND 190 VARIABLES, BEYOND THE PREVIOUS FIXED TABLE LIMITS. THIS
TEST IS SLOW UNDER SIMH; PERFORMANCE AND FULL KCC MAKEFILE FEATURES REMAIN
OPEN. LONG NATIVE KCC COMPILATIONS REMAIN USER-OPERATED ONLY.

## PORTABLE DAIMOS LIBC BUILD (2026-10-09)

THE USERLAND/LIBC MAKEFILE NOW USES EXPLICIT DEPENDENCIES AND SHARED
RECIPES COMPATIBLE WITH GNU MAKE AND BSD MAKE. KCC INVOKES IT VIA $(MAKE)
WITHOUT A SEPARATE GNU-ONLY SUBMAKE. HOST AND NATIVE BUILD OUTPUT DIRECTORY
SELECTION IS UNCHANGED. FULL NATIVE TARGET SELF-HOSTING REMAINS OPEN.

## KCC HOST BUILD DIRECTORY TRANSITION (2026-10-09)

KCC'S HOST BUILD ARTIFACTS NOW LIVE IN KCC/BUILD/ AND ITS NATIVE DAIMOS
BOOTSTRAP ARTIFACTS IN KCC/BUILD-NATIVE/ (WITHOUT -V1 SUFFIXES).
SYSTEM/BOOT/PDP6/MAKEFILE'S DEFAULT KCC_BOOT_BUILD NOW POINTS TO
BUILD-NATIVE/. GNU MAKE AND BSD BMAKE PRODUCE IDENTICAL FIVE NATIVE KCC
DXRS IN THE HOST-SIDE BOOTSTRAP. DAIMOS LIBC REMAINS GNU-MAKE-ONLY.
THIS BUILD-SYSTEM CHANGE DOES NOT VALIDATE FULL ON-TARGET SELF-HOSTING.

## NATIVE KCC D6FS RESIZE CORRECTIONS / KPARSE GATE (2026-10-09)

2026-10-09 KIR/PROCESS-EXIT INVESTIGATION: MANUALLY COMPILING CC.C
REPORTED `? KCC - COULD NOT WRITE KIR1 EXTERNAL DEFINITION`, FOLLOWED
BY A SIMH HALT RATHER THAN A RETURN TO DSH. THE KIR WRITER NOW TRACKS
RECURSIVE GRAPH-BUILD FAILURES AND REPORTS ITS FAILING OUTPUT STAGE,
RECORD INDEX, COUNTS, AND STREAM ERROR STATE WITHOUT HIGH-VOLUME TRACES
(KCC NATIVE BRANCH). IN PROC_EXIT_CURRENT, LAST-PROCESS HALT (OPERAND
ZERO) AND FAILED-RELEASE HALT (OPERAND ONE) ARE NOW DISTINCT TO SUPPORT
FUTURE SIMH DIAGNOSIS. THIS IS NOT YET A FIX FOR THE UNDERLYING HALT.
FRESH KPARSE AND BOOT IMAGE BUILDS SUCCEEDED. DAIMOS-TESTKIT PROCESS
HIERARCHY AND PROCESS EVENTS REGRESSIONS PASSED. REPRODUCTION HARNESS
ATTEMPTS DID NOT REACH KCC: ONE STALLED AT LOGIN, THE DCS RETRY FAILED
ROOT VALIDATION WITH `?RT`. DO NOT MARK NATIVE `MAKE ALL` AS PASSING.

2026-10-09 NATIVE COMPILER FOLLOW-UP: KCC CCEVAL.C CHKOVF USED
`JFCL 11`, WHICH TESTS PDP-6 ARITHMETIC OVERFLOW **AND PC-CHANGE**.
REPLACE WITH `JFCL 10`, TESTING ONLY ARITHMETIC OVERFLOW; THE NATIVE
SMALL-SOURCE TESTKIT REGRESSION `DAIMOS-NATIVE-KCC-OVERFLOW-FLAGS-
20261009-V1` PASSES THROUGH KCPP/KPARSE/KGEN/KOPT WITH NO FALSE
OVERFLOW DIAGNOSTICS. THE FULL CC.C KPARSE REPLAY ALSO ELIMINATED THE
PREVIOUS REPEATED FALSE CONSTANT-EXPRESSION OVERFLOW ERRORS.

CRITICAL HOST TOOLCHAIN PROVENANCE: `/USR/LOCAL/BIN/KCC` WAS AN
OLDER OCT-7 EXECUTABLE, WHILE BUILDING `~/GIT/KCC/KCC` FROM CURRENT
SOURCES PRODUCES CORRECT OPTIMIZED PDP-6 LOOPS. THE TESTKIT TARGET
`DAIMOS-KCC-POINTER-SCAN-PDP6-20261009-V1` FAILS WITH THE INSTALLED
COMPILER (RESULT 1), PASSES WITH THE NEW HOST COMPILER (RESULT 0).
THE STALE COMPILER DROPPED THE LOOP INDEX INCREMENT FROM KIR'S
`FINDPTR`, CORRUPTING SYMBOL IDS AND THE PREPROCESSOR/COMPILER PHASES.
ALL NATIVE KCC PHASE DXRS AND DRIVER WERE REBUILT USING THE CURRENT
`~/GIT/KCC/KCC`, NOT THE STALE INSTALLED EXECUTABLE. THE FRESH IMAGE
PASSES FSCK AND KPARSE BUILDS LARGE GRAPHS PAST THE OLD FAILURE AT
CC.C:1015. FULL NATIVE `MAKE ALL` IS NOT YET VALIDATED; THE KIR
GRAPH-WRITER IS NOW CPU-BOUND ON A LARGE SYMBOL GRAPH.

THE EXPERIMENTAL KIR POINTER RANGE NEGATIVE FILTER IS ONLY A TRIAL:
IT ADDS EIGHT BOUNDS WORDS TO A GRAPH/MODULE AND DOES NOT ALTER THE
IDENTITY/ID MATCH CRITERION. KEEP IT ONLY IF A CONTROLLED BENCHMARK
DEMONSTRATES SUFFICIENT SPEEDUP TO JUSTIFY INCREASED USER RAM.

THE REPRODUCIBLE PREPROCESSOR STALL HAD TWO INDEPENDENT D6FS ASSEMBLER
ROOT CAUSES, BOTH FIXED IN FS/D6FS_PROVIDER_MUTATION.S:

1. A PUSHJ TO D6FS_RESIZE_DECODE_EXTENT IMMEDIATELY PRECEDED THE CALLEE
   LABEL. ITS POPJ RE-ENTERED THE SAME HELPER AND CONSUMED A DATA WORD
   (000000000001) AS A RETURN ADDRESS. THE HELPER WAS MOVED AFTER THE
   RESIZE ROUTINE'S FINAL POPJ, AT NO MACHINE-WORD COST.
2. THREE READER.FIELD ACCESSES USED 6(1) (FCB_COUNT) RATHER THAN 3(1)
   (TOTAL_BLOCKS): INITIAL BLOCK LIMIT, ADJACENT EXTENT CHECK, AND
   ALLOCATION-CURSOR WRAP. THEY FORCED PREMATURE EXTENT EXHAUSTION IN
   THE KCPP TOKEN STREAM. EACH WAS CORRECTED WITHOUT CODE-SIZE GROWTH.

THE D6FS MRES DXR REMAINS IMAGE 004043, RELOCS 000072, TOTAL 2144
WORDS. ALL FOUR-DISK/FOUR-DRUM IMAGES BUILT AND PASSED D6FSCK. NATIVE
KCPP NOW PREPROCESSES CC.C THROUGH 220672 INPUT CHARACTERS AND RETURNS
STATUS ZERO. DAIMOS-TESTKIT PASSED INCREMENTAL D6FS APPENDS, A 3000-WORD
LARGE APPEND, THE NEW 12288-WORD/96-BLOCK EXTENT GROWTH/READBACK TEST,
AND A SOURCE CALL-FLOW REGRESSION.

THE NEXT NATIVE SELF-HOSTING GATE IS KPARSE. IT EMITS REPEATED
"CONSTANT EXPRESSION OVERFLOW" DIAGNOSTICS FOR ORDINARY ARRAY-SIZE AND
OTHER EXPRESSIONS (CCSYM.H:41, CC.H:125, CC.C:319, ETC.) AND DID NOT
COMPLETE WITHIN THE FIRST 260-SECOND TEST. ITS EVALBINOP/CHKOVF INLINE
JFCL CHECK IS BEING EXAMINED. DO NOT CLAIM MAKE ALL, INSTALL, OR THE
FULL SELF-HOSTING COMPILER PASSED.

SIMH HISTORY ALSO IDENTIFIED A SEPARATE KPARSE PERFORMANCE HOTSPOT:
USER PC 134041..134057 (APR BASE 350000) MAPS INTO THE NATIVE
CCKIRWRITE-V1.DOBJ ROUTINE FINDPTR(), WHICH DOES A LINEAR SCAN OVER
POINTER ARRAYS. ONE OBSERVED SEARCH HAD N=016777 (7679 DECIMAL), AND
ITS INDEX WAS ADVANCING. THIS IS QUADRATIC GRAPH-INDEX WORK, NOT AN
INFINITE LOOP AT THAT PC. REPLACING IT NEEDS AN EXPLICIT USER-RAM BUDGET;
A LARGE GENERIC HASH TABLE WOULD BE COUNTER TO THE LOW-MEMORY DESIGN.
ANOTHER PAUSED RUN SHOWED A SLEEPING PROCESS AT EVENT WORD 050636 WITH
NO RUNNABLE SLOT AND DSK STORAGE_STATE ZERO; THE CONNECTION TO THE
OVERFLOW DIAGNOSTICS OR KIR PERFORMANCE IS NOT ESTABLISHED.

## HISTORICAL DIAGNOSTICS SUPERSEDED BY D6FS FIX (2026-10-08)

EARLIER INSTRUCTION TRACES WERE MISATTRIBUTED TO KCPP USER TEXT.
THE FAULTING PC WAS ACTUALLY IN THE RELOCATED D6FS MRES; THE
BOGUS RETURN TO ADDRESS 1 FOLLOWED A PUSHJ WHOSE CALLEE LABEL
IMMEDIATELY FOLLOWED IT. INVALID-UUO EXIT CLEANUP THEN WAITED ON
THE SAME PROCESS'S UNRELEASED FILESYSTEM PROVIDER LOCK. THAT
LOCK WAIT WAS SECONDARY TO THE ASSEMBLER CONTROL-FLOW DEFECT.
THE RESIDENT KERNEL, APR RELOCATION, AND KCPP USER IMAGE WERE
NOT THE CAUSE. SEE THE COMPLETED FIX DETAILS ABOVE.

## NATIVE MAKE AND INSTALL (2026-10-08)

THE NATIVE INSTALL PROGRAM IS AVAILABLE UNDER /SYSTEM/EXEC/INSTALL. IT
COPIES UNMODIFIED 36-BIT WORD STREAMS, SUPPORTS -M OCTAL MODE, AND CREATES
DIRECTORIES WITH -D. MAKE SUPPORTS -Q UPDATE QUERY AND AN EXPLICIT '!' DIRECT
RUN V2 RECIPE FORM FOR COMMANDS LONGER THAN THE 102-CHARACTER DSH -C ABI.
DIRECT COMMANDS REQUIRE AN ABSOLUTE EXECUTABLE AND SPACE-SEPARATED ARGUMENTS;
THE ORDINARY SHELL RECIPE FORM IS UNCHANGED. A DAIMOS-TESTKIT SIMULATOR
REGRESSION PASSES INSTALL DIRECTORY/COPY/COMPARE, MAKE -Q IN BOTH STATES,
AND A DIRECT RECIPE LONGER THAN 102 CHARACTERS.
## KCC SOURCE LAYOUT AND BUILD STATUS (2026-10-08)

THE CANONICAL KCC SOURCE INSTALLATION IS /OPTION/SOURCE/KCC, NOT
/OPTION/KCC/SOURCE. THE BOOT IMAGE INSTALLS CONVERTED SOURCE FILES AND THE
NATIVE MAKEFILE. CHANGES TO BOOT-IMAGE PACKAGING RULES INVALIDATE DISK
IMAGE TARGETS AS WELL AS CHANGES TO THE STAGED SOURCE FILES.

THE BOOT IMAGE NOW STAGES ALL FIVE HOST-BUILT NATIVE KCC BOOTSTRAP
EXECUTABLES, PLUS THE NATIVE KCC CRT0, BOOTSTRAP, SYSCALL, AND LIBC
OBJECTS/ARCHIVE UNDER /OPTION/SOURCE/KCC/BOOT. THE ABI HEADER SNAPSHOT
IS UNDER /OPTION/SOURCE/KCC/ABI. KCC'S AUTHORITATIVE MAKEFILE AND
DAIMOS.MK ARE STAGED UNDER
/OPTION/SOURCE/KCC, NOT A GENERATED DEPENDENCY GRAPH. KCC'S SOURCE-TREE
MAKEFILE OWNS SHARED MODULE INVENTORIES; UNIX.MK HANDLES HOST COMPILATION
AND THE CROSS-BOOTSTRAP, WHILE DAIMOS.MK HANDLES NATIVE BUILD RULES.
THE STAGER UPPERCASES THE SHARED MAKEFILE, SELECTS DAIMOS AS THE PLATFORM,
TRANSLATES PORTABLE RECIPE PREFIXES INTO NATIVE DIRECT-RUN PREFIXES,
AND ENCODES BOTH AS S6REC. THE OLD PYTHON MAKEFILE GENERATOR WAS DELETED.
`MAKE ALL` BUILDS INCREMENTALLY INTO THE LOCAL `B` SUBDIRECTORY; `MAKE INSTALL` COPIES THE
NEW DXRS INTO /OPTION/BASE/EXEC AND /OPTION/BASE/LIBEXEC/KCC.

THE PRE-SPLIT HOST IMAGE BUILD PASSED D6FSCK AND THE TARGET-SIDE
MAKE -Q GRAPH PREFLIGHT PASSED ON THE PREVIOUS GENERATED GRAPH. NEITHER RESULT PROVES SUCCESSFUL NATIVE MAKE ALL OR
LINK/INSTALL. THE USER WILL RUN THOSE TARGET ACCEPTANCE TESTS. THE
BOOTSTRAP ARTIFACTS ARE EXPLICIT PREREQUISITES; A MISSING PREBUILT KCC
PHASE OR LIBC OBJECT FAILS IMAGE PACKAGING INSTEAD OF SILENTLY OMITTING IT.

THE ON-TARGET DAIMOS.MK ALSO SUPPORTS MAKE CLEAN. CLEAN REMOVES ONLY
GENERATED B/*.S, B/*.DOBJ, B/*.DARC, AND B/*.DXR ARTIFACTS; IT DOES
NOT TOUCH ORIGINAL SOURCES, BOOTSTRAP PHASES, OR STAGED ABI/LIBC INPUTS.
THE EARLIER NATIVE SIMULATOR PREFLIGHT PASSED MAKE CLEAN, MAKE -Q, AND
MAKE -Q ALL USING THE OLD GRAPH. THE SHARED-FRAGMENT GRAPH STILL NEEDS
A NEW NATIVE RUNTIME ACCEPTANCE RUN.
THE ROOT IMAGE INCLUDES A WRITABLE /SCRATCH MOUNT POINT FOR THE NATIVE
COMPILER'S INTERMEDIATE FILES.
## NATIVE KCC MAKE PARSER/QUERY ACCEPTANCE (2026-10-09)

A FRESH SIMH BOOT IMAGE WITH THE SPLIT KCC MAKEFILES INITIALLY FAILED
NATIVE MAKE PARSING BECAUSE HOST-ORIENTED REPEATED `+=` INVENTORY
ASSIGNMENTS EXCEEDED NATIVE MAKE'S 256-CHARACTER VARIABLE BUFFER.
KCC NOW PLACES THOSE CROSS-BUILD INVENTORIES IN UNIX.MK, WITH ONLY THE
PLATFORM SELECTOR IN THE SHARED ROOT MAKEFILE AND EXPLICIT ON-TARGET
DEPENDENCIES IN DAIMOS.MK. THE CORRECTED SIMH RUN PASSED `MAKE -Q`
(DEFAULT AND ALL), `MAKE -N` FOR B/CC-CPP.S, AND `MAKE CLEAN`.
THE SINGLE NATIVE B/CC-CPP.S COMPILATION REACHED KCC AND PRINTED
KCC000006, BUT THE TEST TIMED OUT; DO NOT CLAIM OBJECT COMPILATION,
FULL MAKE ALL, INSTALL, OR INCREMENTAL REBUILD PASSES. A NEW TESTKIT
V2 PRE-FLIGHT SEPARATES GRAPH CHECKS FROM LONG COMPILER ACCEPTANCE.

## KCC SHARED MAKEFILE SPLIT (2026-10-09)

HOST AND CROSS-BUILD DRY RUNS PASS WITH KCC/MAKEFILE AND UNIX.MK. THE
ON-TARGET DAIMOS.MK HAS EXPLICIT OBJECT, PHASE-ARCHIVE, LINK AND LOCAL-HEADER
PREREQUISITES. HOST STAGING OF MAKEFILE, DAIMOS.MK, SOURCES, ABI AND THE
INSTALLED BOOTSTRAP LIBRARY NOW PASSES. THE STAGER'S OLD ASSUMPTION THAT
BOOTSTRAP RUNTIME OBJECTS HAVE UNSUFFIXED FILENAMES WAS FIXED TO READ
THE INSTALLED -V1.DOBJ FILES. THE DAIMOS-TESTKIT HOST PREFLIGHT CHECKS
THAT ALL KCC PHASE MODULES HAVE OBJECT RULES AND HEADER DEPENDENCIES.
A NEW NATIVE SIMH MAKE ALL / INSTALL ACCEPTANCE HAS NOT BEEN RUN; DO NOT
TREAT KCC SELF-HOSTING AS QUALIFIED YET.

## D6FS HOST CHECKER OWNER WIDTH (2026-10-08)

THE HOST D6FSCK BLOCK-OWNERSHIP MAP USES 32-BIT OWNERS (FCB INDEX PLUS TWO),
NOT EIGHT-BIT BYTES. THE OLD EIGHT-BIT MAP WRAPPED FCB 254'S OWNER TO ZERO,
MISCLASSIFYING VALID FILE BLOCKS AS ALLOCATED LEAKS IN LARGER BOOT IMAGES.
THIS AFFECTED THE KCC-SOURCE-EQUIPPED BOOT IMAGE AND WAS A CHECKER BUG,
NOT LOST FILE DATA. DAIMOS-TESTKIT NOW COVERS 260 FILES AND THE OWNER-WIDTH
BOUNDARY. THE EXISTING FAILED IMAGE PASSES THE CORRECTED CHECKER WITHOUT
REPAIR.
# DAIMOS PROJECT STATE

DATE: 2026-10-07

THIS FILE IS THE COMPACT AUTHORITATIVE SUMMARY OF CURRENT DAIMOS ARCHITECTURE,
IMPLEMENTATION STATE, HARD CONSTRAINTS, AND DEVELOPMENT ORDER.  DETAILED DESIGN
BELONGS IN PDP10-DOC, OPEN WORK IN TODO.MD, AND COMPLETED IMPLEMENTATION HISTORY
IN GIT.

## 1. MACHINE AND MEMORY BASELINE

THE INITIAL/MINIMUM MACHINE IS A PDP-6 WITH 32K WORDS OF CORE.  PDP-10
BACKENDS MAY ADD MACHINE-SPECIFIC FACILITIES WITHOUT RAISING THAT BASELINE.
THE GENERIC VM/MM BOUNDARY MUST REMAIN REPLACEABLE BY A MACHINE-SPECIFIC PAGER.

THE HARD PERMANENT KERNEL LIMIT IS ADDRESSING BELOW 040000.  KCC IS USED FOR
THE OFFICIAL RESIDENT-SIZE MEASUREMENT.  SHRINKAGE SWEEPS ARE REQUIRED ONLY
WHEN THE CONFIGURED HARD LIMIT IS ACTUALLY EXCEEDED.

KCORE IS THE FIXED RESIDENT KERNEL CORE: THE COMMON SYSCALL, PROCESS, VM, VFS,
INTERRUPT-BRIDGE, AND LOW-LEVEL SUPPORT CODE THAT REMAINS PRESENT AFTER BOOT.
KINIT IS THE TRANSIENT BOOT/INITIALIZATION IMAGE.  IT DISCOVERS HARDWARE AND
THE ROOT FILESYSTEM, RUNS MODULE INITIALIZATION, INSTALLS THE RESIDENT PACKAGES
NEEDED BY THE ACTIVE CONFIGURATION, STARTS /SYSTEM/INIT, AND IS THEN RECLAIMED.
MRES MEANS A MODULE'S RESIDENT PACKAGE: THE CODE AND SMALL STATE THAT A MODULE
MUST LEAVE BEHIND AFTER ITS KINIT/MINIT INITIALIZATION HAS FINISHED.  A MODULE
CAN HAVE A MINIT ROUTINE WITHOUT HAVING A SEPARATE MRES PACKAGE.

LAST CLEAN FULL KCC BASELINE BEFORE THE WORD-I/O REDESIGN:

    KCORE               9728 WORDS
    CTY MRES              58 WORDS    CONSOLE TERMINAL I/O
    CLK MRES              22 WORDS    APR LINE-CLOCK SERVICE
    PTR MRES              39 WORDS    PAPER-TAPE READER
    PTP MRES              25 WORDS    PAPER-TAPE PUNCH
    CR MRES               42 WORDS    CARD READER
    CP MRES               44 WORDS    CARD PUNCH
    DCS MRES              63 WORDS    DATA-COMMUNICATION TERMINALS
    GE MRES               97 WORDS    GE TERMINAL INTERFACE
    DPY MRES              14 WORDS    DISPLAY OUTPUT
    TTY MRES              22 WORDS    COMMON TERMINAL/LINE-DISCIPLINE BRIDGE
    WCNSLS MRES            8 WORDS    WCNSLS DISPLAY INTERFACE
    OCNSLS MRES            3 WORDS    OCNSLS DISPLAY INTERFACE
    DSK MRES             212 WORDS    TYPE-270 DISK SERVICE
    TAPE MRES            330 WORDS    SHARED DTC/MTC TAPE SERVICE
    SLV MRES               2 WORDS    SLAVE INTERFACE
    DRM MRES             112 WORDS    DRM236 DRUM SERVICE
    MEMFS MRES           636 WORDS    MEMORY FILESYSTEM
    DTFS/TSFS MRES      1848 WORDS    NATIVE DECTAPE FILESYSTEMS; ITS/TENEX DISABLED
    BLOCKSET MRES          0 WORDS    SINGLE-ROOT BUILD; 38-WORD PACKAGE WHEN
                                      MULTI-MEMBER BLOCK MAPPING IS INSTALLED
    LOGSTORE MRES         101 WORDS    RUNTIME LOG RING SERVICE
    D6FS MRES           2236 WORDS    NATIVE WRITABLE FILESYSTEM

ROOT-SELECTION AND MONITORFS-DEVICE INITIALIZATION HAVE MINIT ENTRIES WITHOUT
STANDALONE MRES PACKAGES.  LPT NOW HAS AN OPTIONAL MRES BECAUSE ITS S6REC
WORD-OUTPUT ADAPTER IS LARGE ENOUGH THAT ABSENT-PRINTER SYSTEMS SHOULD NOT PAY
FOR IT IN KCORE.  BADMAP IS CONDITIONAL: CLEAN MEDIA NEEDS NO BADMAP MRES;
SUPPORT CAN ALSO BE REMOVED AT COMPILE TIME WITH KINIT_BADMAP=0.

THE CURRENT SUPPORTED BUILD ENABLES NATIVE DTFS/TSFS ONLY.  TENEX/ITS FOREIGN
DTFS PERSONALITY SOURCE REMAINS IN THE TREE, BUT THE PRODUCTION PDP-6 BUILD
SETS DTFS_ENABLE_TENEX=0, DTFS_ENABLE_ITS=0, AND DTFS_ENABLE_FOREIGN=0.  THE
AUTHORITATIVE CURRENT RESIDENT-SIZE MEASUREMENT IS RECORDED LATER IN THIS FILE
AND IN PDP10-DOC STATUS/RESIDENT-SIZE.MD.  THE FINAL PRE-DSH KERNEL TASK
REMAINS ONE RESIDENT-SIZE/CORRECTNESS SWEEP TO RESTORE THE REQUIRED 040000
LIMIT, FOLLOWED BY THE COMPLETE REGRESSION/BOOT/LOGIN CHECK.



DAIMOS 0.9/1.0 REGULAR FILES ARE WORD STREAMS.  VFS AND FILESYSTEM PROVIDERS
TRACK ONLY SIZE_WORDS; THE FORMER GENERIC 9-BIT SIZE_CHARS/OFF_CHARS ABI,
REGULAR-FILE READCHAR/WRITECHAR PATHS, WRITE_CHARS UUO, AND D6FS CHARACTER-TAIL
METADATA HAVE BEEN REMOVED.  NATIVE TEXT IS APPLICATION-LEVEL S6REC/SIXBIT.
BUILD TOOLS CONVERT HOST CONFIGURATION TEXT WITH S6TEXT AND IMPORT THE RESULT
AS ORDINARY WORD FILES; D6FS/TSFS HAVE NO TEXT FILE TYPE.  SYMLINKS REMAIN A
DISTINCT VFS OBJECT TYPE AND STORE A SELF-CONTAINED COUNTED PACKED-SIXBIT
TARGET RECORD.

MONITORFS PROCESS/DOMAIN TEXT LEAVES FOLLOW THE SAME WORD-TEXT CONTRACT: THEY
ARE S6REC READ_WORDS STREAMS, WITH NO PROVIDER-3 READCHAR ABI.  LIVE
NAME/CMDLINE/ENVIRONMENT DATA IS DERIVED FROM THE PROCESS IMAGE WITHOUT
PER-PROCESS KERNEL COPIES; CMDLINE, ENVIRONMENT, AND DOMAIN PIDS USE ONE S6REC
RECORD PER LOGICAL ENTRY.  THE OLD CHARACTER-AT-OFFSET SIXBIT/DECIMAL HELPERS
ARE REMOVED; KFMT RETURNS PACKED SIXBIT DECIMAL DIRECTLY.

PIPES AND FILESYSTEM FIFOS ARE NOW ONE OPAQUE 36-BIT WORD-FIFO FACILITY.  EACH
LIVE OBJECT IS 14 WORDS, INCLUDING AN EIGHT-WORD PAYLOAD RING, AND USES THE
EXISTING READ_WORDS/WRITE_WORDS ABI WITH LEGAL SHORT POSITIVE TRANSFERS.  ON
THE FIXED PRE-REDESIGN BASE THIS CHANGED KCORE FROM 9728 TO 9722 WORDS, SIX
WORDS SMALLER THAN THE OLD PACKED-CHARACTER PIPE.  TEXT/BYTE/MEDIA CONVERSION
IS OUTSIDE IPC.  THE COMMON FIXED-WIDTH TOKEN CONTRACT DEFINES SPECIALIZED
6/7/8/12-BIT MSB-FIRST PACKING; S6REC, TTY, PAPER-TAPE, AND CARD LAYERS OWN
THEIR OWN FRAMING.

TTY WORD I/O CONVERTS COOKED S6REC AT THE TTY MRES BOUNDARY WHILE RAW
READCHAR/WRITECHAR REMAINS CHARACTER-ORIENTED.  COOKED READ_WORDS REQUIRES AN
ATTACHED CANONICAL TTY; UNOWNED CTY BOOTSTRAP INPUT USES READCHAR AND IS NOT
SYNTHESIZED INTO ONE-CHARACTER S6REC RECORDS.  TTY AND LPT SHARE ONE MEASURED
20-WORD KCORE S6REC TEXT-FRAME VALIDATOR; THIS IS SMALLER IN THE COMPLETE
PERMANENT IMAGE THAN DUPLICATING THE CHECK IN BOTH MRES PACKAGES.  PTR/PTP USE FOUR 8-BIT TOKENS
PER WORD WITH THE LOW NIBBLE COUNTING A FINAL 1..3-BYTE PARTIAL WORD.  CR/CP
USE 27 WORDTOKEN12 WORDS FOR ONE FIXED 80-COLUMN CARD.  LPT ACCEPTS S6REC WORD
OUTPUT AND PACKS UP TO FIVE SEVEN-BIT CHARACTERS PER LP10 DATAO.  THESE DEVICE
ADAPTERS ARE SPECIALIZED FOR SPEED AND LIVE WITH OPTIONAL MRES PACKAGES WHERE
POSSIBLE; THEY DO NOT ADD PER-STREAM RESIDENT BUFFERS.

CURRENT WORD-I/O MRES MEASUREMENTS AFTER THE LEGACY/API PURGE ARE: PTR 57
WORDS, PTP 62, CR 68, CP 62, TTY 151, AND LPT 64 WORDS OF INSTALLED RESIDENT
PACKAGE TEXT/STATE.  THE OLD DIRECT UUO-044 GETCHAR ALIAS AND THE TTY
NO-CONTROLLING-TTY S6REC RECONSTRUCTION PATH ARE REMOVED; READCHAR/WRITECHAR
REMAIN THE CHARACTER ABI.

THE 2026-10-01 CORE AUDIT REDUCED THE RESIDENT PDP-6 PI TABLE FROM 13 TO THE
PROVEN EIGHT SIMULTANEOUS HANDLERS REQUIRED BY THE COMPLETE CURRENT DEVICE SET,
RECOVERING FIVE KCORE BSS WORDS.  PDP-6 SKIP-FORM SIMPLIFICATION OF THE SEVEN
PI USER-STACK SWITCHES AND PI RESTORE RECOVERED ANOTHER EIGHT KCORE TEXT WORDS
WHILE REMOVING EXECUTED INSTRUCTIONS; THE PI OBJECT IS NOW 105 TEXT + 31 BSS
WORDS.  COMPRESSED EXEC'S VFS D6LZ REFILL WINDOW IS 64 TRANSIENT STACK WORDS,
UP FROM EIGHT.  CURRENT LOGIN/DSH PAYLOADS REQUIRE ABOUT ONE EIGHTH AS MANY VFS
REFILLS; THE PROCESS-KERNEL-STACK WATERMARK REMAINS 0307 OCTAL WORDS IN THE
ACCEPTANCE WORKLOAD.  THE SAME D6LZ RUNTIME ADAPTER NOW ALSO HAS A DIRECT-
BUFFER ENTRY FOR TSFS: TSFS CONTINUES TO AVOID VFS REFILLS, BUT NO LONGER
DUPLICATES DECODER AC SAVE/RESTORE, STATE INITIALIZATION, OR EXACT-CONSUMPTION
CHECKS.  THE ADAPTER GROWS FROM 42 TO 49 KCORE WORDS WHILE TSFS_RUNTIME SHRINKS
FROM 503 TO 489 WORDS, FOR A NET SEVEN-WORD PERMANENT SAVING.  THE CHANGE ALSO
CORRECTS THE TSFS DECODE DESTINATION TO USE THE ACTUAL MM_ALLOC RESULT STACK
SLOT.  BOTH FRONTENDS STILL FEED THE SAME FIXED 40-WORD LOW-CORE TOKEN ENGINE.

MONITORFS DEVICE STATS ARE INTENTIONALLY COMPACT IN 0.9/1.0: EVERY DEVICE
REPORTS ONLY COMPLETED READ REQUESTS, COMPLETED WRITE REQUESTS, AND ERRORS.
THE UNUSED D6SET LOG STATS LEAF AND SEPARATE MTC-WORD/D6SET-BLOCK LIFETIME
VOLUME COUNTERS WERE REMOVED; NONE PARTICIPATED IN I/O, SCHEDULING, RECOVERY,
OR USERSPACE POLICY.

WCNSLS DEVICE 0420 IS NOW EXPOSED TO PRIVILEGED USERSPACE AS RAW
`/DEV/WCNSLS` BULK WORD I/O AND REMAINS COMPLETELY OUTSIDE THE TTY LAYER.
MONITORFS DEVICE MODE 0600 PROVIDES THE OPEN-TIME ROOT PRIVILEGE GATE.  A
NONZERO READ_WORDS REQUEST RETURNS ONE RAW DATAI 0420 SAMPLE.  WRITE_WORDS
EXECUTES A BATCH IN WHICH BIT 35 TAGS CONO AND AN UNTAGGED WORD IS SENT BY
DATAO; PICTURE STORAGE, COLOR SEQUENCING, REFRESH TIMING, AND ANIMATION ALL
BELONG TO USERSPACE.  THE FILE DESCRIPTOR IS THE CAPABILITY AFTER OPEN, SO A
PRIVILEGED RT CLIENT DOES NOT PAY A CREDENTIAL CHECK PER POINT.  THE RETAINED
WCNSLS PACKAGE IS 22 WORDS WITH NO BSS, PI HANDLER, FRAMEBUFFER, DISPLAY LIST,
OR REFRESH SERVICE.  KINIT CONTINUES TO USE ITS DISPOSABLE RAW PRIMITIVES FOR
PROBE AND THE BOOT BANNER.  DAIMOS-TESTKIT TARGET `test-daimos-wcnsls-raw`
PASSES AGAINST SIMH WITH EXACT DATAI/CONO/DATAO TRACE VALIDATION.

THE KCORE LINK RUNS P10FOLD OVER ALL HANDWRITTEN KCORE ASSEMBLER BEFORE
DLINK.  SIX RELOCATION-AWARE ZERO-EXECUTION-COST DONOR FOLDS REMOVE 37 KCORE
WORDS.  THE COMPLETE 2026-09-30 P10SUPER HANDWRITTEN SOURCE SWEEP ALSO APPLIED
14 PROVEN ONE-WORD PEEPHOLES, REDUCING KCORE FROM 9655 TO 9641 WITHOUT
CHANGING THAT FOLD PLAN.  THE FOLLOW-UP SCAN HAS TEN SCHEDULING-EQUIVALENT
PAIRS BUT ZERO PROVEN FOLD VALUE, SO GLOBAL-PLAN SELECTS NONE.  NO FURTHER
PEEPHOLE, ZERO-FORM, OR AC-RENAME NORMALIZATION REMAINS.  KCC-GENERATED
ASSEMBLER REMAINS DELIBERATELY EXCLUDED UNTIL SELF-HOSTING/COMPILER
CANONICALIZATION WORK.

MRES FOLDING IS PACKAGE-LOCAL.  THE DTFS PACKAGE RUNS P10FOLD ONLY OVER ITS
HANDWRITTEN DTFS_RUNTIME/TSFS_RUNTIME INPUTS AND REMOVES ONE 5-WORD ZERO-COST
DUPLICATE BEFORE MRES PACKAGING.  THE SUPPORTED PDP-6 BUILD NOW DISABLES
TENEX/ITS DTFS COMPATIBILITY AND RETAINS ONLY NATIVE DTFS PLUS TSFS; THE
RESULTING PACKAGE IS 1848 WORDS.  FOREIGN-PERSONALITY SOURCE REMAINS DORMANT
REFERENCE CODE AND IS NOT PART OF THE PERMANENT-SIZE BASELINE.  OTHER HANDWRITTEN MRES PACKAGE
INPUTS CURRENTLY HAVE NO ZERO-COST FOLD AT THE FOUR-WORD THRESHOLD.

THE 32K LOWMEM PROFILE IS NOT A DIFFERENT DAIMOS VARIANT.  IT BUILDS THE SAME
SOURCE TREE FOR A PDP-6 LIMITED TO 32K WORDS, USES THE SMALLER TRANSIENT
KINIT_FULL=0 INITIALIZATION PROFILE, DEFAULTS KINIT_BADMAP=0, AND THEN BOOTS
THROUGH THE SAME NORMAL INIT -> LOGIN -> DSH PATH.

## 2. BOOT AND ROOT POLICY

STAGE1 SELECTS THE ROOT CONTROLLER CLASS.  KINIT DISCOVERS AND VALIDATES THE
ROOT SET AND FILESYSTEM, INSTALLS ONLY THE REQUIRED PERMANENT PACKAGES, LOADS
`/SYSTEM/INIT`, AND RECLAIMS BOOT-ONLY STATE.

SUPPORTED V0.9 ROOT PATHS INCLUDE DSK/D6FS, DRM/D6FS, AND DTC/TSFS.  AUTO ROOT
POLICY PREFERS DSK, THEN DECTAPE, THEN DRM.

TSFS V1.1 SETS MAY USE ALL EIGHT DTC UNITS AS LOGICAL MEMBERS 0..7.  BOTH
KINIT ROOT DISCOVERY AND THE TRANSIENT USERSPACE MOUNT SCANNER ACCEPT EIGHT
MEMBERS.  THE RUNTIME HANDOFF REMAINS COMPACT: ITS THREE-BIT-PER-MEMBER MAP
USES 24 OF THE AVAILABLE 36 BITS FOR AN EIGHT-MEMBER SET.  THE SEPARATE
GENERIC BLOCKSET DESCRIPTOR LIMIT REMAINS SEVEN MEMBERS AND IS NOT A TSFS
FORMAT LIMIT.

HARDWARE/ROOT DISCOVERY STAYS IN STAGE1/KINIT.  ADMINISTRATIVE POLICY SUCH AS
ADDITIONAL MOUNTS AND SWAP/LOGSTORE SELECTION BELONGS IN INIT AND `/CONFIG`.
INIT READS `/CONFIG/FSTAB`; MEMFS SCRATCH STORAGE IS CREATED ONLY WHEN A
MEMFS ENTRY REQUESTS IT AND IS NO LONGER INSTANTIATED BY KINIT.

USERSPACE CONFIGURATION USES TWO SMALL STREAMING TEXT GRAMMARS.  PASSWD/FSTAB/
INITTAB-STYLE FILES ARE COLON-DELIMITED RECORDS WHOSE FIELD SEMANTICS BELONG TO
THE CONSUMER.  SIMPLE POLICY FILES USE `KEY = VALUE ...` RECORDS WITH ZERO OR
MORE SPACE-SEPARATED VALUES.  THE SHARED USERLAND TEXT LIBRARY PERFORMS S6REC
READING, WHITESPACE/COMMENT HANDLING, AND ALLOCATION-FREE TOKENIZATION; IT DOES
NOT OWN APPLICATION POLICY OR BUILD AN IN-MEMORY CONFIGURATION TREE.

## 3. PROCESS, VM, IPC, AND TTY

THE PDP-6 VM BACKEND USES CONTIGUOUS USER EXTENTS, RELOCATION/PROTECTION,
PROCESS COMPACTION, AND WHOLE-IMAGE SWAPPING.  CLEAN RECLAIMABLE FILESYSTEM
CACHE IS DROPPED BEFORE PROCESS SWAP.

MEMFS USES DEMAND-BACKED STORAGE INSTEAD OF A FIXED PREALLOCATED/COMPACTED RAM
POOL.  PHYSICAL CORE REMAINS OWNED BY MM EXTENTS; EVICTED
MEMFS DATA SHARES THE EXISTING D6FS SWAP RESERVATION WITH PROCESS SWAP.  THE
SWAP RESERVATION IS ONE DYNAMICALLY SHARED POOL, NOT A PROCESS/MEMFS
PARTITION.  THE COMMON SWAP STORE SHALL REUSE THE COMPACT D6FS-STYLE
BITMAP/CONTIGUOUS-RUN ALLOCATION MECHANISM RATHER THAN ADDING A THIRD EXTENT
ALLOCATOR.  MEMFS RECLAIM IS INSERTED BEFORE PROCESS SWAP WHERE THAT FREES
CORE CHEAPLY; CLEAN MEMFS DATA WITH VALID SWAP BACKING MAY BE DROPPED WITHOUT
ANOTHER WRITE.

MEMFS ALLOCATES ONE BLOCK-ALIGNED 640-WORD METADATA AREA AT MOUNT: 448 WORDS
FOR 64 SEVEN-WORD NODES, 64 PER-NODE BACKING DESCRIPTORS, 64 DIRECT COMPACT
UID9,GID9 OWNER WORDS, AND 64 MTIME36 WORDS.  FILE-DATA CHUNKS STILL COME FROM
MM ONLY ON FIRST USE; MEMFS RELOCATES ONLY THE FILE BEING GROWN AND RETURNS A
COMPLETELY FREE CHUNK TO MM.  THE OLD GLOBAL DATA-POOL COMPACTION PATH IS
REMOVED.  THE CURRENT
FOUR-CHUNK BOUND LIMITS MEMFS TO FOUR MM DESCRIPTORS.  SWAP-STORE ALLOCATION
IS SHARED AND MEMFS ALLOCATION PRESERVES THE ORIGINAL 0200-BLOCK PROCESS-SWAP
RESERVE.

MEMFS EVICTION/FAULT-IN IS NOW IMPLEMENTED.  EACH OF THE AT MOST FOUR RESIDENT
MM CHUNKS IS SECTOR-ALIGNED INTERNALLY; UNDER MM PRESSURE MEMFS WRITES DIRTY
FILES IN A CHUNK TO SHARED SWAP, PUBLISHES ONE FIRST-BLOCK,,BLOCK-COUNT WORD
PER NODE, THEN RELEASES THE WHOLE MM CHUNK.  READ/WRITE/TRUNCATE FAULT DATA
BACK ON DEMAND.  CLEAN BACKING SURVIVES FAULT-IN UNTIL THE FILE IS MODIFIED,
SO A SECOND EVICTION NEEDS NO WRITE.  MM PRESSURE ORDER IS D6FS CLEAN CACHE,
MEMFS EVICTION, THEN PROCESS SWAP.  MEMFS CANNOT SELF-EVICT WHILE ACQUIRING A
NEW MM CHUNK.  THE PER-NODE BACKING WORDS LIVE BESIDE THE DYNAMIC NAMESPACE,
NOT IN PERMANENT MRES DATA.

FILESYSTEM MEMORY PRESSURE NOW USES ONE MM-FACING RECLAIM INTERFACE WITH TWO
CLASSES: CHEAP/DISCARDABLE CACHE FIRST, THEN BACKED FILESYSTEM DATA.  D6FS AND
DTFS/TSFS SHARE THE COMMON CLEAN BCACHE L1; MEMFS SUPPLIES THE SECOND CLASS
WITH SWAP-BACKED CHUNK EVICTION.  THEIR BACKING SEMANTICS AND MOVABLE MRES
LIFETIMES REMAIN SEPARATE.  THIS REMOVES D6FS/MEMFS POLICY NAMES FROM MM
WITHOUT ADDING A THIRD CACHE/EXTENT MANAGER.

THE STORAGE CONSOLIDATION IS IMPLEMENTED.  PROCESS SWAP AND MEMFS SHARE THE
GENERIC BSTORE BITMAP/RUN ALLOCATOR, WHILE D6FS AND DTFS/TSFS USE THE SHARED
CLEAN TWO-BLOCK BCACHE L1.  LOGSTORE REMAINS A FIXED RAW REGION WITH ITS OWN
RING/RECOVERY SEMANTICS.  AUXILIARY STORAGE IS FILESYSTEM-INDEPENDENT; DO NOT
ADD A SECOND MEMFS CACHE/BACKING ALLOCATOR OR A DIRTY GENERAL BLOCK CACHE.  THE
DETAILED CONTRACT IS IN PDP10-DOC `KERNEL/BSTORE.MD`.

THE NORMAL PDP-6 `MAKE BOOT` MEDIA LAYOUT NOW USES THE HDD SET AS THE D6FS
ROOT AND DRM0 AS NON-ROOT AUXILIARY STORAGE.  DRM0 IS FORMATTED AT THE FULL
TYPE-236 CAPACITY OF 020000 0200-WORD BLOCKS.  BLOCK 0 HOLDS THE AUXSTORE
DESCRIPTOR; THE LOW REGION AFTER IT IS AN EMPTY, INDEPENDENT D6FS FILESYSTEM,
AND THE TAIL HOLDS FIXED LOGSTORE, BACKSTORE, AND CACHESTORE REGIONS.  THE
DEFAULT SIZES ARE LOGSTORE 0400 BLOCKS, BACKSTORE 04000 BLOCKS, AND CACHESTORE
02010 BLOCKS, LEAVING 011367 BLOCKS FOR THE NON-ROOT D6FS PARTITION.  NO KINIT
BOOT STREAM OR ROOT PAYLOAD IS COPIED TO THIS NORMAL AUXILIARY DRUM.  DRM1-3
ARE BLANK FULL-CAPACITY MEDIA.  AN EXPLICIT DRM BOOT (`STAGE1_DEVICE=drm`)
RETAINS THE BOOTABLE D6FS DRUM-SET BUILD PATH.

THE TOP-LEVEL `MAKE BOOT` POLICY IS EXPLICIT: IT INVOKES THE IMAGE BUILD WITH
`BOOT=DSK ROOT=DISK`.  IT THEREFORE ALWAYS LOADS STAGE1 FROM DSK AND SELECTS
THE DSK D6FS ROOT, INSTEAD OF RELYING ON AUTO ROOT PROBING.  ALTERNATE BOOT
SOURCE/ROOT COMBINATIONS REMAIN AVAILABLE THROUGH THE LOWER-LEVEL IMAGE BUILD
AND THEIR EXPLICIT `BOOT=`/`ROOT=` SETTINGS.
THE TOP-LEVEL MAKEFILE ALSO PASSES THESE VALUES EXPLICITLY; ITS GENERIC
`ROOT?=AUTO` VARIABLE IS NOT FORWARDED BY `MAKE BOOT`.  `DISK-BOOT` IS AN
ALIAS OF THAT SAME FIXED DSK/D6FS BOOT POLICY.

OPTIONAL PERSISTENT MEMFS IS IMPLEMENTED: `PERSIST` IN `/CONFIG/FSTAB` MAKES
ORDERLY HALT COMMIT A VERSIONED/CHECKED SNAPSHOT INTO AN EXPLICIT SWAP-TAIL
RANGE, AND A PERSISTENT MOUNT REUSES THAT SNAPSHOT AS DEMAND-LOAD BACKING.
ORDINARY UNMOUNT AND
DEFAULT MEMFS REMAIN VOLATILE; THE NORMAL RUNTIME SWAP BITMAP REMAINS EPHEMERAL.
VOLATILE MEMFS DOES NOT REQUIRE SWAP: WITH A ZERO-LENGTH SWAP TAIL ITS RECLAIM
PATH RETURNS IMMEDIATELY AND MEMFS REMAINS RAM-ONLY UNTIL MM EXHAUSTION.
PERSISTENT MEMFS FAILS ITS PERSISTENT MOUNT MODE IF NO EXPLICIT PERSISTENT
BACKING RANGE EXISTS; IT NEVER SILENTLY BECOMES VOLATILE.  THE FINAL SWAP-TAIL
BLOCK IS THE DISCOVERY/COMMIT HEADER.  KINIT RESERVES A VALID RECORDED RANGE
BEFORE PROCESS SWAP CAN USE IT; MEMFS RESTORES THE NAMESPACE AND LEAVES FILE
DATA DEMAND-BACKED THERE.  SHUTDOWN INVALIDATES THE HEADER FIRST, WRITES FILE
DATA AND METADATA, THEN PUBLISHES THE VERSIONED HEADER LAST.

PROCESS GROUPS, SESSIONS, CONTROLLING TERMINALS, FOREGROUND GROUPS,
STOP/CONTINUE, CHILD STATE REPORTING, DESCRIPTOR I/O, PIPES/FIFOS, AND NATIVE
PROCESS EVENTS ARE IMPLEMENTED.

PROCESS-SLOT EXHAUSTION CANNOT LOCK OUT ADMINISTRATION.  THE HIGHEST TWO
RUNTIME PROCESS SLOTS ARE RESERVED FOR UID 0: ORDINARY RUN ALLOCATION STOPS
BEFORE THAT TAIL, WHILE ROOT/INIT MAY USE THE COMPLETE TABLE.  THE POLICY ADDS
NO PER-UID COUNTERS OR QUOTA TABLE; ON THE 24-SLOT MINIMUM CONFIGURATION IT
LEAVES 21 ORDINARY USER SLOTS PLUS PID 1 AND TWO ROOT-RESERVED SLOTS.

THE STABLE PROCESS U-AREA IS 0407 WORDS.  USER-PREEMPTION AND
SLEEPING-EXECUTIVE REGISTER IMAGES SHARE ONE CONTEXT AREA because those
scheduler states are mutually exclusive and the sleeping syscall's eventual
user-return frame already lives on the stable private kernel stack.  This
saves 17 physical RAM words per live process while retaining the full 0316
private-stack words introduced by the earlier overflow fix.  The watermark
acceptance scans all 0316 words, rejects saturation at capacity as well as the
existing 0310 policy limit, and currently measures a 0301-word high water.
AND `LS /TEMP`, `DU /TEMP`, AND PLAIN `DU` PASS AFTER THE CHANGE.

THE TTY LINE DISCIPLINE SITS BETWEEN TERMINAL DEVICE CHARACTERS AND PROCESS
READ/WRITE CALLS.  IN COOKED MODE IT COLLECTS A LINE BEFORE DELIVERING IT TO A
READER, ECHOES INPUT, INTERPRETS ERASE AND KILL EDITING, RECOGNIZES EOF, AND CAN
TURN CTRL-C/CTRL-Z INTO FOREGROUND PROCESS-GROUP SIGNAL EVENTS.  IN RAW MODE IT
DOES NOT PERFORM LINE EDITING OR WAIT FOR A COMPLETE LINE: INPUT IS DELIVERED
DIRECTLY SO INTERACTIVE PROGRAMS SUCH AS EDITORS AND SHELL LINE EDITORS CAN
IMPLEMENT THEIR OWN KEY HANDLING.

LOGICAL TTY INPUT IDENTITY AND OUTPUT ROUTING ARE NOW SEPARATE.  TTY0 REMAINS
THE CTY KEYBOARD INPUT, TTY1..16 REMAIN DCS0..15 INPUT, AND TTY17..20 REMAIN
GE0..3 INPUT; EACH LOGICAL TTY MAY INDEPENDENTLY ROUTE OUTPUT TO CTY, ANY DCS
LINE, ANY GE CONSOLE, OR THE TYPE-340 DPY SINK.  ZERO ROUTE BITS MEAN THE
HISTORICAL NATIVE INPUT/OUTPUT PAIR.  THE FIVE-BIT OUTPUT ROUTE OCCUPIES
PREVIOUSLY UNUSED BITS 29..33 OF EACH EXISTING TTY RECORD, SO ROUTING ADDS NO
FIXED RAM TABLE AND SURVIVES LOGIN/SESSION DETACH.  ROOT MAY CHANGE THE GLOBAL
ROUTE WITH TTYCTL/DSH TTYOUT; GETOUT IS UNPRIVILEGED.

THE TYPE-340 DPY SINK HAS A RETAINED 84-BY-46 SCALE-2 TERMINAL.  THE FIRST
ACTUAL DPY TERMINAL BYTE LAZILY ALLOCATES ONE 1288-WORD
MM_TYPE_KERNEL_DYNAMIC NATIVE-TEXT EXTENT; THIS WORKING AREA IS NOT PERMANENT
KCORE/MRES AND MUST NOT BE ADDED TO PERMANENT-SIZE RESULTS.  EACH ROW HAS
FOURTEEN SIX-CELL BLOCKS, TWO WORDS PER BLOCK.  A ZERO FIRST WORD DENOTES AN
UNINITIALIZED SIX-BLANK-CELL BLOCK AND MAKES THE SECOND WORD IRRELEVANT.  ON
FIRST USE BOTH WORDS ARE INITIALIZED TO SIX DIRECTLY EXECUTABLE TYPE-342
(SI/SO,GLYPH) PAIRS, THREE PAIRS PER WORD.  EVERY TOUCHED CELL THEREFORE CARRIES
ITS OWN CHARACTER-SET SELECTION.  THE SAME BLOCK IMAGE IS EDITED BY THE TTY
PATH AND FED DIRECTLY TO BLKO: THERE IS NO SECOND SIXBIT CHARACTER PLANE, ROW
LENGTH TABLE, OR COMPILED-ROW CACHE.

LOWERCASE STREAM OUTPUT IS NORMALIZED TO TYPE-342 UPPERCASE.  SHIFTED
PUNCTUATION CHANGES ONLY THE SELECTED FIXED PAIR.  IF CURSOR MOTION FIRST
TOUCHES A LATER BLOCK, PREDECESSOR BLOCKS ARE INITIALIZED TO EXPLICIT BLANK
PAIRS SO THE USED ROW PREFIX IS CONTIGUOUS BLKO DATA.  ROW CLEAR STILL ZEROS
ONLY THE FIRST WORD OF EACH BLOCK.  SCROLLING ROTATES A 46-ROW RING WITHOUT
COPYING THE SCREEN, AND dpy_text_rows_used BOUNDS REFRESH AT THE LAST LOGICAL
ROW THAT CAN CONTAIN VISIBLE TEXT.  THE SCALE-2 TYPE-342 LINE HEIGHT IS 22
DISPLAY UNITS; THE FRAME STARTS AT Y=1010, SO 46 ROWS FIT INSIDE THE 0..1023
TYPE-340 RASTER AND THE TOP-ROW GLYPH EXTENT ENDS AT Y=1022.  FORTY-SIX ROWS
ARE THEREFORE THE MAXIMUM SAFE HEIGHT AT THE CURRENT SCALE.

THE 2026-10-05 SCROLL REGRESSION WAS IN THE COMPACT ASSEMBLY REWRITE, NOT THE
RING DESIGN: dpy_text_scroll LOADED THE OLD RING TOP INTO AC5 BUT THEN USED
`AOJ 1,5`, SO IT DID NOT COMPUTE old_top+1.  THE SCROLL PATH NOW FORMS THE
NEXT TOP DIRECTLY WITH `MOVEI 1,1(5)`.  THE PUBLIC /SYSTEM/EXEC/CLEAR COMMAND
WRITES FORM FEED; THE RETAINED DPY TERMINAL ALREADY DEFINES FORM FEED AS
SCREEN CLEAR PLUS CURSOR/RING RESET, SO NO DPY-SPECIFIC USERSPACE ABI WAS
ADDED.

THE SUBSEQUENT BLKO INTEGRATION EXPOSED A SECOND 42-ROW LIMIT: dpy_io.s STILL
USED THE OLD OCTAL 052 DPY_TEXT_ROWS VALUE, AND ITS PRIVATE BANNER REFRESH
SENTINEL ALSO USED 052.  THE RETAINED STORE AND SCROLL PATH COULD HOLD 46 ROWS,
BUT THE BLKO REFRESH STATE MACHINE TERMINATED WHEN dpy_refresh_row REACHED
LOGICAL ROW 42.  dpy_io.s NOW USES OCTAL 056 FOR 46 TEXT ROWS AND RESERVES
OCTAL 056..060 FOR BANNER/ONESHOT/TRAILER REFRESH STATES, STRICTLY ABOVE THE
VALID LOGICAL ROW RANGE 0..055.

THE NORMAL FULL GUI `make boot` NOW KEEPS TTY0 ON ITS NATIVE CTY OUTPUT ROUTE,
SO LOGIN/DSH AND DIAGNOSTICS REMAIN VISIBLE ON CTY WHILE /DEV/DPY0 IS RESERVED
FOR RAW DISPLAY-PROGRAM TESTING SUCH AS DPYVIEW.  DPY_LOGIN=1 REMAINS AN
EXPLICIT OPTIONAL BUILD SETTING; WHEN SELECTED, TTY0 INPUT STILL USES THE CTY
KEYBOARD WHILE OUTPUT IS ROUTED TO THE TYPE-340 DISPLAY.

MONITORFS ALSO PUBLISHES /DEV/TTYDPY0 WHEN BOTH CTY0 AND DPY0 ARE INSTALLED.
IT IS A COMPOSITE LOGICAL TTY0 ENDPOINT: READS USE CTY0, WRITES GO DIRECTLY TO
THE RETAINED DPY TEXT BACKEND, AND ISATTY TREATS IT AS THE CALLER'S
CONTROLLING TTY0.  IT DOES NOT MUTATE TTY0'S PERSISTENT OUTPUT ROUTE.
ECHO TEST > /DEV/TTYDPY0 HAS BEEN ACCEPTED ON A GUI BOOT; AFTER SEVERAL
REFRESH INTERVALS THE FRAMEBUFFER CONTAINED 56 LIT PIXELS OVER BBOX
(0,37)-(45,50), DISTINCT FROM THE 147-PIXEL BOOT BANNER, WITH CPU PIH ZERO.

/DEV/DPY0 ALSO HAS A WORD-ORIENTED NATIVE DISPLAY-PROGRAM ABI.  A
WRITE_WORDS SUBMISSION REPLACES THE ACTIVE PERSISTENT TYPE-340 LIST.  THE
DRIVER COPIES THE WORDS INTO ONE DYNAMIC EXTENT, PUBLISHES AN IMMUTABLE BLKO
IOWD WITH PI7 DISABLED, AND THE LINE-CLOCK HOOK STARTS ONE FINITE BLKO PASS AT
THE NORMAL 30-HZ CADENCE.  RAW-LIST BLKO OVERFLOW NOW ENDS THAT PASS, DISABLES
DPY PI REQUESTS, AND REARMS THE CLOCK DIVIDER INSTEAD OF IMMEDIATELY RELOADING
THE IOWD AT DEVICE SPEED.  WHILE THE RAW LIST EXISTS IT HAS PRIORITY OVER
RETAINED-TEXT REFRESH; A ZERO-LENGTH WRITE/CLOSE RELEASES THE EXTENT AND THE
NEXT 30-HZ TICK RESUMES TTY REFRESH.

/SYSTEM/EXEC/DPYVIEW IS THE INITIAL USERSPACE PICTURE VIEWER.  THE KERNEL DOES
NOT DECODE IMAGE FORMATS OR CONSTRUCT GRAPHICS OBJECTS; IT ONLY RETAINS AND
REPLAYS THE SUPPLIED NATIVE 36-BIT DISPLAY PROGRAM.  DPYVIEW NOW ACCEPTS
NATIVE FRAMES UP TO 32256 WORDS.  ITS BSS IS 32301 WORDS INCLUDING THE FRAME
BUFFER, WHICH REMAINS BELOW THE CURRENT 077777-WORD DXR BSS FIELD LIMIT.
DPYVIEW SUBMITS THE FRAME ONCE AND, WHEN STDIN IS ITS CONTROLLING TTY, BLOCKS
IN THE NORMAL READCHAR PATH WHILE THE KERNEL REFRESHES IT.  THIS LETS THE
EXISTING COOKED-TTY CTRL-C PATH DELIVER INT TO THE FOREGROUND PROCESS GROUP;
OTHER INPUT IS IGNORED.  NON-TTY STDIN RETAINS THE LOW-COST SLEEP FALLBACK.
THE FORMER TWO-TICK USERSPACE RESUBMISSION LOOP WAS REMOVED BECAUSE IT RESET
AND REALLOCATED THE SAME PERSISTENT LIST WHILE THE DRIVER WAS ALREADY
REFRESHING IT.

DXR V1 USES ALL 15 BSS BITS AVAILABLE BELOW ITS THREE WORD-1 FLAG BITS.  THE
HISTORICAL 020000-WORD BSS ADMISSION CAP HAS BEEN REMOVED; BSS MAY NOW RANGE
THROUGH 077777 WORDS, WHICH IS THE FORMAT'S NATURAL LIMIT and remains far
below the PDP-6 256K-word process-address-space ceiling even with the current
maximum image and fixed stack.  This is required by large static-buffer tools
such as DPYVIEW and does not enlarge any resident kernel allocation.
HOST-SIDE IMG2DPIC IS NOW A C99/MAGICKWAND HOST PROGRAM WITH TWO LOSSLESS
TYPE-340 ENCODERS.  THE VECTOR-RUN ENCODER MEASURES HORIZONTAL AND VERTICAL
SERPENTINE RUNS, INCLUDING BOTH INITIAL DIRECTIONS.  THE INCREMENTAL ENCODER
GREEDILY COVERS THE SAME LIT PIXELS WITH 8-CONNECTED ONE-PIXEL PATHS AND PACKS
FOUR PATH STEPS PER TYPE-340 INCREMENTAL HALFWORD.  EACH PATH START IS
INTENSIFIED EXACTLY ONCE; ONE-TO-THREE RESIDUAL STEPS ARE EMITTED AS ORDINARY
ONE-PIXEL VECTORS RATHER THAN ZERO-PADDED INCREMENT WORDS, SO ENDPOINTS ARE NOT
ARTIFICIALLY BRIGHTENED.  IMG2DPIC MEASURES BOTH REPRESENTATIONS AND EMITS THE
SHORTER PROGRAM.

THE OPTIMIZATION IS LOSSLESS WITH RESPECT TO THE THRESHOLDED 1024X1024 BITMAP.
RENDER-BACK VALIDATION OF ALL 25 CURRENT PICTURES PRODUCED BIT-FOR-BIT IDENTICAL
LIT-PIXEL MAPS.  AGAINST THE PRE-OPTIMIZATION DISPLAY PROGRAMS THE FINAL
ENCODER REDUCES THE CORPUS FROM 359396 TO 239465 WORDS, ABOUT 33.4 PERCENT.
PICTURE08 FALLS FROM 30324 TO 15092 WORDS, WHICH REDUCES THE CURRENT SIMH
FIXED-PER-WORD REPLAY ESTIMATE FROM ABOUT 3.03 TO 1.51 SECONDS.

IMG2DPIC ALSO HAS AN AUTOMATIC LINE-ART STROKE CANDIDATE FOR THE 100-MS
PICTURE-REFRESH TARGET.  IT PERFORMS TOPOLOGY-PRESERVING THINNING, REMOVES
FALSE DIAGONAL CORNER CONNECTIONS, TRACES THE RESULTING STROKE GRAPH, APPLIES
ONE-PIXEL-TOLERANCE DOUGLAS-PEUCKER VECTOR SIMPLIFICATION, AND ORDERS/REVERSES
STROKES TO MINIMIZE DARK BEAM TRAVEL.  THIS APPROXIMATE MODE IS NOT UNIVERSAL
BY ITSELF: IT IS ELIGIBLE ONLY WHEN THE GENERATED STROKES PASS BOUNDED-ERROR
CHECKS AGAINST THE ORIGINAL THRESHOLDED RASTER.  AT LEAST 99.5 PERCENT OF
SOURCE LIT PIXELS MUST LIE WITHIN TWO PIXELS OF THE STROKE RESULT, EVERY SOURCE
LIT PIXEL MUST LIE WITHIN FIVE PIXELS, AND EVERY GENERATED STROKE PIXEL MUST
LIE WITHIN ONE PIXEL OF THE SOURCE.  OTHERWISE IMG2DPIC AUTOMATICALLY FALLS
BACK TO THE LOSSLESS VECTOR/INCREMENTAL ENCODERS.

ON THE CURRENT PICTURE CORPUS FIVE IMAGES PASS THAT LINE-ART GATE.  THEIR
ESTIMATED TYPE-340 EXECUTION TIMES ARE PICTURE08 96.18 MS, PICTURE20 16.28 MS,
PICTURE21 48.50 MS, PICTURE24 50.21 MS, AND PICTURE25 20.29 MS.  PICTURE08 IS
REDUCED FROM THE ORIGINAL 30324 WORDS TO 1923 WORDS IN STROKE MODE.

IMAGE PROCESSING USES A 1024X1024 HOST-SIDE BILEVEL RASTER AFTER IMAGEMAGICK
RESIZE/GRAYSCALE/50-PERCENT THRESHOLDING; THERE IS NO PYTHON OR
INTERMEDIATE-PBM DEPENDENCY.  WHITE INPUT PIXELS ARE INTENSIFIED AND BLACK
PIXELS REMAIN DARK.  RASTER Y IS REFLECTED BECAUSE IMAGE ROW ZERO IS THE TOP
EDGE WHILE TYPE-340 Y=0 IS THE BOTTOM EDGE.

SYS_PROCINFO COMM NOW DERIVES UP TO SIX SIXBIT BASENAME CHARACTERS DIRECTLY
FROM THE LIVE PROCESS STARTUP ARGV[0] RECORD.  THIS REPLACES THE FORMER
HARD-CODED USER NAME WITHOUT ADDING A PER-PROCESS COMM WORD; SWAPPED OR
OTHERWISE UNAVAILABLE IMAGES STILL FALL BACK TO USER.  /SYSTEM/EXEC/KILL PID
USES THE EXISTING PROCCTL EVENT_PID ABI TO SEND TERM, SO PROCESS TERMINATION
NEEDS NO NEW KERNEL SIGNAL MECHANISM.

THE SOURCE-TREE `pictures/` DIRECTORY IS A LOCAL, GIT-IGNORED MEDIA-STAGING
AREA.  D6FS ROOT-IMAGE BUILDS CREATE `/PICTURES` AND IMPORT EACH REGULAR
FILE FOUND DIRECTLY IN THAT DIRECTORY AS A MODE-0444
36-BIT BINARY-WORD FILE.  HOST BASENAMES ARE FOLDED TO UPPERCASE FOR THE
SIXBIT NAMESPACE; STAGED NAMES ARE LIMITED TO 24 CHARACTERS FROM A-Z, 0-9,
DOT, UNDERSCORE, PLUS, OR MINUS.  CHANGING A STAGED FILE MAKES IT A ROOT-IMAGE
DEPENDENCY, SO A SUBSEQUENT MEDIA BUILD REIMPORTS IT.
TSFS ROOT-IMAGE DEPLOYMENT EXCLUDES `/PICTURES` ENTIRELY, INCLUDING ITS
DIRECTORY, ALL PICTURE FILES, AND THE PICTURE-FILE BUILD PREREQUISITES.

TYPE-340 DATA REQUESTS RUN ON LOWEST-PRIORITY PI7, WHILE APR CLOCK AND
SCHEDULER KICKS REMAIN ON PI6.  THE 2026-10-05 ITS COMPARISON CORRECTED THE
EARLIER BLKO CONCLUSION: DEVICE 0130 DOES SUPPORT THE PDP-6 BLOCK-OUTPUT DATA-
CHANNEL MODEL, BUT BLKO MUST BE THE EVEN PI VECTOR INSTRUCTION.  THE FAILED
2026-10-04 EXPERIMENT ISSUED BLKO FROM INSIDE AN ORDINARY HANDLER AND DID NOT
REPRODUCE ITS CONTROL FLOW.  KINIT NOW INSTALLS `BLKO 0130,dpy_refresh_iowd`
IN LOW-CORE PI7 WORD 056 AND A DIRECT JSR TO THE DPY PACKAGE'S PRIVATE
STACKLESS COMPLETION ENTRY IN WORD 057.  BLKO COUNT OVERFLOW SELECTS THAT ODD
VECTOR; THE GENERIC PI7 DISPATCHER IS DELIBERATELY BYPASSED.  SLV IS MOVED TO
ORDINARY PI6 DISPATCH ONLY when DPY owns PI7.  NO-DPY SYSTEMS RETAIN THE
UNMODIFIED GENERIC PI7 VECTORS AND PAY NO DISPLAY RUNTIME COST.

THE RETAINED TEXT FORMAT WAS SIMPLIFIED WITH THE BLKO CONVERSION.  EVERY
INITIALIZED SIX-CELL BLOCK IS TWO DIRECT FIXED-PAIR TYPE-342 WORDS, SO THE
INITIALIZED PREFIX OF ONE ROW IS ONE CONTIGUOUS BLKO SPAN.  ONE CR/LF WORD IS
A SECOND SPAN; THE COMPLETED IOWD ADDRESS ITSELF IS THE ROW-END PHASE MARKER.
THIS REMOVES THE OLD PER-BLOCK SIMPLE/COMPLEX REFRESH DECODER, SHIFT-STATE
TRACKING, AND BLOCK CURSOR FROM THE INTERRUPT HOT PATH.  A FINITE FRAME ENDS
WITH ONE NONPRINTING SI TRAILER SO dpy_pending IS CLEARED ONLY AFTER THE LAST
VISIBLE WORD HAS ACTUALLY COMPLETED.

THE LINKED DPY MRES IS NOW 439 PERMANENT WORDS, DOWN FROM THE 534-WORD
PRE-BLKO CURRENT BASELINE, A 95-WORD PERMANENT SAVING.  THE 46-ROW LAZY TEXT EXTENT IS 1288 WORDS AND STILL SAVES 560 DYNAMIC WORDS
VERSUS THE FORMER 1848-WORD SIXBIT+COMPILED-CACHE DESIGN.  p10super FINDS NO FURTHER REORDERING
OR PEEPHOLE SAVINGS; p10fold FINDS ONLY FOUR-WORD RUNS REQUIRING EXTRA JRSTs,
SO NO AUTOMATIC DPY FOLD WAS APPLIED.

THE CURRENT 2026-10-06 MEASUREMENT AFTER THE PURE-TEXT PROCESS-SWAP
OPTIMIZATION IS KCORE 9691 WORDS, CLK MRES 45, DPY MRES 444, DTFS MRES 1827,
D6FS MRES 2091, MEMFS MRES 1363, KCORE+MRES 16980, PERMANENT_LAST 041203,
MULTI_ROOT 041165, AND MULTI_LAST 041244.  MULTI_LAST IS THEREFORE 676
ADDRESSES ABOVE THE 040000 LIMIT, SO THE FINAL OPTIMIZATION SWEEP REMAINS
ACTIVE.  A DIAGNOSTIC BUILD WITH PROC_SWAP_VERIFY_PURE=1 MEASURES KCORE 9793
WORDS AND MULTI_LAST 041412; THOSE EXTRA 102 KCORE WORDS ARE COMPILED OUT OF
NORMAL KERNELS.  THE PRE-SWAP APR BASELINE WAS KCORE 9487 AND MULTI_LAST
040730.  THE APR CORRECTNESS FIX ITSELF ADDS 20 WORDS TO CLK MRES VERSUS THE
IMMEDIATELY PRECEDING 25-WORD PACKAGE; ITS SIMULTANEOUS-CLOCK PATH SHARES THE
NORMAL TICK BODY RATHER THAN DUPLICATING IT.

THE 2026-10-07 FINAL SHRINK RUN IS ACTIVE.  P10FOLD NOW ACCEPTS RELOCATION-
SAFE TWO-WORD ZERO-COST RUNS IN KCORE, AND GE/TTY/DTFS/D6FS APPLY PACKAGE-
LOCAL POST-LINK FOLD PLANS AT THE SAME THRESHOLD.  THE PERMANENT-SIZE REPORTER
WAS ALSO FIXED TO READ THE LINKED MRES1 RESIDENT EXTENT RATHER THAN SUMMING
UNFOLDED INPUT DOBJS.  WITH THOSE CHANGES KCORE IS 9669 WORDS, D6FS 2083,
DTFS/TSFS 1823, MEMFS 1337, AND MULTI_LAST IS 041144: 612 ADDRESSES ABOVE
040000.  MEMFS REMAINS THE PRIMARY SOURCE-LEVEL SHRINK TARGET; THE RUN IS NOT
COMPLETE UNTIL MULTI_LAST IS BELOW 040000 AND THE ACCEPTANCE SUITE PASSES.

PDP-6 APR FAULT DELIVERY IS NOW PART OF THE PI6 CLK MRES.  APR CONI IS
QUALIFIED BEFORE CLOCK/SOFTWARE-KICK HANDLING FOR NON-EXISTENT MEMORY,
ILLEGAL/PROTECTED ADDRESS, AND PUSHDOWN-LIST OVERFLOW.  THE HANDLER CLEARS
THOSE HARDWARE CONDITIONS WHILE PRESERVING APR PIA6 AND THE 60-HZ CLOCK.
WHEN THE PI6 SAVED RETURN SHOWS USER MODE, THE HANDLER REDIRECTS THE NORMAL
PI DISMISSAL TO AN EXECUTIVE TRAMPOLINE, RESTORES THE PROCESS'S PRIVATE KERNEL
STACK, CLOSES ITS FILES, AND TERMINATES IT WITH THE EXISTING TERM-CLASS WAIT
STATUS.  THIS AVOIDS BOTH LEAVING PI6 HELD AND FREEING THE U-AREA WHILE STILL
EXECUTING ON ITS STACK.  THE SAME APR CONDITIONS INTERRUPTING EXECUTIVE MODE
ARE FAIL-STOP KERNEL FAULTS.  ARITHMETIC-OVERFLOW AND PC-CHANGE APR INTERRUPTS
REMAIN DISABLED.  SIMH IS CONFIRMED TO IMPLEMENT THE PDP-6 NXM, MEMPROT, AND
PUSHOVER APR FLAGS; DAIMOS-TESTKIT COVERS A REAL USER PROTECTION FAULT, NXM,
A REAL USER PUSH OVERFLOW, AND EXECUTES THE PRODUCTION CLK/APR MRES PATH ON
THE INDEPENDENT AAP PDP-6 EMULATOR.

THE EARLIER PERMANENT-SIZE REPORTER COUNTED ONLY MEMFS_RUNTIME FOR MEMFS EVEN
THOUGH THE INSTALLED PACKAGE ALSO CONTAINS MEMFS_DATA AND MEMFS_SNAPSHOT, AND
IT OMITTED BADMAP.  THE REPORTER NOW COUNTS THE COMPLETE INSTALLED MEMFS AND
BADMAP RESIDENT SET.  THE PREVIOUS 56-WORD-BELOW RESULT WAS THEREFORE AN
ACCOUNTING UNDERESTIMATE, NOT A REAL PERMANENT-IMAGE MARGIN.

THE FIRST PDP-6 MEMFS DATA/SNAPSHOT REWRITE IS COMPLETE.  MEMFS_DATA TEXT IS
477 WORDS INSTEAD OF KCC'S 796 AND MEMFS_SNAPSHOT IS 216 INSTEAD OF 343, A
COMBINED 446-WORD TEXT REDUCTION BEFORE LINK-TIME EFFECTS.  MEMFS REMAINS THE
PRIMARY SOURCE-LEVEL SHRINK TARGET.

THE COMMON BCACHE NOW REUSES THE PERMANENT 0200-WORD FS_BLOCK_WORKSPACE AS
CACHE SLOT 0 AND ALLOCATES ONLY ONE PRIVATE 0200-WORD DATA SLOT PLUS ITS TAG
IN MANAGED CORE.  THE PRIVATE SLAB IS 0201 OCTAL WORDS (129 DECIMAL), DOWN
FROM THE FORMER 0403-OCTAL (259-DECIMAL) PRIVATE TWO-BLOCK SLAB.  ONE FIXED
WORKSPACE TAG AND THE REQUIRED INVALIDATION/PRESERVATION CODE ADD 21 PERMANENT
WORDS, SO THE NET ACTIVE-CACHE SAVING IS 109 PHYSICAL RAM WORDS WHILE RETAINING
TWO CLEAN CACHE ENTRIES.

THE MM FIT SEARCH NOW ACCUMULATES TOTAL FREE SPACE WHILE IT WALKS THE ARENA
GAPS.  ON FAILURE THE EXISTING BASE RESULT SLOT RETURNS THAT TOTAL, ALLOWING
ALLOCATION AND COMPACTION TO DISTINGUISH TRUE NO-MEMORY FROM FRAGMENTATION
WITHOUT A SECOND mm_total_free() PASS.  THIS BOTH SHORTENS THE PRESSURE PATH
AND REMOVES 15 LINKED KCORE WORDS.  THE COLD proc_swap_service_one() AND
proc_swap_reclaim() ORCHESTRATION PATHS ARE ALSO PDP-6 ASSEMBLY LEAVES, SAVING
20 MORE KCORE WORDS WHILE LEAVING THE SWAP TRANSFER TRANSACTIONS IN C.

THE DEFERRED PDP-6 RUNTIME-MODULE MOVER NO LONGER HARD-CODES THE FIXED-BINDING
COUNT.  ITS 37-ENTRY BINDING TABLE NOW LIVES BESIDE THE ASSEMBLY MOVER AND THE
LOOP COUNT IS DERIVED FROM LABELS, PREVENTING NEW BINDINGS FROM SILENTLY
FALLING OUTSIDE THE RETARGET LOOP.

THE PDP-6 PROCESS U-AREA NOW OVERLAYS USER-PREEMPTION AND SLEEPING-EXECUTIVE
REGISTER IMAGES.  THOSE TWO SCHEDULER STATES ARE MUTUALLY EXCLUSIVE, AND A
SLEEPING SYSCALL'S USER-RETURN FRAME ALREADY LIVES ON THE STABLE PRIVATE
KERNEL STACK.  AC0..AC17 AND THE SAVED PI RETURN WORD ARE THEREFORE SHARED.
THIS REDUCES PROC_UAREA_WORDS FROM 0430 TO 0407, SAVING 17 PHYSICAL RAM WORDS
PER LIVE PROCESS WHILE RETAINING THE FULL 0316-WORD PRIVATE KERNEL STACK.
THE STACK WATERMARK SCANNER WAS ALSO FIXED TO COVER THE FULL 0316-WORD RANGE;
THE ACCEPTANCE WORKLOAD CURRENTLY MEASURES 0301 WORDS HIGH-WATER.

THE COLD MEMORY-PRESSURE proc_swap_victim() POLICY IS NOW IMPLEMENTED IN
proc_pdp6.s FOR THE PDP-6 BUILD.  THE ASSEMBLY SCAN KEEPS ITS SLOT, DESCRIPTOR,
BEST-SLOT, AND SCORE STATE IN AC10..AC16 INSTEAD OF KCC'S LARGE LOCAL FRAME,
WHILE PRESERVING THE C POLICY EXACTLY: SLEEP/STOP AGE AND WAIT BIAS, RUNNABLE
NICE/CPU BIAS, CURRENT/EXCLUDED SLOT FILTERS, U-AREA/MAPPING/TRANSITION FILTERS,
SWAP-BACKING PRESENCE, AND PHYSICAL-PIN REJECTION.  THE PDP-6 PATH ALSO FOLDS
THE ONE-USE vm_space_can_swap() WRAPPER INTO THE SELECTOR WHILE RETAINING THE
CANONICAL mm_is_pinned() CHECK.  proc.dobj FALLS 870->763 TEXT; THE ASSEMBLY
ROUTINE ADDS 86 WORDS TO proc_pdp6 AND REMOVING THE VM WRAPPER SAVES THE
REMAINDER, FOR 34 LINKED KCORE WORDS NET.  THE PORTABLE C FALLBACK REMAINS FOR
HOST/REFERENCE BUILDS; ITS proc_child_report() HELPER WAS RESTORED UNDER THE
NON-PDP6 EVENT GUARD AND COSTS ZERO TARGET WORDS.  HOST MM VICTIM-RANKING,
32K SWAP-WAIT, 32K SWAP-STRESS, MULTIPROCESS, REALTIME, AND RT-REVOKE
REGRESSIONS PASS.

PROCESS SWAP NOW USES THE EXISTING DXR2 PURE-TEXT FIELD AS AN IMMUTABILITY
CONTRACT RATHER THAN TRYING TO TRACK A HARDWARE DIRTY BIT WHICH PDP-6 DOES NOT
PROVIDE.  FOR PURE IMAGES, COMPLETE 0200-WORD SECTORS COVERED BY THE EXECUTABLE
TEXT PREFIX ARE OMITTED FROM BACKSTORE; THE TEXT/DATA BOUNDARY SECTOR AND ALL
WRITABLE DATA/HEAP/STACK STATE REMAIN IN THE SWAPPED SUFFIX.  SWAP-IN RELOADS
PLAIN TEXT DIRECTLY FROM THE EXECUTABLE.  COMPRESSED PURE DXR2 REUSES THE
NORMAL D6LZ36 VFS DECODER AGAINST THE ALREADY-VALIDATED IMMUTABLE BACKING, THEN
OVERLAYS THE SWAPPED SUFFIX.  NON-PURE IMAGES RETAIN WHOLE-IMAGE SWAP AND MAY
SELF-MODIFY.

WHILE A NONRESIDENT PURE PROCESS DEPENDS ON AN EXECUTABLE, WRITE/TRUNCATE/
UNLINK OF THAT VNODE AND UNMOUNT OF ITS FILESYSTEM ARE REJECTED.  THE CHECK
SCANS THE EXISTING PROCESS TABLE AND ADDS NO PER-VNODE RESIDENT TABLE.
PROC_SWAP_VERIFY_PURE=1 ADDS A DIAGNOSTIC-ONLY RUNTIME ASSERTION WHICH SAVES
THE COMPLETE IMAGE TEMPORARILY, RECONSTRUCTS THE DISCARDABLE TEXT, COMPARES IT
THROUGH THE SHARED 0200-WORD FILESYSTEM WORKSPACE, RESTORES THE IMAGE, AND
HALTS ON A PURE-CONTRACT VIOLATION.  NORMAL BUILDS COMPILE THAT VERIFIER OUT.
DLINK --PURE REMAINS THE FIRST STATIC CHECK AND REFUSES DEFINITE TEXT WRITES;
PURE ENABLEMENT IS THEREFORE PER EXECUTABLE, NOT A BLANKET USERLAND POLICY.

THE SWAP VERIFICATION PASS ALSO FIXED TWO LATENT LOW-LEVEL ERRORS: THE PDP-6
ASSEMBLY TRANSACTION NOW UPDATES proc_swap_blocks_used WITH ADDM ON BOTH
SWAP-OUT AND SWAP-IN, AND d6lz36_decode_vfs TREATS A NEGATIVE VFS READ RESULT
AS AN ERROR RATHER THAN AS INPUT.  THE DIRECT PDP-6 PURE TARGET COVERS RAW AND
COMPRESSED PURE RELOAD, NON-PURE FULL SWAP, BACKING/MOUNT PINS, AND THE
OPTIONAL VERIFIER ASSERTION.  THE HOST MM AGGREGATE, 32K SWAP-WAIT, AND 32K
SWAP-STRESS REGRESSIONS PASS.

COLD FILESYSTEM METADATA PATHS NOW SHARE THREE SMALL SEQUENCES WITHOUT ADDING
OVERHEAD TO NORMAL READ/WRITE I/O.  D6FS RESIZE GROW/SHRINK SHARE THE PACKED
EXTENT DECODER; D6FS TRUNCATE REUSES THE IDENTICAL CHMOD/CHOWN/UTIME FCB COMMIT
AND 035-WORD FRAME UNWIND; AND TSFS MOUNT USES ONE LOOP TO VALIDATE THE PACKED
FILE AND OPTIONAL EXTENT MEDIA RECORDS.  D6FS_PROVIDER FALLS 828->822 TEXT,
D6FS_RUNTIME 1179->1173, AND TSFS_RUNTIME 512->502.  LINKED D6FS MRES FALLS
2211->2199 AND DTFS MRES 1848->1838, SAVING 22 PERMANENT WORDS TOTAL.  THE
PRE-DSH FILESYSTEM CLOSURE, D6FS CONTEXT/MULTIMOUNT/SUPER-RECOVERY, TSFS SCAN,
MULTI-EXTENT, D6LZ, AND LIVE HEADLESS TSFS MOUNT REGRESSIONS PASS.

FILE'S HAND-WRITTEN CURRENT-CREDENTIAL HELPER NOW USES ONLY AC5..AC7 SCRATCH,
PRESERVING NORMAL SYSCALL ARGUMENTS IN AC2..AC4 ACROSS file_check_root().
ROOT-ONLY DTFS MOUNT, TTY ROUTING, STORAGECTL, DTC WRITE, AND RTCTL THEREFORE NO
LONGER SPILL AND RESTORE ARGUMENTS AROUND THE PRIVILEGE CHECK.  FILE_RUNTIME IS
UNCHANGED AT 890 TEXT WORDS; SYSCALL_DISPATCH_PDP6 FALLS FROM 825 TO 802 TEXT
WORDS AND LINKED KCORE FALLS BY THE SAME 23 WORDS.  MULTIUSER AUTH/CREDENTIAL,
RT-REQUIRED, TTY-ROUTING, LOGSTORE, STORAGE-RESERVATION, AND LIVE DTC FORMAT/
WRITE/REBOOT/MOUNT/READ REGRESSIONS PASS.

THE PDP-6 CONTEXT SWITCHER NOW SHARES THE AC0..AC17 COPY CORE BETWEEN USER
AND SLEEPING-EXECUTIVE CONTEXTS.  THE TWO U-AREA IMAGES ALREADY HAVE IDENTICAL
RELATIVE REGISTER SHAPES; USER SAVE/RESTORE USE THE U-AREA BASE AND KERNEL
SAVE/RESTORE USE `CTX_K_AC0` AS THE ADJUSTED BASE.  THIS REMOVES DUPLICATE HOT
MOVEM/MOVE SEQUENCES WITHOUT CHANGING THE U-AREA ABI OR SAVED REGISTER SET.
PROC_PDP6 DROPS BY 33 TEXT WORDS AND THE LINKED KCORE BY 33 WORDS.  THE
RESTORE PATH ALSO REUSES THE U-AREA POINTER ALREADY SUPPLIED BY THE SCHEDULER,
AVOIDING A SECOND SLOT-TO-U-AREA LOOKUP.  SLEEP, PROCESS-EVENT, MULTIPROCESS,
REALTIME, RT-REVOKE, JOB-PIPELINE, AND EXEC-REPLACE REGRESSIONS PASS.

PROC EVENT APPLICATION IS NOW HAND-WRITTEN PDP-6 ASSEMBLY IN proc_pdp6.s.
THE HOT VALIDATED EVENT PATH KEEPS SLOT/EVENT/DESCRIPTOR STATE IN AC10..AC12,
REUSES proc_finish_slot FOR FATAL REMOTE TEARDOWN AND proc_notify_parent FOR
STOP/CONT REPORTING, AND PERFORMS TSTP/CONT/ALRM PACKED-STATE TRANSITIONS
DIRECTLY.  THE C proc_child_report HELPER IS RETAINED ONLY INSIDE THE PORTABLE
NON-PDP6 FALLBACK AND EMITS NO TARGET CODE.  THIS REDUCED KCORE BY 55 WORDS
(10313 -> 10258) WHILE THE PROCESS-EVENT, JOB-PIPELINE,
MULTIPROCESS, REALTIME, RT-REVOKE, AND 60-HZ SLEEP REGRESSIONS ALL PASS.

TARGET-SIDE DPY ACCEPTANCE COVERS FIXED-PAIR CELL EDITS, SPARSE-PREFIX FILL,
ROW CLEAR/REUSE, UPPERCASE NORMALIZATION, SHIFTED PUNCTUATION, AND RING WRAP.
A HEADLESS TYPE-340 FULL BOOT WITH WCNSLS/OCNSLS DISABLED REACHES DSH, WRITES
BOTH PRIMARY TEXT AND `[\]^_` THROUGH /DEV/TTYDPY0, AND REMAINS LIVE ACROSS
MULTIPLE REFRESH PERIODS.  THE INTERRUPT-POLICY REGRESSION NOW REQUIRES THE
PI7 BLKO VECTOR AND DIRECT ODD-VECTOR COMPLETION PATH.

TARGET TESTING OF THE NEW BLOCK LOCATOR EXPOSED A PDP-6 DIVI PAIR-REGISTER
HAZARD: DIVI 2,6 CONSUMES AC2:AC3 AS A DOUBLE-WORD DIVIDEND.  THE FORMER CELL
HELPER HAD BEEN DIVIDING COLUMN,,CHARACTER STATE.  THE NATIVE BLOCK LOCATOR
NOW MOVES THE COLUMN INTO THE LOW DIVIDEND AC AND CLEARS THE HIGH AC BEFORE
DIVI; THE THREE-WAY COMPLEX PAIR SELECTOR AVOIDS DIVI ENTIRELY.  PACKED
TYPE-342 OCTAL WORD CONSTANTS ALSO REQUIRE THE LEADING `0` ASSEMBLER PREFIX;
WITHOUT IT DAS INTERPRETS THE DIGITS AS DECIMAL.

THE BANNER/TEXT REFRESH TRANSITION USES dpy_refresh_row=DPY_TEXT_ROWS AS A
NON-TEXT SENTINEL.  TEXT FRAMES USE -1 FOR THEIR SETUP SPAN.  THIS IS
REQUIRED BECAUSE THE FIRST LAZY TEXT ALLOCATION MAY COMPLETE WHILE A BOOT
BANNER FRAME IS STILL IN FLIGHT; WITHOUT THE SENTINEL THE FINAL BANNER DONE
INTERRUPT CAN BE MISREAD AS TEXT-SETUP COMPLETION, FEED NATIVE TEXT BLOCK
WORDS IN THE WRONG TYPE-340 MODE, STOP THE DISPLAY, AND LEAVE dpy_pending SET.

THE 2026-10-04 RUNTIME DEBUGGING ALSO FOUND TWO LOWER-LEVEL REFRESH FAILURES.
FIRST, PI6 MAY NEST INSIDE THE LOWEST-PRIORITY PI7 DISPLAY HANDLER.  THE PI6
SCHEDULER MUST NOT CONTEXT-SWITCH WHILE CONI PI REPORTS PI7 HELD (CONI HOLD
BIT 0400); OTHERWISE IT CAN SAVE THE NESTED PI7 FRAME AS AN ORDINARY EXECUTIVE
CONTINUATION AND RESUME USER MODE WITH PI7 STILL HELD.  SECOND, dpy_pending IS
FRAME OWNERSHIP, NOT MERELY ONE-WORD OWNERSHIP.  IT REMAINS SET FROM THE FIRST
DATAO THROUGH THE FINAL DONE OF A RETAINED FRAME.  CLEARING IT AFTER EACH DONE
CREATED A PI6 WINDOW IN WHICH THE CLOCK COULD RESET/START A SECOND FRAME WHILE
THE OLD PI7 HANDLER WAS STILL CHAINING THE PREVIOUS FRAME.

THE CLOCK/DISPLAY PATH NO LONGER REPLACES THE LIVE PI6 HANDLER. CLK REMAINS
THE REGISTERED PI6 OWNER AND EXPOSES A POST-SERVICE JRST WHICH DPY MINIT
PATCHES TO A STACKLESS DISPLAY HOOK. THIS AVOIDS AN EMPTY PI6 SPAN AND ALSO
AVOIDS EXTRA PUSHJ USE WHILE KINIT'S EXECUTIVE STACK IS ACTIVE. CLK SAVES THE
PI DISPATCHER'S AC2/AC3 IN TWO RESIDENT WORDS RATHER THAN ON AC17, SO A
SCHEDULER STACK CHANGE CANNOT RESTORE THE DISPATCH CURSOR FROM THE WRONG STACK.

THE SAME DEBUGGING EXPOSED AN INDEPENDENT SLOT-0 SCHEDULER AMBIGUITY. BEFORE
USERSPACE EXISTS, KINIT RUNS WITH proc_current_slot=0, WHICH IS ALSO THE VALUE
USED LATER BY THE PERMANENT EXECUTIVE IDLE LOOP. A PI6 TICK MUST NOT TREAT
BOOTSTRAP/KINIT SLOT 0 AS AN IDLE CONTEXT AND SWITCH AWAY FROM IT. THE PI6
SCHEDULER NOW ALLOWS SLOT-0 SWITCHING ONLY WHEN THE SAVED PI6 RETURN PC LIES
INSIDE proc_idle_loop..proc_idle_loop_end. THIS FIXES THE REPRODUCIBLE HANG
AFTER `DTC OK`, WHERE A BOOT-TIME PI6 SWITCH LEFT THE PI RETURN STATE
CORRUPTED. THE FIXED GUI BOOT PROCEEDS THROUGH MTC, DSK270, DRM236, BSET,
MEMFS, DTFS, SLV, D6FS, AND INIT V1; AFTER ROOT LOGIN/DSH THE TYPE-340
FRAMEBUFFER HAS 234 LIT PIXELS OVER BBOX (0,37)-(129,94) AND CPU PIH IS ZERO.

THE 2026-10-04 DTC BOOT HANG WAS A SEPARATE KINIT LIFETIME BUG EXPOSED BY THE
DISPLAY TIMING. module_run_minits() COPIES THE MINIT TABLE TO THE KINIT STACK
AND THEN RECLAIMS ITS SOURCE PREFIX. THE DTC/MTC/DSK SHARED ENTRY STUBS HAD
BEEN LINKED IMMEDIATELY BEFORE THAT TABLE, SO THE COPIED TABLE RETAINED ENTRY
ADDRESSES INTO RECLAIMED MEMORY. AFTER DTC INSTALLATION, MTC ENTERED AN
OVERWRITTEN STUB AND FELL INTO THE MONITOR-UUO PATH. THE THREE TYPE-136 STUBS
NOW LIVE WITH NON-RECLAIMED KINIT MODULE I/O CODE.

THE EARLIER EXPLICIT GUI DPY_LOGIN ACCEPTANCE BOOT SUSTAINED THE LOGIN
DISPLAY, ACCEPTED ROOT INPUT THROUGH CTY, EXECUTED DSH, AND REMAINED VISIBLE
AFTER MULTIPLE SECONDS OF REFRESH.  THAT REMAINS A VALID OPTIONAL ROUTING TEST,
BUT IT IS NO LONGER THE DEFAULT `make boot` CONFIGURATION; NORMAL GUI BOOT NOW
KEEPS LOGIN OUTPUT ON CTY SO RAW DPYVIEW TESTS HAVE AN UNAMBIGUOUS DISPLAY
OWNER.

LOADABLE/UNLOADABLE/RELOCATABLE MODULES AND USERSPACE OVERLAYS ARE NOT
IMPLEMENTED YET.

THE GENERAL KERNEL FEATURE/ACCEPTANCE WORK REQUIRED BEFORE USERLAND IS
COMPLETE.  THE PURE-EXECUTABLE-TEXT SWAP-OUT OPTIMIZATION IS COMPLETE.  THE
ONLY REMAINING PRE-DSH KERNEL WORK IS THE FINAL PERMANENT-SIZE/CORRECTNESS RUN
DESCRIBED IN TODO.MD.  AFTER THAT RUN IS GREEN, NORMAL DEVELOPMENT MOVES TO
DSH AND USERLAND; FURTHER KERNEL CHANGES SHOULD BE DRIVEN BY A REAL USERLAND
REQUIREMENT OR A REPRODUCIBLE DEFECT.

## 4. VFS AND NAMESPACE

VFS OWNS PATH WALKING, MOUNT CROSSING, FILESYSTEM DISPATCH, DESCRIPTORS,
CREDENTIAL/MODE CHECKING, AND COMMON FILE/FIFO OPERATIONS.  DAIMOS V0.9 UID
AND GID VALUES ARE NINE BITS (0..0777); UID 0 IS PRIVILEGED, EACH PROCESS HAS
ONE PRIMARY GID, AND THERE ARE NO SUPPLEMENTARY GROUPS.  OWNER/GROUP/OTHER RWX
SELECTION, ROOT BYPASS, OWNER-OR-ROOT CHMOD/UTIME, ROOT-ONLY CHOWN, AND THE
PROCESS UMASK ARE COMMON POLICY RATHER THAN FILESYSTEM-SPECIFIC CHECKS.  CHMOD,
CHOWN, AND UTIME SHARE ONE COMPACT VFS SETATTR PROVIDER SLOT.

D6FS PERSISTS MODE, UID/GID, AND MTIME PER NODE.  MEMFS STORES THE SAME
SEMANTICS IN ITS DYNAMIC METADATA AND VERSION-2 PERSISTENT SNAPSHOT.  THE
SUPPORTED DTFS BUILD PRESERVES NATIVE MEDIA UNCHANGED AND EXPOSES THE
MOUNTER'S UID/GID AS MOUNT OWNERSHIP; DORMANT TENEX/ITS PERSONALITY SOURCE
FOLLOWS THE SAME NO-CONVERSION RULE WHEN BUILT FOR DEVELOPMENT.  FILE-LEVEL
CHOWN AND MTIME ARE NOT REPRESENTABLE THERE.  TSFS
V1.1 STORES IMMUTABLE MODE12 AND UID9,GID9 IN ITS EXISTING EIGHT-WORD FILE
RECORD FROM MKTSFS MANIFEST DATA, WITH NO PER-FILE TIMESTAMP.  MONITORFS STAYS
SYNTHETIC/ROOT-OWNED.  DTFS, TSFS, AND MONITORFS REPORT MTIME 0; DAIMOS HAS NO
ATIME OR CTIME.

MONITORFS IS THE ONLY KERNEL-STATE FILESYSTEM:

    /MONITOR/PROCESSES
    /MONITOR/DEVICES
    /PROC                  ALIAS OF /MONITOR/PROCESSES
    /DEV/NAME              DEVICE I/O ENDPOINT VIEW

## 5. D6FS, BLOCKSET, AND BADMAP

D6FS IS THE NATIVE WRITABLE FILESYSTEM.  CURRENT V0.9 SUPPORT INCLUDES
MULTI-MOUNT, DSK/DRM BACKING, REMOUNT, CRASH-RECOVERY SELECTION, UID/GID/MODE,
MTIME, SEEK, CHOWN, RMDIR, A SMALL RECLAIMABLE WRITE-THROUGH BLOCK CACHE, SWAP
RESERVATION, LOGSTORE RESERVATION, AND COMMON BADMAP TRANSLATION.

D6FS TRUNCATE/SHRINK NOW PRESERVES THE RETAINED EXTENT COUNT WHILE CLEARING
DISCARDED EXTENT SLOTS, AND FILE-BLOCK LOOKUP RETURNS -1 AFTER THE FINAL
EXTENT INSTEAD OF FALLING THROUGH INTO THE NEXT ROUTINE.  THE TARGET
TRUNCATE-TO-ZERO THEN REGROW REGRESSION PASSES; THE PRE-FIX TREE FAILS THE
FIRST O_TRUNC REOPEN.

BLOCKSET PROVIDES GENERIC LOGICAL-TO-PHYSICAL MAPPING FOR MULTI-MEMBER SETS.
SINGLE-MEMBER ROOTS BYPASS ITS MRES.  SEMANTIC OWNERSHIP OF SWAP/LOGSTORE DOES
NOT LIVE IN BLOCKSET.

BADMAP HANDLING IS OPTIONAL.  MEDIA CAN BE CREATED WITHOUT A BADMAP, IN WHICH
CASE NO BADMAP PACKAGE IS INSTALLED FOR THAT MEDIA, AND BADMAP SUPPORT CAN ALSO
BE OMITTED FROM THE KERNEL BUILD WITH KINIT_BADMAP=0.

## 6. DSK270 AND DRM236 SCHEDULING

DSK270 USES ITS BOUNDED PER-UNIT CYLINDER ELEVATOR.  DRM236 USES A MINIMAL
ONE-PENDING FIFO.  DRM SCHEDULING MAKES NO ROTATIONAL OR TIMING ASSUMPTIONS,
BECAUSE THE CURRENT SIMULATOR DOES NOT PROVIDE USEFUL ROTATIONAL DATA.


## 7. TSFS

TSFS V1.1 IS A READ-ONLY DECTAPE FILESYSTEM.  `MOUNT.TSFS` PERFORMS TRANSIENT
MEMBER/TABLE/CHECKSUM/EXTENT VALIDATION AND PASSES A COMPACT HANDOFF TO THE
RESIDENT PROVIDER.  LOOKUP, READDIR, STAT, PARENT TRAVERSAL, MULTI-EXTENT AND
CROSS-MEMBER READS ARE IMPLEMENTED.  ITS EIGHT-WORD FILE RECORD NOW CARRIES
IMMUTABLE MODE12 AND UID9,GID9 WITHOUT GROWING THE RECORD; MKTSFS MANIFESTS
SUPPLY THESE ATTRIBUTES AND TSFSCHECK VALIDATES THE V1.1 FORMAT.  TSFS MTIME
REMAINS 0 BY DESIGN.

REGULAR DATA USES 0400-WORD RESTART EXTENTS.  EACH EXTENT IS D6LZ36 WHEN THAT
SAVES DECTAPE BLOCKS AND STORED OTHERWISE; METADATA REMAINS DIRECT.  TSFS MAY
ALSO SERVE AS THE BOOT ROOT.

## 8. LOGSTORE

LOGSTORE V1 IS COMPLETE.  D6FS PERSISTS THE RAW LOGICAL RESERVATION; KINIT
RECOVERS THE RING and publishes the compact runtime state.  THE RESIDENT MRES
PROVIDES STATUS, BOUNDED RAW I/O, COMMIT-LAST APPEND, AND RACE-SAFE WAIT/WAKE
WITHOUT A PERMANENT 128-WORD RECORD BUFFER.

THE ROOT-ONLY LOGCTL TRANSPORT SERVES `/SYSTEM/EXEC/LOGDRAIN`.  LOGDRAIN
IMPLEMENTS NULL, D6FS FILE/ROTATION, SEVERITY-FILTERED CONSOLE, AND MTC
ARCHIVAL SINKS WITH REDUNDANT PERSISTENT DRAIN STATE, LOSS/GAP ACCOUNTING,
EOT/REEL RESUME, AND FINITE-DRAIN FILEMARK.  INIT AND LOGIN PRODUCE NORMAL
OPERATIONAL RECORDS.

## 9. CLOCK AND REAL-TIME MODE

THE STANFORD PHIL PETIT PCLK AT DEVICE 0730 IS THE STANDARD WALL-CLOCK SOURCE.
GETTIME RETURNS UTC TIME36 FOR 2026-01-01 THROUGH 2125-12-31.  THE APR 60 HZ
LINE CLOCK REMAINS THE MONOTONIC SCHEDULER/TIMER CLOCK.

USERSPACE MAY BLOCK FOR A BOUNDED 17-BIT NUMBER OF APR TICKS THROUGH
SYS_EXT_SLEEP.  THE DEADLINE IS KEPT IN THE SLEEPING PROCESS DESCRIPTOR AND A
SINGLE GLOBAL FRONTIER WORD AVOIDS A PROCESS-TABLE SCAN ON ORDINARY CLOCK
TICKS.
THE 2026-10-05 TIMER COMPACTION PRESERVES THAT FRONTIER ALGORITHM AND THE FULL
17-BIT SLEEP RANGE WHILE REMOVING REDUNDANT CLOCK/DEADLINE RECONSTRUCTION.
PROC_PDP6 DROPPED BY 18 TEXT WORDS; THE LINKED KCORE CHECKPOINT IS 10328 WORDS
AND MULTI_LAST IS 041477, 832 WORD ADDRESSES ABOVE THE 040000 LIMIT.
A FOLLOWING MEMFS NAMESPACE PASS SHARES WRITABLE-PARENT/NAME/CHILD VALIDATION
ACROSS CREATE, UNLINK, AND RENAME.  MEMFS_RUNTIME DROPPED BY 23 TEXT WORDS AND
THE LINKED MEMFS MRES BY 24 WORDS, TO 688.  MULTI_LAST IS NOW 041447, 808 WORD
ADDRESSES ABOVE 040000.
THE FILE BULK-WORD I/O PASS THEN SHARED READ/WRITE DESCRIPTOR VALIDATION AND
RESULT UNWIND WITHOUT ADDING A SUBROUTINE CALL TO THE HOT DATA PATH.
FILE_RUNTIME DROPPED BY 15 TEXT WORDS; KCORE IS NOW 10313 WORDS AND MULTI_LAST
IS 041430, 793 WORD ADDRESSES ABOVE 040000.
D6FS NOW RELIES EXCLUSIVELY ON BCACHE FOR RUNTIME BLOCK IDENTITY.  THE FORMER
PRIVATE READER CACHE TAG HAD BECOME WRITE-ONLY; REMOVING ITS DEAD LOAD/COMMIT
STORES CUTS THE LINKED D6FS MRES BY 6 WORDS, TO 2211.  MULTI_LAST IS 041422,
787 WORD ADDRESSES ABOVE 040000.

THE MINIMAL SINGLE-OWNER REAL-TIME USER PROCESS MODE AND RT_REQUIRED EXEC
ADMISSION ARE IMPLEMENTED.  ACQUIRING RT MODE OR LAUNCHING AN RT_REQUIRED
IMAGE REQUIRES UID 0; DISABLE/YIELD BY THE CURRENT OWNER REMAIN NORMAL OWNER
OPERATIONS.

MULTI-USER ADMINISTRATIVE BOUNDARIES ARE ENFORCED IN THE KERNEL: HALT,
FILESYSTEM MOUNT/UNMOUNT, AND RT ACQUISITION ARE ROOT-ONLY; PROCESS EVENT
DELIVERY REQUIRES THE SAME UID UNLESS THE CALLER IS UID 0; AND ORDINARY USERS
MAY LOWER BUT NOT RAISE THEIR NORMAL SCHEDULER PRIORITY.

THE HIGHEST TWO RUNTIME PROCESS SLOTS ARE RESERVED FOR UID 0.  ORDINARY RUN
ALLOCATION STOPS BEFORE THAT TAIL, WHILE ROOT/INIT MAY USE THE COMPLETE TABLE.
THIS PROVIDES ADMINISTRATIVE HEADROOM WITHOUT A PER-UID QUOTA TABLE OR
ADDITIONAL PER-PROCESS RESIDENT STATE.

## 10. NORMAL USERSPACE

NORMAL BOOT IS KINIT -> `/SYSTEM/INIT` -> LOGIN -> CONFIGURED SHELL.  INIT
READS `/CONFIG/INITTAB`; LOGIN USES `/CONFIG/PASSWD`, ESTABLISHES CREDENTIALS,
SESSION/PROCESS-GROUP/TTY STATE, HOME/ENVIRONMENT, AND EXECUTES THE CONFIGURED
SHELL.

RUN/EXEC USE THE BOUNDED COUNTED-SIXBIT STARTUP ABI.  NORMAL USERSPACE BUILDS
WITH KCC/DAS.

DSH HAS BEEN PORTED ONTO THE CURRENT RUN/EXEC/PROCESS-GROUP ABI, INCLUDING THE
FIXED-ARENA PARSER, FUNCTIONS, SCRIPT/-C EXECUTION, REDIRECTION, CONCURRENT
PIPELINES, BACKGROUND JOBS, AND JOB CONTROL.  CURRENT USERLAND PORT WORK ALSO
RESTORES THE DAIMOSV1 CORE-UTILITY SET USING NATIVE WORD/S6REC INTERFACES.
NATIVE MAKE IS NOW PRESENT AT `/SYSTEM/EXEC/MAKE`: IT USES REAL VFS MTIME36
DEPENDENCY DATA, A BOUNDED RULE/DEPENDENCY/VARIABLE ARENA, NATIVE S6REC
MAKEFILES, AND DSH FOR RECIPE EXECUTION.  TARGET ACCEPTANCE COVERS INITIAL
BUILD, OUTPUT, UP-TO-DATE SUPPRESSION, AND REBUILD SELECTION AFTER TOUCHING A
PREREQUISITE.

THE DAIMOSV1 `DU` COMMAND HAS BEEN PORTED TO THE CURRENT VFS AS A BOUNDED
12-LEVEL WORD-COUNT WALKER.  DAIMOSV1 `PACK` IS NOT PORTED: D6FS V2 REMOVED
THE ONLINE PACKER AND `TOOLS/HOST/PACKFS` IS THE AUTHORITATIVE OFFLINE
COMPACTOR.  DAIMOSV1 `LOGARC` IS ALSO RETIRED WITH ITS OLD SYSLOG ARCHIVE ABI;
CURRENT LOGSTORE USES THE CRASH-PERSISTENT `LOGDRAIN MTC` CURSOR/SINK PATH,
WHICH ADVANCES DRAIN STATE ONLY AFTER A COMPLETE TAPE RECORD IS ACCEPTED.  THE
OLD VOLUME/FILE CATALOG AND SEGMENT FORMAT ARE NOT PART OF CURRENT LOGSTORE.

NATIVE `/OPTION/BASE/EXEC/DARC`, `DLINK`, AND `OBJDUMP` ARE BUILT FROM
`USERLAND/TOOLCHAIN`.  THEIR HOST COUNTERPARTS AND THE OTHER CROSS-DEVELOPMENT
TOOLS LIVE IN `TOOLS/HOST`.  DARC STREAMS DOBJ1 MEMBERS INTO DARC1 OBJECT LIBRARIES;
DLINK IS A FIXED-METADATA, MULTI-PASS STREAMING DOBJ/DARC LINKER AND SUPPORTS
THE USERLAND SELF-HOSTING OPTIONS INCLUDING BASE ADDRESS, MAP OUTPUT, PURE/RT
FLAGS, AND DAIMOS UUO RELAXATION; OBJDUMP INSPECTS DOBJ HEADERS, SYMBOLS,
RELOCATIONS, AND TEXT.  TARGET ACCEPTANCE CREATES A MULTI-MEMBER DARC LIBRARY,
LINKS A NEW DXR AT BASE 020 THROUGH THAT LIBRARY, AND EXECUTES THE RESULT.
THE FORMER `PDP10-TOOLS` AND `DAIMOS-TOOLS` SOURCE TREES ARE NOW INTEGRATED:
`TOOLS/HOST` IS THE SINGLE HOST-SIDE SOURCE TREE AND BUILDS OUT OF TREE UNDER
`BUILD/TOOLS/HOST`, WHILE NATIVE DARC/DLINK/OBJDUMP ARE ORDINARY USERLAND
PROGRAMS.  NATIVE DOBJ SYMBOL SCANS ARE SEQUENTIAL, AND DARC PRE-SIZES ITS
EXACT KNOWN OUTPUT AND USES A 16-WORD INDEX BUFFER, AVOIDING REPEATED D6FS
FILE GROWTH DURING ARCHIVE CREATION.  THE NATIVE LINK-AND-EXECUTE ACCEPTANCE
PASSES WITH THE NORMAL 15-SECOND COMMAND LIMIT.
HOST-ONLY DLINK FOLD/MRES/ABSOLUTE-IMPORT IMAGE-CONSTRUCTION OPTIONS REMAIN
HOST-SIDE BECAUSE THEY ARE NOT REQUIRED BY NATIVE USERLAND SELF-HOSTING.
ORDINARY NON-PURE DXR IMAGES MAY NOW CONTAIN UP TO 0300000 WORDS OF STORED
TEXT+DATA, MATCHING THE 96K-CLASS SELF-HOSTING PROFILE.  THIS IS DISTINCT FROM
THE COMPACT PROCESS-SWAP PURE-TEXT FIELD, WHICH REMAINS 037777 WORDS; AN
OVERSIZED PURE TEXT PREFIX IS REJECTED RATHER THAN GROWING THE ONE-WORD SWAP
RECORD.  NATIVE DLINK STREAMS LARGE OUTPUT AND SIZES ONLY ITS RELOCATION
BITMAP FOR THE 0300000-WORD CEILING.  THE EXEC/PROC_SWAP OBJECT SIZES ARE
UNCHANGED BY THIS LOADER-POLICY EXTENSION.

THE FINAL DAIMOSV1 SOURCE AUDIT FOUND NO REMAINING ACTIVE IMPLEMENTATION
DEPENDENCY.  DMAKE/DSED ARE SUPERSEDED BY MAKE/SED; MINIDIAG IS COVERED BY
THE NORMAL PS/FREE/STAT/LOGSTORE DIAGNOSTICS; PIDCHECK/TTYCHECK WERE TEST
HELPERS; CONFIGFS/PROCFS/STATUSFS/DEVICEFS/MOUNTFS WERE REPLACED BY THE
CURRENT /CONFIG + MONITORFS/VFS DESIGN; AND THE STANDALONE TYPE-136 LAYER WAS
REPLACED BY THE CURRENT SHARED STORAGE ROUTER.  DAIMOSV1 IS THEREFORE RETIRED
AS AN ACTIVE DEVELOPMENT REFERENCE.  ITS ARCHIVE MAY BE KEPT AS HISTORICAL
SOURCE ONLY; EXPLICITLY DEFERRED FEATURES SUCH AS THE SECOND JOYSTICK/SWITCH
INTERFACE REMAIN TRACKED IN THE CURRENT TODO INSTEAD OF DEPENDING ON V1.

THE CURRENT DSH CONFORMANCE BASELINE IS 106/106 TARGET SIMH PROBES.  IT COVERS
THE FROZEN QUOTING/COMMENT RULES, CASE/GLOB, VARIABLES AND STATUS, SCRIPT/-C,
SOURCE FRAME RESTORATION, FUNCTIONS/COMPOUND CONTROL FLOW, STATUS 126/127,
REDIRECTION, PIPELINE DATA/STATUS, BOUNDED FUNCTION LIMITS, AND THE FOUR-SLOT
JOB TABLE INCLUDING STABLE `%N`, DONE RECLAIM, STOPPED BACKGROUND TTY READS,
WAIT/BG RE-STOP, AND EXHAUSTION.  LOGIN `-L` NOW LOADS THE SYSTEM PROFILE
BEFORE THE HOME PROFILE, IGNORES MISSING PROFILES, DIAGNOSES PROFILE ERRORS
WITHOUT ABORTING THE LOGIN SHELL, AND PRESERVES A NONZERO STATUS; NON-LOGIN
SHELLS LOAD NO PROFILE IMPLICITLY.  PIPELINES CREATE ONE PIPE AT A TIME AND
CLOSE PARENT ENDS AS SOON AS EACH STAGE IS LAUNCHED, SO THE BOUNDED EIGHT-STAGE
MAXIMUM FITS THE 16-DESCRIPTOR PROCESS ABI. THE HISTORICAL DSH PIPELINE
ACCEPTANCE PEAK USED 21 MM EXTENT DESCRIPTORS. NESTED NATIVE
DSH -> MAKE -> KCC -> KCPP EXHAUSTS THAT CAPACITY: KCPP'S IMAGE LOADS,
BUT THE SUBSEQUENT 0410-WORD CHILD U-AREA ALLOCATION FAILS WITH
MM_ERR_DESCRIPTORS (-3), EVEN THOUGH CONTIGUOUS CORE REMAINS FREE.
THE TABLE NOW HOLDS 32 DESCRIPTORS, COSTING 22 ADDITIONAL RESIDENT WORDS.
A SMALL DIRECT NATIVE KCC COMPILE PASSES WITH THAT CAPACITY. A SEPARATE
NESTED MAKE -> KCC TEST REACHES THE COMPILER BUT HALTS AT PC 000000;
THIS SECOND REGRESSION REMAINS OPEN, AND FULL NATIVE MAKE ALL IS NOT YET
ACCEPTED. A SEPARATE REAL-DCS FOREGROUND ACCEPTANCE PASSES:
DSH SAVES A STOPPED JOB'S RAW/COOKED TTY MODE, REFRESHES IT AFTER A SECOND
STOP, RESTORES IT BEFORE FG/CONTINUE, RECLAIMS FOREGROUND OWNERSHIP, AND
RESTORES SHELL RAW MODE.  MODE CHANGES OCCUR BEFORE FOREGROUND PGRP HANDOFF,
AS REQUIRED BY THE KERNEL'S FOREGROUND-OWNER TTY_SETMODE RULE.  THE SAME
REAL-DCS ACCEPTANCE NOW COVERS A TWO-PROCESS FOREGROUND PIPELINE STOPPED AND
RESUMED AS ONE PGRP, PLUS DEFAULT `FG` SELECTING THE MOST RECENT OF MULTIPLE
LIVE STOPPED JOBS.  WHILE DSH ITSELF OWNS THE RAW/NO-ECHO EDITOR TTY, CTRL-C
NOW CANCELS THE COMPLETE PARTIAL LOGICAL INPUT AND RETURNS A FRESH PROMPT;
FOREGROUND CHILDREN STILL RUN UNDER THE KERNEL'S COOKED/SIGNAL PATH, SO THEIR
CTRL-C PROCESS-GROUP SEMANTICS ARE UNCHANGED.
NONINTERACTIVE DSH STDIN NOW CONSUMES NATIVE S6REC TEXT RATHER THAN CHARACTER
I/O, SO `DSH < SCRIPT` WORKS ON DAIMOS FILES/PIPES.  EXPLICIT
`BEGIN ... END &` PRESERVES A GROUP AST NODE AND LAUNCHES ONE NONINTERACTIVE
DSH WORKER IN A NEW PGRP; THE PARENT SERIALIZES THE BOUNDED AST INTO NATIVE
S6REC RECORDS OVER A PIPE.  THIS USES THE EXISTING DSH LANGUAGE/RUN ABI, ADDS
NO PERMANENT COMMAND BUFFER OR PRIVATE WORKER PROTOCOL, INHERITS EXPORTED
ENVIRONMENT ONLY, AND KEEPS CHILD SHELL-STATE CHANGES OUT OF THE PARENT.

THE PUBLIC COMMAND NAMESPACE NOW CONTAINS ORDINARY EXECUTABLE FILES.  `LS`,
`DF`, `MAN`, `WHICH`, `LOGD`, AND THE OTHER PUBLIC COMMANDS ARE INDEPENDENT
DXR PROGRAMS UNDER `/SYSTEM/EXEC`; THERE IS NO RUNTIME COMMAND MAP AND NO
PUBLIC-COMMAND MULTIPLEXER.  COMMON IMPLEMENTATION CODE IS A LINK-TIME DETAIL:
THE BUILD PLACES ONE COMMAND BACKEND PER MEMBER IN STATIC DARC ARCHIVES, SO
DLINK EXTRACTS ONLY THE SELECTED COMMAND AND THE COMMON LIBRARY MEMBERS IT
ACTUALLY REFERENCES.  THIS KEEPS SMALL COMMANDS SMALL (FOR EXAMPLE `TRUE` IS
ONLY 000300 WORDS) WHILE PRESERVING A NORMAL EXECUTABLE NAMESPACE.  THE BOOT
IMAGES STORE THE INDIVIDUAL DXRS IN D6LZ FORM; THE TSFS ROOT IMAGE USES THE
SAME STANDALONE PROGRAM MODEL.  DSH, XARGS, AND `WHICH` NOW USE ORDINARY
PATH/FILESYSTEM LOOKUP ONLY.  `/SYSTEM/LIBEXEC` IS RESERVED FOR GENUINELY
PRIVATE HELPERS SUCH AS `DSHCOMP`.  A FUTURE SHARED-LIBRARY ABI MAY REPLACE
THE STATIC ARCHIVES WITHOUT CHANGING THIS COMMAND NAMESPACE.

THE OPTIONAL HANDWRITTEN ASSEMBLER UTILITIES NOW SHIP AS NATIVE-BUILDABLE
SOURCE UNDER `/OPTION/BASE/SOURCE/ASMUTILS`.  THERE IS ONE CANONICAL HOST
`Makefile`; THE BOOT BUILD DERIVES THE NATIVE S6REC `MAKEFILE` BY CHANGING A
RECIPE-LEADING TAB TO `>`, NORMALIZING TO UPPERCASE/SIXBIT-SAFE TEXT, AND
S6REC-ENCODING THE RESULT.  NO PARALLEL NATIVE-ONLY MAKEFILE IS MAINTAINED.
THE SAME FILE USES `DASFLAGS = -B`: HOST DAS AUTO-DETECTS ASCII VERSUS S6REC
SOURCE, WHILE RESIDENT DAS READS THE NATIVE S6REC REPRESENTATION.  -A/-S ARE
ONLY EXPLICIT DAS COMPATIBILITY/DIAGNOSTIC OVERRIDES.  NATIVE `MAKE -C
/OPTION/BASE/SOURCE/ASMUTILS INSTALL` BUILDS SEPARATE DXR PROGRAMS FOR THE
ASSEMBLER UTILITIES AND INSTALLS THEM UNDER `/OPTION/BASE/EXEC`.  DXR IS THE
EXECUTABLE FORMAT RATHER THAN A REQUIRED FILE-NAME SUFFIX; ASMUTILS NOW BUILDS
THE FINAL EXTENSIONLESS COMMAND NAMES THROUGH MAKE'S CLASSIC SINGLE-SUFFIX
`.S:` RULE.  IMAGE-SEEDED SOURCE FILES WITH MTIME 0 ARE TREATED AS UNDATED
PREREQUISITES: THEY BUILD A MISSING/UNDATED TARGET ONCE BUT DO NOT FORCE A
VALID-TIMESTAMP TARGET TO REBUILD FOREVER.  INVALID NONZERO MTIMES REMAIN
CONSERVATIVE, AND `MAKE -B` STILL FORCES REBUILD.  NATIVE PDP-6 ACCEPTANCE
CONFIRMS `ARGS.S` WITH MTIME 0 BUILDS `ARGS` ONCE, AN IMMEDIATE NORMAL SECOND
MAKE IS QUIET, AND `-B -N` SELECTS THE DAS RECIPE AGAIN.

THIS NATIVE MAKE PATH EXPOSED AND FIXED A KCC PDP-6 SIMULATED-ADJBP BUG.  A
PEEPHOLE-FOLDED BYTE POINTER WITH AN INDEX REGISTER WAS PREVIOUSLY HANDED TO
`%ADJBPH` WITHOUT FIRST MATERIALIZING ITS EFFECTIVE WORD ADDRESS.  LOCAL
CHAR-ARRAY WRITES COULD THEREFORE DISAGREE WITH THE POINTER PRODUCED BY ARRAY
DECAY; MAKE'S `$(DAS)` VARIABLE NAME WAS A CONCRETE FAILURE.  KCC NOW
MATERIALIZES THE EFFECTIVE ADDRESS AND THEN APPLIES THE BYTE-POINTER BITS.
THE MINIMIZED PDP-6 TARGET REGRESSION PASSES WITH THE FIX.

SIXMD IS THE AUTHORITATIVE PUBLIC COMMAND DOCUMENTATION SOURCE.  `HELP NAME`
RENDERS COMPACT COMMAND HELP FROM THE NAME/SUMMARY, USAGE, OPTIONS, AND
OPERANDS; `MAN NAME` RENDERS THE COMPLETE MANUAL; `APROPOS WORD` SEARCHES THE
ONE-LINE COMMAND SUMMARY INDEX.  DAIMOS DOES NOT RESERVE A PER-COMMAND `-H`
HELP OPTION.  MISSING OR CORRUPT HELP/MANUAL METADATA FAILS CLEANLY WITH
`HELP NOT AVAILABLE` OR `MANUAL NOT AVAILABLE` AND NEVER PREVENTS THE COMMAND
ITSELF FROM RUNNING.  NORMAL USERLAND BUILDS VALIDATE THAT EVERY PUBLIC
COMMAND HAS A SIXMD MANUAL; THE OPTIONAL ASSEMBLER-UTILITY PACKAGE ENFORCES
THE SAME RULE FOR ALL OF ITS PUBLIC PROGRAMS.

INTERACTIVE DSH NOW HAS A SMALL BOUNDED RAW-TTY EDITOR WITH CURSOR INSERTION,
BOUNDED HISTORY RECALL, CTRL-C LINE CANCELLATION, AND GENERIC COMMAND/PATH TAB
COMPLETION.  COMPLETION POLICY RUNS IN THE PRIVATE `/SYSTEM/LIBEXEC/DSHCOMP`
HELPER SO DIRECTORY/COMMAND-INDEX SCANNING DOES NOT BECOME PERMANENT SHELL
STATE.  REAL-DCS ACCEPTANCE COVERS CTRL-B INSERTION, CTRL-P HISTORY, COMMAND
AND PATH COMPLETION, CTRL-C RECOVERY, AND FOREGROUND JOB TTY RESTORATION.  THE
DSH VARIABLE TABLE NOW HAS 64 FIXED SLOTS INSTEAD OF 16.  EACH SLOT IS 13
WORDS: ONE PACKED METADATA WORD PLUS SIX SIXBIT NAME WORDS AND SIX SIXBIT
VALUE WORDS, PRESERVING THE 36-CHARACTER NAME/VALUE LIMIT WHILE REMOVING THE
SEPARATE USED/EXPORTED/LENGTH WORDS.  THE HOT SET/GET/UNSET/ENUMERATION PATH
IS SMALL HAND-WRITTEN PDP-6 CODE; EXTERNAL-LAUNCH ENUMERATION USES STATIC
SCRATCH SO THE DEEP SOURCE/POSITIONAL PATH DOES NOT LOSE USER-STACK MARGIN.
THE FINAL DSH DXR IMAGE IS 035551 WORDS WITH 007261 WORDS OF BSS, BELOW DSH'S
RETAINED 036000-WORD BUILD ACCEPTANCE CEILING AND SMALLER IN IMAGE TEXT THAN
THE FORMER 035773 BUILD.  P10FOLD IS APPLIED ONLY TO DSH'S HAND-WRITTEN
ASSEMBLER OBJECTS;
FOLDING KCC-GENERATED DSH OBJECTS WAS FOUND TO CORRUPT FOREGROUND EXTERNAL
COMMAND EXECUTION AND IS EXCLUDED.  THE FOLD PLAN DEPENDS ON THE USERLAND
MAKEFILE SO A CHANGE OF FOLD SCOPE CANNOT LEAVE A STALE PLAN IN AN INCREMENTAL
BUILD.  A SECOND LOGIN FAILURE WAS TRACED TO A STALE KCC INSTALLED UNDER THE
DEFAULT `$(HOME)/cross` PREFIX: THAT COMPILER PRODUCED A 036200-WORD DSH,
WHICH DSH'S 036000-WORD BUILD GUARD REJECTS.  THE CURRENT KCC INSTALLED INTO
THAT PREFIX PRODUCES THE VERIFIED 035551-WORD IMAGE.  THE PDP-6 BOOT BUILD
NOW DEPENDS EXPLICITLY ON `dsh_var_pdp6.s` AND USES DXRCHECK TO REJECT AN
OVERSIZED DSH BEFORE IT CAN BE COMPRESSED OR INSTALLED INTO D6FS.  THE 106/106
TARGET SUITE, REAL-DCS FOREGROUND/EDITOR
ACCEPTANCE, AND A DIRECT NORMAL-BOOT ROOT-LOGIN/EXTERNAL-COMMAND SMOKE TEST
PASS WITH THIS REPRESENTATION.

THE INTERACTIVE DSH EDITOR NOW NORMALIZES ASCII LOWERCASE TO UPPERCASE BEFORE
APPLYING THE NATIVE SIXBIT PRINTABLE-RANGE CHECK.  THE PREVIOUS ORDER MADE THE
LOWERCASE CONVERSION UNREACHABLE, SO ORDINARY LOWERCASE INPUT WAS SILENTLY
DISCARDED WHILE ENTER AND CTRL-D STILL WORKED.  THE NORMAL PDP-6 SIMH CONFIG
ALSO SELECTS `SET GE ASCII`; SIMS HAS SUPPORTED THIS TRANSPARENT GE INPUT MODE
SINCE COMMIT 0f0d530, BUT DAIMOS HAD LEFT GE IN THE HISTORICAL LEGACY
TRANSLATION MODE, WHERE ASCII `root` ARRIVED AS `2//4`.  MULTITTY ACCEPTANCE
NOW SENDS LOWERCASE SHELL COMMANDS THROUGH CTY, DCS0, AND GE0 AND PASSES ON
ALL THREE PATHS.

DAS REMAINS AN EXTERNAL SOURCE TREE.  THE PDP-6 BOOT BUILD CONSUMES ITS
RESIDENT DAS/DAS1/DAS2 ARTIFACTS AND AUTHORITATIVE SIXMD MANUAL.  NORMAL
`MAKE BOOT` REGENERATES THE FOUR-MEMBER HDD ROOT SET AND THE SEPARATE AUXILIARY
DRUM MEDIA; IT DOES NOT DUPLICATE THE ROOT FILESYSTEM ON THE DRUM.
THE PUBLIC COMMAND LIVES AT `/OPTION/BASE/EXEC/DAS`; ITS PRIVATE DAS1/DAS2
PHASES LIVE UNDER `/OPTION/BASE/LIBEXEC/DAS` AND ARE INVOKED ONLY BY ABSOLUTE
PATH, SO THEY DO NOT POLLUTE THE COMMAND NAMESPACE.
THE PUBLIC DAS DRIVER AND BOTH PRIVATE PHASES ARE KCC-BUILT.  THE
PEEPHOLE/FOLDING OPTIMIZER IS HOST ONLY AND IS COMPILED OUT OF DAS_NATIVE TO
SAVE RESIDENT CODE AND STATE.  STRUCTURAL PHASE-LIFETIME/PARSER/RUNTIME
SHRINKAGE PLUS KCC SHARED FIXED-FRAME RETURN EPILOGUES PUT DAS1 AT 033244
WORDS (BSS 002032, PROCESS 037276) AND DAS2 AT 027501 WORDS (BSS 002342,
PROCESS 034043), SO BOTH PASS THE 034000-WORD IMAGE
BUDGET AND 040000-WORD PROCESS BUDGET WITH THE STANDARD 02000-WORD STACK.  THE
TRANSITIONAL PDP-10 GCC PHASE BUILD HAS BEEN REMOVED.  NATIVE DAS
ASSEMBLE/VERIFY/EXECUTE ACCEPTANCE PASSES.

THE DAS BRING-UP EXPOSED AND FIXED A KCC CORRECTNESS BUG IN LOOP INDUCTION:
A DERIVED LOCAL-ARRAY POINTER COULD BE SPILLED ON ONLY ONE ARM OF A GENERAL IF
AND RESTORED AT THE JOIN, CORRUPTING THE USER STACK.  KCC NOW RESTRICTS THAT
STRENGTH REDUCTION TO BRANCH-SAFE LOOP BODIES; THE PDP-6/KA10/KI10/KS10
SEMANTIC OPTIMIZER REGRESSION AND THE TARGET DAS REGRESSION PASS.

THE KCC-ONLY DAS2 TRANSITION ALSO EXPOSED AND FIXED AN OBJECT-OPTIMIZER BUG:
NEGATION COULD BE PUSHED BACKWARD THROUGH P_UIDIV, WHICH IS INVALID FOR
UNSIGNED QUOTIENT/REMAINDER.  DAS'S PACKED OPCODE LOOKUP THEN SELECTED JUMPN
0326 INSTEAD OF MOVEI 0201 FOR MNEMONIC INDEX 319.  P_UIDIV NOW BLOCKS THAT
NEGATION TRANSFORM; THE MINIMIZED REGRESSION PASSES ON PDP-6, KA10, KI10, AND
KS10, AND THE FULL TARGET DAS REGRESSION PASSES.

THE BUILD GRAPH NOW TREATS THE SELECTED PDP-10 C COMPILER EXECUTABLE AS AN
INPUT.  REPLACING KCC INVALIDATES KINIT/KCORE BOOT STAMPS, KERNEL C-GENERATED
ASSEMBLY, USERLAND C OBJECTS, AND THE PRIVATE NATIVE DAS PHASE ASSEMBLY.
HANDWRITTEN ASSEMBLER INPUTS DO NOT REBUILD MERELY BECAUSE KCC CHANGED.  THIS

DAIMOS NOW PROVIDES A REAL USER BREAK FACILITY FOR SELF-HOSTING USERLAND.
UUO-077 EXTENSION 052 IMPLEMENTS BRK; LIBC IMPLEMENTS SBRK, MALLOC, CALLOC,
REALLOC, AND FREE ON TOP OF IT.  PDP-6 PROCESS EXTENTS GROW IN 02000-WORD APR
QUANTA; HIGH-PLACED PROCESSES CAN REBASE DOWNWARD WITH AN OVERLAP-SAFE COPY SO
GROWTH DOES NOT NORMALLY REQUIRE TWO COMPLETE PROCESS IMAGES IN CORE.  THE
EXACT FLOOR/CURRENT BREAK PAIR COSTS ONE WORD IN THE STABLE U-AREA AND DOES NOT
GROW THE THREE-WORD RESIDENT PROCESS DESCRIPTOR.  RUN AND BOTH EXEC REPLACEMENT
PATHS INITIALIZE/RESET THE BREAK.  A TARGET SIMH TEST HAS GROWN THROUGH AN APR
BOUNDARY, VERIFIED 9000 ALLOCATED C CHARACTERS, SHRUNK BACK TO THE INITIAL
BREAK, AND EXERCISED SBRK SUCCESSFULLY.
PREVENTS STALE PREVIOUS-COMPILER OUTPUT FROM SURVIVING AN INCREMENTAL BUILD.

## 12. DEFERRED V0.9 NON-GATES

THE FOLLOWING ARE EXPLICITLY NOT PRE-USERLAND V0.9 GATE ITEMS:

- RECOVERY/SINGLE-USER DECTAPE ENVIRONMENT;
- GENERAL USERSPACE OVERLAYS UNTIL REAL WORKING SETS ARE MEASURED;
- LOADABLE/UNLOADABLE/RELOCATABLE RUNTIME MODULES UNTIL AFTER OVERLAYS.


## KPARSE NATIVE COMPILATION INVESTIGATION — 2026-10-10 V1

STATUS: OPEN.  THIS SECTION RECORDS THE REPRODUCED DEBUGGING EVIDENCE, NOT
A CLAIM THAT NATIVE KPARSE NOW COMPLETES OR THAT ITS RUNTIME IMPROVED.

- THE INSTALLED KPARSE.DXR WAS REBUILT BYTE-FOR-BYTE FROM THE CURRENT
  KCC SOURCE TO OBTAIN A TRUSTWORTHY KPARSE.MAP.  THE REPEATED USER PC
  133306-133324 MAPS TO FINDPTR() IN CCKIRWRITE.C.  APR RELOCATION WAS
  CAPTURED AT THE SAME BREAKPOINT: SAVED APR WORD AT 023431, RELOCATION
  BASE 0340000, LOGICAL PC 0133324, PHYSICAL ADDRESS 0473324.
- FINDPTR PERFORMS LINEAR POINTER-IDENTITY SEARCHES FOR KIR GRAPH OBJECTS.
  A REVERSE SCAN, KCC COMMIT A081463, RETAINS DISCOVERY IDS, USES ZERO
  EXTRA LOOKUP RAM, AND REDUCES THE KPARSE IMAGE FROM OCTAL 150027 TO
  150020 WORDS (MINUS SEVEN).  THE HOST IDENTITY TEST AND NATIVE LINK
  PASS.  AN END-TO-END SPEEDUP IS NOT PROVEN; THE 185-SECOND SIMH TEST
  TIMED OUT AFTER KCC000006.  FUTURE OPTIMIZATION MUST ACCOUNT FOR RAM.
- THE EARLIER CPP INCLUDE TEXT-MODE AND SINGLE-PASS S6REC EXPERIMENTS
  WERE REVERTED AFTER NATIVE SIMH VERIFICATION FAILED; DO NOT REAPPLY
  WITHOUT TESTING BINARY INPUT/CCSRC AND THE NATIVE STDIO REWIND PATH.
- LONG COMPILER TESTS MUST NOT USE A SIMH DEBUGGER PROMPT AS A SUCCESS
  SIGNAL.  DAIMOS-TESTKIT'S LONGRUN V2 WAITS FOR THE DSH PROMPT;
  HARNESS V6 SUPPORTS --TIMEOUT-SNAPSHOT (CPU HISTORY, PC, REGISTERS)
  AND RETAINS OUTPUTS UNDER A UNIQUE WORK DIRECTORY.  A CONTROLLED
  FIVE-SECOND TIMEOUT VERIFIED SNAPSHOT CAPTURE AND FAIL REPORTING.
- SEPARATE OPEN GATES: SPURIOUS KPARSE CONSTANT-OVERFLOW DIAGNOSTICS
  AFTER SUCCESSFUL KCPP PREPROCESSING (SEE TODO.MD), AN EARLIER
  INVALID-UUO FAULT NOT YET EXPLAINED, AND INTERMITTENT NATIVE READ
  FAILURES THAT REQUIRE INDEPENDENT REPRODUCTION.  BUILDING FRESH D6FS
  TEST DISKS HAS ALSO EXPOSED POST-FORMAT LEAKED-BLOCK CHECK FAILURES;
  REPAIRS OF ISOLATED TEST MEDIA ARE NOT A SOURCE-LEVEL FIX.
- REPRODUCTION SCRIPT: DAIMOS-TESTKIT/TESTS/VALIDATION/
  DAIMOS-KPARSE-LONGRUN-20261010-V2.SH.  THE DEFAULT LIMIT IS 1200
  SECONDS, NOT A MEASURED COMPILATION DURATION.


## KPARSE KIR STACK CORRUPTION AND FOLLOW-ON ALLOCATION — 2026-10-10 V2

STATUS: THE PREVIOUS INVALID-UUO FAILURE WAS LOCALIZED, AND A BOUNDED-STACK
KIR WALK WAS IMPLEMENTED; THE FULL NATIVE COMPILER BUILD STILL FAILS.

- SIMH BREAK AT THE REAL INVALID-UUO HANDLER, PHYSICAL PC OCTAL 021202,
  CAUGHT AN OPCODE-ZERO TRAP.  THE INSTRUCTION HISTORY SHOWED CCKIRWRITE.C
  ADDSYM() RETURNING THROUGH POPJ 17,0 AT LOGICAL 134341.  ITS SAVED RETURN
  SLOT AT LOGICAL 176203 HELD OCTAL 227277, WHICH POINTED INTO NONCODE
  DATA CONTAINING 000000000402.  THE ACTUAL INCORRECT RETURN WAS THEREFORE
  STACK CONTENT CORRUPTION, NOT A MALFORMED SYSCALL GENERATED AT THAT PC.
- INITIAL USER BRK IS ABOVE THE ALLOCATED IMAGE AND FIXED 02000-WORD STACK
  RESERVATION; GRAPH RECURSION REACHED WELL BEYOND THAT AREA.  HEAP/STACK
  OVERLAP IS A STRONG CAUSAL EXPLANATION, BUT THE PRECISE OVERWRITING STORE
  WAS NOT ISOLATED.  KCC COMMIT DD4ACFE REPLACES RECURSIVE TYPE, SYMBOL,
  AND NODE DISCOVERY WITH A QUEUE WALK IN THE EXISTING IDENTITY ARRAYS.
  TESTKIT COMMIT BC4CB5C VERIFIES 12000 NODES, 9000 SYMBOLS AND 5000 TYPES
  WITH A 64-KB HOST STACK, INCLUDING CYCLES.  NATIVE KPARSE LINKS.
- IN AN ISOLATED NATIVE SIMH BUILD THE PREVIOUS UUO DID NOT RECUR.  KCC
  ADVANCED TO AN EXPLICIT ERROR AFTER 7 MINUTES 13 SECONDS: 'KIR GRAPH
  ALLOCATION/IDENTITY FAILURE: TYPES=6389 SYMBOLS=8192 NODES=296'.
  THE KIR1 EXTERNAL DEFINITION COULD NOT BE WRITTEN; MAKE CORRECTLY FAILED.
  THIS IS NOT A PASSING COMPILER BUILD.  IDENTIFY THE EXACT FAILED ARRAY
  OR MODULE-SYMBOL ALLOCATION AND REDUCE PEAK RAM BEFORE RETESTING.
- BOUNDED DISCOVERY INCREASES THE NATIVE KPARSE TEXT IMAGE FROM OCTAL
  150020 TO 150167 WORDS (+0147 OCTAL, 103 DECIMAL), WITH UNCHANGED BSS.
  IT ALLOCATES NO SEPARATE WORKLIST; THREE INDICES WALK EXISTING VECTORS.
- REPRODUCTION LOG: $HOME/tmp/kcc-kir-queue-simh-20261010-v1/evidence/run/simh.log
  (HOME-RELATIVE LOCAL DEVSPACE PATH; THE FILE IS NOT A REPOSITORY ARTIFACT).

## NATIVE KPARSE KIR IDENTITY AND UNSIGNED TARGET WIDTH — 2026-10-10 V3

STATUS: HOST CROSS-COMPILER AND SPLIT-PHASE REGRESSIONS PASS; NATIVE
SELF-COMPILATION IS NOT YET CONFIRMED COMPLETE.

- THE ORIGINAL INVALID UUO WAS REMOVED BY BOUNDED-STACK GRAPH DISCOVERY.
  A SEPARATE INTRUSIVE RUN-QUEUE SELF-LINK WAS FIXED IN DAIMOS 2162614.
- KIR ADDRESS-SPAN REJECTION WAS INVALID ON THE NATIVE PDP-6 POINTER
  REPRESENTATION: DIAGNOSTICS SHOWED REPEATED REGISTRATION OF THE SAME
  TYPE POINTER.  KCC F93D904 REMOVED NUMERIC NEGATIVE FILTERS AND USES
  EXACT POINTER EQUALITY.  THE NEXT NATIVE RUN HAD 208 PERSISTENT SYMBOLS
  RATHER THAN 17,000+, MATCHING THE HOST CENSUS, BUT STILL REPORTED
  65,536 NODES AND 25,192 TYPES AFTER ARRAY INITIALIZER ERRORS.
- HOST SPLIT KCPP/KPARSE ON CC.C PRODUCES 210 GRAPHS, AT MOST 55 SYMBOLS,
  36 TYPES AND 982 NODES PER GRAPH; THE LATEST HOST RUN HAS 207 PERSISTENT
  SYMBOLS AND MATCHES THE PRIOR HOST KIR OUTPUT BYTE FOR BYTE.
- KCC 7F6B59D RESTRICTS HOSTED UNSIGNED CONSTANT FOLDING TO TARGET 36-BIT
  WORDS, AND CORRECTS PIZARRAY'S UNSIZED-ARRAY LIMIT TO 0377777777777.
  HOST TESTS CHECK RIGHT SHIFTS, DESIGNATED INITIALIZERS, AND THE ACTUAL
  CCDECL ASSEMBLY.  TESTKIT F20525A AND CEB7A85 PROVIDE THESE REGRESSIONS.
- THE EARLIER LONGRUN V3 SCRIPT COULD PASS WITHOUT COMPILING BECAUSE IT
  REQUESTED OBSOLETE B/CC-CPP.S.  TESTKIT LONGRUN V4 INSTEAD INVOKES
  MAKE -C /OPTION/SOURCE/KCC -F /OPTION/SOURCE/KCC/DAIMOS.MK
  BUILD/CC-CPP.S, REQUIRES KCC OUTPUT AND REJECTS UNKNOWN TARGETS,
  RECIPE FAILURES, BAD ARRAY DESIGNATORS AND INVALID UUOS.
- A REAL NATIVE SIMH V4 RUN IS IN PROGRESS AT
  $HOME/tmp/daimos-array-bound-verified-20261010-v3/evidence/run/.
  NO NATIVE SUCCESS CLAIM SHOULD BE MADE WITHOUT ITS FINAL RESULT.
  THE INDEPENDENT MKD6FS POST-FORMAT LEAK REMAINS UNFIXED; ISOLATED
  TEST-DISK REPAIR DOES NOT COUNT AS A SOURCE-LEVEL FIX.

## 2026-10-12 GCC / ROOT-SELECTED DECtape FILESYSTEM CHECK

- GCC 3.2 WITH -Os CAN COMPILE AND LINK KINIT/KCORE FOR THE PDP-6.  THE
  INSTALLED GCC PDP10-SIXBIT.H SHADOWS THE PROJECT HEADER, SO A PRIVATE
  INCLUDE DIRECTORY WITH THE CURRENT SIXBIT PACKERS WAS USED FOR THIS TEST.
- DTFS AND TSFS STILL SHARE ONE MRES PACKAGE.  EXPLICIT ROOT=DISK AND
  ROOT=DRUM IMAGES NOW OMIT THAT PACKAGE ENTIRELY.  ROOT=AUTO AND ROOT=TAPE
  RETAIN IT SO DECtape BOOT REMAINS POSSIBLE; AT RUNTIME DTFS_MINIT ONLY
  INSTALLS THE PACKAGE FOR A SELECTED DTC ROOT.  THIS TEMPORARILY REMOVES
  BOTH DECtape FILESYSTEM PROVIDERS FROM D6FS-ROOTED SYSTEMS.
- GCC -Os D6FS-ONLY LINKED KCORE+MRES=15412 DECIMAL WORDS, PERMANENT
  SINGLE-ROOT=15459 AND MULTI-ROOT=15492 (LIMIT 16384).  GCC DISK-ROOT
  AND TAPE-ROOT KINIT DXR LINKS PASS.  A BOOT/RUNTIME PASS HAS NOT BEEN
  ESTABLISHED; DSH IMAGE SIZE AND OTHER BASELINE BUILD FAILURES BLOCKED IT.
- THE 2026-10-12 DAIMOS-TESTKIT FULL REGRESSION SWEEP COMPLETED WITH
  30 FAILED ROWS AND 2 PASSING ROWS.  HOST COVERAGE, TEST-STUB SYMBOLS,
  P10FOLD SIZE PARSING, AND BUILD DEPENDENCY/IMAGE CREATION ARE AMONG
  THE FAILURES.  THESE ARE NOT GCC RUNTIME TEST RESULTS.  SEE
  $HOME/tmp/daimos-userland-regression-20261012-v1.log.

## 2026-10-12 REGRESSION TEST REPAIRS (PARTIAL)

- MAKE DEPENDENCIES FOR MKBOOTBANNER, P10FOLD AND THE NATIVE DAS KCC
  COMPILER NOW RESOLVE THEIR EXECUTABLE PATHS WITH COMMAND -V INSTEAD OF
  TREATING A BARE COMMAND NAME AS A SOURCE FILE.  THIS REMOVES A COMMON
  EARLY BUILD BLOCKER ACROSS THE SIMULATOR MATRIX.
- DAIMOS-TESTKIT NOW FIXES DUPLICATE FILE TARGET STUB DEFINITIONS AND
  MISSING WCNSLS MODULE RUNTIME STUBS.  THE DIRECT TARGET SUITE PASSES.
  INSTALL/KILL NEGATIVE PROBES PROVIDE HOST COMMAND-COVERAGE ENTRIES.
- THE P10FOLD ASSERTION NOW ALLOWS INDEPENDENT MRES FOLD SAVINGS IN
  ADDITION TO KCORE; 65 KCORE WORDS AND 81 TOTAL WORDS WERE MEASURED.
  PERMANENT-SIZE ACCEPTANCE NOW MEASURES ROOT=DISK, WHICH PASSES WITH
  15644 DECIMAL WORDS ON THE PRESENT KCC INSTALLATION.
- BOOT-SOURCE MATRIX REACHED THE DSH EXEC IMAGE GUARD BUT FAILED:
  KCC-BUILT DSH IMAGE 036523 OCTAL EXCEEDS 036000.  DO NOT RAISE THE
  LIMIT TO MAKE THE TEST PASS.  DSH SIZE OPTIMIZATION IS NOT FINISHED;
  THIS FAILURE IS EXPECTED AND DEFERRED, NOT A TESTKIT DEFECT.
  NO COMPLETE BOOT/RUNTIME PASS YET.

## 2026-10-12 FULL REGRESSION RESWEEP

- FRESH DAIMOS-TESTKIT 32-ROW MATRIX WITH THE BUILD DEPENDENCY AND
  TARGET-STUB FIXES: 4 PASS (001 HOST, 002 TARGET, 024 LOGSTORE,
  032 TOOLS), 28 FAIL.  THE FAILURES CANNOT ALL BE ATTRIBUTED TO DSH.
- MOST SIMULATOR ROWS HIT THE EXPECTED UNOPTIMIZED DSH IMAGE LIMIT
  (036523 > 036000 OCTAL).  KEEP THIS GATE AS IS UNTIL DSH OPTIMIZATION.
- AN INDEPENDENT MKD6FS POST-FORMAT D6FSCK FAILURE REPEATED THROUGH THE
  BOOT/FILESYSTEM/SIMULATOR ROWS.  ROOT CAUSE WAS A STALE INSTALLED
  /USR/LOCAL/BIN/MKD6FS FROM OCTOBER 8, PREDATING THE COMMITTED
  D6M_RUN_FSCK EXECVP FIX.  REBUILDING/INSTALLING THE CURRENT MKD6FS
  AND D6FSCK MADE BOTH DISK AND DRUM POST-FORMAT CHECKS PASS.
- THE SWAP WAIT/STRESS TARGETS ALSO LACKED THE TESTKIT'S DEFAULT DAS_REPO;
  ADDING ITS SIBLING CHECKOUT DEFAULT EXPOSED THE EXPECTED DSH BUILD GATE.
- D6FS REMOUNT'S SOURCE EXTRACTOR HAD STARTED INCLUDING VFS_SHUTDOWN_ROOT;
  RESTRICTING IT TO VFS_REMOUNT RESTORED ITS STANDALONE PASS.
- LOG: $HOME/tmp/daimos-regression-20261012-v2.log; ROW LOGS:
  $HOME/tmp/daimos-regression-20261012-v2.logs/.
- MIXED BOOT=DSK, ROOT=DRUM PREVIOUSLY BUILT A BLANK AUXILIARY DRUM,
  SO KINIT COULD NOT FIND /SYSTEM/INIT AND HALTED ?RT.  ROOT=DRUM NOW
  ALWAYS BUILDS POPULATED D6FS DRUM MEDIA, INDEPENDENT OF STAGE1 DEVICE.
  DIAGNOSTIC ROOT-LOGIN LIFECYCLE PASSED DISK AND DRUM PHASES.
- TEMPORARY DSH_IMAGE_LIMIT=037000 IS USED ONLY FOR FURTHER DIAGNOSTIC
  RUNTIME ISOLATION; THE DEFAULT ACCEPTANCE LIMIT STAYS 036000.
  THE SINGLE-MEMBER TSFS ROOT IMAGE STILL FAILS MKTSFS CAPACITY WITH THE
  CURRENT USERLAND PAYLOAD.  THIS IS NOT YET PROVEN TO BE DSH-ONLY.
- DIAGNOSTIC MATRIX ($HOME/tmp/daimos-regression-diagnostic-20261012-v3.log)
  WAS STOPPED BEFORE ROW 017 COMPLETED AFTER CHECKING ROWS 001..016.
  TEN ROWS PASSED: 001,002,003,009,010,012,013,014,015,016.
  SIX FAILED: 004..008 AND 011.  THIS IS NOT A FULL ACCEPTANCE RESULT.
  ROW 011'S ONLY INITIAL FAILURE WAS AN OBSOLETE 0407-WORD U-AREA
  EXPECTATION; THE FOCUSED FIX NOW PASSES FOR 0410 WORDS, INCLUDING THE
  BREAK STATE AT 0407 AND SWAP BACKING AT 0406.
- ROW 004 STILL HAS AN OUTDATED DSK-ROOT-DISCOVERY FIXTURE WHICH EXPECTS
  DSK1..3 UNATTACHED, IN CONFLICT WITH THE DEFAULT FOUR-MEMBER ROOT.
  THE OTHER BOOT CHECKS IN THAT ROW PASSED.  ROWS 005..008 ARE BLOCKED
  IN PART BY THE ONE-MEMBER TSFS MEDIA CAPACITY FAILURE; DISK/DRUM
  ROOT CLASS, CROSS-BOOT, LOGIN, AND COMPRESSED-USERLAND SUBTESTS PASS.

## 2026-10-12 INDEPENDENT REGRESSION CONTINUATION

- DSH IMAGE SIZE AND SINGLE-MEMBER TSFS CAPACITY ARE EXPLICITLY DEFERRED.
  KEEP THE DEFAULT DSH LIMIT 036000 AND EXISTING TSFS CAPACITY UNCHANGED.
  TEMPORARY MAKEFLAGS=DSH_IMAGE_LIMIT=037000 WAS USED ONLY TO DIAGNOSE
  OTHERWISE BLOCKED TESTS.
- DSK-ROOT-DISCOVERY FIXTURE NOW CHECKS ALL FOUR CONFIGURED DSK MEMBERS
  AND ITS FULL SIMULATOR LOGIN/MOUNT REGRESSION PASSES.  THE OLD
  SINGLE-MEMBER ATTACHMENT ASSUMPTION WAS INVALID.
- 12-ROW NON-TSFS DIAGNOSTIC MATRIX ROWS 017-023,025-026,028-030:
  018 STACK, 019 IPC, 020 SWAP, 021 UUO ISOLATION, 022 TRAP COUNTS,
  AND 028 KERNEL/STANDALONE PASSED (SIX).  017 SCHEDULER, 023 STORAGE,
  025 DSK270, 026 DRM236 AND 029 D6FS RECOVERY FAILED (FIVE).
  ROW 030 REPORTED RUNTIME FAILURES BUT WAS STOPPED BEFORE IT FINISHED;
  DO NOT COUNT IT AS A COMPLETED ROW OR ACCEPTANCE PASS.
- SCHEDULER: APR SIMH, MULTIPROCESS, RT REVOKE, RT ADMISSION AND SLEEP
  SUBTESTS PASS.  AAP APR PASSES ON FOCUSED RETEST AFTER SETTING THE
  DEFAULT AAP PDP6 CHECKOUT.  RT YIELD DID NOT SCHEDULE THE NORMAL
  PROCESS BEFORE RESUME.  THE TIMER-FRONTIER FIXTURE NOW LINKS AGAINST
  THE PRODUCTION ASSEMBLER ABI BUT FAILS ITS STOPPED-TIMER ASSERTION
  (__TEST_EXIT=3); DO NOT ALTER THE EXPECTATION WITHOUT ROOT-CAUSE ANALYSIS.
- STORAGE: NINE SUBTESTS PASS, BUT BADMAP-ROOT FAILS WHEN D6SWAP REPORTS
  INVALID BADMAP METADATA.  DSK270-ONLINE ALSO REPORTS INVALID BADMAP
  METADATA AND CANNOT VERIFY RECOVERABLE DIRTY ROOT.
- DRM236: CAPACITY PASSES.  ONLINE NO LONGER FAILS FOR A MISSING SIMH
  VARIABLE, BUT FAILS WITH DRUM READ ACCOUNTING COUNT (FAIL_ID=021).
  MINIT FAILS THE PERMANENT-SIZE GATE FOR ITS CONFIGURATION.  STAGE1
  TEST EXPECTS A LEGACY DR0 MEDIA PATH AND FAILS.
- D6FS RECOVERY: DCS0 TRANSPORT RESET DURING SIMULATOR TEST; UNKNOWN
  WHETHER CAUSE IS BOOT OR TRANSPORT.  RUNTIME ROW 030 ALSO HAS
  STAT/DF/MONITORFS AND NATIVE DTFS MOUNT ASSERTION FAILURES; ITS
  MULTI-TTY TEST DID NOT REPROMPT AFTER REMOTE CARRIER.
- RETAIN ROW LOGS AT $HOME/tmp/daimos-independent-20261012-v1.logs/
  AND RUNNER LOG $HOME/tmp/daimos-independent-20261012-v1.log.

## 2026-10-12 BADMAP / DSK270 / TIMER REGRESSION REPAIRS

- THE BADMAP-ROOT TEST HAD TWO OBSOLETE ASSUMPTIONS: IT OPENED THE CURRENT
  FOUR-MEMBER D6FS ROOT WITH -N 1, AND CHOSE THE *LAST* MKD6FS DIAGNOSTIC,
  WHICH BELONGED TO THE EMPTY AUXILIARY DRUM (SUPER=0/1).  THE TEST NOW
  SELECTS THE DISK FORMAT AND MAPS EACH LOGICAL SUPERBLOCK TO ITS OWN DSK
  MEMBER/PHYSICAL SECTOR.  HOST D6FSCK AND SIMULATOR RECOVERY THROUGH BADMAP
  TO LOGIN BOTH PASS AFTER THE ORIGINAL SUPERBLOCK SECTORS ARE DESTROYED.
- DSK270-ONLINE NOW OPENS THE FOUR-MEMBER SET WITH -N 4.  ORDERLY HALT
  PUBLISHES CLEAN D6FS (VFS_SHUTDOWN_ROOT), NOT DIRTY.  THE TEST NOW REQUIRES
  A CLEAN, CONSISTENT POST-HALT ROOT AND PASSES TARGET READ/WRITE AND D6FSCK.
- PROC_TIMER_SERVICE KEPT THE FRONTIER CLOCK/BASE IN AC6/AC7 WHILE CALLING
  PROC_RUNQ_ADD, WHICH CLOBBERS BOTH.  THE KERNEL NOW SAVES/RESTORES THESE
  TWO ACS ACROSS RUN-QUEUE INSERTION.  THE DIRECT TIMER-FRONTIER TEST PASSES
  DUE/FUTURE, STOP, WRAPAROUND, AND QUEUE CHAINING.  KCC -O D6FS-ROOT
  PERMANENT SIZE = 15648 DECIMAL WORDS, BELOW THE 16384-WORD LIMIT.
- RTCTL YIELD REMAINS OPEN: THE SIMH RT REGRESSION DOES NOT OBSERVE PEER
  EXECUTION BEFORE OWNER RETURN; A TEMPORARY CURSOR-PRIORITY EXPERIMENT DID
  NOT CORRECT THIS AND WAS REVERTED.  TIMER-FRONTIER FIX IS RETAINED.
  DO NOT WEAKEN YIELD ACCEPTANCE OR CLAIM THE ENTIRE SCHEDULER ROW PASSES.
- DSH EXECUTABLE SIZE AND SINGLE-MEMBER TSFS CAPACITY REMAIN DEFERRED;
  PRODUCTION ACCEPTANCE LIMITS ARE UNCHANGED.  TARGETED SIMH TESTS USED
  TEMPORARY DSH_IMAGE_LIMIT=037000 TO ISOLATE UNRELATED FAILURES.

## 2026-10-12 DRM236 REGRESSION CLOSURE

- ALL FOUR DRM236 TARGETS NOW PASS TOGETHER: CAPACITY, ONLINE TYPE-167/236
  DMA AND QUEUED I/O, MINIT/MRES, AND DIRECT DRM STAGE1 TO LOGIN WITH
  DSK0 UNATTACHED.
- REAL DRIVER DEFECT: DRM236_IO.S NEVER UPDATED MFSDEV_DRM_READS OR
  MFSDEV_DRM_WRITES.  BOTH COUNTERS WERE ZERO DESPITE SUCCESSFUL BLOCK
  TRANSFERS.  COMPLETION ACCOUNTING NOW INCREMENTS ONCE AT EACH SUCCESSFUL
  POLLED OR PI2 DRUM COMPLETION (NOT AT THE INTERMEDIATE DP INTERRUPT OR
  WHEN ENQUEUING A REQUEST).  FAILED TRANSFERS INCREMENT THE EXISTING
  DRM STORAGE ERROR COUNTER.  NO NEW KCORE STATE WORDS ARE ALLOCATED.
- TESTKIT MINIT NOW MEASURES AN EXPLICIT ROOT=DRUM PROFILE INSTEAD OF
  AUTO-ROOT INCLUDING THE UNNEEDED TSFS PACKAGE.  THE STAGE1 TEST NOW
  CHECKS THE FOUR-MEMBER DRUM D6FS IMAGE AND REAL BOOT, NOT A REMOVED
  MEDIA/DR0.DRM PATH OR BYTE EQUALITY WITH A LARGER DISK IMAGE.
- LOG: $HOME/tmp/daimos-drm-regression-20261012-v3.log.
- FINAL TARGETED MATRIX RERUN: STORAGE ROW 023, DSK270 ROW 025, AND DRM236
  ROW 026 ALL PASS (3/3) ON THE CURRENT SOURCES WITH THE DIAGNOSTIC
  DSH_IMAGE_LIMIT=037000.  THE NORMAL DSH LIMIT IS STILL 036000.
  LOG: $HOME/tmp/daimos-repaired-storage-20261012-v1.log.
- DISK-ROOT PERMANENT-SIZE RECHECK AFTER DRM ACCOUNTING: 15663 DECIMAL
  WORDS, 721 WORDS BELOW THE 16384-WORD ADDRESS LIMIT.

## 2026-10-12 D6FS HDD RECOVERY FOLLOW-UP

- THE RECOVERY TESTKIT FIXTURE BUILDS A PRIVATE ONE-MEMBER D6FS IMAGE,
  BUT WAS STILL ATTACHING DSK1..3 FROM THE PRODUCTION FOUR-MEMBER IMAGE.
  THE PRIVATE BOOT CONFIGURATION MUST OMIT THOSE EXTRA ATTACHMENTS.
- EVEN WITH THAT FIXTURE CORRECTION, THE RECOVERY ACCEPTANCE IS NOT GREEN.
  PRESERVED SIMULATOR TRANSCRIPT FROM THE ONE-MEMBER TEST SHOWS DAIMON,
  DSK270, AND D6FS INITIALIZATION SUCCEEDING, FOLLOWED BY
  `HALT ... (MUUO114 7,10(16))` BEFORE INIT/LOGIN.
  THE DCS0 SOCKET RESET IS A CONSEQUENCE OF THE SIMULATOR HALT, NOT AN
  INDEPENDENT TRANSPORT ROOT CAUSE.  DETERMINE WHY THE PRIVATE ONE-MEMBER
  BOOT/USERLAND FIXTURE EXECUTES AN INVALID MONITOR UUO BEFORE REPAIRING
  THE RECOVERY ACCEPTANCE.  DO NOT MARK THIS TEST PASSED.
- DIAGNOSTIC LOG: $HOME/tmp/daimos-d6fs-recovery-debug-20261012-v2.log;
  PRESERVED SIMULATOR TRANSCRIPT UNDER
  $HOME/tmp/daimos-d6fs-recovery-v1-28068/run/simh.log.
- UNCORRUPTED CONTROL: A SEPARATE DIAGNOSTIC RUN LEFT BOTH D6FS
  SUPERBLOCKS UNMODIFIED AND STILL HALTED AT THE SAME `MUUO114 7,10(16)`
  BEFORE LOGIN.  THUS SUPERBLOCK CORRUPTION IS NOT THE CAUSE OF THIS
  BOOT/USERLAND FAILURE.  CONTROL TRANSCRIPT:
  $HOME/tmp/daimos-d6fs-recovery-v1-1078/run/simh.log.

## 2026-10-12 SINGLE-MEMBER RECOVERY CONTROL FOLLOW-UP

- THE RECOVERY IMAGE WAS MISSING /SYSTEM/EXEC/MOUNTALL, WHICH CURRENT
  /SYSTEM/INIT ATTEMPTS TO EXEC BEFORE STARTING LOGIN.  THE TESTKIT NOW
  INCLUDES THE REQUIRED EXECUTABLE; THIS IS A NECESSARY FIXTURE UPDATE,
  NOT YET A RECOVERY PASS.
- A FRESH UNCORRUPTED ONE-MEMBER CONTROL WITH MOUNTALL INSTALLED STILL
  HALTS AFTER D6FS MODULE INITIALIZATION AT `MUUO114 7,10(16)` BEFORE
  INIT'S BANNER.  THE FILESYSTEM FORMATTER AND HOST D6FSCK PASS.
  THE ONE-MEMBER ROOT MASK IS SUPPORTED BY D6FS_BOOT_SELECT; AN UNSUPPORTED
  SINGLE-MEMBER ROOT HAS NOT BEEN ESTABLISHED AS THE CAUSE.
- PRESERVED LOG: $HOME/tmp/daimos-d6fs-recovery-v1-23953/run/simh.log;
  WRAPPER LOG: $HOME/tmp/daimos-d6fs-recovery-clean-mountall-20261012-v1.log.
  CONTINUE BY COMPARING FIRST USER EXEC/BOOT CONTEXT WITH THE PASSING
  FOUR-MEMBER BOOT, NOT BY SUPPRESSING THE HALT OR THE RECOVERY ASSERTIONS.
- RT YIELD IS SEPARATELY OPEN.  DSH SIZE AND TSFS CAPACITY ARE DEFERRED.

## 2026-10-12 TSFS-ROOT D6FS ELIMINATION

- ROOT=TAPE / BOOT=DTC BUILDS NOW OMIT THE D6FS MRES PACKAGE AND D6FS
  MINIT ENTIRELY.  THE KINIT ROOT SELECTOR CANNOT PROBE/MOUNT D6FS,
  INCLUDING WHEN DSK/DRM HARDWARE IS ATTACHED.  DISK AND DRUM CONTROLLERS
  REMAIN AVAILABLE FOR RAW SWAP, LOGSTORE AND MEMFS AUXILIARY BACKING.
  TSFS-ONLY BUILDS NOW OMIT THE UNUSED BLOCKSET AND BADMAP MRES PACKAGES,
  SINCE ONLY ONE RAW AUXILIARY DISK/DRUM IS NEEDED.
- IN THE MIXED ROOT=AUTO PROFILE, D6FS MINIT REFUSES TO INSTALL ITS
  PACKAGE IF ROOT SELECTION CHOSE DTC/TSFS.  MIXED AUTO PROFILES STILL
  INCLUDE BOTH PACKAGES IN THEIR MAXIMUM STATIC SIZE REPORT; THE AUTO
  PROFILE IS NOT YET UNDER THE 16K MAXIMUM AND IS NOT CLAIMED GREEN.
- THE TSFS-SPECIFIC RESIDENT SIZE REPORT NOW EXCLUDES D6FS AND COUNTS
  THE RETAINED RAW AUXILIARY MODULES.  ISOLATED PDP-6 KCC BUILD/LINK:
  KCORE+MRES 15322 DECIMAL WORDS; WITH BLOCKSET 15355; LAST ADDRESS
  15402; 981 FREE ADDRESSABLE WORDS BELOW 040000 OCTAL.
- DAIMOS-TESTKIT TARGET `TEST-DAIMOS-TSFS-ROOT-NO-D6FS` PASSES.
  DISK-ROOT PERMANENT-SIZE GATE ALSO PASSES (15663 RESIDENT WORDS).
  LOG: $HOME/tmp/daimos-tsfs-and-disk-regression-20261012-v1.log.
- THE SINGLE-MEMBER TSFS MEDIA-CAPACITY ISSUE REMAINS DEFERRED AND ITS
  CAPACITY LIMIT UNCHANGED.  DSH IMAGE-SIZE OPTIMIZATION IS ALSO DEFERRED.

## 2026-10-12 D6FS RECOVERY RECHECK AFTER DSH WORK

- FETCHED AND FAST-FORWARD PULLED THE CURRENT DAIMOS AND DAIMOS-TESTKIT
  ORIGIN/MAIN.  THE NEW COMMITS ADDED DOCUMENTATION AND SEPARATE D6FS
  HOST REGRESSIONS; THE CORE BOOT/EXEC/PATHNAME FILES WERE UNCHANGED.
- PRESERVED THE SIX-FILE UNCOMMITTED RECOVERY DIAGNOSTICS IN A VERSIONED
  PATCH/STASH AND REMOVED THEM FROM THE WORKTREE BEFORE REBUILDING.
- A FRESH ISOLATED PDP-6 BUILD REACHED LOGIN WITHOUT THE EARLIER MUUO114
  HALT.  THEREFORE THE PREVIOUS BOOT FAILURE IS NOT REPRODUCIBLE ON THE
  CLEAN CURRENT TREE; INTERFERENCE FROM SHARED/STALE BUILD ARTIFACTS IS
  SUSPECTED BUT NOT CONCLUSIVELY PROVEN.
- THE RECOVERY TEST STILL FAILED SIX RUNTIME ASSERTIONS INITIALLY BECAUSE
  ITS PRIVATE IMAGE INSTALLED DSH BUT NONE OF THE EXTERNAL COMMANDS USED
  BY ITS PROBES.  "DSH: NOT FOUND" HAD ALSO ALLOWED SOME NEGATIVE PROBES
  TO PASS SPURIOUSLY.  THE TESTKIT NOW INSTALLS THE NINE REQUIRED PROGRAMS
  (TOUCH, STAT, ECHO, CAT, CP, MV, MKDIR, RM, LS) AND ALL 16 PROBES
  EXPLICITLY REJECT MISSING-COMMAND DIAGNOSTICS.
- THE FULL D6FS RECOVERY REGRESSION NOW PASSES 16/16 PDP-6 USERLAND PROBES,
  VERIFIES THE BOOTSTREAM DID NOT CHANGE, AND REJECTS THE SUBSEQUENT DIRTY
  REBOOT WITH ?RT.  TEST EXIT CODE 0.  THE EARLIER RECOVERY-FAIL STATUS
  ABOVE IS SUPERSEDED; NO DAIMOS KERNEL SOURCE CHANGE WAS NEEDED.
- ISOLATED ACCEPTANCE LOG AND REPORT:
  $HOME/tmp/daimos-recovery-recheck-20261012-v2/run.log
  $HOME/tmp/daimos-recovery-recheck-20261012-v2/report.md
- RT YIELD REMAINS SEPARATELY OPEN.  DSH IMAGE SIZE AND SINGLE-MEMBER
  TSFS CAPACITY REMAIN DEFERRED WITH THEIR PRODUCTION LIMITS UNCHANGED.

## 2026-10-12 RTCTL YIELD REGRESSION CORRECTION

- THE RT YIELD ACCEPTANCE FAILURE WAS CAUSED BY AN INVALID TEST ASSERTION,
  NOT BY A VERIFIED KERNEL/ASSEMBLER BUG.  THE OLD PROBE REQUIRED B/C PEER
  STARTUP MARKERS (ALREADY EMITTED WHILE THE OWNER WAS BLOCKED) OR FULL
  PEER WORKLOAD COMPLETION BEFORE THE YIELDING OWNER RETURNED.
- THE UNMODIFIED PRODUCTION SCHEDULER WAS REBUILT AND TESTED WITH PERIODIC
  PEER-PROGRESS MARKERS.  THE CONTROL SHOWED 156 NORMAL-PEER PROGRESS EVENTS
  AFTER THE OWNER'S W MARKER AND BEFORE ITS YIELD-RETURN Y MARKER.  THIS
  SATISFIES THE DOCUMENTED CONTRACT: A NORMAL PEER MUST EXECUTE, BUT NEED
  NOT FINISH ITS CPU-BOUND JOB BEFORE YIELD RETURNS.
- THE TESTKIT RT TEST NOW EMITS BOUNDED P MARKERS AFTER PEER WORK CHUNKS
  AND REQUIRES A P BETWEEN W AND Y.  THE DIRECT REAL PDP-6 SIMH REGRESSION
  PASSES ON UNCHANGED PRODUCTION SCHEDULER CODE; EXPERIMENTAL CURSOR/
  RESCHEDULER/PI TRACE CHANGES WERE NOT RETAINED.
- LOG: $HOME/tmp/daimos-scheduler-realtime-20261012-v3.log.
- SINGLE-MEMBER TSFS CAPACITY AND DSH SIZE OPTIMIZATION REMAIN DEFERRED.

## 2026-10-12 SCHEDULER ROW 017 FOLLOW-UP

- RE-RAN ALL TEN CONSTITUENT TESTS IN THE SCHEDULER MATRIX WITH THE
  CORRECTED RT YIELD PROBE.  NINE PASS: APR SIMH PROTECTION AND NXM/PDL
  INJECTION, AAP APR, MULTIPROCESS SCHEDULING, RT YIELD, RT REVOCATION,
  RT-REQUIRED ADMISSION, SLEEP, AND TIMER FRONTIER.
- ROW 017 IS NOT FULLY GREEN: THE SEPARATE 32K LOWMEM BOOT TEST ACCEPTS
  ROOT CREDENTIALS BUT REPORTS `LOGIN: EXEC FAILED` INSTEAD OF STARTING
  DSH.  THE EXACT CAUSE HAS NOT BEEN PROVEN; THE ONGOING/DEFERRED DSH
  EXECUTABLE-SIZE WORK IS A POSSIBLE CONTRIBUTOR.  DO NOT MISATTRIBUTE
  THIS FAILURE TO RT YIELD, AND DO NOT RELAX THE LOWMEM TEST.
- LOG: $HOME/tmp/daimos-scheduler-matrix-20261012-v1/logs/017.log.

## 2026-10-12 TSFS COMPACT ROOT MEDIA AND RAW BACKING

- TSFS ROOT DEPLOYMENT NO LONGER CREATES /PICTURES, IMPORTS STAGED
  PICTURE FILES, OR DEPENDS ON PICTURE_FILES.  THE D6FS ROOT IMAGE
  CONTINUES TO INCLUDE THE PREVIOUS /PICTURES PAYLOAD UNCHANGED.
- A REAL MKTSFS BUILD NOW PACKS THE ONE-MEMBER TSFS ROOT SUCCESSFULLY
  WITHOUT THE ~248 KIB OF STAGED PICTURE FILES.  OUTPUT IMAGE SIZE WAS
  ABOUT 578 KIB.  THIS SOLVES THE PREVIOUS MKTSFS PACKING ERROR FOR THE
  TESTED COMPACT CONTENT; NO TAPE GEOMETRY/CAPACITY LIMIT WAS INCREASED.
- ROOT=TAPE NOW ALSO EXCLUDES BLOCKSET MINIT/MRES AND BADMAP MINIT/MRES.
  AUXSTORE DISCOVERY, SWAP, LOGSTORE AND MEMFS BACKING USE RAW DSK/DRM0
  DIRECTLY.  ROOT=DISK/DRUM/AUTO PROFILES RETAIN BLOCKSET/BADMAP.
  THE PREVIOUS TSFS SINGLE-ROOT SIZE REPORT ALREADY OMITTED 33 BLOCKSET
  RESIDENT WORDS; REMOVING THE UNUSED BADMAP PACKAGE SAVES 65 MORE.
  KCORE+MRES = 15257 WORDS, PERMANENT_LAST = 15304, 1079 FREE ADDRESSABLE
  WORDS BELOW 040000 OCTAL.  BOTH THE COMPACT ROOT-IMAGE TEST AND THE
  D6FS/BLOCKSET/BADMAP-FREE LINK TEST PASS; DISK-ROOT SIZE TEST STILL PASSES.
- FULL TSFS ROOT SIMULATOR BOOT IS NOT YET ACCEPTED: WITH DISK STAGE1
  AND TSFS SELECTED AS ROOT, MODULES INCLUDING LOGSTORE/MEMFS/DTFS LOAD
  BUT KINIT HALTS BEFORE LOGIN (MUUO105).  THE BOOT HANDOFF MUST BE
  INVESTIGATED SEPARATELY; DO NOT ATTRIBUTE THE FAILURE TO BLOCKSET
  REMOVAL WITHOUT A CONTROL RUN.  TRANSCRIPT:
  $HOME/tmp/daimos-tsfs-boot-no-pictures-20261012-v1/sim-v2/simh.log.
- THE SEPARATE DSH EXECUTABLE-SIZE GATE REMAINS 036000 OCTAL AND IS NOT
  RELAXED.  THE ROOT IMAGE PACKING TEST USED DIAGNOSTIC DSH_IMAGE_LIMIT=037000
  TO EXERCISE MEDIA GENERATION, NOT TO ACCEPT AN OVERSIZED DSH.

## 2026-10-15 SINGLE-DISK 32K/48K LOWMEM BOOT MEASUREMENT

- A GENUINE ONE-MEMBER D6FS ROOT WAS GENERATED WITH `mkdsk -n 1` AND
  `mkd6fs -n 1` (44799 filesystem blocks; `d6fsck -n 1` CLEAN).
  SIMH ATTACHED ONLY DSK0 FOR THE ROOT; CTY WAS THE LOGIN CONSOLE.
  THE BOOTSTRAP STILL USES THE SIMULATOR PTR READ-IN PATH.  THE STOCK
  LOWMEM KERNEL STILL CONTAINS DCS/GE/TTY RESIDENT PACKAGES EVEN WITHOUT
  SERIAL DEVICES ATTACHED: THIS WAS NOT A CTY-ONLY KERNEL LINK.
- SIMH ACCEPTS `set cpu 48k`.  KINIT'S PREVIOUS MEMORY DETECTION ONLY
  PROBED 32K MULTIPLES, AND REPORTED 32K ON 48K MACHINES.  KINIT_DIAG.C
  NOW PROBES CONTIGUOUS 8K BOUNDARIES ABOVE THE 32K MINIMUM AND PRINTS
  THE ACTUAL DETECTED SIZE.  VERIFIED 48K BANNER: `MEM 48 K`.
- A FRESH 48K LOWMEM BUILD (`SIMH_CPU_KWORDS=48`,
  `BOOT_CORE_WORDS=0140000`) BOOTED D6FS, INIT, CTY LOGIN ROOT, AND
  DSH V1, AND EXECUTED MEMSTAT SUCCESSFULLY.  RUNTIME COUNTERS:
  TOTAL=49152, RESIDENT=12725, PROCESS-WORDS=28440, MEMFS-USED=0,
  MEMFS-CAPACITY=0.  THE DIFFERENCE, 7987 WORDS, IS UNALLOCATED
  ACCORDING TO THESE COUNTERS (NOT NECESSARILY A CONTIGUOUS EXTENT).
  TRANSCRIPT: $HOME/tmp/daimos-48k-single-v4-memstat.log.
- THE SAME OBSERVED RESIDENT+PROCESS FOOTPRINT IS 41165 WORDS,
  EXCEEDING 32K BY 8397 WORDS.  THIS IS A WORKLOAD COMPARISON, NOT A
  PRECISE MINIMUM DSH BOOT REQUIREMENT: MEMSTAT MAY CREATE ADDITIONAL
  PROCESS ALLOCATIONS, AND MM FRAGMENTATION/ALIGNMENT MATTERS.
- SEPARATE STATIC CTY+CLK+DSK+D6FS ESTIMATE, EXCLUDING UNUSED DCS/GE/TTY
  AND BLOCKSET: KCORE=10020, MRES=2478, TOTAL=12498 WORDS.
  THIS IS AN ESTIMATE FOR A STRIPPED LINK, NOT THE RUNNING 48K IMAGE.
  DSH DXR: IMAGE=031321, BSS=012751 (OCTAL); WITH USER ORIGIN/STACK
  ITS VM NEEDS 19658 WORDS, ROUNDED TO 20480 AT 02000 ALIGNMENT.
  THE STRIPPED STATIC KERNEL PLUS THIS DSH VM WOULD NEED 32978 WORDS,
  210 MORE THAN 32K, BEFORE OTHER ALLOCATIONS.
- 32K SINGLE-DISK BOOT REACHES LOGIN; LOGIN->DSH STILL FAILS IN THE
  EARLIER FULL LOWMEM CONFIGURATION.  DO NOT CLAIM 32K DSH ACCEPTANCE.
  A SEPARATE LOWMEM EXEC FRAME-OVERLAP CORRECTION IS IN THE WORKTREE.
- NEXT STAGE (SEPARATE CHAT): OPTIMIZE PERMANENT KCORE TO BELOW 8192
  WORDS, THEN REASSESS THE 32K BOOT GAP WITH ACTUAL LIVE MEASUREMENTS.
  DO NOT SIMPLY DISABLE FEATURES OR RAISE THE 32K MINIMUM; THE 8397-WORD
  WORKLOAD DIFFERENCE CANNOT BE ASSUMED RECOVERABLE FROM KCORE ALONE.

## 2026-10-24 file CHDIR STAT reuse v232

CHDIR now supplies its existing validated VFS STAT frame to the access
policy, rather than performing another provider STAT. Search permissions
and file ABI remain unchanged. GCC lowmem link PASS: KCORE 025115 octal
(10829 decimal), KCORE+MRES=13692, no resident size change. The
PDP-6 target file-walk fixture PASSes, including CHDIR. Continue
proposal B looking for resident text and data reduction.

## 2026-10-24 VFS failure-epilogue audit v238

On the v237 025075-octal (10813 decimal) GCC lowmem full-link baseline,
sharing SETATTR's three-word unwind was size-neutral; sharing SYMLINK
and RENAME's four-word unwind with WRITE_WORDS unexpectedly increased
KCORE to 025077 octal (10815 decimal), KCORE+MRES to 13678, and
PERMANENT_LAST to 13725. Both experiments were reverted. No new
production savings or runtime acceptance; continue proposal B at
larger VFS/provider boundaries. JRST folding and strict 32K work
remain deferred.

## 2026-10-24 VFS/provider architectural audit v239

Reviewed VFS lookup/parent resolution, CREATE/MKDIR/SYMLINK
publication, and MonitorFS leaf lookup/READDIR. Existing common
helpers already cover the main overlapping operations. A tentative
leaf-length loop-invariant move was reverted immediately because AC5
is clobbered by name comparison on subsequent iterations. No
production change, unverified size claim or new runtime acceptance.
KCORE remains 025075 octal (10813 decimal). Proposal B remains open.

## 2026-10-24 file mount-extension update sharing v240

The direct-store and descriptor-copy paths now share the two-bit mount
extension bitmap update sequence in file_runtime.s, preserving the
destination-clearing semantics. GCC lowmem disk-root full image PASS:
KCORE 025067 octal (10807 decimal), KCORE+MRES 13670,
PERMANENT_LAST 13717, six permanent words saved from v239.
PDP-6 file and file-walk tests PASS. Extended native descriptor
mount-extension lifecycle acceptance remains open. JRST folding
and strict 32K acceptance remain deferred.

## 2026-10-24 compact FILE duplicate v241

Two contiguous FILE descriptor words now copy by one bounded BLT
instead of two MOVE/MOVEM pairs in file_dup. GCC lowmem disk-root
full linked size KCORE=025066 octal (10806 decimal), KCORE+MRES=13669,
PERMANENT_LAST=13716, one resident word below v240. Native
daimos-file-mountext-v19 tests PASS 26-mount vnode, lock, DUP/DUP2,
close and unmount; PDP-6 file/file-walk fixtures PASS. Strict 32K
and JRST folding remain deferred.

## 2026-10-24 compact FILE DUP2 v242

The DUP2 descriptor pair uses a bounded BLT, and pipe reference
handling reads the copied destination metadata. The full GCC
lowmem disk-root link PASSes: KCORE 025065 octal (10805 decimal),
KCORE+MRES=13668 and PERMANENT_LAST=13715, one more permanent
word saved. Native daimos-file-mountext-v19 and PDP-6 file and
file-walk fixtures PASS. No ABI, resident data or stack growth.
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 descriptor and GETCWD audit v243

Reviewed descriptor-copy follow-up and the variable-length
GETCWD output zeroing loop. DUP/DUP2 copies and the mount extension
bit update already share compact implementations. A proposed BLT
zero-fill was reverted before measurement because setup and overlap
semantics offered no demonstrated advantage. No production changes.
KCORE baseline remains 025065 octal (10805 decimal).

## 2026-10-24 file path/provider request-frame audit v244

Reviewed resident SYMLINK, RENAME, CHDIR, MKDIR/MKFIFO,
UNLINK/RMDIR, TRUNCATE/STAT and VFS CREATE request staging.
The principal request frames and failure unwinds are already
shared where their lifetimes agree; no safe additional reduction
was established. No production code changes. GCC testkit
vfs-stat-frame and pre-dsh-fs-closure tests PASS; linked baseline
remains KCORE 025065 octal (10805 decimal). Proposal B open,
JRST folding and strict 32K acceptance deferred.

## 2026-10-24 filesystem transfer workspace lifetime audit v245

Reviewed `fs_block_workspace` (0200 octal words) and the
`fs_provider_serialized` lifecycle. DTFS, TSFS and D6FS already
share the resident transfer buffer while holding the serialization
lock over sleeping I/O; MEMFS uses a distinct non-sleeping path.
No second large permanent transfer buffer or safe unprotected
overlap was identified. No source changes; baseline KCORE remains
025065 octal (10805 decimal). Proposal F remains open subject to
explicit concurrency/lifetime proof. Proposal B remains open.

## 2026-10-24 static allocator and VM storage audit v246

Inspected BSS allocations across MM, MEMFS, IPC, PROC and VM.
The MM extent registry is 0100 octal (64 decimal) words of live
descriptors; MEMFS chunk/XIP registries and VM/IPC/PROC singleton
words preserve distinct live state. No safe scratch overlay was
demonstrated. No production change; preserve KCORE 025065 octal
(10805 decimal). Measure MM extent high-water before revisiting
the descriptor count.

## 2026-10-24 MM extent registry validation gate v247

PDP-6 MM permits 040 octal (32) live extent descriptors occupying
0100 octal (64) words. Full-registry insertions return
MM_ERR_DESCRIPTORS (-3), so a smaller limit needs measured workload
high-water and fragmentation evidence. Testkit `mm-core` cannot
currently compile because it references a nonexistent
`system/kernel/proc/vm_pdp6.c`. The existing testkit tree has
unrelated local edits and was not modified. No production change
or accepted memory reduction.

## 2026-10-24 MM host test dependency investigation v248

Temporarily excluded the missing vm_pdp6.c from the testkit
mm-core build, revealing unresolved host symbols
vm_extent_move, vm_space_can_swap and vm_space_destroy. Real
implementations are in the target-side PDP-6 VM code; replacing
them with trivial host stubs would invalidate the test's
movement/compaction/swap assertions. The testkit Makefile was
restored and unrelated pre-existing modifications preserved.
The MM registry remains 32 entries/64 words pending validated
workload high-water data. No production change or size saving.

## 2026-10-24 KCORE bulk device I/O sharing v233

Both resident bulk I/O paths now prepare mapped-buffer arguments once
before device selection and preserve the vnode for ordinary VFS fallthrough.
The full linked GCC lowmem disk-root KCORE decreased 025115 -> 025103
octal (10829 -> 10819 decimal), KCORE+MRES 13692 -> 13682,
PERMANENT_LAST 13739 -> 13729. No static data, stack or ABI growth.
Target PDP-6 file and file-walk fixtures PASS. Native PTR/PTP simulator
fixture stopped with SIMH status 139 and SDL2 window-event warnings; it is
not recorded as a pass. Native card WORDTOKEN12/CR/CP round trip PASSed
on an independent SIMH test with SDL_VIDEODRIVER=dummy. Further native
PTR/PTP and other device I/O acceptance is open.
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 KCORE MonitorFS name copy v235

Two VFS three-word static name-record copies use bounded BLT
instead of six scalar instructions each. GCC lowmem disk-root
full link KCORE 025075 octal (10813 decimal), KCORE+MRES 13676,
PERMANENT_LAST 13723; saving six permanent words from v234.
Target PDP-6 file and file-walk fixtures PASS. A size-neutral
VFS provider fallthrough branch experiment was reverted.
Native SIMH MonitorFS device view-online PASSes (GCC profile).
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 PTR/PTP native verification v237

The v233 bulk I/O pathway's previously incomplete paper-tape native
acceptance is now verified: `test-daimos-pt-wordio` PASS on PDP-6 SIMH
using GCC and SDL dummy video/audio drivers. Both full and partial
PTR/PTP WORDTOKEN8 physical round trips passed. No production changes;
last KCORE measurement remains 025075 octal (10813 decimal), with
KCORE+MRES=13676. The strict 32K gate and JRST folding stay deferred.
