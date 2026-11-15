# P1-C v1585: real independent PI6 swap-in overlaps mapped MEMFS snapshot

Date: 2026-11-15. Physical KCC PDP-6 acceptance under controlled
timer timing, on the genuine four-member Type-236 DRM root VOLUME.
The original scheduler performs two independently selected, actually
completed process swap-ins while a separate process synchronously
persists MEMFS. Both CPU/RAM cold boots complete their normal native
HALT; the second boot also successfully restores the previous
MEMFS/XIP and LOGSTORE content from the unchanged four-member media.

## Physical experiment

The full 128K guest boots the unchanged production D6FS DR0..DR3
root, BLOCKSET INTERLEAVE, mapped BACKSTORE, Type-236 driver/PI2,
MEMFS v2 and LOGSTORE. Userland launches four genuine 24K-word
process children, verifies long timed SLEEP for all, and leaves
actual positive process swap ownership at the snapshot boundary.
The only test-timing perturbation in the disposable source copy
selects one **already sleeping, timed, disk-backed** process and
shortens its ordinary scheduler deadline to the next 60 Hz tick.
It adjusts the existing nearest-timer frontier if necessary, but
never changes the process state to RUN, inserts a run-queue entry,
calls swap-in, triggers PI6 itself, or fakes a DMA completion.

The original PI6 timer must detect the elapsed deadline and wake
the child. The original slot-0 scheduler independently selects and
enters `proc_swap_in`, issues a real mapped BACKSTORE READ over the
original Type-236 DMA driver, receives matching hardware DR_DONE
PI2 completions, and publishes DISK-to-RESIDENT after the real
transaction completes. All of these boundaries are observed
separately in test-only fixed counters. The normal MEMFS snapshot
code simultaneously writes its 36-bit backing sectors, including
its original last-sector v2 commit header.

The observers are strictly opt-in in daimos-testkit and a disposable
kernel copy. Permanent production code, KCORE RAM, ABI, media format
and the actual scheduler policies are unchanged.

## Accepted linked-KCORE and raw-physical evidence

| Evidence | Cold boot 1 | Cold boot 2 |
| --- | ---: | ---: |
| Re-armed true timed sleeping DISK slot | 6 | 6 |
| Original PI6 timer wakeups | 1 | 1 |
| Original slot-0 independently initiated swap-ins | 2 | 2 |
| Successful DISK-to-RESIDENT publications | 2 | 2 |
| Original mapped `backstore_read` requests | 2 | 2 |
| Matching real swap READ Type-236 PI2 DR_DONE | `0700` | `0700` |
| All actual DRM DR_DONE during snapshot | `01311` | `01311` |
| Original 60 Hz ticks while snapshot active | 2,129 | 2,137 |
| Remaining original process swap blocks | `0750` | `0750` |
| Remaining original BACKSTORE blocks in use | `01034` | `01013` |
| Original `mfsdev_storage_errors` | 0 | 0 |
| Snapshot-in-progress and I/O descriptor markers at HALT | 0 | 0 |

The two normal original privileged HALTs reached octal `022004`.
The valid v2 snapshot commit header physically occupies DR3 sector
`017777` on the four-member raw drum VOLUME. The independent media
oracle verifies the actual MEMFS pool of 3,648 words, 275 other
nonzero BACKSTORE sectors, `075774` octal logical VOLUME data
blocks, `02000` octal backing blocks, and all four unchanged D6FSR2
on-drum descriptors. Boot 2 independently recovered previous
MEMFS/XIP data and LOGSTORE record before appending its next
record. The full snapshot/recovery path therefore survived
the independently scheduled concurrent swap-ins.

The v1585 oracle requires BOTH physical boots to satisfy original
PI6 wakeup, independently scheduled slot-0 swap-in, actual mapped
DMA READ, matching original Type-236 PI2 and final MM/process
publication. In addition to 13 inherited false-physical-evidence
mutations, four timer-specific negative witnesses are rejected.
During acceptance, the inherited v1584 verifier had a genuine
negative-test logic defect: it used healthy-only acceptance while
corrupting a known positive successful swap-in. V1585 corrected that
verifier to enforce strict overlap for any originally positive
two-boot overlap dataset; the old v1584 healthy-but-nonoverlapping
baseline still passes normally and still fails strict overlap.
No source mutation is mistaken for an actual production change.

## P1-C residual, freezes and reproduction

This closes the **controlled independently scheduled swap-in during
synchronous snapshot DMA** acceptance, not the entire P1-C gate.
The timer expiry was deliberately aligned with snapshot entry by
a disposable test hook; this is a real independent scheduler/PI2
path, not an accidental coincident deadline. Remaining P1-C work
includes exact recoverable owner/MM/BCACHE rollback and combined
fault/overlap cases; a genuinely damaged medium is not simulated by
this acceptance. P1-D, P1-E, final freeze and 32K optimization
remain OPEN; VERSION 0.5 and production KCORE low end `024364`
octal are unchanged.

Evidence: `daimos-testkit/tests/validation/daimos-p1c-snapshot-timer-
physical-v1585.{sh,md}`, `daimos-p1c-snapshot-timer-v1585.py` and
`daimos-p1c-snapshot-timer-verify-v1585.py`. Physical workdir from
the first fully checked two-boot experiment:
`$TMPDIR/daimos-p1c-snapshot-timer-v1585.hfCA5o`.

A second clean two-boot experiment after the verifier correction also
completed the full runner and all 17 rejected mutants with exit 0:
`$TMPDIR/daimos-p1c-snapshot-timer-v1585.xYkqo8`. Its original
PI6/timed-swap and physical PI2 counts matched the first accepted
case exactly; its snapshot spans were 2,136 and 2,140 ticks. One
intermediate 180-second probe timed out on boot 1 after invalidating
the physical snapshot header and starting data writes. Since no
stopped CPU state was captured in that attempt, it is not established
whether this was simply slower simulator execution or a genuine
scheduler stall. The v1585-only full-guest probe is consequently
bounded to 300 seconds with CPU/PC capture on timeout, leaving all
older test budgets and all production code unchanged. This caveat
does not negate the fully observed original PI6 overlap, but further
timing/repeated-load stability and combined-failure tests remain open.
