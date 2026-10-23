# KCORE - Verified Open Work and Optimization Backlog

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

Date: 2026-10-13
Basis: current DAIMOS checkout, source code, PROJECT-STATE.MD and
active build notes. A staged, host-tested implementation is not considered
completed until it is linked, measured and accepted on the relevant PDP-6.

## P0: 32K resident limit and correctness

- [ ] Shrink the GCC lowmem KCORE link endpoint below 020000 octal; the
  2026-10-13 PROJECT-STATE.MD v41/v42 checkpoint reports 021541 octal.
  Run a fresh full build, not just an object-level estimate.
- [ ] Measure full permanent KCORE plus committed MRES, irreducible scratch,
  KINIT allocations and process startup headroom; retain the separate general
  below-040000 permanent-address limit where applicable.
- [ ] Pass the strict GCC 32K size gate and native 32K boot/INIT/LOGIN/DSH,
  process-pressure and filesystem tests with the same fresh image.
- [ ] Fix compiler, assembler and kernel correctness bugs at their source;
  add regression tests in daimos-testkit before marking the sweep complete.
- [ ] Audit source-level size hotspots, generated objects and real post-p10fold
  link maps; reject optimizations that increase the 32K resident footprint.

## P1: packed-process lifecycle and compression

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
