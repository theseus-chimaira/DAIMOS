# K04 v1785: unpublished SIDL A / post-VM B physical owners

Date: 2026-11-21. Status: **PASS** for independent mixed SIDL/
post-VM physical MM owners, exact first retry timing, complete
release of both owners, safe PID reuse and clean guest HALT.
The overall K04 acceptance matrix remains OPEN.

## Objective

Reverse the proven v1784 mixed-stage ordering. Let owner A
(PID 5) fail RUN at the real hierarchy rollback and remain an
unpublished SIDL VM owner with its exact VM extent physically
pinned through 24 real VM-destruction attempts. After its shared
retry delay saturates at octal 01000, create owner B (PID 6)
and pin its independent post-VM u-area before ordinary mm_free.
B releases on its fifth attempt. Verify a fresh octal 020
retry interval for B, unaffected A ownership, safe B reuse,
and eventual independent cleanup of A.

## Disposable test image

Copy the clean v1773 PDP-6 256K boot baseline into
`$TMPDIR/daimos-k04-v1785-sidl-postvm`, then transform only
the generated scratch `kcore-asm/proc_run.s` and
`kcore-asm/proc_pdp6.s`, using the physically validated v1784
fixture. PID 5's first RUN follows the original hierarchy
failure/rollback path while its first SIDL cleanup remains
unfinished. This fixture disables the intentional failure when
its real MM unpin completes. PID 6's post-VM u-area is pinned
using real mm_pin before unmodified mm_free, and unpinned with
real mm_unpin on its fifth release attempt. The ordinary
vm_space_destroy and process finalizer remain unmodified.

Only disposable instrumentation and scratch BSS are added.
No fake MM error, scheduler policy override, production KCORE
source, fixed RAM, ABI or external testkit modification.
Native DAS for both generated sources, complete image build,
and initial read-only D6FS CLEAN seq=1 PASS.

Scratch-map addresses (octal):

    025117 proc_timer_clock     025121 shared pending/backoff
    025123 SIDL A VM base      025124 SIDL A attempt count
    025125 SIDL A unpin done   025126 post-VM B u-area base
    025127 post-VM B attempts  025130 post-VM B unpin done
    025131 B pending before    025132 B pending after first
    025133 B first clock       025134 B second clock

## Live PDP-6 results: concurrent ownership

Boot the disposable 256K image, log into CTY and DCS0 as ROOT,
and issue CTY `TRUE` first. The failed RUN command returned
to the shell, but its unpublished PID 5 remained owned by the
SIDL reaper. Read-only SIMH examination before starting B:

    25121: 401000007063    ; A pending, capped 01000 interval
    25123: 000000760000    ; exact pinned SIDL VM base
    25124: 000000000006    ; A six real attempts
    25125: 000000000000    ; A still pinned
    25126..25134: all zero ; B never yet injected

Queue DCS `TRUE` while the simulator CPU is stopped, then
continue. DCS TRUE finishes while A retains its SIDL VM.
Read-only SIMH examination after B's fifth MM attempt:

    25121: 401000010100    ; A still pending and capped
    25123: 000000760000    ; A still owns its exact pinned VM
    25124: 000000000013    ; A eleven actual VM attempts
    25125: 000000000000    ; A unpin not completed
    25126: 000000000000    ; B u-area released
    25127: 000000000005    ; B five real MM free attempts
    25130: 777777777777    ; B real MM unpin completed
    25131: 401000007063    ; before B: A capped backoff
    25132: 400020007120    ; after B first failure: fresh 020
    25133: 000000007120    ; B first-refusal qualified clock
    25134: 000000007140    ; B second real MM attempt clock

B's measured retry is octal `007140 - 007120 = 020`, exactly
16 decimal ticks. DCS `PS` immediately afterwards reused
PID 6 with TD=0 RETRY=0 PVM=0 DETAIL=0 ERR=0. Its same
snapshot showed PID 5 still unpublished SIDL (`S=1`) with
TD=3, RETRY=512, DETAIL=2051 and no premature reuse:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    1 2048 3 512   0   2051   0 TRUE
    6   3    2 3072 0   0   0      0   0 PS

## Final recovery / shutdown

Continue the guest. A remains physically pinned and retryable
through attempt octal 025 (21 decimal). On attempt octal 030
(24 decimal), A calls real mm_unpin and the ordinary SIDL
discard finishes. Read-only SIMH final snapshot:

    25121: 000000025100    ; pending flag and retry delay cleared
    25123: 000000000000    ; A VM extent released
    25124: 000000000030    ; A 24 VM destroy attempts
    25125: 777777777777    ; A real MM unpin complete
    25126: 000000000000    ; B u-area released
    25127: 000000000005    ; B five mm_free attempts
    25130: 777777777777    ; B real MM unpin complete

CTY `PS` reused PID 5 with TD=0 RETRY=0 PVM=0 DETAIL=0
ERR=0, confirming no premature reusable slot had been
published while the SIDL owner was pinned. Guest `HALT` stopped
at octal PC 022616, and normal SIMH `exit` returned status 0.
Final read-only `d6fsck -n 4` selected superblock 742 seq=3
CLEAN, 179196 blocks, 360 FCBs.

## Remaining coverage

Both physical post-VM/SIDL orderings now PASS: post-VM A ->
SIDL B (v1784), and SIDL A -> post-VM B (v1785). This includes
the actual first retry gate and complete independent MM
recovery for both orders. SIDL/PREVM combinations, clock wrap,
FILE/HUP, dying-parent WAIT and permanent-refusal scenarios
remain OPEN. No production code, ABI, fixed RAM or external
testkit changes were required.
