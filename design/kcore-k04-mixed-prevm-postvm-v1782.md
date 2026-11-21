# K04 v1782: independently pinned PREVM and post-VM owners

Date: 2026-11-21. Status: **PASS** for mixed-stage independent
MM owners and B's exact first retry timing. K04 remains OPEN.

## Objective

Unlike v1777/v1778 (two post-VM u-area owners), hold owner A
in EXIT PREVM VM cleanup until its shared backoff saturates at
octal 01000, then independently refuse owner B's post-VM u-area
release. Verify B's first refusal starts at octal 020, B's
next actual attempt occurs 020 qualified ticks later, and B's
real MM unpin/release/PID reuse completes while A remains pinned.

## Disposable instrumentation

Create a fresh isolated image at
`$TMPDIR/daimos-k04-v1782-mixed-prevm-postvm` from the clean
v1773 256K PDP-6 production boot baseline. Instrument only the
generated `kcore-asm/proc_pdp6.s` (not production source).

* Owner A, PID 5: at the existing EXIT PREVM `vm_space_destroy`
  call site, use real `mm_pin` on the precise VM base in its slot
  descriptor; verify that base on every retry. Use real `mm_unpin`
  on its 24th attempt (octal 030). This is the proven v1781
  single-PREVM fixture with a longer delay.
* Owner B, PID 6: at `proc_uarea_release`, use real `mm_pin`
  on the exact u-area extent before unmodified `mm_free`; verify
  the same owner base on each retry. Call real `mm_unpin` on the
  fifth attempt. This uses the validated v1777 post-VM mechanism.
* Capture B's shared pending/backoff word before its first pin;
  capture that word and `proc_timer_clock` immediately after its
  first `proc_finish_mark_pending_v892` invocation; capture the
  same clock on B's second MM release attempt. No scheduler
  control or forged MM return is used.

All test-only extra code and ten BSS words exist only in the
disposable image. Native DAS PASS, complete image build PASS,
initial read-only `d6fsck -n 4` CLEAN seq=1. Fixture state in
the scratch map (all addresses octal):

    025117 A VM base           025120 A attempts
    025121 A done              025122 B u-area base
    025123 B attempts          025124 B done
    025125 B before mark       025126 B after first mark
    025127 B first clock       025130 B second clock
    025113 qualified clock     025115 shared retry word

## Live PDP-6 observations

Boot with independent ROOT/DSH sessions on CTY and DCS0.
Run CTY `TRUE` to establish pinned owner A. Before any DCS child,
stop SIMH with Ctrl-E and inspect physical memory. A's VM base
was octal 760000, the attempt counter 000006, and A's done flag
zero. The shared word had reached the capped pending state:

    25115: 401000010110    ; pending, 01000 delay

All seven B scratch words at 025122..025130 were still zero.
While the CPU was stopped, queue DCS `TRUE` and resume the
simulator. The DCS command returned to its shell prompt while
CTY `TRUE` remained blocked. Stop SIMH again; inspection shows:

    25117: 000000760000    ; A still owns pinned VM extent
    25120: 000000000013    ; A: eleven VM release attempts
    25121: 000000000000    ; A: not unpinned
    25122: 000000000000    ; B u-area released
    25123: 000000000005    ; B: five MM release attempts
    25124: 777777777777    ; B: real MM unpin completed
    25125: 401000010110    ; A's saturated pending word before B
    25126: 400020010154    ; pending, reset to 020 after B mark
    25127: 000000010154    ; B first refused-release clock
    25130: 000000010174    ; B second release clock

Thus the actual first retry elapsed `010174 - 010154 = 020`
octal qualified ticks, exactly 16 decimal. Owner A remained
physically pinned even though B had finished.

Resume and run DCS `PS`. The surviving PREVM owner and newly
reused B slot were:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 2048  3  512   1   0      0 TRUE
    6   3    2 3072  0  0     0   0      0 PS

Owner A retained TD=3/PVM=1 with a capped retry interval,
while PID 6 was safely reusable with all teardown diagnostics
zero. The shared retry word also returned to the saturated
pending interval after B's first refusal was serviced.

## Scope

This passes the **PREVM-A/post-VM-B** mixed-owner ordering and
the exact B-first-retry gate. It does not prove the reverse
post-VM-A/PREVM-B ordering or an unpublished SIDL B; those are
separate tests. Clock wrap, FILE/HUP and dying-parent WAIT are
also OPEN. No production KCORE, ABI, fixed RAM, or external
testkit changes were required.
