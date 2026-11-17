# P1-E v1649: original executable guards during pinned FULL restore

Date: 2026-11-17. Native source-linked PDP-6 validation PASS.
Production KCORE/PURE/VM/MM/VFS/scheduler code and permanent RAM
are unchanged. This is NOT a complete two-process physical race.

## Architecture and serialization

An executable-backed swapless PURE process remains a PACKED
process while the original v1391 FULL restore reads and decodes
its executable. The FULL transaction holds exactly one pin on
both the authoritative RAW PACKED suffix and a separate complete
PROCESS/APR destination. The RAW suffix continues to own all
writable data until the bounded loader succeeds and overlays
it; resident publication is later.

The original v1523 VFS unmount preflight calls
`proc_swap_mount_busy` before v1562 can acquire a DTFS V2
quiescence bit. That scan is not followed by a scheduler yield
before acquiring the bit. Because FULL retains the executable
owner and saved backing while the source read is in progress,
a competing ordinary unmount should be refused at preflight,
not permitted to retire a live FULL source.

This ordering means that forcibly dropping a PACKED owner or
injecting an unmount bypass of the original guard would
manufacture a concurrency case that normal DAIMOS cannot admit.
The source code alone does not prove all scheduler interleavings.

## Native PDP-6 source-linked test

The testkit v1649 fixture runs the previously accepted v1395,
v1397 and v1648 fault/retry matrix, and then invokes the actual
original `proc_pure_ram_restore_v1391` transaction using an
exact bounded FULL loader callback. Its native callback checks:

* One PACKED_PROCESS source suffix at SOURCE+OMIT, owner SLOT,
  pin 1, saved executable backing, VM base zero and TRANSITION.
* A separate PROCESS full-APR destination at RESTORE, same
  owner, exact APR size and pin 1. No overlap or extra owner.
* A real DTFS provider-5/public-mount-1/file-index-012345
  root/source identity in the source-linked process fixture.
* Original assembler `proc_swap_mount_busy(1)` returns 1,
  and `proc_swap_backing_busy(exact vnode)` returns 1.
  Different mount and file index return 0.
* All MM descriptors, pins, swap backing and process
  TRANSITION state remain exactly unchanged by those scans.
* The original bounded loader then succeeds; FULL commits to
  one unpinned PROCESS extent with correct immutable prefix,
  authoritative original writable suffix, RESIDENT mode,
  cleared backing and cleared TRANSITION.

The scan code is extracted from the original
`proc_pure_backing_pdp6.s`, supplying only standalone
assembler constants. It is not replaced with a newly written
C predicate. The compiled v1391 and MM rebase/expand assembly
are original source-linked code. Two separately assembled
scanner mutants (inverted PURE text guard and inverted exact
backing identity compare) both execute and fail the native
`__test_exit` oracle, status 002002 octal. The original
v1563 owner/staging suite also passes 26 cases and rejects
its six independent mutations.

Testkit branch:
`test-kcore-p1e-full-mount-guard-v1649-20261117`;
commit `5b51d68`.
Entry:
`tests/validation/pure-full-mount-guard-v1649/run-v1649.sh`.

Evidence: `$TMPDIR/daimos-p1e-v1649-evidence-20261117.tar.gz`.
SHA256:
`bcd7a89817620412fc0c33fda0f5f6000896435d2a3a0f86e3133b9f461b2d78`.
The archive includes original production-compiled FULL
assembler, extracted guard, both executed unsafe scanner
mutants, final guest and mutation reports, linked native
fixture and a versioned SHAR with commit/push instructions.

## Scope and remaining gates

**Accepted:** the real PDP-6 source-linked executable and
mount guard predicates refuse the existing PURE dependency
at the actual two-pin FULL callback, preserving the complete
RAW owner and allowing normal restore without a permanent
KCORE/RAM increase.

**Open:** actual independent user task attempting UNMOUNT
under real PI6 scheduling while a FULL provider call remains
blocked, including clock proof and exact physical MM snapshots;
permanently ambiguous MM ownership; complete 32K/48K/64K
physical peak RAM; and separate KA10 VM/backend. Do not claim
full v1562 concurrency acceptance from this native test alone.
PURE default OFF, strict 32K optimization HOLD.
