# K04 v1800-B: physically ineligible INIT and direct orphan reclamation

Date: 2026-11-22. Status: **PASS** in a booted 256K PDP-6,
with read-only independent verification, guest-owned HALT and
CLEAN D6FS. K04 as a whole remains OPEN.

## Exact ownership sequence

The disposable C89 `/SYSTEM/INIT` guest first creates PID1 INIT
with a real D6FS regular-file FD3 and an interruptible PIPE
read. Its child PID2 is an independent root supervisor. Before
any fatal event, PID2 creates a separate session, the real
PID3 parent and PID4 child; INIT, PID3 and PID4 are verified
SLEEPing on separate live PIPE readers. PID4's actual parent
is PID3. PID2 remains runnable and owns the eventual normal
privileged HALT path even after PID1 terminates.

PID2 delivers TERM to PID1 first. Since PID1 is sleeping in
an interruptible PIPE read, its own resumed private fatal
continuation calls `file_close_all`; it is not a foreign
descriptor-table destruction. A copied VFS `vfs_sync` hook
matches **only** PID1's actual FD3 packed vnode and returns
`-5` while its real `PROC_FATAL_CLOSE_BIT` is set. The hook
holds that FILE owner until PID4's terminal FREE is observed,
then delegates to the unmodified VFS/provider dispatch. An
additional maximum-attempt emergency escape exists **only**
in the disposable fixture; it was not reached and would fail
the orphan-order witness if it occurred first. No persistent
kernel flag or descriptor value is forged.

PID2 independently observes PID1 `GETTEARDOWN` FILE stage
and its retained provider error. It then delivers TERM to
PID3. A second copied-kernel hook uses real `mm_pin` on the
exact PID3 VM extent, and normal `vm_space_destroy` repeatedly
refuses it. PID2 verifies PID3's real EXIT PREVM stage before
sending TERM to PID4.

PID4 completes FILE, VM and u-area cleanup normally.
`proc_finish_slot` finds that original parent PID3 cannot
WAIT and adoptive INIT PID1 cannot WAIT either. The existing
`proc_finish_orphan_free_v1768` is taken exactly once, not
`proc_finish_zombie`. Scratch-only witnesses at those two
real source labels check PID4, its original PID3 parent,
the still-physical PID3 pin, and the not-yet-delegated PID1
FILE refusal. PID2 observes that PID4 is FREE while PID3
is still in PREVM and prints `R`.

Once PID4 is FREE, the next ordinary FD3 SYNC delegates
successfully; PID1's real fatal completion retires INIT.
After the sixteenth real VM destroy attempt, `mm_unpin`
releases PID3's physical VM. PID2 WAITS for PID3, closes
its PIPE handles and uses the normal privileged HALT syscall,
whose root authorization and filesystem shutdown are retained.
Neither INIT's private stack nor a host-forced emulator stop
is needed for terminal control.

## Reproduction and independent observations

The versioned testkit files are:

    tests/validation/daimos-k04-ineligible-init-fixture-v1800b.py
    tests/validation/daimos-k04-ineligible-init-v1800b.c
    tests/validation/daimos-k04-ineligible-init-run-v1800b.sh
    tests/validation/daimos-k04-ineligible-init-verify-v1800b.py

Accepted scratch image and actual SIMH trace:

    $TMPDIR/daimos-k04-v1800b-ineligible-init-physical

C89 cross compile, kernel VFS/process DAS, guest DAS and
link, full 256K boot image construction and initial D6FS
CLEAN check PASS. The guest emitted **`OSFVRP`** and HALTed
normally at octal PC **`022605`**. The independent verifier
accepted all physical memory witnesses (octal):

    025116: 000000000000  ; exact pinned VM base released
    025117: 000000000020  ; sixteen real VM destroy attempts
    025120: 777777777777  ; exact MM unpin completed
    025121: 000000000000  ; zero PREVM pin errors
    025122: 000000000001  ; one PID4 direct orphan FREE
    025123: 000000000000  ; zero PID4 zombie publications
    025124: 000000000000  ; zero parent/INIT ownership errors
    025100: 000000000005  ; five matching PID1 FD3 SYNC calls
    025101: 000000000004  ; four real-vnode error injections
    025102: 000000000001  ; one unmodified VFS delegation
    025103: 000000000000  ; zero fatal-FILE flag mismatches

The verifier's positive and twelve corrupted negative controls
PASS. Final read-only `d6fsck -n 4`: selected superblock 1000,
sequence 3, **state=CLEAN**, 179196 blocks and 360 FCBs.
SIMH exited normally; no build/simulator process remains.

Testkit implementation commit: `a783ebe` on `main`.
Production KCORE, its ABI and permanent RAM remain unchanged.

## Acceptance boundary

This closes the **physical ineligible INIT + dying PREVM parent
+ late-child direct FREE** case, complementary to healthy-INIT
adoption accepted in v1800-A. The simulated FD3 refusal is
injected at the existing VFS boundary, not an organic D6FS
or disk-controller/media fault. The test does not establish
all mixed-owner clock-wrap and pinned-session-HUP cases or
the final integrated K04 acceptance suite. Do not close K04.
