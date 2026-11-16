# P1-E v1629: five genuine PACKED children restore and EXIT under live parent

Date: 2026-11-16. Base: v1628 original 64K source-linked MM owner
matrix demonstrating that a no-PACKED guest is insufficient to
accept the RAW swapless lifecycle. This is testkit-only work; no
production KCORE/VM/MM/process/MRES/driver/ABI/source modification.

## Exact 48K source and live physical experiment

A fresh physical PDP-6 48K TSFS/no-AUX/GCC PURE guest uses the
original v1614 C89 PIPE/WAIT pressure test with five children,
each having two private mutable sentinels and EXIT status 057.
It requires the first five RUNs to succeed and reach SLEEP,
then attempts a SIXTH RUN. The sixth returns -1 under real
memory pressure (as diagnosed at vm_space_create in v1625).
The parent treats this resource refusal as an expected event,
prints `x`, and continues rather than exiting. Parent BRK is a
no-growth 010000 -> 010000 identity check to avoid the impossible
48K growth geometry already diagnosed. `L` is lifecycle-only;
all original PIPE read and exact five PID/EXITED/status-057 WAIT
assertions, plus both parent/child private sentinels, remain intact.

The actual source-backed complete physical console prints:

`aaaaax[B013060/042502]<010000+010000>[A013060/042502]{013060-013060}LwwwwwZ`

It then reaches original simulator HALT. The normal full guest
validation passes; the physical C code only reaches the final
Z after five successful child WAIT/EXIT checks with the parent
still alive. No unexpected fatal BRK/EXIT fallback was used.

## Same genuine guest replayed with original MM table snapshots

The **same compiled guest** and private copies of its original
Type-551 root tapes were replayed with console-triggered SIMH
physical `mm_extents` captures before/after the parent's check and
at terminal Z. The original KCORE source-map matches the memory
addresses. All values below are octal machine words.

| Stage | Extents | PACKED owners | Occupied physical | Free gaps |
|---|---:|---:|---:|---:|
| B | 022 | 5 | 057023 | 016173 |
| A | 022 | 5 | 057023 | 016173 |
| Z | 010 | 0 | 011536 | 063460 |

At B and A there are exactly FIVE unpinned PACKED owners:

* slot 6 `057000..066000`, 007000 retained suffix words;
* slot 5 `067000..076000`, 007000 words;
* slot 4 `077000..106000`, 007000 words;
* slot 3 `107000..116000`, 007000 words;
* slot 2 `117000..126000`, 007000 words.

The parent remains live in slot 1. The MM extents remain ordered
without overlapping or foreign owners. At final Z all five
PACKED owners are gone and no child PROCESS owner survives.
The real child `w` code was reached after the pipe read and
both mutable sentinels; the parent confirmed all five EXIT statuses.
This is genuine PACKED -> runnable user execution -> EXIT/WAIT
and MM cleanup, not a resident-only surrogate. Source-linked
physical testkit auditor PASSes; forged foreign-PACKED-owner
and false-final-WAIT mutations are rejected.

## RAM accounting and still-open gates

Five omitted 001000-octal PURE prefixes yield 2560 decimal
gross omitted words while PACKED. Installed GCC PURE MRES costs
1758 decimal words, nominal 802-word advantage *before*
additional allocations, transient restore headroom and peak
memory. These numbers are NOT a whole-system RAM comparison
against a source-matched no-PURE baseline, and the MM snapshots
are not a transient peak trace.

Exactly which restored children used fast in-place versus
relocated full-APR paths is not established by the B/A/Z
snapshots alone. Full-path failure/rollback, interrupted immutable
source and concurrent PI6 faults, and measured physical net/peak
RAM remain OPEN. P1-E remains opt-in (PURE default OFF) and
strict 32K optimization remains HOLD.
