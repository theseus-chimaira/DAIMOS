# K04 v1783: reverse mixed post-VM/PREVM real-MM owners

Date: 2026-11-21. Status: **PASS** for the reverse mixed-stage
owner independence and B's first measured retry interval.
The broader K04 acceptance gate remains OPEN.

## Purpose and isolation

The v1782 test exercised PREVM A followed by post-VM B. This
v1783 test deliberately reverses the order: A (PID 5) enters
post-VM u-area cleanup and remains physically pinned at a
saturated octal 01000 retry delay. Only then B (PID 6) enters
EXIT PREVM VM cleanup with a different real MM extent pinned.
Verify that B starts at octal 020 rather than inheriting A's
maximum delay, retries exactly 020 qualified ticks later, and
can free its VM and reuse PID 6 while A remains refused.

A disposable copy of the clean v1773 PDP-6 256K production boot
image is built at `$TMPDIR/daimos-k04-v1783-postvm-prevm`.
Instrumentation is confined to the generated scratch
`kcore-asm/proc_pdp6.s`. It reuses the proven v1777 physical
u-area pin for PID 5 and v1781 physical PREVM VM pin for PID 6.
Only authoritative `mm_pin`, `mm_unpin`, and ordinary `mm_free`
/ `vm_space_destroy` are used; no fake provider return or
scheduler override is involved.

A's exact u-area is pinned through its first 23 calls and
unpinned on its 24th (octal 030). B's VM is pinned through four
refused attempts, then unpinned on the fifth. Each fixture
checks the exact owned base on retries and disables itself
after successful physical release. Scratch-only BSS stores the
two owners' bases, counters and completion flags, plus B's
first-refusal retry word and qualified clock timestamps.

Native DAS assembly **PASS**, full PDP-6 image build **PASS**,
initial read-only D6FS check **CLEAN seq=1**. Scratch-map
addresses (all octal; NOT production locations):

    025114 proc_timer_clock
    025116 shared pending/backoff word
    025120 B VM base          025121 B attempts
    025122 B done             025123 A u-area base
    025124 A attempts         025125 A done
    025126 B before pending   025127 B after first mark
    025130 B first clock      025131 B second clock

## Physical Open SIMH PDP-6 observations

Boot the isolated image with ROOT/DSH on both CTY and DCS0.
Start CTY foreground `TRUE` to trigger the PID 5 post-VM MM pin.
**Do not run a DCS child (not even PS) before B's first refusal.**
Stop SIMH and inspect physical memory. A is pinned and the shared
retry has saturated; every B fixture word remains zero:

    25116: 401000011655    ; pending, maximum 01000 delay
    25120: 000000000000    ; B VM base untouched
    25121: 000000000000    ; B attempts untouched
    25122: 000000000000    ; B completion untouched
    25123: 000000054517    ; A exact u-area base
    25124: 000000000006    ; A six calls
    25125: 000000000000    ; A still pinned
    25126..25131: all zero ; B had no prior failure

Queue DCS `TRUE` while the simulator CPU is stopped, then
continue. The DCS foreground TRUE returns while CTY TRUE stays
blocked. Stop SIMH again and examine memory (all values octal):

    25120: 000000000000    ; B exact VM extent released
    25121: 000000000005    ; B five destroy attempts
    25122: 777777777777    ; B real MM unpin complete
    25123: 000000054517    ; A retains exact pinned u-area
    25124: 000000000014    ; A 12 attempts
    25125: 000000000000    ; A remains pinned
    25126: 401000011655    ; BEFORE B: A saturated delay
    25127: 400020012126    ; AFTER B: pending, fresh 020
    25130: 000000012126    ; B first refusal clock
    25131: 000000012146    ; B second VM destroy clock

The first actual retry interval was `012146 - 012126 = 020`
octal qualified ticks, exactly 16 decimal. B's successful
release did not release or overwrite A's exact MM owner.

After B returns, DCS `PS` also proves PID reuse while A is
still refused:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 262143 2 512   0   6      0 USER
    6   3    2   3072 0   0   0   0      0 PS

Thus A is a real pinned post-VM FINALIZE owner (TD=2,
DETAIL=6, RETRY=512), B is safely reusable (all fields zero),
and the global backoff has returned to the maximum while A
remains pending.

## Eventual A release and clean shutdown

Keep the PDP-6 guest running until A's 24th deferred attempt.
The CTY `TRUE` then returns to its shell. CTY `PS` reuses PID 5
with TD=0 RETRY=0 PVM=0 DETAIL=0 ERR=0. Read-only SIMH monitor
examination after both owners have finished records:

    25116: 000000030106    ; no pending bit, no retry delay
    25120: 000000000000    ; B exact VM base cleared
    25121: 000000000005    ; B five VM destroy attempts
    25122: 777777777777    ; B real MM unpin completed
    25123: 000000000000    ; A exact u-area base cleared
    25124: 000000000030    ; A 24 mm_free attempts
    25125: 777777777777    ; A real MM unpin completed

The shared pending retry state has cleared, both exact MM
extents are released, and both process slots were safely reused.
Guest `HALT` stopped at octal PC 022613, SIMH `exit` completed
normally, and final read-only `d6fsck -n 4` selected superblock
742, seq=3, state=CLEAN (179196 blocks, 360 FCBs).

## Scope

Reverse **post-VM A / PREVM B** physical independence, qualified
first-retry timing, both owners' release, safe PID reuse and
clean guest shutdown **PASS**. The remaining K04 acceptance
includes SIDL mixed-stage ordering, clock wrap, FILE/HUP,
dying-parent WAIT, and any untested permanent refusal patterns.
The fixture adds only disposable TEXT/BSS; production KCORE
size, source, ABI and the external testkit remain unchanged.
