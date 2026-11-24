# K05 v1908: physically held foreign FULL target and shared retry cap

Date: 2026-11-24. **Original 48K PDP-6 physical PASS, bounded scope.**

The v1906 fix retains the full target physical address in the
already-allocated child u-area word 067. v1907 proved ordinary
foreign user TERM refuses before touching a TRANSITION-held
saved executive continuation. v1908 isolates the next question:
what happens if nobody ever repairs the test-corrupted foreign
owner during the observation window?

The source-built original v1907 parent/KCORE/PURE and original
five child TSFS/BRK/PIPE/RUN workload are byte-identical.
The source MM free returns genuine -5; the controller releases
the source pin and changes only original provisional target
PROCESS descriptor at `024725` from slot3 to owner2. One
real parent PROCCTL TERM(3) returns `TR` (refused).
No operator repair, native MM free, forged FINALIZE, parent
WAIT-gate release or scripted guest HALT follows.

At TEN independently executed resident parent user HBs under
the foreign claim, the original MM table remains exactly MM18,
with authoritative RAW source and unfreed owner2 PROCESS target
unchanged. Slot3 remains TRANSITION, VMbase0, original saved
executive CTX_PC and FDCTL and u[067] `040000120000`
intact. The original parent remains active (slot1), original
ROOT, OTHER MM owners and WAIT gate stay unchanged. Physical
PCLK advances `015624` -> `016724` octal.

The original post-VM slot-0 retry uses exactly **one existing
fixed BSS word** `proc_finish_last_tick_v881`. Its physical
pending bit stays armed and the interval (octal) follows:

    HB 1  0100
    HB 2  0200
    HB 3  0200
    HB 4  0400
    HB 5  0400
    HB 6  0400
    HB 7  0400
    HB 8  1000
    HB 9  1000
    HB10  1000

The original `01000`-tick cap (512 decimal) is reached and
held for three heartbeats, without resetting to a short retry
delay or starving the parent. The simulator is **operator-
stopped** at HB10: this is not a guest native EXIT or HALT,
nor is a finite test evidence of infinite-time correctness.

The strict v1908 read-only original physical audit checks every
MM descriptor, PCLK, shared epoch and capped interval, private
CPU/FDCTL/escrow, TRANSITION, source/target pin and owner,
original ROOT, original guest TERM refusal and parent gate.
Only three targeted negative controls are used: fake unlock,
above-cap delay, and frozen PCLK; all reject. Earlier v1906
MM and v1907 TERM unsafe controls remain separately accepted.

No production C/ASM, KCORE/PURE image, BSS, process table,
optional policy or RAM use changes. Exact KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
Accepted physical SHA256:
`6d5a43084392f9a32442dc7cb706d8d0e4e451aac0cc029784d35ba810954736`.

Testkit: `daimos-testkit/tests/validation/pure-k05-permanent-owner-v1908/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-HELD-FOREIGN-V1908.MD`.
Evidence: `$TMPDIR/daimos-k05-permanent-owner-v1908-physical-v1/`.

**K05 remains OPEN** for owner state beyond this finite window,
truly terminal cleanup of an unresolvable owner, uncertain MM
metadata and additional real PI6 interleavings. Do not force
terminal finalization of a live saved executive stack. K06-K09
remain open, RAW PURE and mode3 default OFF.
