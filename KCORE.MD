# KCORE - Verified Open Work and Optimization Backlog

## V90 — AUTHORITATIVE KCORE OPTIMIZATION REGISTER (2026-10-21)

**Read this section and the relevant older entries before every KCORE
analysis, experiment, or implementation step.** Record the candidate,
baseline, callers/lifetime, proposed transformation, *linked KCORE delta*,
tests, status, and commit here at each step. `PROJECT-STATE.MD` retains
architecture/current-state summaries; `pdp10-doc` receives verified
completed results; KCORE decisions and ongoing work belong **here**.

### Active requirements and baseline

- Objective: **KCORE alone** strictly below **8192 decimal words**
  (`020000` octal words). The accepted fresh V86 KCC 256K full disk-root
  build has `__kcore_low_end=023002` and **9682** linked KCORE words;
  **1491** must be saved to fall strictly below 8K (1490 to reach 8192).
- **Previous V94:** `__kcore_low_end=022761`, KCORE **9665 words** after
  nine device bulk-I/O entries share one dispatcher; **1474** words
  remain to fall strictly below 8192 (1473 to reach it).
- **Latest V95:** `__kcore_low_end=022754`, KCORE **9660 words** after
  sharing mapped READ_WORDS/WRITE_WORDS syscall code; **1469** words
  remain to fall strictly below 8192 (1468 to reach it).
- MRES size and whole-KCORE+MRES permanent-address gate are **informational**
  during this phase, not optimization blockers; the old total limit can be
  explicitly enabled with `PERMANENT_SIZE_ENFORCE=1`.
- **No 32K machine configurations/tests**, no TSFS work, and no experiments
  altering either subsystem while this KCORE analysis is in progress.
- Optimize RAM first, then speed; preserve functionality and existing ABI.
  No workarounds and no speculative module removals. Prefer genuine KCORE
  removal into a correctly pinned MRES or disposable KINIT when lifecycle
  proves it safe. Verify folded *linked* delta, not DOBJ totals.

### Already optimized — do not rediscover as unimplemented opportunities

| Area | Confirmed state / result | Remaining restriction |
| --- | --- | --- |
| MonitorFS | V86 moves full implementation into pinned MRES, leaves nine KCORE dispatch words; **KCORE 10702 -> 9682, -1020** (corrected in V94); fresh 256K KCC image and DSH 115/115 PASS; prior GCC 48K MonitorFS/swap probes PASS | Do not remove VFS-facing dispatch or the MRES pin |
| Legacy VFS mount arrays | v14 audit removed dead eight-word legacy BSS arrays following KINIT MM-backed slab migration; removed redundant early D6FS reader slot/store; historical **nine-word net** saving | Retain live `vfs_mount_policy` and MM slab bases |
| DSK270 / DRM236 | Two-word KCORE dispatch per controller, validation/conversion relocated to optional MRES; **33 pre-fold KCORE-object words** removed, not independently measured link savings | MINIT patches endpoints; preserve controller-absent behavior |
| Process startup snapshots | V72 replaced obsolete BACKSTORE startup reader with exact-size pinned argc/argv/envp snapshot; KCC linked KCORE `025723 -> 024754`, **-487 words** in that historical comparison | Retain swap-out/in atomicity, descriptor lifetime and live metadata |
| MM assembler | Earlier recovery restored compact PDP-6 MM implementation with **38-word historical linked reduction** in a GCC isolated comparison | Don't reimport alternative stale MM source blindly |
| VFS mount policy | Four-slot readonly bitmap replaced with packed policy word, and mount/MM slab work accepted in earlier states | Keep live reservation/ownership semantics |
| Shared short KCORE wrappers | V88/V89 verified `mm_pin/unpin`, `file_unlink/rmdir`, `vfs_create/mount`, scheduler selectors, `vfs_chmod/chown/utime` already share common implementations | Inlining often duplicates code; measure before changing |
| Bulk device file I/O | **V94 -15 folded KCORE words** (9680 -> 9665): nine device-word paths share a single AC1/AC2 load, indirect PUSHJ via the selected patchable one-word MRES jump slot, and common file-offset result | Keep WCNSLS raw-read append handling and jump-slot ABI; focused unit + live PTR/PTP accepted |
| Mapped READ/WRITE_WORDS syscalls | **V95 -5 folded KCORE words** (9665 -> 9660): map/translate and FILE call argument paths now shared; AC5 selects read versus write handler and survives translation | Keep per-op device/pipe and zero-count write handling; live 256K PTR/PTP full and partial transfers PASS |
| Shared word operations | `fs_copy_words`/`fs_move_words`/`fs_zero_words` serve MM/VM/module and filesystem providers; overlap-safe move already reuses forward copy | Cannot relocate into detachable filesystem MRES without lifetime solution |
| TTY packed fields | Halfword canonical-line bases and pending-byte/session routes already packed; DCS and GE MRES both use KCORE pending interfaces | Retain consistent live shared state |
| P10FOLD | Existing production `-m 2` 21-object census has 31 folds/70 estimated words; `-m 1` yields only +4 more | Don't widen fold hazard surface for four words |
| Process/process-I/O foundations | Existing compact three-word descriptors, intrusive runqueue, boot-selected slots, word-only eight-word pipe | Treat as accepted foundations; profile before altering |

Historical size comparisons may use different compilers, profiles and
revision bases. **Do not add their savings together** or assume a staged
experiment is installed in current V86 KCORE.

### Current candidate register — analyze bottom-up, but prioritize impact

1. **[OPEN] KCORE VFS/file-runtime and syscall common paths:** analyze
   `vfs_readonly`, mutation and file path setup, syscall argument packing,
   duplicate validation/tails and potential common functions. Check KINIT,
   FS-provider MRES and boot callers. `file_runtime` ≈1175 and syscall
   dispatch ≈827 raw words in current build, *not* estimated savings.
2. **[OPEN] Process scheduling/control:** `proc_pdp6` ≈2170 raw words;
   investigate cold process-management paths and call-site commonality,
   independently of latency-critical PI/scheduler hot paths. `proc_swap`
   ≈729 words; packed-process plans are staged/inactive, not a free or
   currently safe substitute for disk swap.
3. **[OPEN] KINIT lifetime:** check every initialization-only KCORE helper
   and MINIT registration path; move only when no post-KINIT references,
   patchable entries or persistent state remain.
4. **[OPEN] KCORE/MRES duplicate implementations:** prove real duplicate
   *linked* code, not similar interface names; retain minimal patchable
   entrypoints for optional removable drivers, and preserve pinned package
   lifetimes for services with open handles.
5. **[OPEN] Shared sequences/folding:** scan same-domain common tails and
   relocation-safe cold cross-domain donors using P10FOLD read-only mode.
   External-anchor folding only with >~20-word net benefit, no KCORE growth,
   hot JRST regression, or removal of module independence.
6. **[OPEN] Small TTY/PI/route helpers:** keep packed-state and semantic
   checks; evaluate merges only after proving a net linked saving.
7. **[DEFERRED] Packed-process eviction and D6CODEC integration:** design
   stages in `design/kcore-packed-process-v1.md` are explicitly inactive;
   do not silently enable or remove current swap path to claim size savings.

### Per-step audit protocol

**V95 accepted native mapped syscall sharing:** `syscall_dispatch_pdp6.s`
formerly duplicated the `native_sys_map_one` / physical-buffer setup /
FILE handler call / mapping-release tail for `READ_WORDS` and `WRITE_WORDS`.
The two entry points now load `file_read_words` or `file_write_words` into
AC5 and enter `native_sys_words_mapped`, which uses `PUSHJ 17,(5)`. This
does not change the distinct prechecks for blocking TTY/pipe reads,
device writes or the zero-count DPY write case. `vm_user_mapping_hold`
explicitly saves/restores AC5, while `vm_user_words` changes only AC1,
AC3 and AC4; the mapping failure path remains unchanged. The syscall
dispatch object is **827 -> 822 words**. The fresh folded KCC KCORE link
is **022761 -> 022754 octal**, **9665 -> 9660 words (-5)**. Targeted
`test-daimos-pt-wordio` native 256K SIMH physical PTR/PTP full/partial
WORDTOKEN8 round-trip PASS, exercising both mapped syscall operations.
No full testkit, 32K or TSFS tests were run. Other syscall services were
not retested in this focused change.

**Next possible sharing candidate (unmeasured):** The mapped user-buffer
translation code in `native_sys_readdir` (`%L97`) and `native_sys_procinfo`
(`%L124`) is similarly structured. Inspect AC5/AC6 lifetimes, their
distinct service semantics and callers; accept only a measured link-time
reduction with focused syscall tests. Larger cold-path relocation remains
an open high-impact task.


**V94 accepted shared FILE device-word dispatcher:** `file_runtime.s`
previously repeated AC1=buffer, AC2=count, `PUSHJ` to MINIT-patchable
device jump slots and return handling for six write and three read device
paths. Each now selects its jump slot in AC4 and tail-branches to a
shared loader / `PUSHJ 17,(4)` / `file_words_result` sequence. These slots
remain ordinary KCORE one-word JRST trampolines patched by MINIT; MRES
lifetime and public file API remain unchanged. The WCNSLS raw *read* path
retains its original separate fallthrough into append-result handling.
`file_runtime.dobj` text fell from 1175 to 1160 words; **folded KCORE**
endpoint `023000 -> 022761` octal, **9680 -> 9665 words (-15)**. There
is an extra indirect device dispatch, accepted here for RAM reduction on
relatively slow device-word transfers; it does not affect the regular
filesystem word I/O hot path. Focused native PDP-6 `target-file-pdp6`
PASS after adding all nine jump-slot, AC1/AC2, short-transfer and offset
tests plus an absent-device test in daimos-testkit. Live 256K SIMH
`test-daimos-pt-wordio` PASS (PTR/PTP full/partial transfer). No full
testkit, 32K or TSFS tests. Other devices were checked by the focused
PDP-6 test, not separate live integration tests.

**V94 accounting correction:** older V86–V93 records subtracted `070`
octal from `__kcore_low_end`, although the real executive start is `060`
octal. `system/boot/pdp6/image/report-permanent.sh` confirms the correct
equation, `KCORE = __kcore_low_end - 060` (octal), an eight-decimal-word
correction to every affected previous size, **not** a code regression.
V86 MonitorFS relocation saved **1020**, not 1028, words from the actual
V85 baseline 10702; V91 and V92 saved one word each. Linker endpoints
and all measured *deltas* remain valid.


**V93 rejected source-only fallthrough cleanup:** The explicit `JRST`
immediately preceding `vfs_lookup_provider` and the corresponding `JRST`
before `vfs_read_words_provider` were removed experimentally (both target
the following instruction). The full folded KCC link **did not shrink**:
`__kcore_low_end` remained `023000` octal (9680 words). Both changes were
reverted. No functional tests were needed or run for the rejected trial.
The effective folded image, not redundant source syntax, governs savings.


**V92 accepted VFS parent helper:** `vfs_parent` formerly decoded the mount
slot into AC3, then copied it to AC4 before calling the shared
`vfs_parent_resolve_slot` (which expects AC4). Directly decoding and
validating in AC4 eliminates that copy. The shared resolver's AC2/AC3
preservation contract remains intact; mount bounds and fallback are
unchanged. Folded KCC 256K full-profile KCORE endpoint `023001 ->
023000` octal: **9681 -> 9680 words (-1)**. Targeted PDP-6
`target-file-walk-pdp6` PASS. No full testkit, 32K, or TSFS testing.


**V91 test scope rule:** Do **not** run the full daimos-testkit matrix after
each KCORE reduction. Run the folded KCORE link-size check and only the
smallest existing functional/regression tests whose code paths or ABI could
be affected. Broader integration suites are reserved for cumulative
milestones or changes affecting shared scheduler/VM/boot interfaces.
Document the chosen tests and any untested paths for each change.

**V91 VFS dead instruction:** `vfs_mutate3_ro` was an unreferenced label
followed by one unreachable stack subtraction before the live
`vfs_mutate2_ro` entry. Removing the dead instruction leaves reachable
control flow unchanged. Fresh KCC folded relink: `__kcore_low_end`
`023002 -> 023001` octal, **9682 -> 9681 KCORE words (-1)**. Full image
rebuild PASS; VFS-targeted regression coverage is to be recorded separately.
The focused `test-daimos-vfs-storage-reservation` test PASSes (PDP-6
runtime). It checks VFS mount/reservation handling, but does **not**
directly execute the removed unreachable instruction. No full matrix was
run. The reachability argument is from the source control-flow inspection.

1. Re-read this register, `PROJECT-STATE.MD`, current V86/V-later KCORE
   link map and relevant historical designs before touching a candidate.
2. Inspect exact source and symbol/reference paths in KCORE, installed
   MRES and KINIT. Distinguish interrupt/indirect/MINIT patched entries
   from apparently unreferenced functions. Classify cold/boot-only vs hot.
3. Estimate gross removable words, new dispatch/call/relocation overhead,
   and any retained state. Reject obvious non-savings before experimenting.
4. For a viable change, produce a versioned isolated build and compare
   both compiler configurations when useful; require a **folded KCORE map**
   and >32K functional acceptance in daimos-testkit. Do not run 32K tests.
5. Update candidate and verified-completed tables **here first**; document
   confirmed results in `PROJECT-STATE.MD` and `pdp10-doc` after acceptance.
   Commit early; attempt push; do not claim remote success without proof.

### Source documents and inventories

- `design/kcore-gcc-size-audit-v10.md`: older GCC/lowmem object audit;
  its MonitorFS priority is completed, and old 32K instructions are stale.
- `design/kcore-legacy-audit-20261016-v14.md`: precise dead-array removal,
  live aliases, KINIT cutover and MM-slab reader lifetime requirements.
- `design/monitorfs-mres-extraction-v13.md`: historical optional extraction;
  superseded by verified V86 default-enabled MRES.
- `design/kcore-packed-process-v1.md`: staged packed eviction/restore,
  not active production KCORE; its atomicity dependencies remain important.
- `pdp10-doc/STATUS/DAIMOS-KCORE-FUNCTION-INVENTORY-20261021-V87.MD`
  and `.TSV`: linker-visible bottom-up inventory, with *upper-bound* entry
  spans, aliases and incomplete relocation call counts.

### Historical sections below

Entries below are preserved for provenance. Their older RAM/32K acceptance
policies and MonitorFS default-off decisions are **superseded by V90**.

## 2026-10-20 V76: MonitorFS MRES cost and live-swap acceptance

The optional pinned MonitorFS MRES has now passed both resident namespace
probes and exact swapped startup-record streams through a real 48K GCC PDP-6
SIMH wake/resume/WAIT lifecycle.  Enabling it shifts 1020 words out of KCORE
but requires a 1039-word MRES, **increasing fixed resident use by 19 words**.
The GCC lowmem/disk-root comparison is 13368 (resident) versus 13387
(pinned-MRES) words for KCORE+MRES, with PERMANENT_LAST 13415 versus 13434.
These numbers include the V75 96-word dynamic slot-0 stack setting; that
stack itself is outside fixed KCORE+MRES accounting.

The MRES gate remains default-off because RAM reduction outranks modularity.
Do not conflate the 1020-word KCORE shrink with a net resident saving.  Any
default enablement must first remove the 19-word overhead and pass KCC
permanent-size and live integration measurement.  Strict 32K is deferred.


## 2026-10-20 V75: measured slot-0 kernel stack depth

The slot-0 stack must cover swap-in and nested MM/I/O work, not just the idle
loop.  The first successful 48K V74 live-swapped-process test used a temporary
`0200`-octal (128-word) stack.  Native `PROC_STACK_WATERMARK=1` measured its
first untouched slot-0 word at offset `0116` octal, i.e. 77 words used.  The
former `0100`-octal (64-word) stack cannot support that call depth.

Final V75 allocation: `0140` octal, or 96 words, in both the C KINIT header
and the PDP-6 assembler.  This is +32 dynamically allocated resident words
versus the old allocation, and -32 versus the oversized 128-word experiment.
A fresh 48K SIMH V74 run with stack-watermark instrumentation **PASSes all
stages**, including confirmed `STATE=SWAP`, exact NAME/CMDLINE/ENVIRONMENT,
pipe wake, swapped-process restoration, pressure-child cleanup and WAIT.
Its watermark is again `0116` octal, leaving 18 untouched words.  The KCORE
linked endpoint is unaffected by this dynamic stack allocation.  This is a
measured peak, not a universal proof against any larger future call depth;
the 32K sizing constraint remains deliberately outside this task.


## 2026-10-20 V73 corrected startup records after heap growth

MonitorFS, PROCINFO COMM and the V72 snapshot capture incorrectly used
`VM_SPACE_WORDS(p)` to find the startup metadata.  On heap growth `BRK`
changes that length but preserves the original startup area, so the correct
origin is `PROC_BRK_WORD(p)` high half (initial VM words) minus `02000`.
All three access paths now agree.  The snapshot path rejects a saved origin
beyond the currently allocated extent.  There is no new per-process state.

The unchanged 256K KCC MonitorFS DSH/LOGIN live oracle PASSes for NAME,
CMDLINE, ENVIRONMENT, relative paths and domain leaves.  The native PDP-6
MonitorFS word-I/O oracle PASSes with a realistic grown-heap u-area, and the
exact-size snapshot oracle PASSes for GCC and KCC, including heap growth
(testkit commit `964ad83`).
The new linked KCORE endpoint `024776` octal is 18 decimal words above V72
`024754`; relative to the old BACKSTORE reader's `025723` endpoint the net
improvement is 469 words.  The live test used the documented KCC DSH test
limit of `037000`.  Strict 32K size work is deliberately deferred; live
swapped-parent integration and optional MonitorFS MRES acceptance remain open.

The same full MonitorFS live oracle also PASSes on a fresh 48K GCC lowmem
disk-root image, KCORE endpoint `024454` octal.  DSH remained resident in
that test; do not misrepresent it as swapped-process integration coverage.

## 2026-10-20 resident startup snapshots, isolated v72

Swapped-process MonitorFS reads no longer need the 691-text-word KCC reader
for BACKSTORE startup strings.  When swap-out has a pinned VM, the new
`proc_startup_snapshot_v72.c` validates and retains only the occupied
`argc/argv/envp` counted-SIXBIT prefix in an exact-size nonmoving allocation.
The single extra u-area word (0410) points to the snapshot and stays zero for
resident processes.  Snapshot words are freed on swap-in or EXIT/abort and
included in MEMINFO; the on-disk swap format and three-word process descriptor
are unchanged.  Allocation uses MM's existing no-reclaim entry to prevent
swap-reclaim recursion; failure aborts swap without losing user memory.

Native PDP-6 snapshot test PASSes (`daimos-testkit` `b23f962`).  KCC object
text is 225 words (versus the
removed disk-reader 691), and the linked KCORE end falls by 487 decimal words:
025723 -> 024754 octal on fresh 256K KCC images.  GCC 48K image plus
permanent-size checks PASS.  Do not infer strict 32K or MonitorFS live
acceptance: 32K KINIT stack reserve still fails, and the unchanged 256K live
oracle has a *resident* PID 2 NAME failure after LOGIN/EXEC, independent of
the swapped snapshot path.  MonitorFS MRES remains opt-in until validated.

## 2026-10-20 Optional Type-270/Type-236 glue relocation v1

The public Type-270 and Type-236 read/write entrypoints now occupy two
patchable KCORE words per controller family (four total).  All validation
and device-address conversion has moved into versioned MRES sources
`drivers/dsk270_mres_v1.s` and `drivers/drm236_mres_v1.s`.  MINIT patches
the public entries to additional MRES exports when the controller is
detected; absent hardware leaves both entries returning `kret_neg1`.
The original raw controller exports and all existing callers retain their
ABIs.  The DSK and DRM packages remain optional at runtime.

Source-object comparison before folding: former KCORE bridges occupied
20+17=37 text words; replacements occupy 2+2=4 words.  The 33-word KCORE
improvement is an **object-level** result and must not be confused with
linked post-p10fold size or a reduction of total permanent RAM.  The
versioned MRES conversion routines occupy 19+16=35 text words, plus
package-export metadata.  The relocation is primarily a low-core saving;
for installed devices, total resident cost can increase slightly.

The isolated GCC lowmem link builds KCORE and both extended MRES exports,
and an isolated 48K disk-root image builds and boots through `DSK270 OK`,
`D6FS LOADED`, and `INIT V1`.  This is boot smoke, **not** native read/write
or absent-controller acceptance.  Before merging into the active checkout,
run the existing daimos-testkit Type-270 disk read/write, Type-167/236 drum,
and controller-absent tests, plus KCC full-profile link and native boot.
The active checkout contains unrelated in-progress storage and MonitorFS
changes; these are not part of the isolated v1 patch.

MonitorFS extraction already exists behind `MONITORFS_MRES_ENABLED=1` and
must remain opt-in until its live VFS/LOGIN probes pass and the current
swapped-process metadata reader is included in the MRES build.  Enabling it
without that reader would silently remove MonitorFS data for swapped parents.

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
