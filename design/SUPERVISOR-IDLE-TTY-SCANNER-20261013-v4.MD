# Idle-TTY scanner activation v4 (2026-10-13)

This is the hardware-facing stage after the disabled kernel notification
hub v3. DCS and GE MRES now each export a small `*_idle_arm` entry.
KINIT patches dedicated KCORE jump slots to these exports only when the
corresponding hardware is installed. With unavailable hardware the slots
retain harmless zero-result implementations.

PID 1's `SYS_TTYCTL_IDLESET` invokes the arm dispatcher after updating
the eligible logical-TTY mask. DCS logical terminals 1..16 share one
receive scanner; GE logical terminals 17..20 share another. Each
backend arms its scanner only when the existing receive mailbox is
empty. It never consumes or overwrites a pending character. CTY0 is
already interrupt-driven and needs no additional arming call.

The interrupt handlers and shared ready-mask hub remain in KCORE/MRES;
all password handling stays in LOGIN. No polling, per-TTY idle process,
or per-TTY permanent input buffer is introduced.

**Concurrency limitation:** DCS/GE each have a single shared receive
mailbox, which temporarily stops that scanner until the active LOGIN
consumes the byte. A simultaneous-input acceptance test for multiple
lines of the same multiplexer remains required. There is no claim yet
that all 16 DCS lines can accumulate input concurrently.

The completed *subsequent* opt-in SUPERVISOR experiment demonstrates
DCS0 and CTY0 waking the correct LOGIN, ROOT reaching DSH, and logout
returning CTY0 to idle. It is recorded separately in stage v5.
