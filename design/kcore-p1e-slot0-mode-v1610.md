# P1-E v1610: slot-0 storage-mode dispatch and RUN restore admission

Date: 2026-11-16
Baseline: v1609 PACKED terminal cleanup, v1608 executable admission.
Status: accepted *mode-selection* implementation in the real removable
swap service, and source-linked staging of RUN reconstruction.
This does **not** activate P1-E RAW reclamation or runnable restoration.

## Production slot-0 MRES change

The existing PDP-6 PI6 selector requests a nonresident winner using
the zero APR relocation base and `proc_sched_cursor` request bit
0400. That mechanism cannot identify DISK versus RAW PACKED,
because both representations have zero resident APR base. The
two-bit `proc_storage_runtime_v29` map already owns this state;
adding another process word or queue would be wrong.

`proc_swap_mres_service_v481`, in the source-selected
`proc_swap_pdp6.s` removable MRES, now verifies the authoritative
mode **before** calling `proc_swap_in`:

* DISK (1): original synchronous disk restore, pressure exchange,
  fatal I/O path and READY_ONCE publication are unchanged.
* PACKED (2): **no** disk read, disk block free, exchange, TERM
  event, or READY_ONCE. Retain the executable record and exact
  inactive raw MM owner, set the existing 0100-octal scheduler
  cooldown and clear the one pending request. A subsequent PI6
  tick may choose another resident process.
* Any other mode (including the unassigned value 3): do not
  reinterpret the record as a DISK run, and clear the invalid
  request without owner mutation.

The PACKED branch is a fail-closed, explicitly **not-yet-
restorable** path; it will be replaced with real prefix/FULL
restore only when executable lifetime, MM ownership and rollback
have physical acceptance. No production path currently creates
PACKED mode. A permanently inactive PACKED owner cannot become
runnable from this v1610 service.

The new check contributes 11 instructions to the removable
swap MRES and **zero permanent KCORE words**. No KCORE data,
per-process word, CPU vtable, compression codec or disk format
is added. Full PDP-6 KCC `ROOT=tape` build and linked swap MRES
package PASS. KCORE `__kcore_low_end` remains `024534`
octal. Both KCORE DXR and map are byte-identical to v1609;
DXR SHA-256:
`ce1c003e6541976a1df6b233a8f06a1f5cf225ae52486964c045a12f158d06f0`.

## Portable lifecycle preflight for future slot-0 restore

`proc_pure_lifecycle_v1604.h` now distinguishes three actions
using the existing argument (0 DROP, 1 TERMINAL, 2 RESTORE).
For DROP, only an excluded SLEEP or STOP target is permitted;
RUN still refuses before MM mutations. For RESTORE, SLEEP
and STOP remain allowed, and RUN becomes admissible **only**
while the processor executes in slot 0 with the target process
already TRANSITION-held. TERMINAL semantics stay unchanged.

This shared lifecycle check deliberately does not interpret
PDP-6 mapping geometry or physical memory. The real v1381
FULL staging, v1391 bounded FULL staging and v1384 in-place
prefix staging now use the role-aware check consistently.
After the generic preflight, all restore paths still enforce
PACKED mode, zero APR base, source mount identity, immutable
executable record and exact physical MM ownership.

This change removes duplicate fast-prefix process-state policy
and prepares RUN wakeup for the later *actual* slot-0 service.
No staged RAW C transaction is production-linked yet.

## Source-linked native PDP-6 tests

* `pure-slot0-mode-v1610/run-v1610.sh` executes the unchanged
  extracted production MRES service and real two-bit storage
  mode machine code. PACKED retains the record, defers for
  0100 ticks and never invokes DISK or TERM; DISK reaches the
  original one-shot ready path; invalid mode 3 also rejects.
* Its private unsafe mutant routes PACKED directly to DISK.
  The executed test correctly rejects the mutant.
* `pure-ram-boundary-v1604` tests RUN admission only in the
  slot-0/RESTORE/TRANSITION combination, not in DROP.
* `pure-ram-prefix-v1384/v1385` execute a real DROP from SLEEP,
  then fail RUN reconstruction outside slot 0 and succeed when
  the owner is RUN in slot 0, with exact MM and suffix checks.
* `pure-ram-rollback-v1395` verifies the same slot-0-only
  rule for bounded FULL reconstruction, and that RUN cannot
  be evicted directly.
* `daimos-swap-in-real-transaction-v1316/v1317` still passes
  its real PDP-6 disk swap, pressure, rollback and unsafe
  mutation checks after the new mode selection.
* v1608 admission tests, v1609 terminal teardown and its
  unsafe-backing mutant still PASS.

This is native source-linked acceptance, not a complete booted
physical full-guest swapless reclaim/restore test.

## Next gate: actual RAW drop and wakeup

The next production step must supply an immutable-source hold
through inactive lifetime, install an appropriately sized
no-BACKSTORE PURE service (not an unused full disk-swap MRES),
and execute the existing zero-copy v1381 DROP and v1384/v1391
restore from actual slot-0 process ownership. It must classify
rollback results before clearing TRANSITION or publishing
READY_ONCE. A refused MM release, invalid file source or
partially restored APR must never be made runnable.

Complete native and physical PDP-6 PI6/slot-0/EXEC/WAIT/BRK/
EXIT/MM-pressure acceptance is still required before P1-E is
considered active. Optional mutable-RAM compression follows
only after the uncompressed implementation is accepted.
