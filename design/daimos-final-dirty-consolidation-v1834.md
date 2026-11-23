# DAIMOS final historical dirty-worktree consolidation v1834

Date: 2026-11-23. Scope: versioned preservation, source comparison,
and documentation reconciliation of the last historical dirty
worktrees, excluding every pdp10-testkit checkout and operation.

## Recovered detached daimos-testkit worktrees

### KCC/KIR v1378-v1388 (`a89afa39`)

Five untracked historical probe and runner files exercised native
KCPP/KPARSE/KGEN and assembly comparison using the v1373 staged
boot. Their v1378 expectation checked that an exit-status *label*
appeared, not that its value was zero; the contemporary v1390
strict regression explicitly identifies that false-positive
failure mode. The current v1390 suite requires zero exit codes and
exact native/host assembly equality, with saved boot/guest logs.
The v1388 probes were likewise superseded by stricter acceptance.
The historical candidates were retained verbatim, not promoted.

Backup:
`~/backups/daimos-testkit-kcc-kir-v1378-v1388-v1831-20261123/`.
Five untracked files archived, SHA256 verified, and retired from
the detached worktree; status is clean.
Archive SHA256:
`8cd0ec36b6aeb028e156e8bb91b6aa63d4eb3a77b8ca443b15a2d2b26f95ccb8`.

### DAS/DOBJ v1404 (`ab8c2ac8`)

Eight untracked v1404 C89 fixture and staging scripts target
external calls, external data and static local data references.
The current v1405 testkit already covers all five modules with
29 strict guest probes including host/native assembly, DOBJ and
DXR image equality and guest return value seven. The v1405 probe
generator was rerun and produced 29 checks. Its historical README
correctly distinguishes an accepted individual fixture from an
interrupted sustained compilation run. No new full-guest claim was
made during this consolidation.

Backup:
`~/backups/daimos-testkit-das-dobj-v1404-v1832-20261123/`.
Eight untracked files archived and verified, worktree clean.
Archive SHA256:
`77cef711c32bb900852e6962a170374105f7760b678940db7638749c7f27c408`.

### KCC/KIR v1374 (`afc33406`)

Five experimental KIR1 inspection and graph-prefix scripts were
untracked. The sixth file was a modified tracked v1373 root builder
adding a provisional extra fixture API. That experimental extension
was not a current production capability, and full guest acceptance
was not available. Instead of mutating the active root builder or
passing a synthetic KIR graph off as native execution, the complete
historical input files and their tracked binary Git patch and base
commit were retained for source recovery. The detached worktree
is now clean.

Backup:
`~/backups/daimos-testkit-kcc-kir-v1374-v1833-20261123/`.
Archive SHA256:
`a2ae10a959a6c57d8168d446a63ca0a26760bbaf1921135e74a6806d9858052e`.

## The four pending pdp10-doc/main edits

An exact SHA256 snapshot of the original four uncommitted files,
their original base and the binary Git patch was verified under
`~/backups/daimos-doc-main-dirty-v1830-20261123/`.
Archive SHA256:
`d00f291a38a38332b2bfb964062e01c8e9e0e53aef3c27153ee51c70b245af05`.

The four documents were reconciled and published as pdp10-doc
commit `9639f0b`:

- `KERNEL/BSTORE.MD`: retain the genuine 256K v1508 checked
  release and PI6 timer evidence, correct the current self-test
  negative count to ten, and distinguish later v1509 resident
  bystander progress from healthy swapped-child recovery.
- `KERNEL/MEMORY-MANAGEMENT.MD`: retain the original v1708-v1710
  physical F07 diagnosis, current-process immobility and proposed
  MM compactor correction; mark its then-open K01 status historical
  because K01 closed at v1718.
- `STATUS/KCORE-FEATURE-STATE-V1508.MD`: retain the expanded
  physical transcript detail but distinguish current ten-mutation
  verifier from an extended 13-mutation historical check and
  label its old open-feature list explicitly historical.
- `TODO.MD`: preserve K01-K04 v1803 closure, P0-B/P1-C/P1-D
  closure and v1809 RAW PURE mainline integration instead of
  reverting to an obsolete unmerged-PURE status. Keep historical
  v1508 and v1604 entries dated rather than asserting their
  past OPEN judgments as the current state.

Source-linked verification: `daimos-backstore-persistent-verify-v1508.py
--self-test` PASS with ten rejected unsafe mutants; the current
`daimos-k01-compact-current-v1710.sh` native PDP-6 suite PASS with
the intentionally unsafe current-owner move rejected. These are
focused reruns; no new full F07 guest/physical acceptance is claimed.

## Completion and boundaries

The 19 dirty historic test files were all archived with exact
per-file SHA256 and removed/restored only after verification.
The four originally dirty pdp10-doc files are preserved and their
reconciled versions were committed and pushed. No Git worktree
or branch was deleted. Contemporary GCC/KCC qsort work in
daimos-testkit/main was left for its existing owner, not folded
into the historical branch artifacts.

No pdp10-testkit operation was performed and no kernel, KCC, DAS,
KCORE, DSH boot-gate or userland production source was modified.
