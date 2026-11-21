# K04: pre-VM zero-base guard v1757

Status: source correctness fix, isolated PDP-6 DAS assembly PASS.
K04 remains OPEN pending booted fault-injection tests.

v1756 guarded a zero stable-u-area base in the VM_NONE deferred
finalizer path. The preceding non-NONE VM path already branched
away before dereferencing `PROC_FINALIZE_OFFSET(base)` if its
base was zero. However, that branch silently skipped the owner,
so a scan could clear the shared pending bit despite retaining an
inconsistent, unreleased descriptor.

v1757 changes the pre-VM path's zero-base branch from SKIP to the
existing failed-owner result. The reaper retains the shared pending
bit and its bounded retry interval, and does not release or republish
any resource. It does **not** correct a low-memory dereference in
that particular pre-VM path, since the old zero-base guard already
prevented that read. Valid PREVM and post-VM owners are unchanged.

This is a defensive consistency guard, not evidence that a normal
execution path can produce a zero-base descriptor. No testkit files
were touched and no physical runtime validation is claimed.
