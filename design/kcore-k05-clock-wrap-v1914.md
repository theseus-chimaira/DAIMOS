# K05 v1914: original PDP-6 last-orphan retry across the 17-bit clock wrap

Date: 2026-11-24. **Scoped original 48K PDP-6 physical PASS.**

## Problem and original source contract

In `proc_idle_loop`, original KCORE reads the single shared
`proc_finish_last_tick_v881` epoch, takes the low 17 bits of
`proc_timer_clock`, subtracts the previous retry epoch and
masks with `PROC_TIMER_CLOCK_MASK=0377777`. Only when the
modular elapsed ticks reach the existing `01000`-tick cap
does it enter original `proc_finish_service_v881`. Original
PCLK PI6 `proc_sched_pi_tick=017225` advances the clock
using the existing `AOS 1,proc_timer_clock` instruction.
No per-process retry table, new code, heap or fixed RAM is
needed to implement 17-bit wrap handling.

This is important for K05: slot3 can remain the only LIVE
owner for a long time while retaining original authoritative
PACKED RAW source, a provisional foreign-owned PROCESS target,
the target escrow and its saved executive continuation.
The retry timer must not prematurely free any of those
resources, report successful release, or HALT merely because
its low 17-bit time counter has wrapped.

## Controlled source-linked physical experiment

The byte-identical source-built original v1910 48K guest,
KCORE and optional PURE execute the entire accepted v1913
last-orphan sequence: genuine PACKED source free -5,
test-only foreign PROCESS target owner2, original parent and
healthy child exits, exact MM10 orphan slot3, fifteen capped
native idle retries and four original PI6 checkpoints.
The entire prior physical prefix re-PASSes both accepted
v1913 and v1911 independent positive verifiers.

The simulator stops at original PCLK PI6 after v1913's last
checkpoint and makes exactly TWO test-only clock-state writes:

- `proc_timer_clock=024530`: observed `000000043736` ->
  `000000377770` (octal)
- `proc_finish_last_tick_v881=024532`: observed
  `401000043736` -> `401000377770`

These are a **synchronized test-only time advance**, not
natural full-uptime evidence. Both clock and retry epoch move
by the same amount; the existing pending bit and capped
`01000`-tick backoff remain set. Original `proc_timer_next`
at `024531` is zero and stays zero. Nothing else is deposited:
no MM extent, process, saved executive word, instruction or
AC1 return is changed.

Original interrupted PI6 first runs and increments the clock
to `0377771`. Then SEVEN more distinct original PCLK PI6
entries at `017225` complete with individual original idle
loop returns at `017552`. Their physical clock sequence
includes the exact 17-bit boundary:

    before original PI6 AOS     000000377777
    after original PI6 AOS      000000400000
    after 17-bit mask           000000000000

The full original 36-bit clock continues increasing, while
the original modular elapsed-time mask wraps to zero.
The retry pending word remains `401000377770` through the
wrap and no premature service/cleanup occurs.

After the actual 17-bit hardware rollover, original
`proc_finish_service_v881=013065` executes three more times
at full physical clock values `0400770`, `0401770`, and
`0402770` octal, exactly `01000` ticks apart. Original
retry epoch right half becomes `000770`, `001770`, `002770`
respectively, preserving the shared pending bit and cap.
Each native service returns to a distinct original idle-loop
checkpoint, not a repeatedly armed SIMH instruction.

The complete physical MM10 table, ROOT, sole live slot3
orphan `047017600001` (TRANSITION, parent0, VMbase0),
RAW `111200..120000` owner3, provisional full PROCESS target
`120000..130000` foreign owner2, private saved CTX_PC
`340000016704`, FDCTL `000000004010`, u[067] escrow
`040000120000`, and all other free process slots are
invariant across pre/post clock rebase, each real PI6, and
all three post-wrap retry entries/completions. No test-only
target owner repair, forced FINALIZE or native HALT occurs.
The simulator ends by OPERATOR EXIT only.

FOUR independent unsafe physical trace mutations reject:
incoherent synchronized epoch, missing original hardware
wrap, an early post-wrap retry, and a false owner2->owner3
correction. The first invocation of the strict verifier
rejected the valid trace because it had assumed a fixed
v1913 pre-rebase clock value `042342`; original PCLK timing
is non-deterministic across complete guest runs. The proper
source-linked physical baseline `043736` from the SAME
run replaces that hardcoded assumption. The unchanged
physical transcript and KCORE/PURE then PASS; this was a
verifier defect, not an observed compiler/assembler/kernel bug.

Original normalized physical SHA256:
`7b7d9afefbe9e2d59e8fc46876d22ce57e3538cb0cacc39ad6fd3d09c72fa705`.
Source-linked simulator script SHA256:
`932f7607e91bb140e119158120f17938b643c1cfbec6bd74cef737fc42550c1f`.
Unchanged fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Unchanged optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.

Testkit: `daimos-testkit/tests/validation/pure-k05-clock-wrap-v1914/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-CLOCK-WRAP-V1914.MD`.
Physical evidence: `$TMPDIR/daimos-k05-clock-wrap-v1914-physical-v1/`.

**Scope:** one controlled near-wrap clock/epoch advance and
real hardware PI6 rollover of low 17 bits, followed by
three capped native retry scans. This proves that exact
boundary ordering without added permanent RAM; it does not
prove indefinite uptime, natural elapsed-time reachability,
all allocator/PI6 fault permutations or all platforms.
K05 OPEN; K06-K09 OPEN; RAW PURE/mode3 remain default OFF.
No production C/ASM, IMAGE, BSS, RAM or default change.
