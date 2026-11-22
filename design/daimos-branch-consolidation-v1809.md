# DAIMOS and testkit mainline branch consolidation v1809

Date: 2026-11-22. Scope: Git history consolidation, not RAW PURE
feature acceptance or default-mode promotion.

## Mainline ancestry

The accepted DAIMOS v1803 mainline was 495060f6. The isolated
RAW PURE integration tip ec4ec261 included main as an ancestor and
contained all 70 topic heads that were not yet merged into main.
The DAIMOS main branch was fast-forwarded to ec4ec261, without
rebasing, squashing or changing the tested integration source.
After the fast-forward, no local or origin-tracking topic branch
remained unmerged relative to local main.

The corresponding daimos-testkit mainline was 3a4604e, and its
source-matched integration tip was 4841f2e. Its eight unmerged
topic branches were all ancestors of that tip. The testkit main
branch was fast-forwarded to 4841f2e; no named local or
origin-tracking testkit branch remained unmerged.

The detached archival DAIMOS v1285 candidate and testkit v204
history already have explicit archive tags from v1804. Devspace
recovery references are preservation checkpoints, not named topic
branches; do not fold unrelated recovery objects into main.

## Acceptance and retained boundaries

The exact ec4ec261 DAIMOS integration source with 4841f2e
testkit passed the complete opt-in v1808 source-linked native
matrix, including v1677-v1684 owner/finalizer tests, K03/K04
controls, both v1808 lost-success-reply restorations and the
v1667 mode3 compatibility negative. An independent default-OFF
full `make build` on this source passed, including all PDP-6
Stage-1 assembler targets and host tools.

Previously documented v1805-v1808 physical results remain scoped:
256K DSK root default-OFF smoke, 64K TSFS PURE boot and two
independent 48K GENERAL/STRICT RAW reclaim/restore witnesses.
The v1807 physical reports remain the primary evidence; no new
physical 48K pair is claimed for the v1809 history operation.

The default build still selects
`DAIMOS_PURE_MRES_ENABLED=0`,
`DAIMOS_PBM_V1667=0` and
`DAIMOS_SWAPLESS_ONLY_V1668=0`.
The measured v1804 default-OFF permanent last-address delta
relative to v1803 was +48 words, split into -5 fixed KCORE
words and +53 removable SWAP MRES words. This is not evidence
that PURE is enabled, that permanent RAM has been optimized,
or that the original 32K full-boot capacity issue is closed.

K01-K04 remain closed under their existing accepted profiles.
K05-K08 failure, ownership, source and PI6 pressure matrices
remain open. K09 production enablement and full multi-root,
hardware and release acceptance are also open. Bitmap mode3
remains disabled and unaccepted. The pre-existing dirty-root
reboot behavior is documented separately in v1806.

## Dirty worktree preservation and branch retirement

Before the fast-forwards, the six DAIMOS main worktree changes
and 21 testkit main worktree changes (including untracked files)
were preserved in complete versioned local file archives,
binary diffs and Git stashes. Local annotated safety tags:

- archive-daimos-dirty-main-v1809-20261122
- archive-daimos-testkit-dirty-main-v1809-20261122

These snapshots must not be blindly reapplied: several older
working documentation edits predate the accepted v1802-v1803
status and could reintroduce stale claims. No dirty auxiliary
worktree was reset. Attached or dirty branches must not be
removed merely because their commit tip is now contained in
main. All historical commits remain reachable through main;
record commit IDs before considering deletion of clean,
superseded references.

This report supersedes the isolated-candidate status in v1804
and v1807 only with respect to mainline Git ancestry. It does
not expand any engineering acceptance claim in those reports.

## Completed safe topic-ref retirement

After both new mainline heads were pushed, the live GitHub head
sets were checked for exact SHA/name equality with fetched
remote-tracking refs. Every proposed deletion was separately
proved an ancestor of the published origin/main. Only refs
unattached to any existing worktree were eligible.

Atomic remote deletion retired 87 DAIMOS and seven testkit
superseded topic heads. Ordinary `git branch -d` then retired
88 local DAIMOS and seven local testkit heads. No forced deletion,
force-push or commit-history rewrite was used. All retired
commits remain reachable from the published mainline and are
recoverable using the exact pre-retirement TSV manifest at
`design/daimos-branch-ref-inventory-v1809.tsv`.

Final audit: DAIMOS retains four local branches and three live
remote heads; testkit retains three local branches and three
live remote heads. Each remaining non-main local branch is
checked out in an existing worktree. Dirty D6CODEC, self-host
and testkit worktrees were not altered. The two clean integration
worktrees were also retained with their ignored build/test
artifacts, rather than forcing their removal.

Both local mainlines match origin/main, and zero local or remote
topic heads are unmerged in either repository. The independently
maintained pdp10-doc/toolchain/simulator repositories are outside
this branch-retirement operation.
