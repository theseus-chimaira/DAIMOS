# K04: VM_NONE DISCARD must not satisfy fatal HUP v1762

Status: source correctness fix, isolated PDP-6 DAS assembly PASS.
K04 remains OPEN for booted fault-injection acceptance.

The v1760 session HUP retry path recognized a committed EXIT
finalizer by TRANSITION, VM_SPACE_NONE and FINALIZE. It did not
exclude the FINALIZE|DISCARD marker, which represents an unpublished
RUN rollback after VM destruction and a refused u-area MM release.
The PREVM branch already excluded DISCARD, but the VM_NONE branch
could silently treat a rollback owner as an already delivered HUP.

v1762 checks the DISCARD bit in the shared marked-owner path. A
FINALIZE|DISCARD owner is never accepted as a committed fatal HUP;
the ordinary event-delivery path rejects its TRANSITION state and
retains the session leader's retry owner. Only a marked non-DISCARD
EXIT owner may be skipped as an already delivered fatal HUP.

The change does not force MM release, alter process identity, or
add persistent RAM. It preserves the distinct rollback and EXIT
contracts introduced in v1758. No testkit or physical guest was run.
