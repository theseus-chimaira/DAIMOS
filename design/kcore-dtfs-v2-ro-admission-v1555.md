# KCORE P0-B v1555: DTVOL2/CLEAN read-only production mount admission

Date: 2026-11-14. First real production DTFS MRES cutover for the
durable-volume format. **P0-B OPEN: writable V2 remains disabled.**

## Production implementation

The v1550-v1554 standalone native parser, durable write-intent
services and pinned MM helpers now have a deliberately narrow
production entry point: a verified DTVOL2/CLEAN medium can be
mounted **read-only**, never writable. This is an actual change
to the production `dtfs_mount_unit` routine in
`system/kernel/fs/dtfs_runtime.s`, not an isolated helper
or offline image-model-only stage.

The existing mount code continues to read the native directory
RBN0144 directly, bypassing clean BCACHE identities and checking
all reserved ownership/magic invariants. It then physically reads
RBN0. If the header magic is DTVOL2, it first requires the
explicit VFS_MOUNT_RDONLY bit from the mount flags. The actual
`dtfs_intent_validate_v1550` parser validates all 128 words and
must return exactly CLEAN=0; DIRTY=1 or INVALID=-1 refuses.
Only then does the mount form a media word containing the spare
`DTFS_MEDIA_FORMAT_V2_V1553` bit. Both already-checked random
36-bit ID halves are saved into the same existing MM-backed
per-mount shape/extent identity slots before returning success.
The VFS publisher receives the requested RO bit. An unreadable,
damaged, dirty, or writable V2 request never publishes a root,
changes its identity owner or reaches a DATA/DIR write.

For a DTVOL1 stamp or zero legacy reserved block, admission
continues through the original `dtfs_identity_validate_v1522`
source, preserving existing legacy policy and all caller tests.
The old parser remains strictly V1/legacy; a corrupt V2 record
cannot downgrade to V1 or all-zero identity.

`dtfs_dtc_write` now calls a zero-allocation per-physical-unit
V2 read-only guard before any cache eviction or raw Type-551
WRITE. The guard scans *published* VFS DTFS roots alongside
the MM-backed per-mount media table and refuses a write when
ANY live V2 mount references the target physical unit, including
an internal directory, chain/resize or cache-related caller
that may not have passed through VFS readonly enforcement.
A different physical unit and legacy-only mounts retain their
old write semantics. A guard refusal does not fake an I/O fault
or release/poison ownership: the controller was never invoked.
Ordinary physically uncertain V1 writes still use the original
WRITE_UNCERTAIN marking. The raw v1551 intent writer remains
outside this production module; it cannot recurse through
or accidentally bypass the V2 guard during this stage.

The userspace `dtfs_probe.c` now recognizes exactly DTVOL2/CLEAN
including both identity halves, both checked complement words
and 122 zero padding words. It refuses DIRTY, torn/unknown state
or malformed V2, and continues to probe legacy V1 and all-zero.
Probe classification alone never grants a mounted identity or
write permission; KCORE re-reads RBN0 and checks RO independently.
No target MKFS behavior changes: it still clears old RBN0 before
publishing new directory contents on explicit reformat.

## Build and compatibility

`system/kernel/Makefile` generates its `dtfs_runtime.s` by
concatenating the production DTFS runtime and unchanged v1550
assembly validator, so the parser becomes part of the same
optional MRES, with no extra registration or resident KCORE
interface. `system/boot/pdp6/Makefile` tracks the new validator
source as a rebuild prerequisite. The real KCC **full-profile
DTFS MRES package** successfully assembled, folded and linked
against the generated KCORE map. No new permanent KCORE data,
syscall ABI, media blocks or default formatter choice is added;
optional DTFS MRES code grows for the feature.

Native PDP-6 testkit `daimos-dtfs-v2-ro-admission-v1555.sh`
executes the real mount source with genuine host-generated V2
and V1 physical DECtape image bytes. **15 cases PASS; 9 unsafe
source mutations rejected.** It proves correct read-only
publication, DIRTY/RW/corrupt rejection, preservation of both
72-bit mount-ID halves and old V1/legacy read-write handling.

Native PDP-6 `daimos-dtfs-v2-ro-write-guard-v1555.sh`
executes the actual common write veneer. **12 cases PASS; 7
unsafe source mutations rejected.** Even a separate legacy
DTFS mount on the same physical unit cannot bypass a published
V2 read-only guard. Raw write and pending DATA buffer checks
are verified at the controller boundary.

The C89 `daimos-dtfs-v2-probe-v1555.sh` runs actual probe source
with 12 format/error outcomes PASS. Full prior v1519 direct
mount, v1520 identity/target MKFS and v1522 read/write suspect
regression suites PASS, including all 7+5+8 native mutation
checks; those older test oracles remain under daimos-testkit.
Details: `daimos-testkit/tests/validation/
daimos-dtfs-v2-readonly-admission-v1555.md`.

## Still open for P0-B

This is deliberately a **READ-ONLY production integration**.
The per-mount first-write gate and subsequent DIRTY revalidation
from v1551-v1554 are still unlinked from the ordinary mounted
write path. V2 read-fault recovery is conservatively refused by
the old V1-only identity-revalidation leaf; no writable or
recoverable CLEAN transition is inferred from it. A physical
guest boot with V2 mounted read-only has not yet been accepted,
despite the successful optional MRES build and native admission.

The next coherent writable-feature cutover must install a
provider-retained MM scratch handle, strict serialized volume
identity-to-unit association, a **mandatory** verified DIRTY
barrier for all DATA/DIR mutations including chain/resize,
and an exclusive unmount phase that blocks new owners across
sleeping CLEAN RBN0 seal/write/readback. No path may allow
read-only V2 to become writable by remount or a later provider
call. Full booted Type-551 partial-media, reinsertion/replacement,
old-vnode denial and cold reboot acceptance are mandatory.

The physical tape-exchange interval between an RBN0 recheck
and subsequent DATA transfer is unresolved. A provider lease
cannot mechanically prevent an operator or controller media
event from changing the volume between two transfers; full
P0-B acceptance needs a proved media-generation/fail-stop
contract, not a guessed assumption.

P0-A complete; **P0-B OPEN**; P1-C separate. VERSION 0.5,
strict 32K and optimization HOLD unchanged.
