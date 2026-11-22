# v1678: PDP-6 PURE physical original-MM precommit refusal (scoped PASS)

Date: 2026-11-19. Unmerged PURE development branch
`kcore-pure-reclaim-pressure-v1673-20261118`. Test-only original MM
geometry refusal, source-linked 48K GENERAL and STRICT PDP-6 physical
execution. This is not full P1-E, PI6 fault-interleaving, or write-watch
high-water acceptance.

## Original MM fault and unchanged production KCORE

The v1678 native source-linked two-owner preflight exercised two genuine
MM refusals: invalid zero pin base and invalid rebase geometry after a
successful original pin. Each refusal left the affected process RESIDENT,
with exactly the original physical owner, contents, zero leaked pins,
matched PI disable/restore and no TRANSITION. The first independent
owner can still restore and the failed owner can retry later with the
unaltered assembler. Two separately compiled executed unsafe source
mutants (missing cleanup unpin, retained TRANSITION) were rejected by
the new native test, not by an older baseline-only oracle.

The physical v1678 build copies the original DAIMOS source into a
versioned isolated scratch source tree and replaces *only* its copy of
`proc_pure_ram_v1381.c` through the testkit generator. The original
`mm_pure_rebase_pinned_v1381` assembler actually receives a one-word
invalid PURE sector skip on its first eligible call, *after* the
original source pin. Its actual `MM_ERR_INVAL` return increments the
one-shot test-only `v1678_rebase_fault_count` word; the original C DROP
cleanup unpins and leaves the source/resident process untouched.
Subsequent DROP calls use the unmodified original geometry/assembly.
This is not a fabricated successful MM operation, nor a modification
to production DAIMOS, the permanently linked KCORE or its scheduler.

The two independently source-built GENERAL/STRICT 48K systems share an
identical fixed `kcore.dxr` (SHA256
`66296e13440e1f4db3bf263e181dc33e085f55dc4b0f07c09f147e6c1c88b506`,
byte-identical to v1674). The test-only PURE MRES is identical across
the two profiles (SHA256
`d1df7423d2b8698cb71b1edd53452eec2b1eb93666c63e4f112f52c6fae01e01`).
Neither build is installed in the production tree. The original
v1674 deterministic nine-child PIPE-held TSFS fixture ensures real
native RUN, BRK, blocked PROC_SLEEP, reclaim, restored mutable-suffix
canary, EXIT/WAIT and a genuinely fragmented refused tenth RUN.

## Paired original physical A/B/C acceptance

| Actual 48K MM/PROC observation | GENERAL | STRICT |
| --- | ---: | ---: |
| A free / largest extent | 27323 / 27323 | 27323 / 27323 |
| B free / largest extent | 2548 / 1523 | 2548 / 1523 |
| C free / largest extent | 27323 / 27323 | 27323 / 27323 |
| A / B / C original MM live descriptor count | 9 / 26 / 9 | 9 / 26 / 9 |
| B actual RAW PACKED children | 8 | 8 |
| B failed-rebase still RESIDENT child | slot 2 | slot 2 |
| B original MM pins and process TRANSITION | no leak | no leak |
| Native refused tenth RUN, EXIT/WAIT | PASS | PASS |
| Original A/B/C MM/PROC/MEMINFO oracle | PASS | PASS |
| Original rebase refusal witness | exactly 1 | exactly 1 |
| Physical A/B/C MM/PROC/MEMINFO mutants rejected | 5 | 5 |
| Source-linked JRST/counter witness mutants rejected | 3 | 3 |

Physical `proc_swap_reclaim` at octal `023401` contains its installed
JRST `254000032065`, whose target and the source-built MRES-local
reclaim offset locate the real test-only counter at physical `035533`.
Both independent original checkpoint-B reads return `000000000001`.
Every source-linked run checks this count was produced by the genuine
assembler refusal, not merely a fake host JSON event.

The v1678 testkit-specific original MM owner oracle inherits the exact
v1673 physical table/process/source reader but appropriately requires
**eight** PACKED children plus one *unchanged* RESIDENT failed child at
B. It also checks native SYS_MEMINFO against original memory, all
owners/pins and logical BRK words, final bit-identical live descriptors,
empty TRANSITION, one rejected MM rebase, full nine-child EXIT/WAIT,
and exact restoration of A's free extent at C. It does NOT substitute
for or relax the separate v1673/v1674 nine-PACKED normal-pressure tests.

Independently produced physical state JSON reports are byte-identical,
SHA256
`ef48a65af796f55e8601e3957d6d5d7f028c2f82d758c4503724075d0f9fd291`.
The original physical snapshots rejected five distinct tamperings per
profile: failed RESIDENT child's TRANSITION retained, a PACKED extent
span corrupted, a PACKED MM pin forged, the final live descriptor
corrupted, and an incorrect native MEMINFO process-word count. Three
further source-linked installed-JRST/relocated-fault-counter mutations
are rejected in each profile. The v1678 native original MM test and
its two executed original source mutations pass again on current code.

## Limits and next test

This gate proves one physical precommit invalid-geometry rejection,
correct unpin/TRANSITION cleanup, bystander reclamation and full final
recovery. It does NOT prove arbitrary persistent pin/rebase failures,
failed FULL backing reads, source disappearance, terminal failure policy,
all possible PI6 interrupt schedules, KA10 or a continuous physical
MM-write high-water. Full 71-write-watch fault-pressure experiments
are separately required; the accepted **normal** v1674 low-water
of 1255 free words is not a v1678 fault minimum.

Next: physical persistent/repeated MM failures (including retained pins)
with terminal recovery or explicit fail-closed disposition; physical
FULL/backing-source fault interleavings; then matched ROOT=auto,
DSK/DRM and KCC FULL with PURE before mainline merge. Mainline
integration includes independently inventoried/cleaned local and remote
DAIMOS **and daimos-testkit** branches/worktrees only after acceptance,
as required by `design/daimos-branch-consolidation-v1677.md`.

Tests and reproduction: daimos-testkit branch
`test-pure-mm-precommit-v1678-20261119`, commit `d5b27d3`, directory
`tests/validation/pure-mm-precommit-v1678/`, including native source
mutants, full physical source-copy builder, original A/B/C reader,
paired compare and physical MM/witness negative suite.
No production DAIMOS source changes or KCORE resident word growth.
