# Proposal B v404: alternative FLOCK contract and metadata budget

Status: design comparison only. No semantics change authorized.

## Source-grounded state space

The current native implementation stores independently selected READ and
WRITE permission bits, a DIR/APPEND bit, a two-bit regular/lock state and
a four-bit duplication-family identifier. OPEN only sets APPEND as retained
metadata for regular files; for other types it may initialize the offset
to EOF without retaining APPEND. SEEK and FLOCK require regular state.
READDIR requires DIR state. READ/WRITE reject DIR and require their own
permission bits. Character/device endpoints need permitted access flags
despite not being regular files. All these conditions must survive DUP and
RUN and cannot be recomputed from vnode identity alone.

Current fixed representation: 9 bits per FD * 16 = 144 metadata bits
(4 words), packed into vnode spare bits; extension map uses 1 u-area word.
This is the direct-field model, not the information-theoretic minimum.

## Candidate L1: per-process vnode FLOCK (semantic redesign)

Possible new contract for discussion: SHARED/EXCLUSIVE/UNLOCK apply to
all descriptors of the same canonical vnode owned by the current process,
regardless of which OPEN created them; DUP keeps copied offsets but does
not create a distinct lock family. Conflicts are only among owners defined
by the new contract, not by historical per-OPEN families. This requires
an explicit rule for process-to-process exclusion, lock release on CLOSE,
RUN inheritance, forced teardown and unmount. The existing resident
FLOCK loop only examines the current process FILE table, so calling this
a global file lock without further machinery would be incorrect.

L1 removes four family bits per FD, but it does not eliminate all lock
state. If two lock-state bits remain per descriptor, current remaining
direct fields are READ, WRITE, DIR/APPEND, REGULAR/LOCK(2) = 5 bits/FD,
or 80 bits, requiring 3 metadata words. Compared with today's one-word
extension map, this is +2 process words, before linked-text savings.
For a new process-level shared/exclusive state keyed by vnode, separate
storage and lifetime/lookup costs must be included instead.

## Candidate L2: packed access/type code plus FLOCK state

One may encode permission and type combinations as a compact code rather
than independent bits. But minimally representing three useful access
sets (R, W, RW), a regular-vs-nonregular distinction, and a regular APPEND
choice already produces up to 3*3 = 9 combinations (nonregular,
regular nonappend, regular append), before distinguishing directories
from other nonregular endpoints. This is at least four bits for those
combinations, and at least two more bits if all three lock states and
unlocked regular state are stored per FD. That is a lower-bound argument
for the stated candidate space, not proof all combinations are reachable.
No <=36-bit per-process representation for all 16 FDs was discovered.

## Decision

Neither L1 nor L2 currently proves lower physical RAM than the existing
packed FILE representation. In particular, simply replacing family tokens
with per-process vnode equality makes previously independent OPENs share
lock ownership: that is a major deliberate behavior change, not an
equivalent compression. It also risks losing lock exclusions outside the
current process if cross-process locking is claimed.

Next: inspect actual application FLOCK users and required concurrency
semantics; then evaluate a *packed vnode with simplified lock semantics*
as a distinct candidate, avoiding canonical-vnode metadata displacement.
Do not write a new FLOCK implementation until the contract is approved.
Reference measured v388 link remains KCORE=9109, KCORE+MRES=15657,
PERMANENT_LAST=15704; no v404 linked build or runtime pass was made.
