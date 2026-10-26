# Proposal B v402: canonical descriptor compression feasibility

Status: design-only; no production modification authorized.

## Observed semantic constraints

The full vnode and independent 36-bit offset fill both descriptor words.
The currently packed states are READ (1), WRITE (1), DIR/APPEND (1),
regular/shared/exclusive (2), and duplicated-descriptor lock family (4).
The lock-family token is copied in DUP/DUP2 and RUN; separate OPEN calls
of the same vnode do not necessarily share this token. Substituting vnode
equality for owner equality would alter locking behavior and is invalid.

The metadata used in READ/WRITE and SEEK is not universally derivable from
the vnode: two handles for one vnode may have different access modes and
APPEND flags. VFS STAT can identify filesystem type, not an open handle's
authorization or requested append mode. Repeated provider calls would also
penalize the common I/O path.

The credential word at u-area offset 066 contains UID9, GID9, UMASK9 and
the live PURE backing slot-extension bit 27. Its upper bits are not a
free 36-bit replacement map. The existing FD mount-extension map at 067
has 36 bits, of which 32 currently encode mount extension fields.

## Capacity bounds, not a proposed encoding

Retaining the present field model requires 9*16 = 144 metadata bits,
four entire 36-bit words. Replacing the single current extension word
with four words would cost +3 process words if no other slots are proven
reusable. Even deleting the 4-bit family token without replacing its
behavior leaves 5*16 = 80 bits, exceeding the current 36-bit word.

If a future encoding proves a smaller number of distinct valid handle
states, its *information-theoretic* minimum must be calculated separately
from the instruction cost of addressing packed fields. These counts are
not evidence that a zero-growth B1 encoding has been found.

## Alternative ownership designs to investigate, NOT accepted

* Sparse family objects shared by duplicates: can avoid four bits per FD,
  but requires lifetime tracking, allocation, failure rollback and RUN
  remapping. Account for every object and allocator descriptor in total RAM.
* Explicit lock ownership keyed by a distinct opened-handle identity:
  potentially permits simpler per-FD metadata but requires a bounded,
  collision-free identity and must not silently merge independent opens.
* Revised FLOCK semantics are permitted only after a separate decision and
  proof of necessary OS capabilities, with caller/test migration.
* Keeping the present packed descriptor remains a legitimate candidate if
  canonical encoding increases total RAM or slows hot paths.

## Decision gate

Before source changes, derive a complete lifecycle/ownership contract,
prove metadata capacity and preservation of all relevant READ/WRITE/APPEND,
SEEK, DUP/DUP2, RUN, FLOCK, provider and mount behavior. Then compare
full linked KCORE/MRES, per-process RAM, peak MM RAM, and runtime costs.
No new measurement is claimed. Reference v388: KCORE 9109,
KCORE+MRES 15657, PERMANENT_LAST 15704.
