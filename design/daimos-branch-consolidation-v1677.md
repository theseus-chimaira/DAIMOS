# DAIMOS branch consolidation inventory v1677

Date: 2026-11-19. This is a **read-only branch-tip audit**, not a
claim of mainline integration or permission to delete unverified files.
The exact 180-ref inventory with commit IDs and ancestry classification
is in `design/daimos-branch-ref-inventory-v1677.tsv`. It was generated
from a fetched/pruned `origin` using `daimos-testkit`'s versioned
`tests/validation/daimos-branch-inventory-v1677/audit-branches-v1677.py`.
The script is non-destructive and refuses any tip outside BOTH tested
ancestry heads. Explicit expected heads for this snapshot:

- main: `850bbd46f0c74c6c4665f3f63217fca2202c704d`
- PURE development: `5038159ecafcfc0370bfa759dce266977e40466f`

## Audited references and blockers

- 91 local branches; 89 real origin branches (excluding symbolic HEAD).
- 44 refs are ancestors of both heads; 134 refs only of PURE; two
  mainline refs only. Zero unique/divergent branch tips at snapshot.
- Three local-only branches: `d6codec-block-opt-v1441-20261110`, `daimos-native-selfhost-v1414-20261109`, `tsfs-codec-bitmap-mres-v1434-20261110`.
- One remote-only branch: `f10-identifier-cleanup-v1152`.
- 15 attached worktrees, including the main and active PURE worktree.
  Four carry local uncommitted/untracked changes:
- `refs/heads/main`: 4 uncommitted/untracked paths
- `refs/heads/d6codec-block-cutover-v1452-20261110`: 18 uncommitted/untracked paths
- `refs/heads/kcore-p1e-cache-placement-v1635-20261116`: 1 uncommitted/untracked paths
- `refs/heads/daimos-native-selfhost-v1414-20261109`: 7 uncommitted/untracked paths

## Companion `daimos-testkit` branch audit

The same read-only script audited the exact testkit parent repository
using `--feature test-pure-pi6-fault-v1677-20261119`: **2 local and 2
origin branch refs**, none with independent ancestry outside `main`
and the v1677 fault-test development branch. Its **20 attached worktrees
include 11 with uncommitted/untracked files**. All must be reviewed
and preserved before retirement; testkit v1677 source and physical
fixtures must first be accepted into testkit main. No testkit branch
or worktree has been deleted. Exact audited refs:

| Scope | Branch | Commit |
| --- | --- | --- |
| `local` | `main` | `605da958a919` |
| `local` | `test-pure-pi6-fault-v1677-20261119` | `b55d364567d3` |
| `origin` | `main` | `605da958a919` |
| `origin` | `test-pure-pi6-fault-v1677-20261119` | `b55d364567d3` |

## Mandatory integration/retirement sequence

1. Finish current P1-E physical PI6/MM/backing failures, ROOT=auto,
   KCC FULL regression and agreed KA10/mode3 feature boundaries.
   Preserve accepted mainline source and existing default-OFF behavior.
2. Refresh `git fetch --prune` and rerun the **complete** audit before
   integration. Refuse deletion if any branch tip has unique work not
   included in the reviewed target. Reconcile mainline-specific changes
   against the PURE branch, not simply fast-forward a stale mainline.
3. Inventory all attached worktrees and untracked files. Preserve dirty
   patches/source and independent commits before retiring a worktree;
   never use `git worktree remove --force` or blanket `git branch -D`.
4. Merge only after source-linked build, boot/MM, PI6 and negative tests
   pass. Confirm all *local and remote* topic branch tips are ancestors of
   the new `main` commit, including formerly PURE-only branches and the
   unpaired remote/local names above.
5. Consolidate the linked `daimos-testkit` local and remote topic
   branches and worktrees too; the v1677 test branch is an integration
   dependency, not a permanent fork. Refuse to discard any of the
   currently dirty testkit worktrees or untracked test sources.
6. Retire superseded clean worktrees and local topic refs with checked
   ancestry (`git branch -d`), then remove corresponding remote topic
   branches only after fresh origin verification. Keep `main` and other
   active/specially retained refs. Confirm no untracked files are lost.
7. Run final complete inventory: zero unmerged tips, zero orphan worktrees,
   no stale superseded local/remote refs, clean retained worktrees. Record
   the resulting final branch map and pushed main commit. Do not claim
   consolidation complete before that validation.

No branch/worktree was removed by this audit. The present testkit
native v1677 MM fault preflight PASS is not yet physical PI6 acceptance.
