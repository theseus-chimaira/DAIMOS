# PID 1 idle TTY event contract (2026-10-13 v2)

The existing process wakeup routines use *distinct* child and event
classes: `proc_notify_parent` wakes child waiters, and
`proc_wakeup_event` wakes event-address waiters. Neither is a reliable
replacement for a combined wait. In particular, reading WAIT/NOHANG
and then calling an independent TTY wait loses a child transition in
the interval before sleeping.

The first deterministic testkit model is
`tests/validation/daimos-idle-tty-event-model-20261013-v2.py`, which
exhaustively checks 60 orderings of two independent TTY arrivals, one
child completion, readiness snapshot and sleep registration. It keeps
the triggering byte in the original mailbox and never loses a wakeup.
The intended kernel protocol is:

1. Maintain one shared event generation and one idle-TTY eligibility
   mask (logical TTY 0..20), not a per-TTY process.
2. Under an atomic kernel scheduling/PI boundary, capture generation,
   inspect queued input and reportable child states, and sleep only if
   neither condition changed. Hardware publication increments the
   generation and wakes the shared channel.
3. On awakening, inspect both conditions before rearming. Preserve the
   first input byte for the AUTH image; do not make PID 1 its consumer.
4. Account for CTY, the DCS scanner and the GE receiver separately:
   simply adding a wake bit to the PI handler does not arm either shared
   multiplexer, which is currently reader-driven.

The model tests the concurrency specification but is not a substitute
for compiling and booting a real kernel. Stage v2 deliberately makes
no production behavioral change: the shared event API and hardware
arming need coordinated implementation and additional real-hardware
regression coverage before activation.
