# KCORE checked BACKSTORE refusal through slot-0 scheduler, v1493

Date: 2026-11-11. Feature correctness only. This does not enable
swapless PURE reclaim or modify compressed image formats, MRES module
semantics, userland, or any size optimization.

## Production ownership transaction

`system/kernel/proc/proc_swap_pdp6.s` keeps the original DISK process
record and BACKSTORE allocation authoritative while `proc_swap_in`
allocates and reads a replacement APR extent. After a successful read
and `mm_unpin`, it calls the real `backstore_free`. Only successful
release publishes RESIDENT metadata, decrements backing usage, and
clears TRANSITION. On refused free it calls the MM rollback leaf,
preserves the disk record and leaves the bitmap/count intact.

The caller `proc_swap_mres_service_v481` handles a negative transaction
result as a fatal `SYS_EVENT_TERM`, rather than pretending that the
process was restored or treating corrupt backing as transient MM
pressure. Positive MM resource pressure still uses the existing
exchange/cooldown path. The service's TERM decision is intentional
fail-closed behavior, not evidence that broken bitmap ownership can
be forcibly cleaned or retried safely by a full guest.

## Native acceptance performed

`daimos-testkit/tests/validation/daimos-swap-checked-scheduler-v1493.sh`
extracts the real production slot-0 service, swap-in transaction and
complete BACKSTORE bitmap validator/free leaf and runs them under a
PDP-6 instruction executor. The v1405 MM/read owner fixture supplies
an intentionally incomplete bitmap. Results PASS:

- One real read; release refusal triggers one MM extent rollback.
- Exactly one TERM request, slot 2; no READY_ONCE, exchange or cooldown.
- DISK record, backing identity, TRANSITION clearance, bitmap and both
  usage counters remain unchanged; no RESIDENT publication.
- Executable negative controls rejecting ignored free failure and
  ignored service negative-result dispatch both PASS.
- Existing v1405 source-linked checked-free native regression reruns
  PASS, including repair-and-retry and its original unsafe mutation.

No new production code, resident RAM or kernel ABI change was needed.
This is not a full-device or full-scheduler guest test: `proc_event_apply`
and the later kernel finalizer were observed through a test stub.

## Remaining open gate

Test a deliberately refused BACKSTORE bitmap release in a real 256K
physical-swapping full guest. Observe the complete slot-0 scheduler,
actual TERM event delivery and finalization, held-provider interactions
and owner/accounting aftermath. Do not modify unrelated media or make
the assertion depend on a simulated success. Explicitly classify the
policy for an irreparably corrupt bitmap: fail closed without lying
about released ownership. The accepted v1405/v1466 normal-swap guest
cases are not substitutes for this corrupted-release full-guest case.

Other storage F2 mixed-media and provider failure tasks listed in
`design/kcore-feature-source-sweep-v1489.md` remain open. No KCORE
optimization is authorized or implied.
