# On-demand LOGIN: working opt-in SUPERVISOR v5

## Implemented behavior

Build with `SUPERVISOR_IDLE_TTY_DISPATCH=1` (ordinary builds retain the
existing eager LOGIN model). PID 1 recognizes only respawn services whose
packed path is `/SYSTEM/EXEC/LOGIN` and registers their TTYs as idle.
Other INITTAB services continue to start immediately and obey their
existing ONCE/RESPAWN policy. Duplicate TTY registrations are excluded.

The combined PID-1 event loop snapshots the kernel generation before
checking ready TTYs and exited children. For each ready idle TTY it
removes registration and starts LOGIN using the established SYS_RUN V2
record; LOGIN then attaches its session and performs authentication.
Successful LOGIN EXEC replaces its image with DSH. After a child exits,
PID 1 re-registers that terminal rather than immediately respawning
LOGIN. The first input byte stays in the kernel receive mailbox for
LOGIN; other waiting terminals retain their idle registration.

The source change uses existing MRES drivers, LOGSTORE compact events,
RUN, WAIT, and the versioned PID-1-only TTYCTL interface. It does not
move password verification or parsing into KCORE or SUPERVISOR.

## Measurements (octal unless marked otherwise)

| | Existing SUPERVISOR v11 | Opt-in v5 |
| --- | ---: | ---: |
| Image | 001177 | 001420 |
| BSS | 000051 | 000053 |
| Origin | 000020 | 000020 |
| Stack | 000400 | 000400 |
| Required | 001670 | 002113 |
| APR allocation | 002000 | **004000** |

The temporarily enlarged supervisor costs one extra APR block (1,024
decimal words). For three unused INITTAB LOGIN terminals the baseline
would use 3 x 3 APR blocks = 9 APR blocks in LOGIN images. With v5, none
are present when idle: an approximate 8-APR-block resident reduction
before accounting for new KCORE/MRES support. A four-terminal profile
would potentially save more, but **was not tested**.

## Targeted simulator acceptance

- Fresh PDP-6 image with all three default LOGIN entries
- DCS0 input wakes LOGIN, ROOT reaches DSH, LOGD/LOGDUMP PASS
- CTY0 input wakes LOGIN, ROOT reaches DSH, LOGD/LOGDUMP PASS
- DCS0 and CTY0 shell exit returns to idle, next input starts LOGIN
- PS on DCS0 shows PID 1 using 2048 decimal words, an active USER
  process and transient PS command, **no idle LOGIN processes**

No full DSH conformance run is claimed here: the last full 115/115
regression belongs to the earlier disabled-event-hub stage. Focused
multi-DCS-line, GE console, 32K, stack high-water and stressed repeated
respawn checks remain before enabling the feature by default.

## Next steps

1. Shrink SUPERVISOR code/BSS back to the 002000 total APR extent
   without reducing its existing 00400 stack safety reservation.
2. Test concurrent receive events on different DCS lines and GE
   consoles, especially the one-mailbox scanner backpressure case.
3. Run an actual 32K multi-terminal boot and high-water instrumentation.
4. Make the dispatcher default only after full acceptance.
