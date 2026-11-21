# K04 v1778: new physical MM owner resets saturated retry delay

Date: 2026-11-21. Status: **PASS** for post-VM independent-owner
first-refusal timing, including a previously saturated `01000`
octal (512 decimal) shared delay. Other K04 acceptance gates
remain OPEN.

## Objective

Establish on a real PDP-6 guest that owner A can be pinned until
the shared retry interval reaches its `01000` maximum, and then
a newly refused owner B starts its own `020`-tick interval rather
than inheriting A's delay. Verify the actual next retry occurs
after at least `020` ticks, B can release and reuse its slot while
A remains pinned, and the writable root shuts down cleanly.

## Scratch-only instrumented image

The image lives under `$TMPDIR/daimos-k04-v1778-fresh-delay`.
It derives from the same clean 256K v1773 production boot image
used in the v1777 tests. The existing **disposable** two-owner
`proc_uarea_release` fixture is retained: real `mm_pin`, normal
`mm_free` with genuine MM_ERR_BUSY, and real `mm_unpin` on the
twenty-fourth release attempt of PID 5 and fifth of PID 6.
Each fixture owner verifies its exact u-area base across retries.

Four more scratch BSS words observe PID 6 without controlling
the scheduler or forging a provider return:

* `k04_b_before_retry_v1778` (octal 025122) records the shared
  retry word immediately before B's first physical pin/failed free.
* `k04_b_first_retry_v1778` (025123) records it immediately after
  `proc_finish_mark_pending_v892` arms B's first refusal.
* `k04_b_first_clock_v1778` (025124) records the qualified clock
  at that first marker.
* `k04_b_second_clock_v1778` (025125) records it on B's second
  entry to the unchanged MM release path.

The fixture also retains PID 5 base/calls/done at 025114..025116
and PID 6 base/calls/done at 025117..025121. All addresses are
from the **scratch** KCORE map and do not belong to production.
Isolated native DAS and the full PDP-6 image build both PASS;
initial read-only D6FS check is CLEAN seq=1.

## Negative test-harness finding

Two exploratory runs observed B's proper `0400` -> `020` reset
and an exact `020`-tick first retry, but they did **not** establish
the required `01000` predecessor. The DCS `PS` used to inspect
owner A was itself process PID 6; its exit activated B's MM pin
fixture **before** the later intended `TRUE` command. The saved
first-refusal snapshots therefore described that earlier PS, not
the intended new child. This is a harness/ordering error, **not**
a K04 kernel regression. Neither exploratory run is counted as
the saturated-delay acceptance trial.

## Accepted uncontaminated physical PDP-6 run

After verifying a CLEAN root, boot the complete image under Open
SIMH PDP-6 and log in as ROOT/DSH independently on CTY and DCS0.
Start foreground `TRUE` from CTY to hold owner A (PID 5).
**Do not run any DCS child (including PS) before introducing B.**
Use the SIMH console's read-only `examine` to observe the
`proc_finish_last_tick_v881` word at 025112, inferred directly
from the scratch map's `proc_timer_clock` (025110) followed by
`proc_timer_next` and the shared retry word. Allow A to retry
until the LH contains pending + the capped interval:

    sim> examine 25112
    25112: 401000005134

While the CPU is stopped, verify PID 6's scratch base, attempt,
done, and all four snapshot words at 025117..025125 are **zero**.
Thus no earlier PID 6 child has touched the fixture. Queue the
DCS foreground `TRUE` while the CPU remains stopped, then resume.

Read-only monitor inspection after B's first and second attempts
recorded (all values octal):

    25114: 000000054513    ; A still owns exact u-area
    25115: 000000000012    ; A: ten release attempts
    25116: 000000000000    ; A: not unpinned
    25117: 000000000000    ; B u-area freed
    25120: 000000000005    ; B: five release attempts
    25121: 777777777777    ; B: successfully unpinned
    25122: 401000005134    ; raw shared word BEFORE B failure
    25123: 400020005477    ; raw shared word AFTER B first mark
    25124: 000000005477    ; B first refusal clock
    25125: 000000005517    ; B second release clock

The before/after LH changes from `0401000` to `0400020`:
pending remains set, but interval resets from octal `01000`
(512 ticks) to `020` (16 ticks). The difference between B's
second and first clocks is `005517 - 005477 = 000020` octal,
exactly 16 ticks. This is a real measured interval, not merely
a predicted scheduler state.

The DCS `TRUE` parent returned to its prompt. A subsequent DCS
`PS`, now safely **after** B finished, confirmed:

    PID PPID S WORDS  TD RETRY PVM DETAIL ERR COMM
    5   2    2 262143 2  512   0    6      0 USER
    6   3    2 3072   0    0   0    0      0 PS

This proves owner B's real MM release and PID reuse while owner A
remains physically pinned and retrying at the maximum interval.
The independent DCS shell then requested guest `HALT` with A
intentionally still refused. SIMH stopped cleanly at octal PC
022607 and exited. Final read-only `d6fsck -n 4` reported
superblock 742 seq=7 CLEAN, 179196 blocks, 360 FCBs.

## Acceptance and limitations

**PASS:** two physical post-VM MM owners; A at saturated `01000`;
B first refusal resets the shared retry clock to `020`;
the first actual retry occurs exactly 16 qualified ticks later;
B releases while A remains refused; PID 6 becomes reusable;
guest operator HALT and read-only filesystem integrity pass.

The scratch image adds test-only TEXT and BSS; it does **not**
change production KCORE size, source, ABI or permanent state.
`daimos-testkit` was not touched during its external sweep.
This does not close SIDL rollback, PREVM release, clock wrap,
FILE provider failures, HUP or dying-parent WAIT acceptance.
