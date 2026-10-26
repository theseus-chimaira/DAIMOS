# Proposal B v460: RENAME cross-layer cutover gate

This is a source-grounded architecture decision, not a RAM-saving edit.
The currently linked v452 baseline remains KCORE 8961,
KCORE+MRES 15508 and PERMANENT_LAST 15555.

## Current resident call ownership

1. `syscall_dispatch_pdp6.s:native_sys_two_paths` obtains one VM
   mapping hold and translates both user pointers. If either mapping
   fails, it releases the lease and returns -1.
2. `%L112` invokes `file_rename` and enters
   `native_sys_mapped_return` to release the lease even after a
   sleeping provider call.
3. `file_runtime.s:file_rename` owns 14 local words and one saved
   mapped new-path pointer. It resolves old and new parent/name
   records independently and validates write/search permission for
   both parents before invoking `vfs_rename`. One cleanup tail handles
   all paths.
4. `vfs.s:vfs_rename` checks both parent mount readonly policies,
   rejects provider/mount namespace mismatch, and dispatches opcode
   012 to `fs_provider_reg_call`.
5. `fs_mres.s:fs_provider_reg_call` retains a process-private register
   frame when waiting for the serialized sleeping physical provider;
   this lifetime is independent of the user VM mapping hold.

## Safe consolidation boundary

A worthwhile replacement must remove the RENAME syscall case and
FILE wrapper together, not add a new trampoline. The replacement
would own one VM mapping lease, two validated parent/name records,
two permissions, namespace/readonly checks, one provider transaction,
and one return/cleanup path. User pointers must remain mapped for the
entire pathname resolution and blocking operation; provider callbacks
must not retain them after the call. The stack must remain process-
private across provider waits.

## Acceptance gates before production cutover

- Native test invoking the actual syscall veneer plus actual FILE and
  VFS bodies, including invalid first/second user pointer.
- Both pathname and permission failure positions, cross-provider and
  cross-mount names, source/destination readonly, provider failure.
- Real or deterministic provider sleep/wakeup with a stable user
  mapping lease, no workspace corruption, and balanced cleanup.
- Fully linked GCC PDP-6 lowmem baseline comparison including KCORE,
  MRES, other permanent data, process stack and MM allocation costs.
- Reject any size-neutral or positive-RAM replacement; keep the
  existing ABI and filesystem semantics until an integrated cutover
  is actually tested.

Existing v455-v457 native tests cover the VFS, FILE and DTFS leaves
separately; they do not prove the proposed integrated ownership
boundary. No speculative production edit is accepted in v460.
Strict 32K acceptance remains deferred.
