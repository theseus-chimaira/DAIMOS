# K04: pre-VM zero-base guard v1757

Status: source correctness fix, isolated PDP-6 DAS assembly PASS.
K04 remains OPEN pending booted fault-injection tests.

v1756 guarded a zero stable-u-area base in the VM_NONE deferred
finalizer path. Source review found the preceding non-NONE VM path
also dereferenced `PROC_FINALIZE_OFFSET(base)` before reaching that
guard. A descriptor retaining UAREA and TRANSITION with base zero
could therefore still read the low-memory word at offset 067.

v1757 changes the pre-VM path's zero-base branch to the existing
failed-owner result. The reaper retains the shared pending bit and
its bounded retry interval, and does not release or republish any
resource. Valid PREVM and post-VM descriptors are unchanged.

This is a defensive consistency guard, not evidence that a normal
execution path can produce a zero-base descriptor. No testkit files
were touched and no physical runtime validation is claimed.
