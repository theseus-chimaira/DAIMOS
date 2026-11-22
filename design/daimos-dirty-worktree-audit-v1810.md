# DAIMOS dirty-worktree and recovery-source audit v1810

Date: 2026-11-22. Status: source-preservation and triage COMPLETE;
candidate feature/bugfix promotion NOT YET ACCEPTED.

## Preservation

The post-v1809 inventory found 17 dirty worktrees: two DAIMOS,
ten daimos-testkit, one KCC, two pdp10-doc, one pdp10-tools,
and one PDP-6 simulator. Each was captured without modifying
the original checkout. The persistent device-local archive is

    ~/backups/daimos-dirty-worktrees-v1810-20261122/

Its manifest-v1810.tsv records each original worktree path, HEAD,
branch state, file count, archive filename and SHA256 digest.
Each archive contains the exact contents of all modified and
untracked nonignored paths. Adjacent file manifests record
per-file SHA256, and retained binary Git diffs and status outputs
permit source-level review. All 17 archive SHA256 values were
independently verified. Ignored build products are not included
unless explicitly untracked by Git.

Two dirty mainline states parked before the v1809 fast-forwards
were copied from the earlier safety archive into
pre-consolidation-v1809/. Their original local annotated Git
stash tags remain intact. Those snapshots include six prior
DAIMOS files and 21 testkit files. They must not be blindly
applied over newer v1803-v1809 status or tests.

The source snapshots are local backups, not GitHub releases.
Before deleting any original dirty worktree, recheck archive
integrity and review its source against current main.

## Source candidates worth integrating

### P0: toolchain correctness and test reliability

- pdp10-tools dirty main: dlink.c ignores undefined EXTERN
  declarations that are never named by a DOBJ relocation.
  The existing unresolved-symbol and archive-extraction loops
  otherwise demand a provider for an unused declaration.
  The patch adds symbol_has_relocation with no persistent
  relocation index. DAIMOS tools/host/dlink.c and native
  userland/toolchain/dlink.c independently have the older
  unresolved-EXTERN policy: reconcile all three implementations,
  including the native file-backed relocation lookup.
- pdp10-tools dirty main: p10run.c classifies SIMH Step expired
  and Time expired as failures. Without those checks an unchanged
  initialized zero exit word can be mistaken for a completed guest.
  DAIMOS tools/host/p10run.c needs the corresponding fix.
  Require source-linked negative tests for timeout/step expiry.
- daimos-testkit dirty KCC/DAS trees contain uncommitted native
  KIR global-symbol probes (v1374, v1378, v1388), DOBJ relocation
  fixtures (v1404, v1412), and a DAS symbol-difference test (v1453).
  Preserve and review those as compiler/assembler regressions
  before more KCORE development.
- The parked pre-v1809 testkit main contains source-linked K01
  F07 RUN/MM cause probes (v1708-v1709), VFS/BACKSTORE refusal
  tests, and real-swap/DTFS diagnostics. These are useful diagnostic
  cases but need source-map and negative-control review.

### P1: demonstrated RAM and userland-value candidates

- DAIMOS native-selfhost dirty worktree: u_text_copy_line streams
  an S6REC record directly to an output descriptor; CAT no longer
  requires its 259-char automatic line buffer or truncates long
  native-source records. Existing bounded u_text_getline remains
  intact for callers that need it.
- The same worktree: MAKE goal names are relocated into the
  existing string arena and referenced by stable offsets rather
  than a two-dimensional automatic array. The historical v1414
  testkit README reports a reduction in main's compiler frame
  from 01113 to 0172 octal words (465 decimal words), with no
  extra BSS. This result has not been independently remeasured
  against v1809 main.
- That worktree also stages DAS and MAKE native source trees
  and carries two native self-hosting Makefiles. Its boot Makefile
  raises DSH_IMAGE_LIMIT from 036000 to 037000 and changes boot
  root contents. Do not merge that broader policy change merely
  to obtain the two userland memory fixes. The corresponding
  untracked v1413-v1415 testkit harnesses check staging and
  dry-run plans, not full native self-hosting acceptance.
- daimos-testkit's v1682 dirty STOP/foreign-TERM verifier adds
  the observed APR growth marker and five physically mutated
  negative controls; preserve for paired native/physical review.

### P2: standalone regression and diagnostic candidates

- Testkit untracked original DTFS v206 write-preservation,
  v1636 low-BCACHE/six-owner physical-pressure, v1073 manual
  inventory, v1071 offline-MKSWAP, and staged DOBJ/KCC scripts
  remain useful, but source assumptions and historical fixtures
  must be reconciled before promotion.
- Older D6CODEC dirty tree contains alternative v1454-v1458
  fixed decoders and an unused v1457 VFS adapter. The current
  integrated v1458 fixed decoder is already 53 words, versus
  the historical 56-word candidate. Do not replace it with the
  older implementation; retain codec fixtures for comparative
  regression and provenance.
- Both pdp10-doc dirty trees contain real additions to clock,
  sixmd, BACKSTORE, memory management and KCORE state. Their
  dated acceptance/status claims need reconciliation; do not
  simply overwrite newer accepted documents.

### Archival only, without current source promotion

- The dirty KCC checkout has untracked build binaries,
  generated DOBJ files and build traces, not identified source
  modifications. The dirty PDP-6 checkout has a generated
  headless emulator executable only. Both are preserved in
  snapshots, not candidates for source merging.
- Detached DAIMOS v1285 requester-only OOM experiment is
  superseded by the accepted v1298/v1803 production policy.
  Its detached worktree and older archive tag are retained.
- Historical device/TTY/WORDTOKEN commits dated 2026-09-30
  have implemented successors on main; their original code
  must not be mechanically cherry-picked. The old 160x92 DPY
  variant and odd-even MM plan are unaccepted experiments.

## Distinct old commits protected on origin

The v1810 audit added remote annotated archive tags without
changing main:

- archive-daimos-offline-mkswap-v1073-v1810: c57f9580,
  guarded offline MKSWAP implementation/manual support absent
  from current production. Needs rebase onto the newer AUXST1
  writer, physical-media safety checks and the preserved v1071
  test before production use.
- archive-daimos-mm-odd-even-plan-v1-v1810: ac981677.
- archive-testkit-mm-odd-even-reference-v1-v1810: 14d9775.
- archive-testkit-ptr-tsfs-image-v1-v1810: 6504acb,
  an old test alternative to newer PTR/TSFS evidence tests.
- archive-testkit-tty-pi6-v1147-v1810: b530aa7,
  partially superseded by current TTY owner tests.

The Devspace recovery namespace also contains 1,235 historical
review-snapshot commits not in main. These are not conventional
unmerged topic branches. A source filename inventory was made
without deleting recovery refs. Many absent paths refer to
retired layouts, old temporary tools and intermediate source
names; filename absence is not proof of useful unmerged code.
Do not delete the recovery namespace without further
object-level content review.

## Acceptance boundary and order

This pass preserves files and determines their likely value.
No dirty candidate implementation was silently merged,
installed or labeled regression-PASS. Maintain RAM-first
priorities, but fix toolchain/assembler correctness before
KCORE optimization. Proposed order:

1. Add source-linked regression and deliberate negative tests
   in daimos-testkit for the unused-EXTERN and p10run-expiry
   defects; reconcile host/native toolchain implementations.
2. Extract CAT streaming and MAKE arena-offset changes alone;
   test long S6REC lines, goal handling, stack size, and native
   use against current source. Keep the broader self-hosting
   boot policy separately gated.
3. Reconcile valuable native KCC/DAS, K01 and DTFS fixtures,
   avoiding duplicate or stale source-specific test claims.
4. Retain MKSWAP and the MM experimental designs as archived
   candidates until their separate safety/usefulness gates
   justify implementation.

Main remains v1809 default-OFF RAW PURE. K01-K04 are not
reopened by this audit. K05-K09 and their physical acceptance
requirements remain open.
