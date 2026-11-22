# v1681: PDP-6 original-MM PURE terminal and PREVM finalizer integration

Date: 2026-11-19. Status: **native original-instruction PASS**; complete
48K physical PI6/terminal gate **OPEN**. This is an additive test-only
acceptance on the unmerged PURE branch. There are no production source,
KCORE/MM object, ABI or permanent resident-memory changes.

## Purpose and correct termination boundary

An ordinary v1674 PIPE-held child wakes and restores its complete APR
before running user EXIT. That path cannot prove what occurs when a
nonresident RAW PACKED owner must be finalized by another process while
its only mutable suffix remains pinned. The original terminal leaf
`pdp6_pure_detach_v1609` refuses nonzero MM pins before freeing, and
the permanent `proc_swap_detach` only clears backing records after
accepted MRES terminal release. On a PREVM teardown refusal, the actual
PDP-6 `proc_finish_slot` must retain TRANSITION and FINALIZE+PREVM,
not report ZOMB or discard the process's unique MM suffix. The idle
stack `proc_finish_service_v881` owns safe retries and shared capped
retry backoff; it cannot run arbitrary device I/O or unquarantine a
still-pinned owner.

## Source-linked original executable test

`daimos-testkit/tests/validation/pure-terminal-finalizer-v1681/`
source-checks and extracts original production PDP-6 assembler
instructions from `proc_pdp6.s`, `vm_pdp6.s`, `mm_pdp6.s`,
`proc_pure_packed_detach_v1609.s` and the original storage-mode
assembler. KCC compiles the actual permanent `vm_pdp6_swap.c` /
`proc_swap_detach`. The test does **not** reimplement terminal/VM/MM
behavior in C. It uses a source-linked fixture for the simulated
process table, MM extent, old u-area, TTY/session callbacks and
post-VM u-area release failures. No production file is edited.

A foreign process finalizer is invoked on slot 2, whose original
single PACKED MM descriptor has the correct RAW writable-suffix span,
owner, read-only executable provenance and absent resident APR.
The original `mm_pin` call acquires a **real descriptor pin**.
The original `proc_finish_slot` fails safely and preserves the exact
owner plus FINALIZE+PREVM, status, transition and pending retry clock.
The original `proc_finish_service_v881` retries once while still
pinned; the owner must remain intact, with no premature VM or backing
publication and no extra sleeping HUP. The original
`proc_finish_backoff_advance_v885` doubles the shared interval from
octal 020 to 040 without adding a permanent state word.

Only the original `mm_unpin` removes the retained pin. A following
original finalizer retry can now execute the original PACKED terminal
leaf and `mm_free`, publish RESIDENT only after exact deletion,
clear the real KCORE swap/backing records, and publish VM_NONE.
Subsequent fixture u-area release refusals exercise the existing
post-VM continuation, which must not repeat VM/bare MM release.
The final retry publishes the zombie and the original backoff
cleanup clears pending state. Original status, runq and HUP counters
are verified; no additional MM owner remains.

## Executed unsafe-source controls

Each of the four mutations changes a private copy of an actual
source-extracted original PDP-6 assembler function and runs the
**same** native test to a specific assertion exit. Timeouts, link
errors and unrelated failures are not accepted:

| Unsafe alteration | Rejected native exit (octal) |
| --- | --- |
| Publish success despite `vm_space_destroy` refusal | `000000000006` |
| Drop FINALIZE+PREVM's PREVM marker | `000000000006` |
| Ignore original `mm_pin` increment | `000000000004` |
| Suppress original exponential retry-backoff doubling | `000000000035` |

All four PASS as unsafe mutations rejected. The unmodified integrated
production code PASSes with `__test_exit=0`, after native source
recompilation. The previous v1680 component tests still cover one- and
two-pin terminal refusals, repeated definite `mm_free` rejection,
peer-owner isolation and another unsafe pinned-guard mutation.

## Limits and next physical acceptance

This is **not** a complete booted 48K KCORE/TSFS/PI6 integration test.
The native fixture constructs an already-PACKED process; the actual
MM pin/unpin instructions run, but the guest fixture schedules the
finalizer and backoff calls directly, not through a hardware PI6 tick
or genuine idle-stack scheduler dispatch. It uses a test u-area and
test callbacks instead of full TTY/FILE/EXEC/root service ownership.
It does not establish a permanently held/uncertain MM pin's full
system behavior, source loss, PI6-safe timer progress, parent WAIT
semantics or a continuous fault-specific MM write-watch high-water.

A physical terminal acceptance test must find a real reachable way
to terminate or discard a *nonresident* PACKED owner. Releasing the
original blocked PIPE child first restores the APR and is not such
an event. The test must then physically capture a retained pinned
original MM owner, the original PREVM finalizer and idle retry/backoff,
followed by either real unpin/recovery or verified fail-closed
quarantine. Do not instrument an impossible concurrent user path, or
claim this native result closes the physical gate. P1-E remains OPEN.

Testkit branch: `test-pure-terminal-finalizer-v1681-20261119`, commit
`05cdf28`. Eventual mainline acceptance also requires the separate
all-local/all-remote branch and dirty-worktree consolidation procedure
for both DAIMOS and daimos-testkit. No branch retirement occurred.
