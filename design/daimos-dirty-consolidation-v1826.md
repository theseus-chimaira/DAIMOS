# Dirty-worktree consolidation v1822-v1826

Date: 2026-11-23. Scope: DAIMOS, daimos-testkit and pdp10-doc;
all pdp10-testkit checkouts and test ownership are excluded.
This is a completed preservation/reconciliation audit, not a new
kernel or native self-hosting feature.

## D6CODEC kernel and testkit history

Historical DAIMOS cutover chose the 56-word v1457 decoder. Current
production uses v1458: 53 TEXT words, zero DATA/BSS/relocations,
fixed address 000060 and a 58-word VFS adapter. The untracked old
v1458 include was otherwise identical to production except a stale
size comment. Rerun of native v1458 acceptance PASSed six valid and
five malformed fixed-core cases, eight valid and nine malformed VFS
cases, five complete DXR cases, a 52-word stack canary and all five
Stage-1 device assemblies (8-9 words saved against v1452).

The old detached kernel worktree's 11 tracked changes and seven
untracked candidates were verified and retired after archiving:
`~/backups/daimos-d6codec-v1457-cutover-retire-v1822-20261123/`.

The detached testkit held nine earlier D6CODEC native/speed/host
tests. Its two v1458 tests were byte-identical to tests already
tracked on main. The distinct host v1452 test was adapted to the
current build as v1824, removing an obsolete exact 29-DXR count.
The source-linked host C compressor's 36-bit output matches the
independent Python v1421 reference exactly on 31 real boot DXRs,
the full DSH and eight stress streams: 94,276 source words.
Production C roundtrips each stream and rejects seven bad streams.
Published testkit commit `5f62723`; originals archived at
`~/backups/daimos-d6codec-legacy-testkit-v1824-20261123/`.

## Native self-hosting proposals and diagnostics

The old v1414 CAT and MAKE edits were superseded by v1814. Current
CAT streaming passes 0, 7, 258, 259 and 1000 character S6REC records;
MAKE main-frame size is 587 -> 122 words (465 saved), while TEXT
decreases 7876 -> 7748 with unchanged BSS 839.

The historical v1413 source-staging script, two native DAS/MAKE
MAKEFILEs, and DSH gate increase 036000 -> 037000 were **not**
merged. The old source branch's five tracked and three untracked
files are archived, with a Git patch and original base, under
`~/backups/daimos-selfhost-v1413-v1414-dirty-retire-v1823-20261123/`.
The separate DSH 036133 > 036000 boot-image limit is still open.

The detached v1413-v1415 testkit contained 23 untracked self-host
dry-run probes, two SIMH watchpoint/checkpoint harness variants,
and three early DAS relocation fixtures. These tests depended on
unaccepted native source staging, and no full native compiler build
was performed. The originals are SHA256-preserved under
`~/backups/daimos-selfhost-testkit-v1413-v1415-diagnostic-v1826-20261123/`.

## Stale standalone documentation

The old detached pdp10-doc edits referenced a retired host mkswap
utility and manual, stale SIXMD inventory assumptions and an older
50-Hz simulator SDL error. Current v1820 SIXMD checking passed
77 public, two private and 26 optional assembler utility manuals;
later clock-native tests cover both 50 and 60 Hz. Those historical
claims were not reintroduced as current status. Four tracked edits
plus one untracked document were saved with SHA256 and a Git patch:
`~/backups/daimos-pdp10-doc-v1072-v1073-retire-v1825-20261123/`.

## Remaining dirty boundary

Read-only inventory (excluding pdp10-testkit): **4 dirty worktrees,
5 tracked edits, 18 untracked files**. DAIMOS and daimos-testkit
main were clean when scanned. Four existing documentation changes
remain in pdp10-doc main, intentionally unmodified. Three detached
daimos-testkit compiler/KIR/DOBJ worktrees (`a89afa39`, `ab8c2ac8`,
`afc33406`) were preserved rather than moved or archived because
their intended owner falls inside the excluded compiler test scope.

The versioned exact inventory and SHA256 reside at
`~/backups/daimos-branch-report-v1826-20261123/`.
No branch/worktree was deleted. Every retired dirty file has a
verified archive and per-file SHA256; tracked modifications also
retain their original Git binary patch and base commit. No excluded
repository, compiler, kernel source, or device image was changed.
