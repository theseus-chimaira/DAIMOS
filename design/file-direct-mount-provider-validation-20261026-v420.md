# Proposal B v420: D6FS provider acceptance after FILE direct-mount cutover

Status: three focused native PDP-6 provider tests PASS; no production edit.

Executed against committed FILE v417 source using existing daimos-testkit:

* `test-daimos-d6fs-multimount`: PASS (independent readers, vnode
  identities, DRM-set backing and mount slots).
* `test-daimos-d6fs-remount`: PASS (read-write/read-only A/B mount
  publication, failure atomicity and unmount cleanup).
* `test-daimos-d6fs-direct-backing-drm`: PASS (DSK compatibility,
  direct/multiple-DRM backing dispatch and safe no-driver path).

These tests validate distinct provider/mount operations but do not
establish an end-to-end login or generic filesystem boot pass. The
`test-daimos-root-login-lifecycle` test is a separate acceptance gate.

The linked low-memory size remains the last measured v417 result:
KCORE 8999, KCORE+MRES 15547, PERMANENT_LAST 15594, each 110 words
below the pre-cutover baseline. No new size measurement or process
u-area allocation reduction is claimed here. Strict 32K acceptance
and unrelated JRST folding remain deferred.
