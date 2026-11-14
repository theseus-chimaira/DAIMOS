# KCORE P0-B v1558: eliminate stale and false-successful physical RBN admissions

Date: 2026-11-14. Production DTFS MRES safety correction and native
source-linked acceptance. **P0-B is still OPEN; no writable V2 cutover.**

## Source-sweep discovery

After v1557 established unique physical Type-551 mount ownership
whenever DTVOL2 is involved, the remaining v1551-v1554 first-write
service still required provider-retained MM scratch, mandatory
DATA/DIR routing and exclusive CLEAN/unmount. Reviewing the
controller path revealed a more immediate physical-admission flaw:
`dtfs_mount_unit` and `dtfs_verify_replacement_v1531` previously
invalidated BCACHE, but did **not** initialize the entire shared
0200-word transfer workspace before a raw RBN0144 or RBN0 read.
The standalone v1551 physical DIRTY-arm leaf also failed to
initialize its independent pinned MM scratch before its initial
RBN0 read (though it cleared it before the final readback).

A Type-551 raw transfer can be incomplete even if its callback
reports success. A stale complete RBN0144 directory or a valid
DTVOL2/CLEAN header left in RAM can then satisfy structural
checks, incorrectly authorizing a mount, replacement or on-media
DIRTY write without receiving a complete new physical record.

**Important legacy exception:** an intentionally all-zero RBN0
is a valid older DTFS volume. Zeroing the workspace before
mount-time RBN0 is *not* fail-closed: a false-successful transfer
with no bytes looks exactly like a full authentic zero header.
The strict six-word V2 parser and checksum cannot fix that
because zero legacy admission is a separate explicit policy.

## Implemented production rules

`system/kernel/fs/dtfs_runtime.s` now shares a single leaf
`dtfs_workspace_clear_v1558` / `dtfs_workspace_poison_v1558`.
The two entry points fill precisely the entire 128-word common
Type-551 workspace with zero or full 36-bit all-ones sentinel.
They consume only scratch AC4..AC6, allocate no memory, issue
no I/O and preserve the live mount's private-stack registers.

Initial native mount and admin poisoned-volume replacement both
wipe the workspace **before** the independent raw directory
RBN0144 read, not merely before parsing it. Their RBN0 raw
reads instead use the **full nonzero sentinel**, so any missing
identity bytes survive into the structural validator and are
refused. A genuine full zero legacy header overwrites all 128
sentinels, preserving old-format mount compatibility. The
subsequent strict V1/V2 validation and exact saved 72-bit-ID
checks are not weakened; an incomplete nonzero stamped header
is refused by its checksum or padding checks. No read result
can be accepted solely because the previous BCACHE tag was
invalidated without clearing its data.

`system/kernel/fs/dtfs_intent_arm_v1551.s` now invokes its
single exact-length scratch wipe before BOTH the initial raw
RBN0 read and the final independent dirty-readback. The first
raw read cannot accept an old MM scratch buffer containing a
valid CLEAN header if the physical controller transfers no
bytes or only the header prefix. A physical marker write is
still forbidden until the exact fully read CLEAN record has
been validated against both independently saved ID halves.

The standalone arm continues to be UNLINKED from live writing,
as it must until provider MM scratch retention and final
CLEAN/unmount ownership quiescence are implemented. The mounted
DTFS V2 read-only guard and physical-unit exclusivity are
unchanged. No KCORE BSS/data, MM allocation, public syscall
ABI or default on-media format is changed.

## Actual PDP-6 acceptance

Testkit `daimos-dtfs-intent-arm-v1551.sh` now seeds the independent
scratch with genuine old V2/CLEAN bytes, exercises zero-byte/
three-word false-success initial reads and a false-successful
zero-byte final readback: **17 native cases PASS, ten unsafe
mutations rejected**. Those mutations specifically include
skipping the initial and final scratch clears.

The actual native production `dtfs_mount_unit` test using
host-generated V2/V1 physical DECtape records expands to
**28 cases PASS, 13 unsafe source mutations rejected**, including
no-byte/short-success RBN0144 and RBN0 transfers. The novel
critical mutation, initializing RBN0 with zeros instead of the
sentinel, is rejected because it would falsely admit an empty
raw read as healthy legacy DTFS. Correct genuine all-zero legacy
mount remains accepted, as do strict DTVOL2/CLEAN READ-ONLY
and physically unique unit ownership.

New `daimos-dtfs-replace-raw-preflight-v1558.sh` links the
actual PDP-6 production `dtfs_verify_replacement_v1531`, shared
fill leaf and native V1 parser with independently formatted
physical V1/V2 RBN0 and native RBN0144 bytes. **20 scenarios
PASS, six unsafe source mutations rejected**, including stale
directory false-success admission, unchanged original volume
and malformed, missing, old or wrong-format identity.

Existing direct-mount v1519, V1 ID/target-MKFS v1520, and the
actual complete **KCC full-profile DTFS MRES assembly/fold/link
PASS**. Existing v1555 central V2 write guard and v1556 mounted
recovery remain present; writable V2 has not been enabled.

Source-linked test methodology:
`daimos-testkit/tests/validation/daimos-dtfs-raw-read-sentinel-v1558.md`.

## Next P0-B work

This fixes a concrete physical-admission correctness issue; it
is **not** a completed durable write transaction or a physical
guest/controller test. The provider still needs a failure-retaining
MM scratch owner, v1551-v1554 DIRTY verification on every
DATA/DIR mutation under the serialized provider lease, owner-safe
exclusive final CLEAN/unmount across sleeping physical I/O,
and full-guest partial-write, reinsertion and cold-boot proof.

Manual removal or exchange between physical RBN0 verification
and the later DATA write still requires an explicit hardware
media-generation or fail-stop contract. V2 remains READ-ONLY.

P0-A complete; **P0-B OPEN**; P1-C separate. VERSION 0.5,
strict 32K and optimization HOLD.
