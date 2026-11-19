# v1680: PDP-6 terminal RAW PACKED owner pinned-retirement preflight

Date: 2026-11-19. Status: **native source-linked PASS, physical PI6
terminal/backoff gate OPEN**. Unmerged PURE branch
`kcore-pure-reclaim-pressure-v1673-20261118`.

## Source audit and fail-closed contract

The original optional-MRES assembler `pdp6_pure_detach_v1609`
requires exactly one PACKED MM owner with expected type, size,
provenance, backed u-area, absent runnable APR and a *zero* MM pin field
before it calls `mm_free`. The permanent KCORE `proc_swap_detach`
clears the swap record and u-area backing only after the selected
MRES terminal service returns success. The caller `vm_space_destroy`
propagates the failure without clearing original VM ownership.

The original `proc_finish_slot` (PDP-6 assembly) marks a failed
pre-VM release with a FINALIZE+PREVM continuation and retains
TRANSITION rather than publishing ZOMB or freeing the u-area. The
idle-stack `proc_finish_service_v881` retries only eligible marked
owners, and the timer code uses existing shared exponential retry
backoff with **no new permanently resident word**. This contract
must be checked in an independent real 48K physical/scheduler test.

## Executed original terminal native gate

The testkit native v1680 fixture links actual KCC-compiled
`vm_pdp6_swap.c`, original `vm_pdp6.s` `vm_space_destroy` leaf,
original `proc_pure_packed_detach_v1609.s`, original storage-mode
machine code and the established v1609 MM fixture. It introduces
a second independent PACKED MM descriptor. Two cases retain one or
two MM pins on the victim while invoking the real VM terminal release
**four times per case**. Every attempt must fail before even calling
`mm_free`, preserve its exact RAW owner, backing/u-area, mode,
TRANSITION and VM image, and not change the adjacent peer's MM owner.
After pin release, the real leaf must destroy only the victim,
publish RESIDENT, clear its backing once and remain idempotent on
a repeated destroy. The independent peer must remain live throughout.

A third case presents an exact *unpinned* PACKED owner but makes the
MM fixture return definite `MM_ERR_BUSY` on **four consecutive**
`mm_free` calls. No original MM or process state may change until
that refusal is lifted; the fifth real terminal call succeeds and
leaves the independent peer intact. This is a definite *no-commit*
refusal and cannot establish behavior after a falsely reported success
or uncertain MM owner disposition.

A separately compiled and executed unsafe mutation deletes the
original pinned-descriptor guard from the source-linked assembler.
The same new native test detects the illegal call into `mm_free` on
a pinned owner and rejects the mutation with exact guest status
`000000001102`. Original v1609 RAW terminal baseline still PASSes.
The fixture checks the actual numeric 8-bit MM pin count rather than
incorrectly treating `MM_EXTENT_PINNED` (Boolean) as a count.

## Limits and next physical gate

The fixture directly adjusts its descriptor to simulate eventual
pin release. **No actual PI6, MM-unpin instruction, idle-stack retry,
backoff timer, or real 48K EXIT/WAIT under retained pins is covered.**
This is a native component preflight, not physical P1-E completion.
The next physical v1680 test must inject a genuine persistent pin into
one real nine-child RAW PACKED original owner *before* terminal EXIT,
then use actual original physical MM/PROC/FINALIZE observations to
prove pending retry does not publish a failed process or free the
wrong extent. A later controlled pin release may permit recovery,
but only after all original owner and source invariants pass. An
uncertain permanently pinned owner must stay fail-closed; do not
create a fake successful WAIT or release the process to satisfy the
test. Source identity, FULL backing errors and a fault-specific
71-watch continuous MM write oracle remain separate P1-E gates.

No production code, KCORE RAM or real MM descriptor representation
was changed. Source-linked testkit branch:
`test-pure-terminal-pin-v1680-20261119`, commit `c290b5e`, directory
`tests/validation/pure-terminal-pin-v1680/`. Final mainline integration
still requires independently checked local AND remote DAIMOS and
daimos-testkit branch/worktree consolidation and all accepted fault,
ROOT and KCC FULL regressions.
