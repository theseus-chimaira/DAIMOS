# KCORE P0-B v1561: production DTFS first-write/MM intent bridge

Date: 2026-11-14. **Production source-linked native feature accepted.**
P0-B physical write durability still **OPEN**; DTVOL2 stays READ-ONLY.

## Closed production-link and owner-routing gap

The production central `dtfs_dtc_write` veneer was already the
common path for native DTFS DATA and DIR writes. Its entry ABI
is AC1=physical Type-551 unit, AC2=block, AC3=pending buffer;
it has no mounted VFS vnode or zero-based mount ID. v1557
closed conflicting physical-unit mounts, and v1560 supplied a
one-word per-mount provider-private MM scratch basep in the
existing KINIT six-array VFS slab. The v1551-v1554 physical
DIRTY arm, already-armed verifier, MM pin/alloc/release and
authenticated first-write gate were previously **unlinked**.

v1561 links the existing unchanged physical/MM primitives
into actual optional DTFS MRES alongside the v1550 parser and
v1557 physical-mount exclusivity helper. The new
`dtfs_intent_mount_gate_v1561.s` resolves AC1's physical unit
to a unique published VFS DTFS mount. It refuses invalid unit,
null mount/identity tables, wrong public mount ID, duplicate
V2/V1 ownership of one physical unit and a nonzero retained
scratch extent. Legacy V1/V1 sharing and unrelated physical
units retain their preexisting behavior.

For an authenticated V2 unit the bridge checks that the VFS
slot is RW **before any physical RBN0 arm**, fetches the exact
`&vfs_mount_aux_base_v1560[slot]` word, and invokes the real
`dtfs_intent_first_write_v1553`. This checks native format,
suspect bits, exact saved 72-bit identity and MM ownership,
and either arms CLEAN->DIRTY via v1552/v1551 or physically
revalidates an existing DIRTY marker via v1552/v1554. Both
require physical RBN0 readback and an exact MM scratch release.

The lower helper uses registers AC10..AC16 internally and
restores them on return. It can sleep across DMA. After return,
the bridge verifies the same published root vnode, the expected
media word (with only INTENT_ARMED newly set), writable policy
and zero retained MM owner. A failed check refuses rather than
authorizing an ordinary DATA/DIR write. The provider's serialized
filesystem lease remains required throughout the operation.

The actual central `dtfs_dtc_write` now calls this bridge after
the existing v1555 mandatory V2 RO veto. It saves physical
UNIT, BLOCK and BUFFER on the process-private stack across the
bridge, restoring AC1..AC3 before BCACHE invalidation and raw
DATA/DIR transfer. On refusal, neither the pending buffer nor
the data block is written. RBN0 intent itself continues using
the raw DTC jump directly, avoiding recursion. The complete
bridge/physical/MM stack builds and links in the real KCC MRES.

## Why DTVOL2 is still read-only

The earlier `dtfs_v2_ro_write_guard_v1555` is intentionally
**unchanged** and always executes BEFORE the new bridge. Every
currently admitted DTVOL2/CLEAN mount is read-only. It rejects
V2 DATA/DIR writes before the bridge can allocate MM or mark
RBN0 DIRTY. The linked v1551-v1554 code therefore introduces
no new on-media V2 writes in a production full guest. Removing
that veto is NOT part of v1561 and remains unsafe without an
exclusive CLEAN/unmount protocol and physical media-change
contract. A separate native gate test uses hypothetical RW
V2 state solely to verify the binding and failure behavior.

## Native acceptance and memory

The actual `dtfs_intent_mount_gate_v1561.s` executes in the
PDP-6 simulator across **35 cases PASS**, and **13 unsafe
actual-source mutations are rejected**. Testkit stubs only the
already independently accepted lower v1553/MM/physical service,
with strict slot and basep ABI and controlled post-I/O changes.
The source-linked production v1555 central write-veneer suite
executes **12 native scenarios and rejects nine unsafe mutants**,
including two new AC2/AC3 stack ABI violations while the lower
bridge deliberately clobbers those registers. v1554 real first-
write/physical revalidation **23 cases/eight mutants PASS**;
v1552 production native MM allocator/pin/free PASS; v1559
write-path audit **11 known sites/five bypass mutants PASS**.
The complete KCC full-profile KCORE and DTFS MRES assemble,
fold and link PASS with all v1551-v1554 leaves now included.

No new fixed KCORE data word, mount slab word, MM allocation at
boot, public syscall ABI or on-media structure is introduced.
The optional DTFS MRES executable code grows to include the
previously standalone native physical/MM transaction helpers;
no code-size optimization was attempted (feature completion).

Detailed commands/results are in `daimos-testkit/tests/validation/
daimos-dtfs-intent-mount-v1561.md`.

## P0-B still open

Next is the physical CLEAN/unmount transaction with an exclusive
VFS owner-admission barrier spanning all sleeping device I/O,
verified original V2/DIRTY state and stable 72-bit ID, checked
physical CLEAN marker write/readback, and mount retirement only
after successful seal. Also required: full guest Type-551 partial
block writes, damaged original tape reinserted, distinct replacement,
second cold boot, and a physical tape-generation/quiescence contract
covering exchange between RBN0 verification and DATA/DIR transfer.

The test fixtures do not provide that hardware assurance. Do not
remove the existing v1555 V2 veto yet. P0-A complete, P0-B OPEN,
P1-C separate; VERSION 0.5, strict 32K/optimization HOLD.
