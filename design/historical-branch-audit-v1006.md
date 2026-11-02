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
