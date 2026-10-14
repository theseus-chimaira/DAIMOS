# Packed teardown regression expansion v085

`daimos-testkit/tests/system/packed-destroy-v085` exercises the already
committed v084 storage-aware destructor against actual packed owner/registry
and MM rollback implementations in both dense and sparse layouts.

It checks no destruction for invalid slots or absent TRANSITION, no early
release of executable backing after a pinned extent refusal, correct
retirement of the packed MM extent and backing after success, invalid storage
mode rejection, legacy DISK dispatch with no premature freeing of backing,
and safe packed slot reuse. The tests pass with the host MM simulator; the
v084 C dispatcher separately cross-compiles for PDP-6.

This adds regression coverage only, not another divergent implementation.
The active VM/scheduler remain unmodified. KINIT allocation, the eventual
whole-kernel link, and booted teardown still require acceptance; 8K size
optimization is deferred by request.
