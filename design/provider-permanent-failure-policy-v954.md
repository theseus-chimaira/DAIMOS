# Provider permanent-failure ownership policy (v954)

## Scope

Shared KCORE serialized filesystem-provider entry and fatal FILE cleanup on
PDP-6. D6FS is in scope; DTFS-specific behavior is deferred until its rewrite.
This is a safety contract, not a new administrative syscall.

## Current ownership and progress guarantees

- `fs_provider_ready` contains owner slot + 1 in its left half, with a zero
  right half while a physical provider is active. The nonzero right half
  indicates a free lease. A separate `fs_provider_wait_event_v917` signals
  waiters; wakeups only permit them to retry the lease acquisition.
- The provider returns through `fs_provider_unlock`, which releases its
  lease and wakes waiters. A recursive invocation by the owner fails with
  MM_BUSY-like error -5; slot zero cannot sleep on another owner's lease.
- A blocked `proc_wait_event` still owns its private executive continuation,
  including the saved arguments on its private kernel stack. Releasing that
  u-area or resetting the provider lease from a foreign context would permit
  a second request to overwrite shared workspace before the first returns.
- v981 removed the unused cross-process OOM selector. Only a requesting
  process can be a future OOM termination candidate, and no automatic
  termination is currently enabled. A stalled provider remains protected.
- Fatal owner-side FILE close retains unsuccessful descriptors and the
  failure witness, sleeps with exponential delay capped at 01000 octal
  ticks, and resumes on that same owned stack. v953 demonstrates eventual
  completion after the provider starts succeeding again.

## Recovery policy

1. **Transient refusal:** retry the existing owned request; never discard
   the descriptor or overwrite the first failure witness within a pass.
2. **Indefinite returned errors:** retain the owner and allocation; report
   FILE stage, descriptor and most recent provider error through the existing
   read-only GETTEARDOWN query. Keep independent users runnable.
3. **Nonreturning provider:** the lease, continuation, shared block
   workspace and referenced allocations remain owned. No forced unlock,
   synthetic wakeup, PID recycling, or MM free is permitted on timeout.
4. **Administration:** an operator may diagnose, repair external media or
   reconnect an underlying device outside the kernel. Kernel cancellation
   would require a provider-specific operation proving that all outstanding
   asynchronous transfers and callbacks have stopped, its private stack can
   safely resume or unwind, and no other request can observe stale state.
   No such cancellation protocol is implemented today. Reboot is the
   isolation boundary when the provider cannot become quiescent.
5. **OOM:** no stalled-provider owner is a killable victim. There is no
   allocation-free bounded reclamation guarantee for permanent failures;
   therefore enabling the automatic OOM killer is still prohibited.

## Tests and remaining gaps

- v955: native MM u-area failures distinguish legitimate multi-pin
  refusal (-5) from wrong type, owner and missing extents (-4), without
  modifying retained extent records. Releasing the pin and then freeing
  works normally. This does not provide authority to repair corrupted
  metadata by guessing an extent's rightful owner.
- v866/v915/v916/v917: native serialized lease, recursive refusal, foreign
  waiter, unlock and event behavior.
- v891/v950/v951: booted retained descriptor and provider error reporting
  with continued bystander progress.
- v953: booted provider revival after 11 failed closes and capped backoff.
- v954: source-contract checks for acquisition, wait, normal unlock,
  unknown-owner OOM exclusion and non-forced private fatal retries.
- Outstanding: provider-specific asynchronous quiescence proof, physical
  device cancellation, permanent MM extent failure recovery, and full
  allocation-free OOM teardown acceptance.

These tests establish containment and recovery after resumed service, not
bounded automatic reclamation of a permanently dead provider.

## v987 lease containment audit

The v987 source-linked test confirms that the owner-encoded lease is only
released on the normal provider return path; recursive access refuses,
foreign waiters sleep and revalidate, and fatal close retries cannot force
an unlock or free the owning u-area. The test does not prove quiescence,
cancellation or eventual recovery of permanently nonreturning hardware.
Automatic faulting-process OOM termination remains disabled.
