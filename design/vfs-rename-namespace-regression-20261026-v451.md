# Proposal B v451: RENAME namespace regression closure

Strengthened the existing v450 regression in `daimos-testkit` to
assert that all 4096 six-bit provider/mount namespace identities
are pairwise distinct under the resident VFS RENAME mask, in addition
to the previously checked preservation of identity and independence
from vnode-local kind/index bits. This closes a coverage gap where
individual self-comparisons need not detect cross-namespace collisions.

The strengthened check passes. Existing PDP-6 FILE, pathname-walk,
and native provider-lock regressions also pass. The test does not
substitute for a complete runtime RENAME transaction fixture.

No DAIMOS production code or persistent allocation changed. Last
measured full linked lowmem baseline remains v450: KCORE 8963,
KCORE+MRES 15510, PERMANENT_LAST 15557. No new size saving is claimed.

Structural review: two RENAME readonly checks must distinguish the
old and new parent, while the common mutation path has only one
vnode and uses a three-register stack frame. Reusing that path
without a replacement ownership/call contract would add adapter
instructions rather than eliminate the duplicate boundary. Likewise,
CREATE/SYMLINK share provider dispatch but require output-vnode
publication, which ordinary UNLINK/CHMOD do not. No speculative
cross-path adapter is accepted without a full-image RAM reduction.

Strict 32K acceptance remains deferred.
