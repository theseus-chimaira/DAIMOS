# KCORE P0-B v1562: transient mounted-DTFS V2 unmount quiescence

Date: 2026-11-14. Source-linked production implementation and native
PDP-6 acceptance. **This is not an on-media CLEAN seal. P0-B remains OPEN.**

## Problem

The existing `vfs_unmount` checked FILE/cwd, PIPE and swapped backing
owners, then performed sleeping SYNC/PREPARE, followed by a final no-I/O
owner check and mount retirement. A future DTVOL2 CLEAN marker must be
physically written and read back *after* that final owner check, and those
operations can sleep. New owners could appear in that interval. Writing
CLEAN without preventing further accesses would certify media that is
still being mutated. Provider serialization alone does not prevent a
process from publishing an FD/cwd after a provider call returned, nor
does it prevent physical tape exchange.

## Implemented prerequisite

Production `vfs.s` acquires a per-mount transient quiescence bit,
`0020000` in the otherwise spare DTFS media right-half, after its first
owner scan and **before** sleeping SYNC. The bit reuses the boot-MM-backed
mount media word, with no additional fixed DATA/BSS or mount slab word.
Acquisition requires an exact published V2 root/public mount ID, no
existing quiescence or sticky media-failure bit and no retained MM intent
scratch. Non-DTFS and legacy DTVOL1 operations retain their old path.

`vfs_dtfs_owner_admit_v1562` rejects new FILE/cwd publication to a
quiescing V2 mount and also rejects a stale DTFS vnode referring to an
unpublished, retired or repurposed public mount slot. It accepts both
raw vnodes and packed FILE descriptors, stripping the same descriptor
metadata bits as `file_node_from_fd`. Real `file_new_fd` and `file_chdir`
call the guard immediately before their non-sleeping stores.

The DTFS MRES dispatcher guards normal vnode operations before provider
execution; internal SYNC is allowed under the barrier and raw MOUNT_UNIT
has no mount vnode yet. The mounted V2 physical write bridge additionally
rejects the quiescence bit independently of any future RW cutover.
Provider calls continue using the existing serialized lease.

On VFS success, after the second all-process owner check and the v1560
retained-MM check, the barrier is released without yielding and the
mount is retired immediately. On failed SYNC or owner check, only a
barrier acquired by that same VFS transaction is released, with the
original published root authenticated. Refused acquisition never clears
another caller's barrier. **No physical CLEAN is attempted by v1562**;
therefore this abort release does not falsely recover a partially written
seal. Future CLEAN write failures must retain uncertainty and not use an
unconditional abort path.

## Acceptance

- New testkit `daimos-dtfs-quiesce-v1562.sh`: 41 source-linked native
  cases for V2 and legacy admission, packed FILE metadata, reused mount
  IDs, exact MM scratch, begin/end, preserved media flags and fail-closed
  boundaries. Seven unsafe actual-source mutations rejected.
- New testkit `daimos-dtfs-quiesce-unmount-v1562.sh`: 12 full actual
  `vfs_unmount` native scenarios including a modeled foreign OPEN attempt
  during sleeping SYNC, failed SYNC, pending media error, existing owner,
  legacy V1 semantics, unique owner and successful retirement. Three
  unmount source mutations rejected. Only provider I/O is simulated.
- Historical v1514 non-root unmount, v1523 all-process FILE/cwd,
  v1560 provider-private MM and retirement, v1559 write-path coverage,
  v1561 intent/veneer, v1555 V2 RO admission and v1556 recovery suites
  pass. Earlier fixtures were adjusted for the extra v1562 stack-private
  flag and helper symbols, without weakening their original assertions.
- Full 256K-profile KCC KCORE and optional DTFS MRES assemble/fold/link
  and image generation pass. The current KCORE placement is 152 words
  larger than the v1561 map (the cost is fixed TEXT, not a new table);
  the `permanent-size` 32K placement gate remains intentionally on HOLD.

## What remains before physical CLEAN/writable V2

1. Prove an exclusive barrier against *all* new FILE/cwd, PIPE,
   swapped-owner and direct provider admissions, including inheritance,
   duplication and in-flight pre-barrier operations. Test actual
   asynchronous process scheduling, not only the modeled SYNC callback.
2. Implement a final provider-serialized, owner-checked physical
   DTVOL2/DIRTY -> CLEAN RBN0 seal with independent pinned MM scratch,
   physical readback, exact 72-bit ID and media-generation verification.
   No failed/torn CLEAN may retire the mount or clear uncertainty.
3. Establish a physical Type-551 tape-generation/quiescence contract
   covering exchange between RBN0 verification and DATA/DIR transfer.
4. Only then consider removing the v1555 V2 RO veto; run actual Type-551
   partially written DATA, original dirty reinsertion, different-volume
   replacement and second cold-boot physical guest acceptance.

P0-A complete; P0-B OPEN; P1-C/D/E unchanged. VERSION 0.5.
No general optimization or 32K admission was undertaken.
