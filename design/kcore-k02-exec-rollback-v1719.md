# K02 transactional EXEC staged-owner rollback v1719

Date: 2026-11-20. Status: partial K02 fix; K02 remains OPEN.

## Source-derived failure

The PDP-6 resident `exec_replace_current` stages a replacement image in
three temporary process words on its current process's kernel stack.
`proc_swap_records[slot]` is shared by that staged image and the still-live
old descriptor. During normal commit, the code temporarily restores the
old backing record before `vm_space_destroy(old)` and later republishes
the new one. On an old-VM destructor refusal, the old code immediately
called `vm_space_destroy(staged)` while the slot still held the old record.
`proc_swap_detach` uses the slot, not the staged descriptor, to find the
record; the wrong executable could therefore be detached.

The same path treated an old VM destructor error as resumable even if
`vm_space_destroy` had already freed the resident PROCESS extent and
published VM_NONE before a subsequent detach refusal. Another path
ignored unsuccessful destruction of the temporary staged descriptor,
which has no persistent process-table or finalizer owner.

## Change and safety contract

1. Start staging with VM_NONE so early loader errors have no invented VM
   owner. If loading returns an error with an actual outstanding staged
   image, call the checked destructor again before unwinding the frame.
2. When startup fails, check the staged destructor's return. When the
   old VM destructor fails, publish the saved new backing record for
   staged destruction; restore the saved old backing and mount-bit state
   only after the staged teardown has committed.
3. A refused staged destruction reaches a distinct HALT 4. Since the
   staged owner exists only on the current kernel stack, it cannot be
   safely returned to userspace or scheduled into an ordinary finalizer.
   This fail-stop is intentional, not forced MM release or an OOM policy.
4. After old-VM destructor refusal, VM_NONE marks an irreversible
   physical release. Return EXEC_REPLACE_FATAL after staged cleanup;
   the caller's pre-existing fatal-FILE closure/EXIT continuation owns
   the remaining old-process resources. Non-VM_NONE refusals restore
   the old executable and return ordinary EXEC failure.

The change adds no persistent data. In matched fresh GCC lowmem disk-root
links, KCORE grows 10,431 -> 10,454 words (+23); KCORE+MRES grows
17,156 -> 17,179; permanent last grows 17,203 -> 17,226. Both
baseline and changed link pass the linker but fail the strict 040000
octal permanent-address acceptance gate (baseline -820 free-word
accounting, patch -843). No linked success within that limit is claimed.

## Validation

`daimos-testkit/tests/validation/daimos-k02-exec-stage-owner-v1719.sh`
extracts the real EXEC transaction into a PDP-6 native harness. Six
cases cover early load rejection, a retained VM on loader error,
startup failure, old-VM refusal before release, old-VM refusal after
VM_NONE commit, and successful image commit. A negative control replaces
the new backing with the old backing before staged destruction; it must
be rejected. The test verifies the shared backing owner seen by each
destructor, call counts, VM ownership, and return status. A second
test-only destructor refusal proves HALT 4 instead of returning from an
unpublished, stack-only owner. Both negative controls pass.

The existing eight-mode native `daimos-exec-commit-rollback-v200.sh` and
`daimos-exec-pure-bit-v20.sh` pass against this version.

## Open work

This change does not close K02. A permanently refusing staged owner
still requires controlled offline recovery after fail-stop. Physical
fault injection, the broader EXIT/EXEC/RUN resource closure matrix,
FILE/PIPE provider lifetime and cross-interleaving validation remain
open. OOM killing stays disabled. No completed-feature update is made
to `pdp10-doc` until K02 is finished.
