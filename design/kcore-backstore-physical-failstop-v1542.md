# KCORE P0-A v1542: full physical BACKSTORE fail-stop accepted

Date: 2026-11-13. Feature completion only, not optimization.
Follows the v1541 selected-root-equivalent BACKSTORE service policy.

## Actual full KCC 256K PDP-6 result

The current v1541 fixed KCORE runs on a real complete SIMH PDP-6
KCC disk-root guest. The testkit reuses v1511's Type-236 PI2
error-completion instrumentation in a disposable guest copy, armed
only for a valid backing WRITE after BACKSTORE is enabled and has
at least one allocated owner. The actual Type-236 request, descriptor,
PI2 completion and KCORE service return are exercised. The transfer
error is not injected by returning -1 from a software test stub.

The simulator halted at **octal 003747**, precisely seven instructions
after linked fixed v1515 fatal entry 003740. This is the distinct
BACKSTORE-specific HALT, not the root HALT at 003743 or an unrelated
PC-zero user stop. The physical post-HALT linked-word examinations
show the selected Type-236 error counter = 1, BACKSTORE enabled = 1
and backstore_blocks_used = 0320 octal. Four pre-fault sampled
snapshots had zero injected PI2 errors. The previous v1511 expectation
of a subsequent scheduler retry is intentionally superseded by v1541.

The independent testkit v1542 verifier accepts that actual evidence
and rejects nine corrupted source/evidence mutations (wrong or
duplicated HALT, fake controller result, missing owner, altered gate
or PI2 driver, false pre-fault baseline). Original artifact SHA-256s
and exact commands are recorded in
daimos-testkit/tests/validation/daimos-backstore-physical-failstop-v1542.md.

## What this closes, and what remains

BACKSTORE fail-stop after a required exhausted Type-236 transfer is
physically accepted. The shared fixed gateway and source-linked native
tests separately cover DSK270/DRM236 DEVICE and mapped BLOCKSET VOLUME,
both reads and writes, missing mapper, and disabled-but-owned behavior
(17 cases, six unsafe source mutations PASS v1541).

This is a test-injected real PI2 error completion, not a demonstration
of physically permanently removed drum hardware. A failing final
BACKSTORE transfer is intentionally sufficient; there is no new
permanent-media-status detector. Successful bounded controller retries
remain normal. A final failed active/owned backing transfer must never
resume a swapped process or free its backing allocation.

Remaining P0-A full-guest profiles are alternate DRM-root, negative
unrelated DSK/DRM media loss, and shared DEVICE/VOLUME root-and-
BACKSTORE faults as actual hardware configurations permit. P0-B,
P1-C, P1-D and P1-E remain open separately. VERSION 0.5 and 32K/
general optimization gates remain HOLD; no production code was
added in v1542 beyond v1541's already committed policy.
