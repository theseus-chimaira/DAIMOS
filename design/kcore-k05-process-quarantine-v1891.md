# K05 v1891: original 48K physical process TRANSITION quarantine

Date: 2026-11-23. **Scoped actual PDP-6 physical PASS**.
K05-K09 OPEN; RAW PURE and mode3 OFF by default.

## Original hardware and source-matched fault

The identical 48K opt-in PURE five-child image, KCORE and MRES from
v1855/v1890 are reused. The source-linked original FULL loader at
`036202` and original service return at `033067` (octal) are derived
from the actual installed PURE base `032417`, found by inspecting
the physical KCORE reclaim JRST. The test-only simulator deposits
one additional MM pin into the live temporary slot3 target descriptor
at physical KCORE word `024705`, changing `000011000003` to
`000021000003`. Its authoritative PACKED source keeps pin1;
the complete PROCESS target now has pin2. The original FULL returns
`777777777773` (`MM_ERR_BUSY=-5`). A real fixed-KCORE PI6 at
`017205` confirms no loss of either owner, selected TSFS root or
unrelated physical MM entries. DTC3 is not detached.

v1891 extends that already accepted complete physical v1890 owner
oracle with the real process-table pointer and full original
three-word process descriptors **after** the fault and clock.
The actual pointer `proc_table` at permanent KCORE `024615` is
physically read as `044712` octal. The entire original process
table through slot6 is examined at this same observed address.

## Accepted physical slot states

The native source `proc.h` defines three process words, flags in
`meta` bits 16-17, TRANSITION bit 16 and the physical VM base in the
RH of the second word. The actual physical records show:

* Slot3 at `044723`: TRANSITION **set**; VM base **zero**; scheduler
  state `PROC_SRUN=2`. The scheduler must not dispatch this
  TRANSITION-held process, and no provisional APR is published.
* Independent slots4,5,6: TRANSITION **clear**, VM base **zero**, each
  with original `PROC_SRUN=2`. They retain their own separate PACKED
  MM extents; an uncertain slot3 owner has not contaminated their
  process descriptors or introduced a fabricated RESIDENT APR.
* Parent slot1 remains `PROC_SLEEP=3`, waiting for its child, and
  exited slot2 is free. This is *not* the guest's normal all-child
  WAIT/Z/HALT completion, which would be unsafe to claim here.

The test compares the physical process-table pointer with the
previous original v1890 PCLK pointer, then verifies it again in
the current run before reading the actual descriptor words. It
requires the entire original v1890 physical MM/owner/service-return
oracle to PASS first. Four independently executed forged physical
trace mutations are rejected at their specific checks:
missing slot3 TRANSITION, a false nonzero slot3 runnable VM base,
an invented TRANSITION flag on healthy slot4, and a wrong original
physical `proc_table` pointer.

The second original SIMH run ends cleanly after these source-matched
physical snapshots; its saved transcript and independent complete
read-only re-audit, including all four negative controls, PASS.
Original v1891 physical transcript SHA256:
`dfdefc29dd3f4665223192754a97693e4425a001b63e7f46459878cbc607e76b`.

The original fixed KCORE and opt-in v1855 PURE MRES binaries are
unchanged, with SHA256 respectively
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`
and `a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
No new code, MM registry, permanent kernel data, BSS or process bit.

Testkit:
`tests/validation/pure-k05-physical-overpin-v1890/` files with
`*-v1891.py` and `run-process-v1891.sh`; commands
`make test-pure-k05-process-quarantine-v1891` and
`make test-pure-k05-process-quarantine-audit-v1891`.
Physical test logs in
`$TMPDIR/daimos-k05-process-quarantine-v1891-real/`.

## Bystander execution remains OPEN

The source-built original 48K MM arena is `043645..140000` octal.
After holding the extra pinned target, no free gap can hold an
aligned `010000`-word full APR, even though the raw largest gap
is `011107` words: it starts at `050071`, and the next permitted
`02000`-aligned base `052000` would extend past the gap end
`061200`. Thus the separate unaffected PACKED children cannot be
assumed able to restore user images within the retained core budget.
A disposable follow-up seeking a NEW secondary child `w` produced
none before being manually stopped. It is explicitly *not* a
completed acceptance test and cannot establish either successful
bystander user completion or a scheduler bug.

The next distinct physical goal is an already-resident independent
peer with an observable postfault user action, or a carefully
source-matched larger/free-memory fixture that still provokes the
same original FULL fault. Persistent source free/allocator ownership
errors, terminal cleanup, all PI6 interleavings and K05-K09 release
remain OPEN. See `design/kcore-k05-physical-overpin-v1890.md`.
