# K05 v1909: actual PI6 in the bounded foreign FULL retry window

Date: 2026-11-24. **Original source-built PDP-6 48K physical PASS.**

## Original hardware witness

Reuse exact v1907 five-child TSFS parent and v1908 never-repaired
foreign-owned temporary FULL target; source `mm_free(PACKED)`
returns true -5, parent user TERM(3) truly returns refused `TR`,
and all ten resident parent heartbeats retain MM18, exact RAW
and foreign PROCESS target ownership, TRANSITION/VMbase0,
original saved executive CTX_PC/FDCTL and escrow
`u[067]=040000120000`. No operator MM repair or forged exit.

Immediately after actual parent HB4, arm a physical breakpoint
on the **permanent KCORE** PCLK PI6 handler entry. The original
PDP-6 reaches `proc_sched_pi_tick=017225` with authentic
first opcode `350040024530` (`AOS 1,proc_timer_clock`). The
real PI6 save word at `pdp10_pi_level6=023714` is
`340000017560`, pointing to executive PC `017560` inside
the original slot-0 idle interval `017552..017620` (exclusive).
Current slot at the actual interrupt is 0. The original
MM18 table, ROOT, TRANSITION, VMbase0, saved private CPU/FDCTL
and exact target-base escrow are byte-identical across the
HB4 -> PI6 -> HB5 checkpoints. Parent progresses to HB10 and
the original backoff reaches and holds `01000` octal ticks.
After HB10 the operator stops the SIMH run; NO native cleanup
or guest HALT is claimed for the deliberately foreign-owned target.

The independent source-backed v1908 positive verifier re-PASSes.
The v1909 PI6 verifier additionally requires one actual breakpoint,
the correct source-linked opcode and saved return PC inside idle,
the full MM18 comparison, and unchanged original u-area/slot
state. Three targeted physical mutations of opcode, interrupted
PC and escrow are rejected. No redundant broad MM fault matrix.

## Source-level interrupt ordering

Original `proc_pdp6.s:proc_sched_pi_tick` expressly permits
slot-0 preemption only when `pdp10_pi_level6` saved PC lies in
`proc_idle_loop..proc_idle_loop_end`; ordinary executive code
completes without a PI6-induced context switch. The original
PURE arm helper first sets the pending tag and then writes
the original scratch base to u[067] RH while slot0 executes
the MRES. PI6 may tick during that code, but cannot switch
that in-flight executive FULL into a second idle-loop reaper
before its continuation finishes. This is source-structural
reasoning, not a claim that the physical trace interrupted
the handful of instructions between tag and escrow store.

The actual v1909 physical event instead probes the real
preemptible idle-loop window after the escrow is installed,
while the foreign target prevents recovery. It verifies that
this relevant PI6 ordering preserves all ownership and liveness.

No production code, IMAGE, fixed KCORE BSS, process storage
or default policy changed. SHA256 KCORE:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`;
PURE: `09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`;
accepted physical `fbe72332fc0540331c9d4b131d0efded53e42d3da5b746b089ecc99f0f4cbb52`.

Testkit: `daimos-testkit/tests/validation/pure-k05-held-pi6-v1909/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-HELD-PI6-V1909.MD`.
Accepted private physical trace:
`$TMPDIR/daimos-k05-held-pi6-v1909-physical-v1/`.

**K05 remains OPEN:** arbitrary distinct interrupt timings and
unresolvable terminal/allocator ownership remain unproven;
never clear TRANSITION or force FINALIZE on an uncertain MM
target or live executive continuation. K06-K09 OPEN;
RAW PURE and mode3 default OFF.
