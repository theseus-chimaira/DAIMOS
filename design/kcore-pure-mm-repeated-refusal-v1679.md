# v1679: physical PDP-6 PURE repeated original-MM precommit refusal (scoped PASS)

Date: 2026-11-19. Branch:
`kcore-pure-reclaim-pressure-v1673-20261118` (unmerged).
This closes a bounded **repeated original-MM precommit invalid-geometry
refusal** gate, not the complete P1-E PI6/backing/persistent-uncertain
failure matrix or an intermediate-MM high-water bound.

## Test-only source and original hardware behavior

The separate daimos-testkit v1679 native test drives both a real original
MM zero-base pin refusal and a real original PDP-6 MM sector-geometry
rebase refusal **four times** against the same second RESIDENT owner,
while a healthy PACKED first owner restores independently. Each attempt
preserves every source word, MM type/base/length/owner/pin, VM APR base,
storage mode, clear TRANSITION, and balanced PI disable/restore. After
the original-MM refusal is removed, the second owner performs actual
zero-copy DROP and independent scheduler READY_ONCE restore. Two
separately compiled and executed original production-source mutants
are rejected by the new test: failed-rebase cleanup pin leak (guest exit
`000000003201`) and premature invalid RESIDENT APR clearing (guest exit
`000000003101`). The original v1654/v1612 service prerequisites pass.

The physical test makes a disposable source copy of the exact PURE
branch, modifies **only** its copy of `proc_pure_ram_v1381.c`, and
builds a source-linked optional PURE MRES. On every attempt for child
slot 2, the original DROP pins its source and calls the unmodified
`mm_pure_rebase_pinned_v1381` assembler with one invalid sector-skip
word. The assembler genuinely returns `MM_ERR_INVAL` before any
original MM descriptor commit. The original C error path unpins and
leaves the child intact as RESIDENT. Other processes still use original
MM code with valid geometry; no KCORE, scheduler, u-area or MM source
was edited in the production development checkout.

A separate versioned C89 test-only PID1 modifies only the accepted
v1674 deterministic PIPE-held fixture. It launches nine real children
and requires the genuine tenth native RUN to refuse on physical
fragmentation. It makes **two more actual refused RUN attempts**
before checkpoint B without releasing any of the nine pipe-blocked
processes. These additional ordinary RUN requests re-enter the original
PURE reclamation/MM path and attempt the same source's rebase again.
At B the original installed kernel `proc_swap_reclaim` JRST and the
source-linked relocated optional MRES reveal `v1679_rebase_fault_count`
at physical octal **035532**, with value `000000000003` in BOTH real
simulator guests. This is an actual original MM assembler refusal
counter, not an invented return code or an injected synthetic JSON.

## Paired original physical 48K results

| Original physical MM / native process measurement | GENERAL | STRICT |
| --- | ---: | ---: |
| A free / largest extent | 27324 / 27324 | 27324 / 27324 |
| B free / largest extent | 2549 / 1524 | 2549 / 1524 |
| C free / largest extent | 27324 / 27324 | 27324 / 27324 |
| Nine native RUN children admitted, tenth refused | PASS | PASS |
| Additional real RUN refusals under pressure | 2 | 2 |
| Actual original-MM rebase invalid-geometry returns | **3** | **3** |
| B original MM PACKED RAW children | 8 | 8 |
| B original RESIDENT intact refused child | slot 2 | slot 2 |
| A/B/C MM/PROC/native SYS_MEMINFO verifier | PASS | PASS |
| Exact live MM descriptors/pins after EXIT/WAIT | PASS | PASS |
| Real original guest exit and nine-child WAIT | PASS | PASS |
| Original fault JRST/counter mutations rejected | 3 | 3 |
| Original MM/PROC/MEMINFO mutations rejected | 5 | 5 |
| Missing real extra refused-RUN markers rejected | 2 | 2 |

The original physical MM reader is the accepted v1678 mixed-mode
source-linked parser: at B precisely eight RAW PACKED and one full
RESIDENT original process owner, expected logical BRK sizes, unchanged
original MM pin table, correctly mapped VM and storage mode, native
SYS_MEMINFO agreement, no TRANSITION left behind and exact active MM
state restored at C. It is not the nine-PACKED v1674 normal pressure
oracle, which remains separately accepted and unmodified. The v1679
wrapper additionally requires one unique source CTY marker for each
of the two extra native refusals. All ten changed original physical
witnesses per profile are rejected. A separate forged paired-report
negative is rejected by the same binary/state comparator.

The GENERAL and STRICT image identities match byte-for-byte, including
maps. Original fixed KCORE DXR SHA256 is
`66296e13440e1f4db3bf263e181dc33e085f55dc4b0f07c09f147e6c1c88b506`
(identical to v1674). The versioned **test-only** PURE MRES DXR SHA256
is `a24748d9ca7c3ab2451a411444c833c5a9ca80b8ec8ab643a61898f5e726b74b`.
The independent complete original physical A/B/C JSON reports have
the same SHA256:
`f731089e19ec472dc57a7537180a3988edb1d266a036cadb85d745a9a823de8e`.
No production KCORE/MRES data/text was changed by this acceptance.
The test-only image differs from a production RAW PURE module and
must not be installed or included in a release size report.

## Explicit exclusions and next work

The accepted v1679 physical gate intentionally uses the normal
unwatched A/B/C MM/PROC source observations. The 71-watch **continuous
physical MM writes** mode for the eight-PACKED/one-RESIDENT fault case
is NOT implemented or accepted; the v1679 runner refuses non-fast mode
instead of silently using the incompatible v1674 nine-PACKED oracle.
The v1674 normal guest minimum of 1255 free words is not a v1679
fault-injected minimum. These results do not imply successful recovery
from a genuinely retained unknown pin, permanent/uncertain MM owner,
multiple concurrent in-progress faults, failed FULL backing reads,
source disappearance or terminal cleanup, every PI6 interrupt
interleaving, KA10 or a final merge. P1-E remains OPEN. Next priorities
are source/loader loss, terminal uncertain-owner behavior, the
fault-specific continuous physical MM oracle and alternate-root/KCC
FULL regression before integration.

Source-linked reproduction and negative tests are in daimos-testkit
branch `test-pure-mm-persistent-v1679-20261119`, accepted commit
`e238a8f`, directory `tests/validation/pure-mm-persistent-v1679/`.
The prior v1677 lost-success and v1678 single-precommit-failure
gates remain independent. The future mainline merge **must** include
verified consolidation of every local/remote DAIMOS and daimos-testkit
branch/worktree, preserving unique commits and dirty files as required
by `design/daimos-branch-consolidation-v1677.md`.
