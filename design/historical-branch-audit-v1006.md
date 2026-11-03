## v1038 selective-MRES driver extraction and VFS reconciliation

The `kcore-mres-selective-integration-20261023-v1` branch is only
partially reconciled. Its earlier disk/drum MRES extraction was rebuilt
against present KCORE and MINIT and then reduced by three more linked
MRES words. Nevertheless, total permanent code/BSS grows one word
when both device packages are installed (KCORE -31, DSK +18, DRM +14
decimal words). Since RAM has priority over fixed-KCORE size alone,
the driver extraction was rejected and production hardware code left
unchanged. Its value in no-device profiles is real, but insufficient
to justify worst-case growth without a better implementation.

The branch's VFS parent-path change is valid and has been restored:
mount-slot extraction now uses AC4 without clobbering AC3, saving one
linked KCORE word. A new native PDP-6 v1037 test fails with exit 3
before the fix and passes after it. Historical `daimos-testkit` VFS
mount-mutation v9 was also repaired to include the real current
`vfs_readonly` implementation and now passes. The syscall dispatch
folding in the branch remains for a later ABI-aware review; preserve
the branch until that comparison is complete. 32K tests remain deferred.

## v1033 selective MRES compiler checkpoint

The October 23 `kcore-mres-selective-integration-20261023-v1` tip
contains a compiler-selection change for the MonitorFS swapped-reader.
Do not replay it into current main: later main deliberately compiles
this cold MRES leaf with GCC. Under one identical KCC KCORE map the
GCC leaf links into a 003070-octal-word MRES, while a KCC leaf yields
003265 (+125 decimal resident words). Both toolchains assemble and
link, but the GCC output is smaller and the entry uses an established
assembly ABI. The v1033 testkit guard now enforces this size-conscious
compiler exception. The branch is retained for further review of its
distinct Type-270/236 optional-MRES dispatch extraction; that candidate
may save KCORE words and must not be discarded without a size and
driver-MINIT correctness audit. No 32K tests were executed.

## v1030 MM v125 branch review

`mm-kcore-skeleton-baseline-v125-20261014` is a historical planning
checkpoint, not an unapplied current allocator change. Current KINIT
already reserves the descriptor backing immediately after fixed KCORE;
native MM uses a boot-selected `mm_extent_capacity` for bounded table
insertion. The distinct usable artifact is the v125 descriptor-limit
test, adapted and passing on native PDP-6 with a 256K synthetic core
ceiling and test-owned descriptor backing. No 32K-specific test or new
kernel size measurement was run. Do not replay the older branch's MM
implementation or claim its historical 552-word baseline as current.

## v1028 D6CODEC branch consolidation

The v075/v078/v089/v092 DAIMOS single-format grouped cutover histories
are now reachable through the sole DAIMOS experimental archive branch
`archive/d6codec-experimental-cutover-v1028-20261103`. Their six testkit
counterparts (v075/v078/v079/v085/v089/v091) are reachable through
`archive/d6codec-grouped-tests-v1027-20261103`. Each archival merge used
`ours` to preserve distinct historical commit trees, not to claim the
experimental implementations are compatible with current production.
All ten original local and origin branch references were deleted after
checking ancestry and exact remote tips. The active v055 staged-codec
native PDP-6 ABI check passes; the v091 PURE swap test no longer links
against current backing-store and process state APIs and was not ported.
The existing D6LZ36 production path is unchanged and 32K tests remain
deferred. Archive branches are retained for a future atomic codec cutover.

## v1019 historical DSH failure cleanup recovery

The old DSH v53 source branch was not wholly redundant: the current
`dsh_exec_pipeline` failure path lacked its successful-child WAIT loop.
The source fix was re-applied against current main, paired with the
historical three-case simulator acceptance probes, and verified under a
fresh 256K full-profile KCC image (3/3). The historical testkit media
isolation facility was likewise missing and has been recovered with a
passing host isolation regression. These are current integrated changes,
not merely a historical documentation comparison. The source was not
wholesale-merged because other branch code predates newer DSH changes.

# Historical branch integration audit v1006 (2026-11-02)

Scope: DAIMOS branches evaluated against `main` at e4e65995. This is a
non-destructive branch review, not a declaration that all old work is merged.

## Reviewed branches

* `archive/devspace-detached-20261011-v1`: `git cherry main` marks its
  sole commit 013b2e03 patch-equivalent (`-`). No replay is necessary.
* `backup/pre-push-20261005-v1`: all three commits are patch-distinct
  (`+`) according to `git cherry`, but the earlier DPY scroll/CLEAR
  change 30bcaad6 has a later implementation at e03c0562. The current
  tree contains `userland/manual/CLEAR.SIXMD` and its image integration.
  Do not replay the backup wholesale. Its picture staging and other
  optimization changes still need semantic comparison.
* `checkpoint/shrink-20261006-v1`: its hardware-documentation commit
  33d0e264 is patch-equivalent; its kernel shrink commit 02fa024e is
  patch-distinct. Compare against current MM and resident-accounting
  implementation before reusing any hunks.
* `v155-dtc-stage1-wip-20261015`: the Stage-1 file differs from current
  `main`, which subsequently added the TSFS hybrid boot format (v156).
  Never directly replay the old 106-word optimization without checking
  the hybrid raw/metadata skip path and testing native 32K boot.

## Rules for remaining reconciliation

1. Keep all historical refs and worktrees until every distinct patch is
   classified as implemented, superseded, or a verified missing feature.
2. Do not infer missing functionality merely from divergent commit hashes.
3. Verify compiler/assembler correctness and native boot before merging
   any old optimization into the current source.
4. Commit each independent, tested integration to a versioned branch and
   push without force; keep regression programs in daimos-testkit.

No source changes, native test passes, or size savings are claimed here.

## v1007 follow-up: DTC Stage-1 status check

The old v155 patch contains a valid local simplification: Type-136 status
bits 034 (controller/data error) and 002 (premature end zone) may be checked
by one `CONSZ` with their combined mask 036. This change was reapplied to
the current hybrid-TSFS Stage-1 without changing its streaming logic.
The current standalone DAS and DXRCONVERT assembly succeeded. A same-tool
before/after DXRCHECK reports octal `000217` (143 decimal) before and
`000214` (140 decimal) afterwards: three linked image words saved.
The historical `daimos-stage1-size-v1.sh` cap of 106 words is stale for the
later hybrid loader: the updated DTC image reports 140 words and fails
that cap.
Do not count a 32K or hybrid tape boot acceptance until the appropriate
native test is rerun and the size policy is reconciled independently.

## v1008 checkpoint implementation ownership review

The assembler routines introduced in checkpoint 02fa024e still have
corresponding files in current `main`: `mm_pdp6.s`, `memfs_data_pdp6.s`,
`memfs_snapshot_pdp6.s`, `d6fs_provider_namespace_pdp6.s`, and
`module_runtime_pdp6.s`. The current MM assembler has undergone later
functional changes, including pinned extent growth and swap transfer
validation; the old checkpoint is therefore not a safe wholesale replay.
Its report-permanent accounting correction and historic size figures are
also not interchangeable with modern linked/package measurements.
Any remaining optimization must be rederived against the present code and
measured independently before being counted as saved RAM.

## v1009 native Stage-1 entry acceptance

The testkit `daimos-stage1-kinit-entry-v1.sh` was executed against the
v1007 three-word DTC Stage-1 reduction on current `main`. After supplying
the actual sibling DAS and KCC source repositories in the isolated
worktree layout, the complete image built and the simulator reached
KINIT entry `53326` (octal): PASS. D6FS image checks also completed.
The first two isolated attempts were blocked by missing sibling DAS/KCC
source trees; these were harness layout problems rather than target
failures. This PASS proves the test's entry breakpoint contract, not
every 32K hybrid TSFS read or failure-injection path.
The historical 106-word DTC size cap remains obsolete: the current
hybrid loader is 140 words after the reduction. Existing KCC identifier
truncation and unused-symbol warnings remain separate toolchain work.

## v1010 non-32K checkpoint reconciliation

The v02fa024e `module_runtime_pdp6.s` fixed-binding label-count change
was subsequently integrated as 65651a49, and current code continues to
derive the loop count from the table labels rather than a hardcoded count.
The checkpoint's low-memory EXEC descriptor preservation is also already
present: the current routine retains the original META word, restores it
after loading, and resets swap backing and BRK state on commit. No replay.

The old `mm.c` optimization combined free-word accounting with fit search
to avoid a second scan on fragmented allocations. Current production
`mm_pdp6.s` already performs that accounting in `mm_find_fit_gap` and
returns total free words on a failed fit, so that saving is also present.
All three have changed since the checkpoint; do not mix historical C and
current assembler implementations. This reconciliation is source-only;
no new linked size reduction or native execution test is claimed.

The requested 32K-specific tests remain deferred. Further branch review
must avoid running the 32K boot, memory-pressure, and size gates until
that deferral is explicitly lifted.

## v1011 filesystem and RUN checkpoint comparison

Reviewed historical backup commit 79b90a71 against current `main`.
Its four MEMFS scan-loop `AOJA` substitutions are already present in
`memfs_runtime.s` (find-child, new free slot, unlink child, readdir).
The D6FS provider free-map population count is also present in
`d6fs_runtime.s`: fewer saved accumulators, AC6/AC7 quotient/remainder,
and the tail-bit path reuses the common population-count loop.

The historic `proc_run.s` change separated allocation, startup record,
hierarchy, file-table, and descriptor-map failures. Those labels still
exist and now funnel through shared `proc_run_claimed_bad` cleanup.
Later RUN changes, notably the v896 ownership-publication correction,
have superseded the old control-flow details. No older RUN hunks should
be replayed without testing against the latest slot-lifetime contract.

This comparison identifies already integrated functionality only; it
does not claim new savings or a regression pass. No 32K tests were run.

## v1013 branch pruning and rejected experiments

Git ancestry was evaluated across local branches and origin-tracking refs
in DAIMOS, daimos-testkit, and pdp10-doc. Before deletion, 69 independent
commit tips covered the three repositories; every other candidate tip
was an ancestor of one of those retained tips. The cleanup excludes main,
live worktree branches, and the independent tips. Remote branch deletions
use exact expected-SHA leases to avoid removing concurrently updated work.
Ancestry redundancy is not a claim that the remaining independent work
is merged to main; those tips require individual review.

The 2026-10-08 odd/even MM metadata packing experiment was explicitly
rejected in its own measured report. At 32 descriptor slots it saved 16
resident data words but added 48 code words, **growing total resident
memory by 32 words** and adding owner/pin limitations. The production
two-word MM descriptor implementation remains authoritative. The paired
testkit odd/even prototype is specific to this rejected representation.
The 160x92 scale-1 DPY terminal experiment similarly enlarged its lazy
text buffer from 1288 to 4968 words and was never selected for main.
Their experimental refs may be discarded under the RAM-priority policy;
this note preserves the decision and quantitative reason.

The KCC native-source documentation was separately recovered into the
current pdp10-doc main, corrected to match MAKEFILE plus DAIMOS.MK
staging and current build rules. No target self-hosting acceptance or
32K testing is claimed by this cleanup.
