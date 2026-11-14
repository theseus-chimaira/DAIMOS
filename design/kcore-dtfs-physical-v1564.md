# KCORE P0-B v1564: full physical DTVOL2/CLEAN and DIRTY cold boots

Date: 2026-11-14. **Physical PDP-6 acceptance PASS; P0-B OPEN.**

## What was tested

v1555 admitted valid DTVOL2/CLEAN only in READ-ONLY mode, and v1562
added transient mounted-owner quiescence; v1563 fixed staged PURE
owner races. Their positive V2 acceptance was previously proven by
source-linked native PDP-6 tests, but not by a fully booted PDP-6
guest mounting an independently formatted physical Type-551 DTC3 tape.

v1564 makes no production source changes. It extends the accepted
v1525 TSFS-root physical guest harness and v1526's independent
PID1/PID2 RUN and pipe-rendezvous test with two real physical boots:

### Boot 1: valid CLEAN native DTVOL2

- Current host `mkdtfs.c` builds a full physical 578-block DTC3
  tape with `--durable-v1550`. Independent host verification checks
  36-bit DTVOL2 marker, both nonzero 36-bit volume-ID halves,
  complement, CLEAN salted seal, all 122 zero words of RBN0 padding,
  and the RBN0144 directory allocation owner at native word 14.
- With the real KCORE, linked DTFS MRES and Type-551 controller,
  a writable mount is refused, and a subsequent RO mount succeeds.
  The real second child owns the mounted cwd and then a FILE;
  unmount refuses on each. Once the child has released both, the
  parent retires the mount while the child is still alive.
- After reap, another RW mount is rejected, a second RO mount and
  retirement succeed. The independent complete-tape SHA-256 is
  identical to the value before the guest started.

### Boot 2: valid WRITE_INTENT native DTVOL2

- From a separate freshly formatted V2 physical tape, the harness
  writes an exact valid WRITE_INTENT state and the correct salted
  checksum into RBN0. This is not an invalid checksum or missing tape.
- The physical guest refuses both RW and RO admission; no VFS mount
  slot is published and an unmount of the absent mount fails.
- The full tape SHA-256 remains exactly unchanged across the guest.

The old v1526 physical DTVOL1 cross-process test was also rerun
against the same current KCC image after the shared-harness change:
PASS. All actual guest stage labels are in testkit
`tests/validation/daimos-dtfs-v2-physical-v1564.md`.

## Boundary

The complete current KCC 256K kernel, userland INIT, production
syscalls, DTFS provider and physical SIMH Type-551 controller were
used. There are **zero changes to production KCORE/MRES code**,
zero new fixed RAM words and no changed on-disk default format.
Native v1555 RO guard remains active.

This closes the previously untested *physical-guest* DTVOL2/CLEAN
RO admission/unmount and WRITE_INTENT rejection scenarios. It does
not prove writable DTFS: torn final CLEAN I/O, independent physical
readback, removable-tape media generation/exchange, partially written
data, same-volume repair/reinsert and cold boots after actual RW
transactions remain open. Independent scheduling is exercised by
PID1/PID2 cross-process cwd/FILE ownership but not by a deliberately
delayed SYNC plus simultaneous new owner publication.

**P0-B OPEN**, P0-A CLOSED, P1-C/D/E unchanged. VERSION 0.5.
Strict 32K/performance optimization remains HOLD.
