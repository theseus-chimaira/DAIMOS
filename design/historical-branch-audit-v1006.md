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
