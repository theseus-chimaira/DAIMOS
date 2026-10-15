# KCORE - Verified Open Work and Optimization Backlog

## 2026-10-15 allocator retry setup consolidation v140

The native PDP-6 `mm_alloc_aligned` contains two copies of the
`mm_alloc_aligned_noreclaim` argument/stack setup. A single private helper
now owns both sequences while preserving the pressure stages and the
compaction retry ordering. With the GCC lowmem disk-root 32K image and
the same KINIT configuration, the complete image packages successfully:
MM assembly text 482 -> 478 words; KCORE exclusive endpoint 023456 ->
023452 octal (4 resident words saved), BSS unchanged at 70 words.

The helper adds one transient stack word. Native PDP-6 allocation/reclaim
and compaction regression acceptance remains pending; this measurement is
build/link/package evidence only. The standalone `daimos-testkit` host
`mm-core` test has an existing missing-symbol link blocker documented
below. Do not count the code change as fully runtime accepted yet.

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
