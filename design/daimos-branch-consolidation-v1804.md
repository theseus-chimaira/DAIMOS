# DAIMOS branch consolidation v1804: integration candidate

Date: 2026-11-22. Status: **IN PROGRESS**. This report describes
the isolated `kcore-raw-pure-integration-v1804` worktree, not
an accepted or merged mainline release.

## Read-only ancestry inventory

The DAIMOS mainline base is `495060f6` (K01-K04 v1803 closure).
The latest RAW PURE development head is `5ef8b15a` on
`kcore-pure-reclaim-pressure-v1673-20261118`. There are 91
local DAIMOS branches, including main. All 90 non-main branch
tips are reachable from the RAW PURE tip; 23 are also ancestors
of main, and 67 are PURE-only relative to main. The two
independent named-branch tips are main and RAW PURE. Thus the
67 divergent names do not represent 67 independent histories.

An additional detached DAIMOS worktree points at `78259106`,
a one-commit v1285 OOM-enable candidate not reachable from
either named head. Its implementation must not replace the
accepted v1298 requester-only OOM path. Preserve that object
until the detached worktree is retired deliberately.

The initial four DAIMOS worktrees include dirty main, dirty
D6CODEC and dirty native-selfhost worktrees, and the clean
detached OOM worktree. The new isolated worktree contains the
only v1804 candidate merge. No existing worktree, branch or
untracked file has been removed or reset.

## Companion repositories

`daimos-testkit` has nine local branches including main.
Its named-branch independent tips are main `3a4604e`,
PURE multiowner `983c5fb`, and PURE stop/foreign `5fa1a2a`.
A detached worktree points at independent `c43d599`, which
adds the historical DTFS v204 mount/lookup test fixture.
The testkit has twelve worktrees at inspection, eleven dirty;
the independently maintained test fixtures must be preserved.
An isolated `testkit-pure-integration-v1804` branch now joins
the two PURE tips to main (merge commits `601cc99` and
`2a19c80`); `c43d599` is cherry-picked as `10c32f4`.
The testkit main branch remains unmodified.

`pdp10-doc` has only main `93e1985` as a named branch;
both detached worktree heads are already its ancestors.
Two of its three worktrees are dirty. Permanent documentation
will be updated after the implementation is accepted.

## Isolated RAW PURE merge

The trial `git merge-tree --write-tree` of main and the PURE
tip reported three textual conflicts: `KCORE.MD`,
`PROJECT-STATE.MD`, and `TODO.MD`. No source-level textual
conflicts were reported. The candidate merge retains mainline
versions of those current-status documents and retains the
PURE history in the second merge parent; PURE-specific design
reports are preserved in the resulting source tree.

The candidate adds or changes 48 files under `system/`.
The merged Makefile retains default-OFF flags
`DAIMOS_PURE_MRES_ENABLED=0`, `DAIMOS_PBM_V1667=0`,
and `DAIMOS_SWAPLESS_ONLY_V1668=0`. Textual conflict freedom
does **not** establish runtime compatibility with K01-K04,
especially MM ownership, teardown, slot-0 serialization,
backing lifetime, PI6 progress, and KCC compilation.

## Acceptance boundaries

1. Validate the candidate in isolation with the installed
   toolchain and existing mainline build gates.
2. Reconcile the two independent testkit PURE heads and the
   detached v204 test fixture without destroying dirty worktrees.
3. Run mainline K01-K04 refusal and lifecycle regressions and
   the development PURE source and physical gates; repeat
   48K/64K pressure and 256K disk-root regressions.
4. Keep mode3 and RAW PURE default-OFF until source, owner,
   MM, scheduler, loader, and provider contracts pass.
5. Only after acceptance, merge the reviewed candidate into
   main, reconcile current status docs, and safely retire
   topic branches whose tips are ancestors of the new main.

No source-level release acceptance, RAM win, branch deletion,
or mainline integration is claimed merely because the
candidate tree can be merged.

## Early integration test results

On the source-combined isolated worktree, all standard PDP-6
Stage-1 loader targets assembled. The default-disabled
KINIT/KCORE source generation passed with native KCC. The
independent testkit source-linked v1684 FULL/PURE two-owner
native test PASSed, including its two unsafe mount/backing
negative controls. The K03 v1802 static guard PASSed with five
negative mutants. The K04 v1801 **read-only historical**
closeout verifier PASSed, including its v1800-A/B D6FS CLEAN
checks. These replayed witnesses are not new v1804 guests.

The complete v1804 disk-root image build, physical boot and
48K/64K pressure acceptance are separate gates; passing the
earlier source-generation stages does not certify them.
