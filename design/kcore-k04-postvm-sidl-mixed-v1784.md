# K04 v1784: post-VM A / unpublished SIDL B physical owners

Date: 2026-11-21. Status: **PASS** for mixed post-VM/SIDL
ownership, exact first SIDL retry interval, independent physical
MM release, PID reuse and clean guest shutdown. K04 remains OPEN.

## Test design

The disposable 256K PDP-6 image at
`$TMPDIR/daimos-k04-v1784-postvm-sidl` derives from the clean
v1773 boot baseline. Only generated scratch `proc_pdp6.s` and
`proc_run.s` are instrumented. PID 5 (A) uses the validated
v1783 post-VM u-area `mm_pin` fixture and remains pinned for
24 actual `mm_free` attempts. The first PID 6 RUN (B) takes
the original `proc_run_hierarchy_bad` rollback path, as in
v1780, and physically pins the exact VM extent before ordinary
SIDL `vm_space_destroy`. B unpins on its fifth attempt; the
fixture then permits subsequent RUN children normally.

Only authoritative `mm_pin`/`mm_unpin` and unmodified MM/VM
providers are used. B's first pending word and clock, and its
second actual VM destroy attempt clock, are captured in
test-only BSS. No fake MM error, scheduling override or
production kernel edit. Native DAS for both generated files,
full image build, initial D6FS CLEAN seq=1 PASS.

Scratch-map addresses, octal:

    025120 qualified clock      025122 shared retry word
    025124 B VM base            025125 B attempts
    025126 B unpin done         025127 A u-area base
    025130 A attempts           025131 A unpin done
    025132 B before pending     025133 B after pending
    025134 B first clock        025135 B second clock

## Live evidence

Two independent ROOT/DSH terminals (CTY, DCS0). Run CTY
`TRUE` first. Stop SIMH and inspect: A base octal `054523`,
attempts `006`, done=0; shared retry word `401000004366`
is pending at maximum octal `01000` delay. Every B fixture
word is still zero. Queue DCS `TRUE` and continue the CPU.
The first PID 6 RUN is deliberately rejected and enters real
SIDL rollback. DCS returns to the shell while CTY remains
blocked. Read-only SIMH monitor inspection shows:

    25124: 000000000000    ; B exact VM extent released
    25125: 000000000005    ; B five destroy attempts
    25126: 777777777777    ; B real MM unpin completed
    25127: 000000054523    ; A still physically pinned
    25130: 000000000013    ; A 11 MM release attempts
    25131: 000000000000    ; A unpin not completed
    25132: 401000004366    ; A saturated before B first refusal
    25133: 400020004471    ; B fresh 020 delay after first refusal
    25134: 000000004471    ; B first-refusal qualified clock
    25135: 000000004511    ; B second actual destroy clock

The actual first B retry elapsed `004511 - 004471 = 020`
octal qualified ticks (16 decimal), not A's `01000` interval.
After B completed, DCS `PS` reused PID 6 with all teardown
fields zero, while A was still TD=2 RETRY=512 DETAIL=6:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 262143 2 512   0   6      0 USER
    6   3    2   3072 0   0   0   0      0 PS

After A's 24th attempt, CTY TRUE also returned. CTY PS reused
PID 5 with all teardown diagnostics zero. Final SIMH snapshot:

    25122: 000000022451    ; no pending bit or retry delay
    25124: 000000000000    ; B VM base released
    25125: 000000000005    ; B five attempts
    25126: 777777777777    ; B unpin done
    25127: 000000000000    ; A u-area base released
    25130: 000000000030    ; A 24 attempts
    25131: 777777777777    ; A unpin done

Guest `HALT` PC 022617 and SIMH exit PASS. Read-only
`d6fsck -n 4` selected superblock 742 seq=3 CLEAN,
179196 blocks and 360 FCBs.

## Remaining coverage

This passes **post-VM A -> SIDL B**. The reverse **SIDL A ->
post-VM B**, SIDL/PREVM combinations, clock wrap, FILE/HUP,
dying-parent WAIT and permanent refusal scenarios remain OPEN.
No production source, fixed RAM, ABI or external testkit edits.
