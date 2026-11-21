# K04: post-VM owner must not retain session membership v1761

Status: production correctness fix; isolated PDP-6 DAS assembly PASS.
K04 remains OPEN pending physical multi-owner fault injection.

## Orphaned TTY scenario in the v1760 source

Two processes share a session and a canonical TTY. Process A exits
first. Its `proc_tty_release_session` finds B still alive, so it
correctly leaves the TTY assigned to the session. A destroys its VM,
but `proc_uarea_release` refuses its pinned u-area. A therefore
retains TRANSITION, VM_NONE, FINALIZE and the old session ID while
slot zero retries the u-area MM release.

B exits while A is still pending. Before this change,
`proc_tty_session_has` counted A as a live session member merely
because it was not FREE/ZOMB and still had its u-area. B therefore
skipped the TTY release. When A later succeeded at u-area release,
its post-VM finalizer went straight to descriptor/zombie publication:
it did **not** call `proc_tty_release_session` again. The TTY's
session and possibly its canonical input buffer were stranded.

The same ordering can occur after an unpublished RUN rollback whose
VM has been destroyed but whose u-area remains MM-busy.

## v1761 ownership distinction

`proc_tty_session_has` now excludes a matching process only if all
of the following hold:

- It has TRANSITION set, i.e. is not an ordinary live member.
- Its VM is exactly VM_SPACE_NONE (full-word -1).
- Its stable u-area contains FINALIZE but **not** PREVM.

Such an owner has already passed the TTY release stage. A later
session member can therefore retire the TTY while the post-VM owner
continues to own its pinned u-area and process identity. An EXIT
PREVM owner remains a member even if its VM base was already cleared
by partial backing detach: PREVM must revisit TTY release before
completing. Unmarked TRANSITION, ordinary live processes and their
session membership are unchanged. `SETFG` also uses this scanner
and must not accept an already-finalized process group.

The scanner now rejects an inconsistent UAREA flag with zero base
as -1 instead of dereferencing low memory. The release caller
already interprets nonzero as "do not retire TTY"; the SETFG caller
now explicitly rejects zero or negative results. This preserves
ownership and prevents an invalid descriptor from authorizing a
foreground group.

No permanent RAM, process-table fields or ABI changes. The fix
does not forcibly release a refused TTY or MM extent. The native
PDP-6 assembler and whitespace checks passed; no testkit or booted
guest acceptance is claimed. Physical acceptance still needs the
ordered A-postVM/B-exit/A-unpin case, permanently refused owners,
clock wrap and WAIT/PID reuse.
