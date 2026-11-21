# K04: reject DISCARD owners from EXIT finalization v1758

Status: source correctness fix; isolated PDP-6 DAS assembly PASS.
K04 OPEN pending physical fault-injection acceptance.

`proc_finish_slot` is called both by the deferred reaper and by the
foreign fatal-event path. Its TRANSITION resume check previously
accepted any FINALIZE-marked u-area, including the FINALIZE|DISCARD
marker owned by an unpublished RUN rollback. The reaper normally
dispatches that marker to `proc_slot_discard`, but the public EXIT
finalizer entry point did not enforce the same ownership distinction.

v1758 rejects DISCARD during the EXIT resume check, before VM or
u-area release. This prevents an incorrectly routed foreign finalizer
from publishing an unpublished child as a zombie or performing EXIT
child adoption. The existing rollback reaper and valid EXIT paths
remain unchanged. No new persistent state is introduced.

This guard is based on the exposed entry point and marker semantics;
it does not claim a demonstrated runtime collision with an SIDL
rollback. No testkit files were changed.
