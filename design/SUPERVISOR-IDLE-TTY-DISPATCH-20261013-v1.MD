# Experiment: PID 1 idle-TTY dispatcher (2026-10-13 v1)

## Objective

Remove every **idle** LOGIN process. A terminal that has no user
session should use no process extent at all. PID 1 remains within its
existing single 02000-octal-word APR allocation, and authenticated
sessions continue to replace the authentication executable with DSH.
The goal is *net* resident RAM saved, not shifting code into KCORE.

## What the current source actually supports

- `userland/init/supervisor.c` starts configured children via SYS_RUN,
  tracks PIDs, sleeps in SYS_WAIT and respawns on exit.
- `userland/login/login.c` creates the session, attaches the TTY,
  sets its foreground process group and mode, authenticates using
  `passwd_reader.c`, logs and EXECs the shell.
- `proc_wait_status` (`proc_pdp6.s`) accepts NOHANG or ordinary child
  wait. Ordinary wait uses a dedicated PROC_WAIT_CHILD sleep channel;
  it cannot simultaneously wait for a TTY-ready event.
- `cty_io.s` has a one-byte mailbox and an existing kernel event word.
  DCS and GE receive scanners similarly use one shared mailbox/event;
  the scanner is armed by a reader, and non-target bytes are deferred
  into existing per-TTY process records. An idle-line event cannot be
  implemented merely by inspecting canonical line state.
- There is no public readiness operation covering CTY, DCS and GE and
  no combined child/TTY wait ABI. Polling or blocking on one chosen TTY
  would break idle terminal fairness and is forbidden here.

## Proposed implementation stages (not yet merged)

1. Establish a safe *kernel-owned* idle-TTY registration mask and a
   single readiness event, reusing existing DCS/GE scanner resources.
   Every received character must remain associated with its actual
   terminal; activation must not consume the first username byte.
2. Integrate completion of a child and readiness of any registered idle
   TTY in one race-free wait/notification path. Arm and recheck under
   the same scheduler/interrupt rules as the existing event sleep.
   Avoid periodic polling and any per-TTY permanently allocated buffer.
3. Let PID 1 keep its original INITTAB service model for arbitrary
   programs. Apply idle dispatch **only** to explicit login services;
   every non-login respawn/once entry keeps the existing behavior.
   Authentication should start as the first active process, not as a
   second simultaneous child of a waiting LOGIN.
4. On failed authentication, return to idle registration without an
   orphan controlling session or abandoned input; on shell exit,
   unregister the old owner and rearm the line exactly once.
5. Verify CTY0, each class of DCS/GE terminal, concurrent arrivals,
   boot-time input, repeated logout, and stopped/background shell jobs.
   Measure 32K peak memory with four idle configured terminals.

## Critical design choices

- A new combined wait is NOT equivalent to SYS_WAIT_NOHANG followed by
  blocking on a terminal event. The child may terminate after the scan
  and before sleeping, losing wakeup; both conditions need an atomic
  recheck, or a common event generation counter.
- The DCS scanner is shared over sixteen lines, so keeping input
  disabled unless a process reads a specific line is insufficient.
  Registration has to explicitly arrange receive arming and routing.
- PROCCTL NEWSESSION/TTY_ATTACH and SETFG currently happen inside
  LOGIN. A replacement AUTH executable must own the same trust/session
  boundary; PID 1 must never parse or verify passwords.
- The existing supervisor image is already 001670 including stack and
  origin, leaving **000110 octal words** until another APR block.
  Even a small new dispatcher is unlikely to fit without reducing
  existing code or using a carefully measured kernel service.

## Experiment result

Source inspection confirms that an event-driven single-PID-1 idle-TTY
dispatcher is feasible as a **new integrated scheduling/driver ABI**, not
as a safe userspace-only modification. No kernel/supervisor behavioral
change is committed by this note. The existing v3 LOGIN continues to
run normally; the independent v4 direct-reader experiment remains
uncommitted. Do not advertise an idle-process RAM saving until an
actual four-TTY PDP-6 test confirms it.
