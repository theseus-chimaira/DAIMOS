# KCORE P0-B v1554: physical DIRTY revalidation for already-armed DTFS

Date: 2026-11-14. Staged native PDP-6 source acceptance, NOT yet
linked into the live DTFS provider. P0-B remains OPEN.

## Discovered issue and correct durability rule

v1553 authenticated the first WRITE of a DTVOL2 mount, then set an
in-RAM `INTENT_ARMED` bit after a physical RBN0 WRITE_INTENT commit.
Its already-armed branch could return success without a further
physical transfer. On removable Type-551 DECtape this leaves a
subsequent DATA/DIR write vulnerable to a changed physical medium:
an apparently current mount word and 72-bit identity in RAM do
not prove that the inserted tape still carries a durable DIRTY
intent record. An older discarded volume can also retain its
original readable directory and volume ID.

The v1554 rule is stronger: **every subsequent mounted DATA/DIR
write must freshly read RBN0 physically, validate complete
DTVOL2/WRITE_INTENT, and match both original 36-bit identity
halves before any filesystem data transfer**. The volatile ARMED
bit selects the correct verification operation; it is never
accepted by itself as evidence of physical media integrity.

## Production source additions and changes

`system/kernel/fs/dtfs_intent_verify_v1554.s` provides a raw
read-only PDP-6 leaf. AC1 is the actual physical DTC unit,
AC2/AC3 the previously admitted original 72-bit ID, and AC4
an independently owned and pinned 0200-word scratch buffer.
It clears that buffer *before* physical RBN0 READ, bypasses
BCACHE, calls the unchanged v1550 six-word validator, requires
the exact DIRTY state, verifies both identity halves, and only
then returns zero. It never attempts to rewrite the identity,
DIRTY marker or a DATA block. The source neither adds persistent
state nor touches the caller's pending `fs_block_workspace`.

If the read fails, the header is CLEAN/old/unknown/torn, either
ID half differs, or padding is nonzero, the leaf invalidates
BCACHE and sets sticky `WRITE_UNCERTAIN` for mounted DTFS on
that physical unit. A mere READ_SUSPECT marker, which the
existing identity-only recovery check can clear, is not safe
for a missing previously verified WRITE_INTENT state.
Invalid zero expected IDs or a null scratch pointer refuse
before physical I/O.

The existing versioned MM wrapper in
`system/kernel/fs/dtfs_intent_owned_arm_v1552.s` now exports
`dtfs_intent_owned_verify_v1554`. Both the original ARM entry
and this new VERIFY entry use the *same* no-reclaim 0200-word
MM_TYPE_KERNEL_DYNAMIC, owner 015 octal, with an MM pin across
all Type-551 I/O, exact unpin/free after completion, and
caller-retained ownership on refused cleanup. There is no
second allocator, permanent scratch slab or duplicated pin
lifetime. The original ARM ABI is preserved.

`system/kernel/fs/dtfs_intent_first_write_v1553.s` now takes
the already-armed branch through this owned physical VERIFY
entry. Both first and subsequent writes perform post-I/O
checks of the exact original media word, both still-saved ID
halves and the retained MM scratch base. Even a successful
physical verification is refused if these change or the MM
buffer was not released while the provider request slept.
Every successful state transition still preserves the existing
18-bit UID/GID mount owner and physical unit/personality tags.

All three entry points remain deliberately **UNLINKED** from
ordinary DTFS provider operation. No normal DTVOL1 mount, live
write syscall, host default format or persistent filesystem
has been modified. No fixed KCORE data/BSS words or syscall ABI
are added by v1554. The optional provider code gains a read
check per write if integrated; this is a correctness feature,
not an optimization or performance claim.

## Executed native acceptance

The new testkit `daimos-dtfs-physical-verify-v1554.sh` links
real native v1554 verifier and v1550 record parser with decoded
actual opt-in host `mkdtfs` physical image bytes and a controlled
Type-551 interface. **17 PDP-6 cases PASS, ten unsafe source
mutants rejected.** These detect false read success with zero
actual bytes, a torn physical prefix, stale scratch, CLEAN
media, torn or unknown seals, changed high/low ID, wrong DTC
unit/RBN and missing sticky poison or BCACHE invalidation.
The code under test is the actual production assembly, not a
reimplementation of the parser in Python.

The new `daimos-dtfs-armed-gate-v1554.sh` exercises the actual
mounted gate source with a strict lower physical/MM service
oracle. **23 PDP-6 cases PASS, eight new unsafe source mutants
rejected.** It specifically verifies that physical readback
refusal, a changed media word, an altered saved high/low ID and
an unresolved MM allocation after I/O cannot authorize writing.

The new `daimos-dtfs-verify-real-mm-v1554.sh` executes the actual
production PDP-6 allocator, extent tracking, MM pin/unpin/free,
and the revised shared MM wrapper for physical VERIFY. It
successfully allocates/pins/frees twice and then refuses a real
no-arena allocation without I/O. **PASS, four unsafe mutations
rejected**, including selecting ARM instead of VERIFY, missing
the physical MM pin, and skipping unpin or free. Only the
controller operation is simulated in this real-MM suite.

The v1552 strict-MM source suite still passes 11 cases and ten
mutants, and the v1553 18-case/13-mutant gate regression still
passes with the new VERIFY boundary supported. All historical
test updates remain inside daimos-testkit.

Detailed methods: `daimos-testkit/tests/validation/
daimos-dtfs-armed-revalidation-v1554.md`.

## Remaining P0-B safety gates

Neither these native tests nor the separate controlled raw
I/O oracle constitute a booted 256K PDP-6 Type-551 guest.
No DTVOL2/CLEAN mount admission, mandatory direct/write routing
through the gate, per-mounted-volume retained MM scratch handle,
or exclusive owner-safe final CLEAN/unmount has been enabled.

A physical read followed by a separate data WRITE cannot by
itself make manual tape exchange mechanically atomic. The
provider lease excludes other filesystem operations, NOT the
operator or a transient controller media-change event. The
full integration must either hold a real media-generation
contract across both transfers or provide fail-stop behavior
on a detected exchange; this residual TOCTOU boundary must
be explicitly exercised, not silently assumed away.

Next priority is to coordinate versioned physical DTVOL2 mount
admission with an MM-retained scratch owner and mandatory
DATA/DIR gate on every mutation path (including resize and
chain writes), then exclusive CLEAN-unmount quiescence, followed
by the actual Type-551 partial media, reinsertion and cold boot
tests with independent FILE/cwd/PIPE/swap owners.

P0-A complete; **P0-B OPEN**; P1-C separate. VERSION 0.5;
strict 32K and optimization HOLD.
