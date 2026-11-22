# K04 v1800-A: late child completion under pinned dying parent

Date: 2026-11-22. Status: **physical PASS**, 256K PDP-6,
independent verifier, normal guest HALT and CLEAN D6FS.
K04 remains OPEN; the ineligible-INIT v1800-B gate is separate.

## Ownership and order exercised

The C89 guest creates actual PID1 INIT, PID2 parent and PID3
child with real RUN and PIPE descriptors. INIT observes both
children asleep on their own PIPE reads before delivering TERM
to PID2. The scratch-only hook pins PID2's actual VM extent
using `mm_pin` immediately before its ordinary
`vm_space_destroy` during fatal PREVM EXIT. Original VM/MM
operations remain in use; no fabricated process table or
terminal state is introduced.

INIT waits for PID2's reported EXIT PREVM stage, then sends
TERM to PID3. PID3 completes its own teardown after PID2
has become ineligible to WAIT. INIT observes PID3 ZOMB with
PPID 1, verifies PID2 is still in its physical PREVM retry
stage, and successfully reaps PID3. Only later does PID2's
real `mm_unpin` complete; INIT then reaps PID2. Both PIPEs
close and process, process-word and FILE occupancy return
to their initial baseline before privileged HALT.

## Versioned reproduction

Testkit:

    tests/validation/daimos-k04-late-child-fixture-v1800.py
    tests/validation/daimos-k04-late-child-v1800.c
    tests/validation/daimos-k04-late-child-run-v1800.sh
    tests/validation/daimos-k04-late-child-verify-v1800.py

Accepted disposable run:

    $TMPDIR/daimos-k04-v1800a-r2-late-child-physical

The guest compiled as C89, assembled and linked using the
native PDP-10 toolchain; scratch KCORE assembled and the
full 256K image booted. The first exploratory image printed
`SVA!d`: the child adoption had succeeded but the test's
900-iteration observation window expired before the
sixteenth physical MM retry. The diagnostic HALT left D6FS
CLEAN, but the run was **not accepted**. The corrected guest
uses a 4000-iteration observation bound and a new scratch
image; it does not change kernel logic or pin lifetime.

Accepted guest console: `SVAP`, normal HALT at octal PC
`022531`. Post-HALT physical witnesses (octal):

    025036: 000000000000  ; pinned VM base released
    025037: 000000000020  ; 16 real VM destroy attempts
    025040: 777777777777  ; real MM unpin completed

Independent verifier: positive plus seven negative controls
PASS. Real guest memory witnesses PASS. Read-only D6FS check:
superblock 777, sequence 3, state=CLEAN, 179196 blocks,
360 FCBs. No active simulator remained after the run.

## Limits

The test proves late-child adoption by **healthy INIT**
while its original parent is an ineligible physical MM
owner. It checks no premature reuse of the pinned PID2
during the observed adoption/reap phase and restores resource
occupancy, but does not force a post-release PID2 reuse.
It does not cover INIT itself becoming ineligible, organic
media failure, mixed-owner clock wrap, or all other K04
gates. Production KCORE source, ABI and permanent RAM
footprint are unchanged.
