# v1677: source-linked 48K PDP-6 RAW PURE lost-return fault PASS

Date: 2026-11-19. Branch: `kcore-pure-reclaim-pressure-v1673-20261118`.
Scope is a single deterministic failed-result after a genuinely
committed PURE RAW DROP, plus accepted independent native two-owner
failure scenarios. This is **not** completion of P1-E PI6 fault pressure.
No original production KCORE, scheduler, MM or PURE transaction source
was modified. The original fixed KCORE image is byte-for-byte identical
to v1674. All source substitutions are test-only daimos-testkit inputs
in separate versioned build/output directories, never installed in
`system/kernel/` or a production checkout.

## Test fixture and fault trigger

The original 48K three-member TSFS root contains the deterministic
v1674 test PID1 and nine real 36-bit PIPE-held PURE children. Their
real RUN/BRK, MM reclaim, state transitions, PI6 schedule, restored
writable-suffix canaries, EXIT and WAIT paths execute unchanged.
The ninth child is held through original CTY checkpoint B, and a
native tenth RUN is refused under physical MM fragmentation.

The test-only version of `proc_pure_service_pdp6_v1612.c` calls
**the actual** `proc_pure_ram_drop_v1381(slot)`, commits the original
MM rebase and owner/unpin effects, then overrides only its first
successful return with `MM_ERR_BUSY` (one lost-success reply). A
single MRES-private word `v1677_fault_return_count` counts exactly
one such original successful DROP followed by erroneous refusal.
The original MRES owner proof must use the real PACKED descriptor,
clear TRANSITION only when the owner is singly owned and unpinned,
and continue normal reclaim/restore without a second registry or an
unsafe blind second unpin. No new fixed-KCORE data or instructions.

Two actual independently built PDP-6 images use exactly the same
source-built fixed `kcore.dxr` (SHA256
`66296e13440e1f4db3bf263e181dc33e085f55dc4b0f07c09f147e6c1c88b506`,
identical to v1674) and versioned test-only PURE MRES
(SHA256 `ebb45033ecacb80fb99c783c59ed6ef78ee027f323e9279e8f5c35fc112512c0`).
The GENERAL KINIT includes the normal DISK MRES candidate; STRICT
omits it, with same post-user KCORE and PURE MRES instructions/data.
Only the isolated fault MRES is +7 managed words against the accepted
v1674 normal PURE control; its extra test data is never shipped.

## Original physical results (PASS, scoped)

| 48K original physical state | GENERAL | STRICT |
| --- | ---: | ---: |
| A free / largest MM extent | 27332 / 27332 | 27332 / 27332 |
| B free / largest MM extent | 2685 / 1532 | 2685 / 1532 |
| C free / largest MM extent | 27332 / 27332 | 27332 / 27332 |
| Native RUN children launched | 9 | 9 |
| Native tenth RUN refused | Yes | Yes |
| B original physical MM PACKED RAW owners | 9 | 9 |
| MRES injected lost-success return at B | 1 | 1 |
| Original physical A/B/C and SYS_MEMINFO oracle | PASS | PASS |
| Original owner/pin recovered after EXIT/WAIT | PASS | PASS |
| Corrupted MRES physical-witness negatives | 3 rejected | 3 rejected |

The original patched permanent KCORE `proc_swap_reclaim` at physical
octal `023401` contained JRST word `254000032065`. The separate
physical verifier derives the installed PURE MRES base from this actual
jump and the source-built module-local reclaim symbol; it then reads
the test-only counter at physical `035522` as `000000000001` at B in
both completely independent guests. A forged zero count, an invalid
JRST opcode, and a duplicate physical word are each rejected.
The original `measure-v1673.py` verifies real v1673 native MEMINFO,
PIN/OWNER and physical descriptor invariants at all three barriers;
`compare-fault-v1677.py` compares the source-built kernel/MRES images
and independently constructed general/strict v1673 reports.
The original paired physical reports are byte-identical:
SHA256 `7ba78a492c00f4b99f17d8ebecee8e0a170e4907c62424066bd9a08e4db6779a`.

## Independent native negative evidence

The v1677 native PDP-6 source-linked two-owner test includes normal
unpin, a true committed unpin whose result is lost, and a genuinely
refused second-owner unpin. The healthy owner can independently be
restored to READY_ONCE and the still pinned failed owner must remain
TRANSITION-held. The compiled source mutant deleting the original
`MM_EXTENT_PINNED` exact-owner guard fails the **new** native test
with guest exit `000000002202`. Original v1654/v1612/v1658 and
v1650 slot-0 PI6 serialization/mutants still PASS.

## Explicit exclusions and next gates

The accepted physical tests deliberately omit 71 continuous MM
write watches for speed. The earlier full v1677 watch runs were
stopped *before* B and left as diagnostic evidence, not accepted
high-water. The accepted v1674 normal-workload committed-MM high-water
remains 1255 free words but is **not** an injected-fault high-water.
This scoped result is no proof of all real PI6 interrupt interleavings,
permanently pinned/uncertain MM failures, a failing FULL loader,
physical source disappearance/replacement, multiple simultaneous
faults, bad reaper terminal disposition or KA10. P1-E remains OPEN.
Do not merge the PURE feature to mainline because of this isolated
PASS. Next physical fault tests must exercise genuine original MM
pin/rebase failures, source/restore errors and PI6 scheduling with
both A/B/C provenance and relevant original MM write watches.

All validation code lives in the separate `daimos-testkit` branch
`test-pure-pi6-fault-v1677-20261119`, directory
`tests/validation/pure-pi6-fault-pressure-v1677/`, last accepted
scoped result commit `7ebee81`. All local/remote DAIMOS and testkit
branch/worktree integration and safety procedure is separately
recorded in `design/daimos-branch-consolidation-v1677.md` and remains
required on eventual P1-E mainline merge.
