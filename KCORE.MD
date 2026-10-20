# KCORE - Verified Open Work and Optimization Backlog

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
