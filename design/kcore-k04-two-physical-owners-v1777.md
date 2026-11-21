# K04 v1777: two independently pinned post-VM MM owners

Date: 2026-11-21. Status: **PASS for independent owner progress**;
first-refusal delay independence and other K04 gates remain OPEN.

## Isolated fixture

The v1777 fixture is a disposable 256K PDP-6 image under
`$TMPDIR/daimos-k04-v1777-twoowners`, copied from the production
v1773 boot baseline. Only the generated `kcore-asm/proc_pdp6.s`
is instrumented; production source and external testkit are not
modified. Native DAS assembly and complete PDP-6 image build PASS;
fresh D6FS media is CLEAN seq=1 before boot.

For post-VM u-area release in `proc_uarea_release`, PID 5 and PID 6
have separate scratch base, attempt and completion words. Each
calls the real `mm_pin` on its exact owned u-area, then lets the
unmodified `mm_free` return real MM_ERR_BUSY. PID 6 calls real
`mm_unpin` on its fifth release attempt; PID 5 stays pinned until
its twenty-fourth attempt. Both verify the same exact u-area base
on retries and fail closed if the descriptor changes generation.
Other slots use the unmodified path. The six scratch BSS words
and extra TEXT exist **only** in the disposable fixture.

SIMH map addresses, octal:

    025073 PID5 base     025074 PID5 attempts  025075 PID5 done
    025076 PID6 base     025077 PID6 attempts  025100 PID6 done

## Actual PDP-6 observations

CTY ROOT/DSH started foreground TRUE (PID 5), which remained
blocked by a pinned u-area. DCS0 ROOT/DSH started a second
foreground TRUE (PID 6). The live SIMH monitor showed both
independent pins active, with the distinct exact bases:

    25073: 000000054441   ; PID 5 pinned
    25074: 000000000011   ; PID 5 attempt 9
    25075: 000000000000
    25076: 000000055051   ; PID 6 pinned
    25077: 000000000004   ; PID 6 attempt 4
    25100: 000000000000

After the next retry, the DCS foreground TRUE returned to its
shell while the CTY foreground TRUE remained blocked. Read-only
SIMH examination confirmed PID 6 had unpinned, while PID 5
remained pinned and advanced to attempt 11:

    25073: 000000054441
    25074: 000000000013
    25075: 000000000000
    25076: 000000000000
    25077: 000000000005
    25100: 777777777777

DCS PS then reused PID 6 while PID 5 remained in teardown:

    PID PPID S WORDS  TD RETRY PVM DETAIL ERR COMM
    5   2    2 262143 2  512   0    6      0 USER
    6   3    2 3072   0    0   0    0      0 PS

This verifies independent MM owner quarantine, continued deferred
retry for the refused owner, real MM release for the second owner,
and PID reuse without prematurely freeing the first owner.

The guest was then allowed to run until PID 5 reached its own
twenty-fourth attempt. The CTY foreground TRUE also returned to
its shell prompt. Final SIMH read-only fixture state:

    25073: 000000000000   ; PID 5 base released
    25074: 000000000030   ; 24 attempts
    25075: 777777777777   ; PID 5 unpin complete
    25076: 000000000000   ; PID 6 base released
    25077: 000000000005   ; 5 attempts
    25100: 777777777777   ; PID 6 unpin complete

Guest `HALT` stopped at octal PC 022566, followed by SIMH
`exit`. Final read-only `d6fsck -n 4` selected superblock 742
seq=3 CLEAN, 179196 blocks and 360 FCBs. Thus both independent
owners eventually completed and the writable guest shut down
cleanly. No production source or testkit was changed.

## Boundaries

The test does not independently establish that PID 6's *first*
refusal interval is octal 020 rather than inherited from PID 5;
the observations prove eventual independent completion, not exact
first-refusal timing. Both owners use post-VM FINALIZE, not SIDL
or PREVM. The clock wrap, FILE refusal, session HUP and dying
parent cases remain OPEN. The fixture does not become a permanent
testkit fixture while the external testkit sweep is in progress.
