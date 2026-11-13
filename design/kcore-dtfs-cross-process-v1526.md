# KCORE v1526: physical cross-process mounted DTFS ownership

2026-11-13. This is a feature acceptance increment against
the **unchanged v1525 fixed KCORE and optional DTFS/TSFS MRES**.
No optimization, module policy change or resident allocation
is included.

## Why v1525 was not sufficient

The v1525 physical PDP-6 guest had already checked actual
native DTC3 mount admission, FSINFO, GETCWD and both
cwd/FILE unmount refusal phases for **one** PID.
The KCORE v1523 owner scanner is required to cover
all live processes, including those other than the
unmount caller, but this still needed real guest
acceptance with an independent process owner.

## Actual v1526 test

The new daimos-testkit
`tests/validation/daimos-dtfs-cross-owner-fullguest-v1526.sh`
uses the existing physical TSFS root, DTFS provider
and SIMH v1524 corrected Type-551 implementation.
It changes only the **disposable** PID1 test executable,
not any production KCORE or userland command.

PID1 mounts real stamped DTC3 native DTFS and verifies
FSINFO provider 5 at `/MOUNT/DT0`. It RUNs a
**second genuine user process** from TSFS, with
two kernel pipes and phase ACKs (no speculative
sleep or process-status assumptions).

- The child alone holds DTFS cwd. PID1 remains at
  `/`, owns no DTFS FILE, and UNMOUNT fails without
  losing FSINFO mount publication.
- The child opens the mounted directory as a
  real FILE, leaves that cwd, and ACKs. PID1's
  UNMOUNT fails again, now because **the other
  PID's FILE is open**.
- The child closes FILE and ACKs but remains
  **alive and blocked**. PID1 checks real
  PROCINFO for a non-zombie child, successfully
  unmounts, and verifies the mount slot empty
  while the other process still exists.
- PID1 releases the child, waits and verifies
  normal exit status zero.

The actual PDP-6 CTY contains `DTFSCROSS V1526
CROSS_CWD_REFUSED`, `CROSS_FILE_REFUSED`,
`CROSS_RETIRED`, and final `PASS`. Two
physical runs succeeded, including the final
more stringent PROCINFO and PID checks.

The completed **physical cross-process** mount
ownership part of P0-B can be removed from
outstanding work. This does **not** accept
actual DTC3 physical read faults, volume
exchange, 72-bit same/different-volume return,
WRITE_UNCERTAIN after partial write, or
administrative recovery. P0-A still needs
authoritative critical root/BACKSTORE loss
evidence, not guesses based on generic DTC
select or PI errors.

The fixed KCORE and DTFS/TSFS MRES binaries
are unchanged by v1526. Controlled v1525
optional MRES remains octal `003746` text,
`000002` BSS, and no KCORE RAM was added.
VERSION 0.5, strict 32K gate HOLD.
