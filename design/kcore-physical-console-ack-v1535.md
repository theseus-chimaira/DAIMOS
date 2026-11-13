# KCORE v1535: deterministic physical-guest console acknowledgment

Date: 2026-11-13. This change is in the physical-test harness only:
the complete 256K PDP-6 KCORE, installed DTFS/TSFS providers,
physical media, 72-bit identity admission, FILE and VFS operations
are unchanged. VERSION remains 0.5; strict 32K/optimization HOLD.

## Observed failure and source diagnosis

Following the accepted v1534 mixed-provider gate, a v1527 physical
detach/replacement/return regression stopped after the guest's
`DTFSMEDIA V1527 READY_SWAP` and eventually timed out before its
`DTC4_PREPROBE_OK` checkpoint. This was not a verified filesystem
or controller error: the guest never printed a negative FSINFO
result or a provider-specific error. A monitor break showed the
fixed KCORE scheduler's idle loop rather than a completed
physical-I/O error; the test was marked UNRESOLVED, not PASS.

The original test script paused SIMH with Control-E, ran physical
DETACH/ATTACH monitor commands, and then wrote `cont\n` and
`A\r` immediately to the *same simulator stdin*. These are not
one coherent delivery channel. SIMH monitor command processing
and the running PDP-6 CTY device poll console input separately
(`sims/scp.c`, `sim_console.c` and `PDP10/kx10_cty.c`).
The acknowledgment can arrive while monitor parsing is still
active or before PID1 has reached its blocking CTY READCHAR.

A versioned v1535 guest witness placed immediately after the
`READY_SWAP` READCHAR, without altering any v1527 identity or
FILE assertions, distinguished missing acknowledgment from
subsequent DTC4 revalidation. An unchanged raw-input replay
passed once; another reproduction timed out without reaching
the witness. In a diagnostic run, the simulator was interrupted
at `proc_idle_loop`; `SHOW SEND` confirmed that input from
an early test of SIMH's queued console SEND had already been
consumed. A clean run with that short delay also passed.

The source explains the second timing window: CTY output uses
one 10000-instruction service period per character
(`kx10_cty.c`). The host can observe `READY_*` before
the trailing CR/LF has finished and before PID1 is inside
READCHAR. A `SEND AFTER=10000` therefore remains too early
to guarantee the intended state.

## Correct v1535 transport

`daimos-dtfs-tape-root-mount-v578.sh` now queues
`SEND AFTER=200000,DELAY=20000,"A\r"` through the SIMH
monitor **before** issuing `CONT`. The SEND engine releases
characters only while the simulated CPU runs, rather than
relying on the competing monitor stdin and CTY read paths.
The first injection is delayed by twenty CTY output periods,
longer than the remaining CR/LF and guest-call transition.
The second character is separated by two periods so that the
receiver has time to consume the first. The host checks that
SIMH accepts the SEND command and the normal monitor prompt
returns before resuming. There are no wall-clock sleeps,
retrying stale acknowledgments, invented controller status
values, kernel hooks, or relaxed feature assertions.

The testkit keeps its v1530 unique tape images and private SIMH
do-file per physical guest. The same transport is shared by the
pre-existing v1527/v1528/v1529/v1531 and accepted v1534 physical
gates, which must all remain functional.

## Acceptance and limitations

The strict v1527 guest physically mounted independently
stamped DTC3 and DTC4, refused a detached DTC3 and an
independently stamped replacement, confirmed DTC4 remained
accessible, restored and fully revalidated both halves of
the original stamped 72-bit DTC3 identity, reused the original
open FILE and cleaned up both mounts. Its unchanged
`RETURN_VERIFIED` and `PASS` gates pass with the new
queued transport. The witness confirms a received second
acknowledgment before the replacement probe.

This change is a test-transport correctness correction,
not a new KCORE feature or a workaround for an unproven
permanent-device-loss condition. It does not close
P0-A, durable P0-B admission, mixed DSK/DRM/LOGSTORE
P1-C, remaining P1-D inventory profiles or P1-E
swapless PURE/MM/APR.
