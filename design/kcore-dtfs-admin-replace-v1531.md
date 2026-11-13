# KCORE v1531: owner-checked destructive DTFS replacement

Date: 2026-11-13. Scope: P0-B administrative completion
for noncritical physical DTFS WRITE_UNCERTAIN;
not general filesystem repair or code optimization.

## Operational safety model

The v1529 physical Type-551 test proved a real native
DECtape block containing simultaneously an old and a
new data word after an incomplete WRITE. The existing
per-mounted-unit WRITE_UNCERTAIN bit, octal 0400000,
correctly makes cached files, FSINFO, ordinary FILE
CLOSE (which calls SYNC), and ordinary UNMOUNT fail.
The original FILE and VFS slot therefore remain owned.

This state must not be cleared by reading a matching
physical DTVOL1 ID. A same-ID tape can contain damaged
directory or file data, and normal unmount cannot
silently abandon another process's descriptor.

v1531 adds **privileged discard/replacement**, not
in-place repair. Userspace invokes extension UUO 077,
`SYS_EXT_DTFS_RECOVER_V1531=054`, with AC2 action
and AC3 descriptor or zero-based mount slot:

* `SYS_DTFS_DROP_FD_V1531=1`: root explicitly
  drops exactly one current-process FILE descriptor
  only if its live vnode belongs to a currently
  published noncritical native DTFS mount already
  marked WRITE_UNCERTAIN. Both descriptor words
  are cleared. It performs no SYNC and cannot
  release a PIPE, device or healthy FILE. Other
  processes' descriptors are untouched.
* `SYS_DTFS_REPLACE_SLOT_V1531=2`: root requests
  retirement of an owner-free poisoned **non-root**
  DTFS slot only after real raw physical checks
  establish that a distinct, structurally valid
  native tape has actually replaced the damaged
  medium on its original DTC unit.

The old volume is discarded, not repaired. Any
required salvage is the administrator's separate
responsibility, ideally with the old tape retained
offline. A replacement remains a new filesystem.

## Fixed KCORE ownership and retirement

`native_sys_dtfs_recover_v1531` enforces
`file_check_root` before dispatch. The mount
guard in `vfs_dtfs_poisoned_slot_v1531`
verifies an occupied MM slot, matching vnode
mount ID, DTFS provider 5, native personality,
WRITE_UNCERTAIN, initialized media/identity
table bases, exclusion of the selected root,
and exclusion of both critical VFS owner tags.
Missing/healthy/read-suspect/foreign/mismatched
slots are denied before side effects.

`vfs_dtfs_discard_slot_v1531` invokes the
existing `vfs_unmount_owner_check_v1523`.
The latter checks PIPE references, process
swap backing and every live stable-uarea
cwd and FILE descriptor across *all* processes,
including swapped ones without resident VM.
There is no stale vnode-ID remapping while an
owner remains. No forced closing or owner
teardown is performed.

The new DTFS MRES operation is dispatched
through the existing per-provider serialized
lease. It may sleep during physical I/O.
Immediately afterward the fixed KCORE
revalidates the mounted poisoned slot and
**all process owners again**, so a new FILE/cwd
opened while the verifier slept prevents
retirement. From that final check through
the commit there is no sleep or device I/O.
Only then does the existing
`vfs_mount_retire_slot` clear the target/root
and mount-policy bit. The per-slot DTFS media
word and two reused 72-bit identity words
are cleared as part of that same nonblocking
commit. The namespace root and unrelated
mounts remain unmodified.

## Optional DTFS MRES physical replacement validation

`dtfs_verify_replacement_v1531`, DTFS-private
operation octal 026 (22 decimal), verifies
the old published slot remains WRITE_UNCERTAIN
and has two nonzero saved identity halves.
It evicts the existing shared directory/BCACHE
tags without freeing MM data or mutating media.
It then performs **physical** Type-551 reads,
not hits from old cached block data:

1. Native RBN0144 directory, checking the
   same structural native magic and reserved
   owner-map positions as cold MOUNT_UNIT.
2. Native RBN0, using the existing complete
   128-word DTVOL1 marker, nonzero high/low
   identity and complement/padding validator.
3. Nonlegacy ID **different in either half**
   from the old saved full 72-bit identity.

No actual WRITE is issued. The MRES returns a
boolean only; it neither clears poison nor
unmounts the original slot. On any failure,
the old VFS/FILE ownership is retained.
A normal subsequent DTFS mount independently
revalidates its new tape.

This is **not durable quarantine of the discarded
original volume**. v1531 records no permanent
blacklist or repaired-data certificate. After
explicit discard, a later operator could
physically reinsert the old damaged tape,
and after a reboot the volatile WRITE_UNCERTAIN
marker is also lost. Its otherwise valid
RBN0 identity and directory can pass an
ordinary fresh MOUNT despite corrupted
file data. Administrators must keep the
old tape offline for salvage or disposal;
the v1531 API is not a persistent bad-volume
registry. If policy requires the OS to
refuse old damaged volumes after restart/
reinsertion, that is an **additional P0-B
durability/integrity feature gate**, still
OPEN and not silently solved here.

## Physical and native acceptance

The full 256K KCC PDP-6 TSFS-root PTR-boot
guest tests a real write failure on DTC3,
ordinary CLOSE refusal, explicit root
DROP_FD and residual cwd denial, same-ID
physical return rejection after all owners
are gone, and physically attaching a newly
stamped different DTC3. Only then does
REPLACE_SLOT retire the old mount, FSINFO
become empty, fresh DTFS mount succeed,
the abandoned filename disappear, and
normal UNMOUNT and independent root
access succeed. The guest and host PASS.

Independent native PDP-6 source-linked
tests verify the actual all-process owner
guard with a swapped stable-uarea FD15,
independent cwd, PIPE/swap exclusion and
a remote FILE race inserted during provider
verification. Four executed source mutants
are rejected, including bypassed post-I/O
owner recheck and physical verifier.

All older v1526-v1529 full physical guest
regressions PASS on the rebuilt kernel.
Test implementation and diagnostics:
`daimos-testkit/tests/validation/
daimos-dtfs-admin-replace-v1531.md`.

## Footprint and remaining scope

Only code has been added; no permanent data
words, owner tables, on-media format fields,
automated recovery heuristics or retry state.
The full KCC image was rebuilt after the final
native-personality guard and both DXR headers
were compared with the accepted v1527 baseline:

| Region | v1527 (octal) | v1531 (octal) | Delta (decimal words) |
| --- | ---: | ---: | ---: |
| Fixed KCORE image | 023116 | 023261 | +99 |
| Fixed KCORE BSS | 000415 | 000415 | 0 |
| Optional DTFS/TSFS MRES text | 003747 | 004057 | +72 |
| Optional DTFS/TSFS MRES BSS | 000002 | 000002 | 0 |

This is a **feature-size increase**, not an
optimization. Strict 32K placement remains
HOLD until feature completion.

This closes the **explicit destructive
replacement** scope for P0-B without claiming
in-place data repair, durable cross-reboot
quarantine or a physical multi-process
administrator test. P0-A permanent critical
root/BACKSTORE loss remains OPEN. P1-C mixed
storage errors, P1-D FSINFO inventory and
P1-E swapless PURE/MM/APR remain OPEN.
VERSION 0.5; strict 32K/optimization HOLD.
