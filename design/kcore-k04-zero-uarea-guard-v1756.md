# K04: zero u-area guard in deferred finalizer v1756

Status: source guard and isolated PDP-6 DAS assembly PASS.
K04 remains OPEN; no physical fault-injection acceptance is claimed.

The slot-zero `proc_finish_service_v881` checked the stable u-area
flag before dispatching a marked deferred finalizer. For VM_NONE
slots it then loaded the u-area base from the descriptor LH and
unconditionally read `PROC_FINALIZE_OFFSET(base)`. A corrupt
descriptor with the u-area flag set but a zero LH base would read
the finalizer marker from low memory instead of rejecting the
inconsistent owner. This differs from `proc_uarea_release`, which
explicitly rejects a zero base as MM_ERR_INVAL.

The v1756 guard rejects a zero base before reading its finalizer
word. It takes the existing failed-owner result path so the shared
pending flag stays set, preserving quarantine and bounded retry
rather than declaring cleanup complete. No new storage or release
operation is introduced. Valid descriptors retain their original
control flow.

Isolated production PDP-6 assembly and `git diff --check` PASS.
The external testkit sweep remains untouched. K04 still requires
booted multiple-owner retry, clock-wrap, WAIT/slot reuse, and
permanent-refusal verification before closure.
