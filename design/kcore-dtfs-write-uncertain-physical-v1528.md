# KCORE v1528: real physical DTFS write failure and sticky containment

Date: 2026-11-13. Feature-completion scope only, following
the accepted v1527 physical read-error/return and BCACHE
identity fix. This increment tests production KCORE behavior
but deliberately does **not** implement a new recovery syscall,
automatic flag clear, media policy or optimization.

## Genuine physical failure

The v1528 daimos-testkit full PDP-6 KCC 256K DTC-root
guest starts a disposable PID1 user process on actual TSFS
root DTC0-2. It mounts stamped native DTC3 **read/write**,
creates `/MOUNT/DT0/FAULT`, writes and verifies one word
and successfully commits file mode metadata on that tape.
The Type-551 transport performs physical write operations
in the successful setup.

At the synchronized `READY_DETACH` stage the testkit
pauses the running simulator from CTY, physically DETACHes
DTC3 only, resumes the guest and signals the blocked PID1.
The guest performs another CHMOD metadata commit through
the real `dtfs_commit` / `dtfs_dtc_write` path; this fails
on unavailable physical transport. No test provider, fake
error callback or patched raw JRST is involved.

At `READY_RESTORE`, the simulator ATTACHes the original
identically stamped DTC3 image. The guest checks the
original mount remains *quarantined*: real FSINFO and STAT
return negative even though the same 72-bit volume is
physically available. It attempts FILE CLOSE twice; both
return errors without dropping the descriptor because
`file_close` requires successful `vfs_sync`. Ordinary
UNMOUNT also returns error and retains the original mount.
The host runner exits zero after the real PID1 reports
`DTFSWRITE V1528 WRITE_STICKY` and `PASS`.

The real Type-551 CONO trace was collected on a repeat
successful run with the existing optional trace switch;
it contains physical DTC3 FNC_WRIT commands
(octal command `220735`). The source-linked v1522
native test separately verifies failed raw writes set
octal `0400000` WRITE_UNCERTAIN and preserve the
actual mount-slot ownership. Same-ID physical return
does **not** clear that bit.

## Interpretation and remaining administrative work

This is **real physical write refusal**, not proven partial
transfer after a subset of a 128-word block was committed.
We DETACHed the tape *before* the failed write. The
WRITE_UNCERTAIN classification conservatively covers
either case because a negative physical status cannot
establish that nothing reached media.

One attempted test assertion expected CLOSE to succeed;
the first actual guest correctly refuted it. The fixed
test explicitly verifies repeated CLOSE refusal and
mount retention. This matters for recovery: ordinary
FILE close performs SYNC, and a still-open poisoned
FILE keeps the all-process v1523 mount-owner guard busy.
Neither same-ID revalidation nor normal UNMOUNT can be
used as an administrative workaround.

The remaining P0-B task requires a **separate explicit,
checked policy** for discarded/fully repaired media and
every outstanding owner, before safely retiring a poisoned
mount. It must never accept a clean RBN0 ID alone as
proof of a good data block or directory after uncertain
write, must never recycle an occupied mount slot, and
must preserve independent mounted devices and root/
BACKSTORE critical-loss policy. A real mid-block or
multi-block partial-write interruption remains another
positive acceptance gate.

Production v1527 KCORE and optional MRES are unchanged
by the v1528 test. Controlled optional DTFS/TSFS MRES
size remains octal `003747` text / `000002` BSS.
No fixed resident instructions, new data words or
separate RAM buffers. VERSION remains 0.5, strict 32K
gate HOLD.
