# KCORE v1524: physical DTC3 RBN0 simulator bug and source-backed status

Date: 2026-11-12. KCORE P0-B physical guest acceptance investigation
following v1523. This is a **simulator correctness fix**, not
new KCORE functionality or an optimization stage. No KCORE source,
on-media DTFS identity format, memory allocation or fixed kernel
image layout was changed. DAIMOS VERSION remains 0.5.

## Reproduced physical failure

The v1523 full tape-root guest successfully booted a PDP-6 to
the DSH prompt, but a secondary DTC3 `MKFS.DTFS` or
`FSCK.DTFS` operation did not finish in the fixture.
Target `MKFS.DTFS` writes physical RBN0 first to clear any
old identity, then physical 0144 for the directory.
The v1520 stamped native DTFS format likewise needs raw
RBN0 for mount and post-error revalidation.

Before modifying either production driver or simulator, a
small native PDP-6 diagnostic was derived from the accepted
real `daimos-dtc-stream-online-v1.s` test and linked against
the **actual** DAIMOS `tape_io.s` and
`storage_router.s` with real Type-551/Type-136 PI/DCT I/O.
Each case attached a distinct disposable host-generated full
native DTFS physical image to **DTC3**. Original simulator
and exact same driver/object/pattern:

| Physical block | Old installed SIMH | Updated SIMH |
| --- | --- | --- |
| `000000` | FAIL on native first write | PASS write/read and compare |
| `000001` | PASS | PASS |
| `000144` | PASS | PASS |

The initial two-block RBN0 trial also failed. A subsequent
**single** 128-word RBN0 trial established failure precisely
at the write, independently of crossing a block boundary.
The corrected simulator passed both the RBN0 single-block
and the normal tests. An automated v1524 pre-fix negative
control rejects the old simulator at the actual
`__test_exit` assertion.

## Root cause and correction

SIMH `sims/PDP10/pdp6_dtc.c` sets `DTB_REQ` when
selecting and starting a stopped Type-551 unit. At the first
forward RBN mark, even though Type-136 was prearmed by
the real DAIMOS controller driver, `FNC_SRCH` delivered
the mark only if `DTB_ACT` was already set. On a fresh
end-zone-to-RBN0 start it was still `DTB_REQ`. Consequently
the first usable RBN mark was skipped and no software
could match RBN0 during this pass. Subsequent block marks
were delivered normally.

The simulator now promotes the pending `FNC_SRCH` request
to `DTB_ACT` **at that first forward mark before forwarding
the RBN word to Type-136**. This fixes the model at the
controller boundary, not by moving the DTFS stamp or
inventing a kernel seek workaround. Only eleven lines
were added to the simulator source, with no C interface
or KCORE runtime change.

## Executed checks

- SIMS PDP-6 rebuilt with production `make pdp6`, with
  `-Werror`, and passed its RegisterSanityCheck.
- `daimos-testkit/tests/validation/daimos-dtc-first-mark-v1524.sh`
  source-links production driver and actual device model:
  physical RBN0, RBN1 and RBN0144 all PASS, plus
  the pre-fix simulator real native negative control.
- Existing actual DTC counted-stream and reverse
  transfer tests PASS on corrected simulator.
- v1517 actual simulator detach/select status, v1089
  data watchdog, v1522 real mounted-volume identity
  and v1523 FILE/cwd ownership native source-linked
  tests all PASS unchanged.

## Still OPEN

The above establishes a **real, formerly missing physical
RBN0 transport operation**. A separate 256K KCC
`ROOT=auto BOOT=dsk` guest subsequently booted D6FS
and completed **real target MKFS.DTFS on DTC3 followed
by `FSCK.DTFS NATIVE OK`**. That proves physical
formatting/checking now works and narrows the blocker.

However, `module_minit.c` intentionally installs
DTFS/TSFS MRES **only for a DTC-selected root**.
The disk-selected auto-root has the package linked
but not installed, and its `MOUNT.DTFS` correctly
printed an error. Merely STATing/entering the existing
D6FS directory `/MOUNT/DT0` is not a mounted DTFS.
The old v578 guest fixture erroneously reported PASS
despite the MOUNT error and two failed UNMOUNT calls.
The fixture now rejects such errors, checks actual
FSINFO/MOUNT publication, and checks post-retirement
disappearance; a separate negative-policy test expects
the disk-root refusal. A first scratch-image rerun
halted before LOGIN with `?RT`; after rebuilding
the disposable D6FS root media, the independent
negative-policy **full guest PASSED**: target
MKFS.DTFS returned, FSCK.DTFS printed NATIVE OK,
MOUNT.DTFS refused, and the real MOUNT inventory
listed D6FS and MEMFS but no DTFS. This is a
physical **negative-policy** acceptance, not
mounted-volume or FILE recovery acceptance.

The true secondary DTFS mount/cwd/FILE, return and
dirty-write exchange must still run under a
**DTC-selected root** or an independently justified
module-installation policy change. Expanding permanent
installed modules merely to satisfy the fixture would
increase resident memory and violate current boot policy.
The original DTC-root run spent extensive time loading
executables from the primary DTC0 root before reaching
any physical DTC3 operation.

The v1522/v1523 source-linked KCORE P0-B boundaries remain
accepted in exactly their tested scope. Authoritative
P0-A permanent root/BACKSTORE loss classification and its
v1515 fatal caller remain open; root/BACKSTORE
reconstruction is cancelled. P1 mixed storage, dynamic
FSINFO production guest, uncompressed swapless PURE/MM/APR
and final freeze remain open. Strict 32K KCORE is on HOLD.
