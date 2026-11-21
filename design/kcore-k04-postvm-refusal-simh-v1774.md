# K04 v1774: live PDP-6 post-VM MM refusal and recovery

Date: 2026-11-21. Status: **controlled post-VM return-code refusal
PASS** on an isolated real SIMH PDP-6 guest; physical MM pin and
multi-owner acceptance remain OPEN.

## Method and limits

Start from the production v1773 256K PDP-6 boot image, copied with
independent writable media into a versioned `$TMPDIR` directory.
Verify the copied `kcore.dxr` is byte-identical to production and
read-only D6FS is CLEAN before boot. Modify only the disposable
simulator configuration: use DCS0 port 21161 and remove the trailing
simulator `exit` so Ctrl-E enters the interactive SIMH monitor.
No production source, binaries or testkit are modified.

Boot, log in ROOT on DCS0, stop the simulator at the SIMH monitor
and inspect the existing `proc_uarea_release` call at octal 013616:

    sim> examine 13616
    13616: 260740002316        ; PUSHJ 17,mm_free

The production map identifies `mm_free` at 002316 and `kret_neg5`
at 004715. Inject an exact `MM_ERR_BUSY` (-5) **only** at the
u-area release call site, leaving VM and other MM frees unmodified:

    sim> deposit 13616 260740004715
    sim> examine 13616
    13616: 260740004715        ; PUSHJ 17,kret_neg5
    sim> continue

The DCS ROOT shell runs foreground `TRUE`. Its prompt does not
return while u-area release is refused. Pause the simulator and
inspect the live process table, whose pointer is stored at 025077:

    25077: 000000050236        ; process descriptor table base
    25103: 000000000006        ; high slot bound
    50255: 054363601405        ; child slot 5, TRANSITION + u-area
    50256: 777777777777        ; VM_SPACE_NONE, post-VM stage
    50257: 200024000000        ; scheduler state
    54452: 440000000000        ; u-area 054363 + 067: FINALIZE + ARM
    24772: 000000012752        ; qualified APR clock sample 1

Continue with the refusal in place and pause again after more than
one retry interval. The exact slot, VM_NONE and FINALIZE+ARM
ownership remain unchanged:

    50255: 054363601405
    50256: 777777777777
    50257: 200024000000
    54452: 440000000000
    24772: 000000014026        ; qualified APR clock sample 2

The clock advanced octal 01054 (556 decimal) ticks while the
refused owner remained quarantined. This confirms persistence
across repeated retry opportunities; the test did not directly
measure each retry epoch or provider call count.

Restore the exact original instruction while the simulator is
stopped, then continue:

    sim> deposit 13616 260740002316
    sim> examine 13616
    13616: 260740002316
    sim> continue

The previously blocked DCS shell returns to its `#` prompt after
the pending retry completes. A new `PS` runs as PID 5; its
TD/RETRY/PVM/DETAIL/ERR fields are all zero, showing that the
old quarantined child slot has been released and reused. Guest
`HALT` finishes at PC 022471. The final read-only `d6fsck -n 4`
reports superblock 742 seq=5 CLEAN, 179196 blocks, 360 FCBs.

## Interpretation

This is a *synthetic MM_ERR_BUSY provider return*, not a physically
pinned MM extent. It exercises the genuine kernel post-VM owner
and idle reaper under a live PDP-6 clock, with no guest-visible
test hook or additional resident RAM. It demonstrates that the
owner is retained across refusal, and that restoration allows
the real MM provider to release it and unblock WAIT/PID reuse.

It does **not** validate the MM extent pin accounting, the exact
020/040/0100 retry timing, independent multi-owner first-refusal
timing, the 17-bit clock wrap, FILE provider errors, or deferred
session HUP. These remain K04 acceptance requirements.

One initial attempt to enter the SIMH monitor was terminated by
the default trailing `exit` command while D6FS was mounted; its
disposable disk was replaced from the untouched CLEAN copy before
the successful run. No DIRTY disk was used as the test baseline.
