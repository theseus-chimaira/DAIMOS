# K05 v1901: real PDP-6 quarantined child foreign TERM safety boundary

Date: 2026-11-23. **Original full-system 48K physical PASS, 19
executed unsafe transcript controls rejected.** This milestone
identifies a necessary lifecycle safety constraint, not a
completed K05 terminal owner release. Production code, fixed
KCORE, optional PURE MRES, MM/process layout and defaults are
byte-identical to accepted v1900.

## Why v1900 terminal cleanup could not be invoked directly

Original v1900 physical source-free -5 can hold a slot in
`TRANSITION` with BOTH original MM allocations: PACKED RAW
source and temporary complete PROCESS target. The terminal
MRES v1900 supports exactly these validated dual-owner extents
once an authentic FINALIZE+PREVM finalizer owns the slot.
However, `proc_event_apply` in fixed KCORE intentionally
rejects all event deliveries when the slot is `TRANSITION`.
The KCORE `proc_finish_slot` similarly rejects an ordinary
TRANSITION slot without an explicit, already-owned FINALIZE
marker. Forcing such a marker without lifetime validation
would free an outstanding private executive continuation.

To test the actual intended path, a new TESTKIT-ONLY five-child
original 48K guest uses the real resident parent's user-mode
`SYS_PROCCTL_EVENT_PID` with `SYS_EVENT_TERM` targeting slot3.
It is otherwise the original v1892 ROOT/secondary DTC3,
RUN/BRK/PIPE pressure and resident parent heartbeat source.
The exact original fixed-KCORE `mm_free(PACKED)` must first
return a genuine -5 with BOTH physical descriptors preserved.

An exploratory fixture sent TERM too early during original
parent admission and emitted `HBTA` BEFORE the MM fault. That
run is **REJECTED**. Correct v1901 introduces only a test-only
volatile word `v1901_term_gate` in parent USER RAM. The
original linked user map identifies logical `005265` octal,
the accepted resident parent APR base is `130000`; the
controller alone deposits `012345670123` at user physical
`135265` AFTER first postfault parent HB and snapshots.
No kernel word, MM metadata, process flag, syscall result or
native PC is deposited. The next user code truly executes
the original PROCCTL TERM syscall. The actual console marker
is `TR` (TERM refused), not an injected failure result.

## Independent original physical facts

- Native original FULL source `mm_free(PACKED)` call `035310`,
  return `035311`, actual `MM_ERR_BUSY=-5`; installed PURE
  original base `032417`.
- Actual process table at `045014`. Slot3 process metadata
  `045025=046133600401` with TRANSITION, original VM word
  `045026=010000000000` with VMbase ZERO.
- Authoritative PACKED RAW `111200..120000` pin1, meta at
  `024703=000014000003`; temporary complete PROCESS
  `120000..130000` pin0, meta `024705=000001000003`.
  Original MM count18 remains exact through TERM and HBs.
- Original slot3 u-area `046133`; saved physical `CTX_PC`
  at `046153=340000016664`. Its saved USER-return bit
  `010000` is **CLEAR** in the left half. A private executive
  continuation remains owned by this child. An immediate
  foreign VM/u-area free would be unsafe.
- Original FDCTL `046157=000000004010` and FINALIZE marker
  `046222=000000000000` are unchanged by rejected TERM.
  The user event never manufactures a FINALIZE+PREVM owner.
- Actual current slot is resident parent1 at both user HBs
  and TERM checkpoint. Original PCLK advances `022015` ->
  `022115` octal while the disabled child remains quarantined.

The accepted source-built fixed KCORE SHA256 remains
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`;
corrected optional PURE MRES SHA256 remains
`09bcb71e4cbb172222f6389153c27293353fdaf6d3210eeef4630ed06de934f2`.
The only changed image is TEST-ONLY user INIT SHA256
`e678526d30901f58308bf3d10bba0df0c8b0c648d75367fc32eaabca60f4ffd9`.
Accepted original normalized PDP-6 physical transcript SHA256:
`ce5460ee0b43593eeb158b60f11df7dd7336918e9ad0977d9afc7677ae8bc251`.

The versioned testkit `pure-k05-transition-term-v1901/`
source build, controller, read-only verifier and audit execute
NINE independent unsafe original physical term/gate/context/
MM mutants and TEN accepted v1895 physical refusal/owner/
PCLK mutants against the same source-linked 48K trace.
All **19 negative controls correctly reject** fake success,
premature gate, missing gate, forged MM deletion, false
FINALIZE, process transition loss and saved context corruption.
The previous complete original v1900 full-physical acceptance
and v1893-v1899 historical matrix remain unchanged.

## Correct next recovery sequence (K05 OPEN)

Do **not** remove the foreign-event TRANSITION guard to force
early terminal teardown. This observed child has a pending
executive continuation and cannot be remotely released.
Also do not synthesize FINALIZE or discard original backing
while physical MM ownership remains uncertain.

The actual unresolved production path is eventual recovery
of a failed FULL with two retained MM extents after external
pins clear. Existing `ppure_retryable_v1612` only accepts
EXACTLY ONE original slot-owned MM extent; the original service
therefore leaves the two-owner failure TRANSITION-held and
ineligible for scheduler restoration. A correct bounded recovery
pass from safe slot-0 context must prove both exact original
owners, reconcile or retire the provisional PROCESS target via
real native MM, preserve PACKED source and saved executive
context, and clear TRANSITION only after the authoritative
single-owner source state is established. Then existing normal
owner continuation, event delivery, EXIT and FINALIZE can
proceed; separately verify actual user termination and parent
WAIT under resumed memory pressure. How to schedule that
reconciliation without new persistent state is the next
production design question, not resolved by v1901.

All K05-K09 remain OPEN, RAW PURE/mode3 OFF by default.
