# KCORE P0-B v1551: raw DTVOL2 dirty-before-data barrier

Date: 2026-11-13. Native PDP-6 service implemented and tested;
not installed in the production DTFS provider. P0-B remains OPEN.

## What was added

`system/kernel/fs/dtfs_intent_arm_v1551.s` is a standalone
assembly service with AC1=physical DTC unit, AC2/AC3=expected
nonzero 36-bit volume-ID halves, and AC4=independently allocated,
writable 128-word RBN0 scratch. It returns zero only on a fully
verified dirty on-media commit; all other outcomes return -1.
The caller MUST hold the DTFS provider's serialized lease,
retain the scratch MM owner through physical I/O, and have already
admitted a DTVOL2/CLEAN volume. The interface deliberately does
not offer a CLEAN operation or silently authorize a DIRTY mount.

On entry the service reads RBN0 directly through the raw Type-551
driver patch point, bypassing BCACHE and without overwriting
`fs_block_workspace` or `dtfs_dir`. It invokes the actual v1550
native 128-word record validator and compares **both** ID halves
with the saved mount identity. Only a valid CLEAN record proceeds.

The service changes RBN0 words 4 and 5 to the fixed WRITE_INTENT
state and its computed 36-bit seal. It invalidates all clean BCACHE
tags before invoking the raw physical RBN0 WRITE entry, bypassing
the ordinary `dtfs_dtc_write` veneer to prevent eventual recursive
arming. On reported success it erases the entire independent
scratch buffer, performs a fresh raw RBN0 READ, and verifies an
exact DTVOL2/DIRTY record with the original high/low identity,
checked seal and 122 zero padding words before returning success.

If the raw header WRITE was attempted and then failed, or the
readback/identity check failed, the service calls the existing
`dtfs_mark_unit_suspect_v1522` with `WRITE_UNCERTAIN` on the
physical DTC unit. No mount or FILE owner is freed and the
service never attempts to erase a partially written marker.
Unrecognized legacy, preexisting DIRTY, wrong ID, or an initial
read error is refused **before any on-media write**.

The helper uses four saved AC registers on the existing stack
and has no permanent DATA/BSS words. The existing v1522 media
poison leaf is now exported for the eventual optional MRES link;
its code and runtime behavior are unchanged. Neither the helper
nor the v1550 record validator is currently linked in the
production boot/MRES object list; regular DTVOL1 mounts and
write semantics remain unchanged.

## Native validation

`daimos-testkit/tests/validation/daimos-dtfs-intent-arm-v1551.sh`
feeds actual host `mkdtfs` DTVOL2 and DTVOL1 image records to
the unchanged, source-linked v1550 parser and v1551 arming code.
The only test-controlled boundaries are physical Type-551
read/write completion, the BCACHE eviction leaf, and existing
DTFS volatile WRITE_UNCERTAIN marking. Fourteen genuine PDP-6
CPU scenarios PASS. Eight actual-source unsafe mutations fail:
clean admission bypass, high/low identity bypass, dirty readback
bypass, poison bypass, BCACHE invalidation bypass, physical readback
bypass, and ignored WRITE errors. The test checks exact raw
read/write/poison/eviction counts and preserved pending DATA buffer.

The test exposed and corrected its own MACRO-10 fixture encoding
and wrong-low-ID oracle coverage; none required changing the
native production service. Details and reproduction command:
`daimos-testkit/tests/validation/daimos-dtfs-intent-arm-v1551.md`.

## Not yet implemented

Native test-controlled raw I/O is **not** a full physical Type-551
guest. The currently mounted provider has no automatic MM scratch
lease, no DTVOL2/CLEAN mount admission, no per-mount armed state
or dirty-before-every-first-data-write gate, and no final CLEAN
seal under exclusive owner-safe unmount. None of those are
implicitly supplied by the standalone helper.

Next: integrate the helper into a versioned DTFS MRES transaction
with an optional, reclaimable MM-owned 128-word scratch and clear
allocation/failure paths; prevent data writes until an actual
verified intent; add the CLEAN transition only after final owner
checks and no possible concurrent new opener. Then repeat v1529
physical partial-media, old-media reinsertion and cold boot with
actual independent healthy owners. P0-B OPEN, P0-A COMPLETE.
VERSION 0.5, strict 32K/optimization HOLD.
