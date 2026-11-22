# P1-E v1624: classify sixth RUN refusal in 48K physical PURE guest

Date: 2026-11-16. Base: v1623 operator-trust TODO cleanup, v1621
physical restored user execution. No production code change.

## Reproduction and original evidence

Both experiments use the original KCC/GCC lowmem 48K PDP-6 no-AUX
TSFS root and the experimental RAW PURE MRES, with immutable PURE
DXR2 source linked from the real v1614 test. The only generated
test-guest changes are parent child count, BRK target, and distinct
diagnostic branches for a failed RUN versus a child failing to reach
PROC_SLEEP. The original v1621 !c marker conflated those two causes.

* Five children, target BRK 017000 octal: the genuine physical
  console prints `aaaaa[B063060/042502]<010000+017000>wwwww`
  followed by a normal simulator HALT. All five children reach
  the ready/sleep check; parent BRK does not return successfully.
  This is NOT a successful reclaim, WAIT/EXIT or peak RAM result.
* Six children, target BRK 016000 octal: the independent physical
  console prints `aaaaa!rwwwww` and HALTs, without attempting BRK.
  The sixth RUN actually returned an invalid PID before the parent's
  PROC_SLEEP wait. It was NOT a sleep-poll timeout. Five previously
  admitted children complete their source-authentic `w` markers.

The versioned `daimos-testkit/tests/diagnostics/
pure-pressure-cause-v1624` source generator, source-integrity checks,
physical trace auditor and falsified-trace controls PASS. The legacy
v1614 positive-pressure runner correctly returns nonzero in both
negative outcomes; do not relabel these as full guest PASSes.

A separate v1615 source-verified fatal-close breakpoint replay of
the five-child guest timed out before reaching that breakpoint and
does not identify the parent BRK failure code. It is NOT a finding
of MM_ERR_NOMEM or a confirmed fatal-close path.

## Remaining work before a P1-E completion claim

1. At the sixth RUN, capture the original source-linked RUN stage
   (invalid record/slot, MM allocation, image load or startup), with
   live MM/process owners. Do not infer the stage from PID -1.
2. Establish a physical pressure geometry that can complete parent
   BRK while at least four independent inactive PURE owners retain
   their mutable suffixes, and verify exact owner/pin transitions.
3. Complete parent PIPE/WAIT/EXIT, immutable executable lifetime,
   fast/full restore rollback and independent PI6 scheduling.
4. Measure net and peak physical RAM, including the 1758-decimal-word
   GCC PURE MRES. No whole-system RAM saving is established here.

Default PURE OFF, strict 32K optimization HOLD, VERSION 0.5.
