# K04 v1787: physical SIDL clock-wrap recovery

Date: 2026-11-21. Status: **PASS for the single-owner physical SIDL
17-bit clock-wrap case**. K04 as a whole remains OPEN.

## Scope and provenance

This extends the source audit in `kcore-k04-clock-wrap-audit-v1786.md`
with a real Open SIMH PDP-6 256K guest. The disposable image is derived
from the v1785 validated MM-pin fixture; generated scratch
`kcore-asm/proc_pdp6.s` and `proc_run.s` only are instrumented. The
guest uses the real `mm_pin`, `vm_space_destroy`, deferred SIDL
reaper and `mm_unpin`, without a fabricated MM refusal result.

On the first unpublished PID 5 RUN rollback, the fixture physically
pins its exact VM extent and sets the qualified PI6 clock to octal
`0377770`. A first failed destroy starts a real `020`-tick retry.
The second and third *actual* VM destroy entry clocks are captured in
scratch BSS. The fifth destroy attempt unpins the exact VM and permits
the unmodified destroy path to finish. Scratch BSS also retains the
first refusal epoch and pending/backoff word. No production timer
policy, kernel source, fixed resident RAM or ABI is changed.

The scratch image and driver are versioned under:

    $TMPDIR/daimos-k04-v1787-wrap-sidl
    $TMPDIR/daimos-k04-v1787-wrap-fixture.py
    $TMPDIR/daimos-k04-v1787-wrap-run-v2.py
    $TMPDIR/daimos-k04-v1787-wrap-results-v2.log

The disposable build completed with an initial read-only D6FS check
of superblock 743, sequence 1, CLEAN. SIMH had unique DCS/GE ports
21587/21687 and ran at reduced CPU priority, separate from the
concurrently running `pdp10-testkit` sweep.

## Exact real PDP-6 evidence

The guest logged in as ROOT on CTY and invoked `TRUE`. Its deliberately
rejected first RUN left the unpublished SIDL VM owned for deferred
cleanup. After real retries, `PS` reused PID 5 with `TD=0`,
`RETRY=0`, `PVM=0`, `DETAIL=0`, `ERR=0`:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 3072 0  0     0   0      0   PS

The guest then issued a normal `HALT`, at PC octal `022644`.
Only after that HALT, the disposable SIMH command file performed a
read-only memory examination; it then exited normally:

    025145: 000000402041   ; raw qualified PI6 clock
    025147: 000000000350   ; shared retry word, pending/delay cleared
    025151: 000000000000   ; exact SIDL VM base released
    025152: 000000000005   ; five real VM destroy attempts
    025153: 777777777777   ; real mm_unpin completed
    025154: 000000000000   ; second-owner post-VM base unused
    025155: 000000000000   ; second-owner attempt counter unused
    025156: 000000000000   ; second-owner unpin marker unused
    025163: 000000377770   ; injected 17-bit near-wrap clock
    025164: 400020377770   ; first pending flag, 020 delay, epoch
    025165: 000000377770   ; first refused VM destroy clock
    025166: 000000400010   ; second actual VM destroy clock
    025167: 000000400050   ; third actual VM destroy clock

The raw clock naturally continues above octal `0377777`; both reaper
due checks mask it to 17 bits. The two measured modulo-17-bit elapsed
intervals are therefore:

    (0400010 - 0377770) & 0377777 = 020
    (0400050 - 0400010) & 0377777 = 040

There is no premature retry at the boundary: the second attempt
occurs after exactly `020` qualified ticks, and the third after
exactly `040` more. Five real attempts and subsequent MM release
complete without leaving a SIDL owner or retry pending.

Final read-only `d6fsck -n 4` selected superblock 743, sequence 3,
state CLEAN, with 179196 blocks and 360 FCBs.

## Test validation and control-session correction

`daimos-testkit/tests/validation/daimos-k04-clock-wrap-verify-v1787.py`
rechecks the symbol map, complete post-HALT SIMH transcript, exact
retry clocks, MM owner state, PID reuse and read-only D6FS CLEAN.
Its self-test passed a positive example and five negative controls.
The validator independently accepted this physical transcript and
rejected an additional one-tick mutation of its actual second-attempt
clock. The testkit validator was committed as `81a99b0`.

Two earlier exploratory control sessions were **not** accepted: their
CTRL-E inspection interrupted a `boot.ini` script containing an
explicit `exit` after `go 020`, causing SIMH to exit before guest
HALT and leaving the disposable D6FS dirty. Both media sets were
discarded and recreated CLEAN before the successful run. The final
method performs no live SIMH interrupt: the guest HALTs first, then
the scratch `boot.ini` executes the read-only `examine` command
before its normal `exit`. The experiment's MM pin/refusal concept
was valid; the issue was the original monitor-control sequence.

## K04 boundaries

This **closes only the physical single-SIDL-owner clock-wrap case**.
It does not validate a newly refused second owner across wrap, the
reverse SIDL/PREVM mixed-order physical cases, permanent refusal
and operator recovery, FILE/HUP ownership or a dying parent's
WAIT/PID reuse. Those remain K04 OPEN under the v1771 physical
acceptance plan. No KCORE optimization or production code edit was
performed; the running external test sweep was not disturbed.
