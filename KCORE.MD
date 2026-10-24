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
remains open. No changes to JRST folding or the deferred 32K gate.

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
