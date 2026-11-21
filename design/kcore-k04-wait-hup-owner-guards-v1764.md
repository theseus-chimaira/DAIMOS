# K04: WAIT report and HUP scan owner guards v1764

Status: source correctness fix; isolated PDP-6 DAS PASS. K04 OPEN.

## WAIT cannot report job-control state for a teardown owner

The native `proc_wait_status` scanner checks ZOMB before inspecting
the pinned u-area for a STOP/CONT report. A deferred EXIT or an
unpublished RUN rollback retains its u-area under TRANSITION while
the MM provider refuses release. The old non-ZOMB branch did not
check TRANSITION, and could consume an old STOP/CONT report from
this dead owner instead of waiting for its terminal publication.

v1764 excludes TRANSITION descriptors from STOP/CONT reporting and
also rejects a zero u-area base before reading the control word.
The child remains visible to WAIT as a child, but is neither
reported nor reaped until it actually reaches ZOMB. Normal live
STOP/CONT and ZOMB paths are unchanged.

## HUP member scan must not read through a zero u-area base

The session leader's HUP pass used the u-area flag and then read
the member control word without checking the base. If a member
had an inconsistent zero base, the scan could consult low memory
and decide incorrectly that HUP was complete. v1764 fails the
session teardown instead: its TTY and retry owner remain intact.

These checks add no persistent state or user ABI. Native PDP-6
assembly PASS; no testkit or physical guest was run. Multi-owner
refusal, clock-wrap and WAIT/PID reuse acceptance remain OPEN.
