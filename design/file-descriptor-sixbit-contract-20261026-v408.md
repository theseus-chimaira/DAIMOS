# Proposal B v408: six-bit FILE candidate and lock-contract decision

Status: DESIGN ONLY; requires explicit approval before production edits.

## Bit-exact 63-mount descriptor candidate

Assuming providers and local kinds remain restricted to 0..7, the
36-bit descriptor first word has canonical vnode bits 0..20,24..32
and metadata bits 21..23,33..35. The offset second word remains an
independent, complete 36-bit value. Mount IDs 0..63 fit directly.

    bit 35  READ                 octal 400000000000
    bit 34  WRITE                octal 200000000000
    bit 33  DIR or APPEND        octal 100000000000
    bit 23  RESERVED            octal 000040000000
    bit 22  REGULAR/LOCK high    octal 000020000000
    bit 21  REGULAR/LOCK low     octal 000010000000

IMPORTANT: this table uses absolute 36-bit bit indices; do not use the
historical FILE_META_* constants without verifying bit positions.
The two-bit state is 0=nonregular, 1=regular unlocked,
2=regular shared, 3=regular exclusive, as in the old state machine.
DIR/APPEND disambiguation uses nonregular versus regular state.
For nonregular nodes the bit identifies a directory. For regular
nodes it identifies APPEND. The reserved bit remains zero.

This format requires removal of the existing four-bit family token.
Removing it is NOT equivalent to existing FLOCK semantics, so this is
not an implementation-ready encoding.

## Candidate replacement per-process FLOCK contract

The proposed contract is vnode-wide within ONE process: all live FILE
descriptors naming the same complete canonical vnode share the same
lock state, regardless of which OPEN created them. FLOCK SHARED,
EXCLUSIVE and UNLOCK update every such descriptor; CLOSE releases only
that descriptor and the remaining descriptors retain the lock.
An independent OPEN to a vnode already locked within the process joins
the existing lock, so FLOCK no longer rejects an exclusive request
solely for differing OPEN-family tokens. RUN remaps and copies lock
state as part of the descriptor, but no cross-process conflict is
claimed. Close-all removes all local state. Unmount cleanup clears the
state of all descriptors belonging to that mount before retirement.

This intentionally changes existing independent-OPEN conflict behavior.
It also requires choosing an explicit rule when RUN maps only some
descriptors, and when OPEN creates a new descriptor for a vnode already
locally locked; both must preserve coherent per-vnode state. A definitive
rule for those transition points has not been approved.

## RAM and performance gate

No third descriptor word or allocation is necessary for the candidate;
the existing u-area word 067 could become available for reuse, but
the 0410-octal u-area and allocated memory MUST NOT be claimed reduced
unless the actual allocation boundary shrinks. Retiring extension map
decoding and copying offers possible linked-text and hot-path savings;
FLOCK will instead compare full masked canonical vnodes with no family
comparison. Check all producer and consumer code, especially RUN,
FIFO substitution, syscall fast paths, and provider 7 TSFS/PIPE.

The current generic VFS permits six-bit provider/local-kind, whereas
this FILE candidate permits only three; this is a separate required
interface restriction and an explicit future-expansion limitation.

## Required proof and tests

Before coding, obtain approval for per-process vnode-wide FLOCK, its
OPEN/RUN interaction and the narrower provider/kind contract. Add
tests in daimos-testkit for all 0..63 mount IDs, distinct same-local
vnode mounts, regular/directory/device/append flags, independent
offsets, CLOSE failure, DUP/DUP2, RUN, FIFO refcounts, FLOCK lifecycle,
and process/unmount cleanup. Measure fully linked PDP-6/PDP-10 images,
total resident and peak RAM and hot-path instructions before accepting.

Reference baseline only: v388 KCORE=9109, KCORE+MRES=15657,
PERMANENT_LAST=15704. No new link or runtime acceptance claimed.
