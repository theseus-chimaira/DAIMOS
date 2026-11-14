# KCORE v1568: physical DTVOL2 DATA partial-write cold boot

Date: 2026-11-14. **Real PDP-6 Type-551 physical test PASS.**
No production code, ABI or fixed RAM changed; DTVOL2 remains RO.

The full 256K KCC PDP-6 guest used an independently stamped DTVOL2
Type-551 DTC3 tape and unmodified production v1551-v1554 first-write
intent, MM ownership, readback and physical DATA/DIR write routing.
The production v1555 RW refusal was checked first. Only an isolated
source-matched test image permitted DTC3/mount2 RW *after* the real
provider published the complete 72-bit identity and mounted owner.
The production DTFS and VFS source remains unchanged.

An actual guest created a two-word native file, wrote its initial
data and directory, and the simulator DETACH persisted the tape.
An independent complete-image oracle verified RBN0 was valid DIRTY,
RBN0144 directory and RBN0145 DATA had changed, and all other
physical blocks remained untouched. The real first-write barrier
therefore preceded DATA/DIR mutation.

The diagnostic Type-551 `PARTIALWRITE=0145:1` then interrupted a
second genuine DATA write after one changed 36-bit word. A second
DETACH plus byte-for-byte full-tape comparison proved precisely
one new word reached RBN0145, the other requested word stayed old,
and the entire on-media DIRTY RBN0 stayed valid and unchanged.
The guest retained poisoned ownership and refused stale FSINFO,
cwd and unmount operations without affecting TSFS root.

The exact persisted damaged tape was attached to a **second cold
boot of unchanged production KCORE/MRES**. Both RO and RW mounts
were refused, no public DTFS slot was published, and complete tape
SHA-256 was unchanged. The test does not assert old-data recovery.

See `daimos-testkit/tests/validation/daimos-dtfs-v2-partial-data-v1568.md`
for exact physical checks and reproduction.

**P0-B OPEN:** physically partial V2 directory updates, same/replaced
reel return after real user-data RW, and Type-551 operator-controlled
media exchange across transfers still need a defensible contract
and physical acceptance. The Type-551 lacks an atomic physical
reel-generation interlock; a software-only identity reread cannot
guarantee uninterrupted media ownership. Production DTVOL2 stays
read-only. P0-A closed, P1-C/D/E unchanged, VERSION 0.5, 32K HOLD.
