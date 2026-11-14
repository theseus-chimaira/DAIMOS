# KCORE P0-B v1571: Type-551 physical media-retention contract

Date: 2026-11-14. **Hardware safety gap demonstrated with a negative
real PDP-6 cold-boot/Type-551 test.** No production code changed.
Writable DTVOL2 remains disabled by v1555; P0-B remains OPEN.

## Required invariant

For ANY actual DTVOL2 WRITE to physical DTC unit U, the exact same
physical reel identified by both original 36-bit V2 ID halves must
remain retained from the authenticated RBN0 state check, throughout
the subsequent DATA/DIR transfer, until its physical write result
and any readback complete. The same requirement also covers the
initial CLEAN->DIRTY write and final DIRTY->CLEAN seal. A process
lease, in-memory mount epoch or raw readback cannot impose this
invariant on an operator manipulating a real Type-555/TU55 drive.

The v1551-v1554 software already provides the required on-media
WRITE_INTENT and fresh physical RBN0 revalidation under its
serialized DTFS provider lease. v1566 provides a quiesced physical
CLEAN and v1570 the checked destructive replacement after known
poison and owner release. None can stop an externally changed reel
*between* the last valid read and the following controller transfer.

## Physical transport audit

The unchanged production `system/kernel/drivers/tape_io.s` handles
selected-unit Type-136 transfers and PI errors, and keeps transient
per-unit motion hints; it neither latches media generation nor has
a physical reel-retention input. The original Type-551 uses program
selection/motion and status, not a sticky reel-change certificate.
The 1966 DEC TU55 instruction manual section 3.3 says reel loading
requires changing REMOTE/OFF/LOCAL to OFF, releasing the brakes,
then returning to LOCAL or REMOTE. Both operator actions can be
completed while the PDP-6 is between two controller transfers. An
unrelated physical reel with valid tape geometry and mark timing
can then complete the next request without a controller error.

Primary historical manual:
`https://www.bitsavers.org/pdf/dec/dectape/tu55/H-TU55_MaintMan_Nov66.pdf`
(chapter 3, section 3.3). See `pdp10-doc/HARDWARE/dectape.md`
for the separate v1517 physical-versus-SIMH status limitation.

SIMH's `UNIT_ATT` check sets `DTB_ILL` if a selected unit is
unattached. This is **not** a persistent media-change latch:
after a monitor DETACH and ATTACH of a different valid reel,
the current unit is simply attached. No failure occurs if the
guest makes no request while the unit is absent. A v1517 real
PDP-6 status test demonstrates that a formerly detached unit and
an unrelated currently unattached unit give the same status;
status cannot prove which reel was previously present.

## v1571 reproduced negative proof

The testkit boots a complete unchanged production 256K PDP-6
KCC/TSFS-root image. It first physically admits DTC3 on a valid
DTVOL2/CLEAN reel READ-ONLY, having refused V2 RW admission.
The guest blocks on CTY without DTC3 I/O. In ONE uninterrupted
SIMH monitor episode the host detaches the old reel and attaches
a DIFFERENT separately formatted full DTVOL2/CLEAN reel, with a
different verified nonzero 72-bit ID and full salted seal.

The guest resumes WITHOUT ANY controller failure and FSINFO, STAT,
and CHDIR against the old public mounted identity all succeed.
It then changes back to TSFS root and UNMOUNT succeeds. Independent
whole-reel SHA-256 fingerprints prove both valid tapes remained
unchanged and distinct. Two separate full PDP-6 boots PASS the
negative oracle. Full details:
`daimos-testkit/tests/validation/daimos-dtfs-v2-silent-exchange-v1571.md`.

This is not the v1527 observed-read-fault case: after a real
read error, READ_SUSPECT forces an uncached RBN0 identity read
and mismatched replacement is rejected. In v1571, NO read failed,
so READ_SUSPECT is never asserted. The kernel cannot infer an
external change from silence. It also does not prove a wrong
DATA payload was returned or that any real physical tape was
written on the wrong reel; the demonstrated acceptance is mounted
FSINFO/STAT/CHDIR and unmount while identity differs.

## Conditions for safely lifting the production RO veto

1. **Externally enforce media retention:** physical access to the
   reel must be prevented while a writable mount exists, including
   between the last identity check and each subsequent write.
   A hardware lock or equivalent proven mechanical interlock must
   prevent removal, not merely report an earlier transient OFF
   switch state. A voluntary operator agreement is a site policy,
   NOT an enforceable hardware guarantee.
2. **Enforce before physical write:** the write controller must
   reject a transfer whenever the retention guarantee is absent.
   A latch identifying a historical OFF/LOCAL transition is useful
   for fail-stop, but is insufficient alone if a reel can still be
   removed after rearming and before the next transfer. A transport
   lock and an interlocked write path must share the same physical
   control boundary.
3. **Define lock lifetime:** acquire before RBN0 admission or first
   physical DIRTY; hold through every DATA/DIR operation, physical
   final CLEAN readback, and all owner/MM release; physically unlock
   only after the VFS mount is actually retired. Upon device loss,
   reset or uncertain lock state, refuse new writes and preserve
   the old poisoned owner. No automatic same-ID repair.
4. **Test adversarial removal:** verify removal is physically
   impossible or physically inhibits writes at every gap (including
   media switch OFF/LOCAL/REMOTE, controller RESET, STOP/START,
   full block search, failed DATA/DIR and final CLEAN) rather than
   relying on a SIMH DETACH error after software has polled status.

Neither present physical Type-551 hardware, the current simulator
nor production DAIMOS provides such an interlock. Do not invent a
software-only `MEDIA_LOCKED` bit, a repeated RBN0 probe, an operator
acknowledgment syscall or a guessed status-register flag and call
that safe. An administratively managed installation can explicitly
assume trusted operators and no live reel exchange, but **that
changes the safety contract** and would require a deliberate release
decision, not an undocumented workaround.

## Release disposition and footprint

v1567-v1570 have accepted physical partial CLEAN/DATA/DIR,
explicit poisoned-owner drop, original-reel refusal, valid CLEAN
replacement and second production cold boots. v1571 conclusively
shows why the remaining no-exchange premise is NOT enforceable by
the present stock controller. P0-B cannot be marked CLOSED for
unrestricted production RW. Retain the v1555 V2 READ-ONLY veto.

No production KCORE/driver/MRES code, data, BSS, mount words, or
ABI changed in v1571. `__kcore_low_end` remains octal `024364`.
P0-A CLOSED; P1-C/D/E unchanged; VERSION 0.5 and strict 32K HOLD.
