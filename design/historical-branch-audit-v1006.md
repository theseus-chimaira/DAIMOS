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
