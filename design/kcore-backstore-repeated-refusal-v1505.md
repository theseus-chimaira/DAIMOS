# KCORE BACKSTORE repeated checked-release refusal, native gate v1505

Date: 2026-11-11. Scope: production swap transaction invariants,
not scheduler retry cadence, arbitrary permanent corruption recovery,
or final physical acceptance.

Testkit `b0e5075` source-links the production PDP-6 swap-in,
slot-0 `proc_swap_mres_service_v481`, checked bitmap verifier and
`backstore_free`. The existing v1405 native backend controls only
MM allocations, payload reads and deallocation; event delivery is an
observer. Independent real-event native v1498/v1500/v1501 tests and
the physical v1503 coalesced-TERM gate remain separate evidence.

The test makes **six deliberately repeated** slot-0 requests against
the same invalid bitmap bit of one nonresident DISK allocation. Every
attempt returns failure with the exact original backing run and
bitmap unchanged, backed block counts unchanged, no published RAM
owner, no READY_ONCE, no leaked MM pins or transition lock and no
invariant violation. Totals after six requests: six device reads,
six successful MM rollback releases, six TERM requests and zero VM
publishes. Existing v1493 negative mutations rejecting unchecked
release and false READY_ONCE also PASS.

Only after the test **explicitly repairs** the original bitmap bit
does a seventh request succeed, publish one RESIDENT VM and free the
original backing with exact zero final allocation counts. The real
service success result is the selected slot *with READY_ONCE set*
(`01002` for slot 2), not an untagged slot number. The inline
production API comment was corrected accordingly. Likewise its
failure result cannot promise that the requested TERM was completed:
the physical v1503 guest proved an already-pending TERM coalesces and
retains the original owner.

This proves deterministic **fail-closed ownership** under multiple
bad bitmap transactions. The loop supplies each request directly;
it does **not** prove that production repeatedly schedules the owner
without a bound, nor that permanent damage is recoverable. The
unsolved feature remains explicit quarantine or safe owner-side
resolution if the bitmap can never be repaired. No additional
resident storage or functional KCORE change was introduced.

Next physical test: disposable v1504 owner-return trace, which must
prove whether the selected process's pending TERM is consumed at its
actual syscall-return boundary after repair. Normal F07/v1336 must
not permit fatal swap-in errors; retain a separate fault-specific
gate. Mixed-device and PURE-text feature completion follows.
