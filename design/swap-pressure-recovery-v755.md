# v755: Swap-in resource pressure and scheduler liveness

Status: design audit, not implemented or accepted.

## v756: Typed transaction result implemented (partial)

The swap-in transaction now returns `PROC_SWAP_IN_PRESSURE` (+1) for
allocator resource errors `MM_ERR_NOMEM`, `MM_ERR_FRAGMENTED`, and
`MM_ERR_DESCRIPTORS`. It retains `PROC_SWAP_IN_FATAL` (-1) for invalid
records, failed storage/restore, and rollback failures; zero is success.
The classification is implemented in the native PDP-6 assembly and
the C reference path, with matching constants in `proc_swap.h`.

The versioned testkit `daimos-mm-swap-result-contract-v756.sh` passes
assembly and interface checks. **The scheduler still treats any nonzero
return as fatal and is not changed by v756.** Consequently v756 does not
resolve the native writer TERM or establish a recoverable scheduler.
Production commit `e720c3a`; testkit commit `fb6bbcf`.

Do not change `proc_swap_mres_service_v481` to ignore +1 in isolation:
the idle loop triggers a new reschedule immediately, and ordinary runq
selection can choose the same swapped process again. A complete atomic
scheduler retry/admission policy remains necessary.

## Reproduced failure

The native v750-v754 trace in `design/kcore-feature-baseline-v735.md`
establishes an MM_ERR_NOMEM return while restoring slot 2, with only
907 physical words free. BACKSTORE has 176 free blocks; a full-size
resident victim requires 208 blocks. The SWAP MRES service converts
every nonzero swap-in result to a fatal TERM, including this case.

## Source-level invariants to preserve

* `proc_swap_in` preserves the swapped disk record when allocation fails
  and clears PROC_TRANSITION. No disk read or image publication occurs.
* The existing `proc_swap_in` return is binary (0/-1); it does not say
  whether the failure was capacity, malformed record, or disk I/O.
* The PDP-6 scheduler scans a single run queue and chooses the lowest
  effective priority. A swapped winner requests slot-0 swap service and
  becomes an idle-context job; RT ownership is selected separately.
* The idle loop always requests another reschedule after servicing a swap
  request. Suppressing TERM alone would immediately reselect the same
  swapped job and can starve resident runnable processes.
* Process extents require native APR 02000-octal alignment. Any solution
  must preserve existing pin, transition, and storage-publication rules.

## Required production fix

1. **Typed swap-in result:** retain zero for success, distinguish
   reversible `MM_ERR_NOMEM`/`MM_ERR_FRAGMENTED`/descriptor pressure from
   invalid swap metadata, irrecoverable read error, and failed MM cleanup.
   Check C and PDP-6 assembly transaction parity. Do not classify every
   negative result as retryable.
2. **Forward progress:** when pressure prevents a swap-in, allow an
   existing runnable resident process to run, without evicting or killing
   the valid swapped process. Prevent immediate repeated selection of the
   same unrestorable slot; consider RT owner behavior explicitly.
3. **No permanent deadlock:** define the no-resident-runnable case and
   an admission/headroom invariant, especially if BACKSTORE cannot hold
   one additional full process image. A deferred swap request must have a
   source of future resource progress or an explicit nonfatal failure
   surfaced to the admission caller; indefinite idle retries are invalid.
4. **Native acceptance:** cover both the 176-free/208-needed pressure
   fixture and a recovery fixture in which another process exits and frees
   memory. Verify writer normal EXIT/WAIT, preservation of swapped image,
   no spurious TERM, bounded scheduler retries, and corruption/I/O fatal
   paths. Keep tests in daimos-testkit.

Do not use a larger swap partition as a substitute for fixing scheduling
semantics. Do not change the backstore record format or introduce resident
telemetry merely to hide this capacity failure. The final code-size sweep
remains deferred until feature/correctness acceptance.
