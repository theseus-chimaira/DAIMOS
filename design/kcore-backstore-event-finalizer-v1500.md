# Native real TERM-to-PREVM finalization, v1500

Date: 2026-11-11. KCORE feature-correctness gate; no optimization.

## Integrated production leaves

Testkit v1500 (`5674a7e`) links unchanged production PDP-6
`proc_event_apply`, `proc_finish_slot`, and
`proc_finish_service_v881` into the **same** native test. The
victim retains a valid u-area, an authoritative nonresident DISK
VM, no pending fatal/WAIT, and a saved user-mode return. The actual
foreign TERM path therefore legitimately enters finalization.

The controlled FILE close succeeds, while the controlled VM teardown
refuses its first release. The real finalizer must retain
TRANSITION, FINALIZE+PREVM and terminal TERM status, leaving the DISK
VM owner intact. The real idle finalizer then retries VM teardown,
accepts it on the second call, and handles two subsequent refused
u-area MM releases before publishing a zombie. The test verifies
there is no repeated HUP, FILE close or committed VM destruction,
and that a later event or idle scan cannot duplicate finalization.

An executable negative mutation removes the production PREVM
`proc_finish_mark_pending_v892` call. It is rejected by the native
fixture. All positive and negative v1500 tests PASS. KCORE source,
resident RAM and ABI are unchanged.

## Scope and remaining feature work

The actual `vm_space_destroy` and physical `backstore_free` calls
are **controlled** in this native test; the full guest v1499
disposable branch probe separately tests real physical I/O, real
bitmap refusal and two event calls. Native v1500 cannot substitute
for full physical acceptance.

The outstanding distinction is safe foreign finalization versus
deferred TERM on a private executive continuation. The physical
v1495 guest had no immediate PREVM mark after either TERM call.
v1498 proved the decision rules. v1499 must establish the actual
pre/post owner state and branch. A separately designed test is still
required for permanently corrupt backing, eventual TERM delivery,
exact owner release and normal v1336 strict assertions.
