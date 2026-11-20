# K02 destructive low-memory EXEC resource ownership v1720

Date: 2026-11-20. Status: partial K02 fix, K02 remains OPEN.

## Problem and exact ownership boundaries

When ordinary transactional EXEC cannot stage an additional full VM,
`exec_replace_current_lowmem` first allocates an MM-owned scratch copy of
the validated launch request, releases its user-map hold and destroys
the current VM. The old code treated **every** failed `vm_space_destroy`
as a noncommitted failure, restored the user-mapping hold and returned
an ordinary EXEC error. However, `vm_space_destroy` deliberately publishes
the entire `VM_SPACE_NONE` word immediately after a successful PROCESS
MM free, before the separately fallible executable-backing detach.
If backing release fails then, the old APR has already been retired:
restoring the user mapping would permit execution against unowned RAM.

The second error is specific to the destructive loader using the *live*
process descriptor. In `exec_load_process`, a failed read/attachment may
already have changed the descriptor's left half to an executable entry,
or cleared it with `hrrzs 0(10)` on an internal rollback. For the current
process that same left half normally owns the stable u-area. Fatal FILE
close, HUP and terminal VM/u-area cleanup must continue with the original
u-area base and original process identity, not an entry-PC remnant.

## Implementation

- After rejected old-VM destruction, compare the entire descriptor
  `vm_state` to `[-1]`. If it is VM_NONE, return `EXEC_REPLACE_FATAL`
  without restoring a mapping hold. The private current-process fatal
  continuation retains any backing record for the existing teardown
  retry machinery. If VM is not NONE, restore the hold and report the
  normal recoverable error. Never infer a live APR from a stale base.
- After a loader or startup fatal failure, restore `EXEC_LM_META` to
  the stable process descriptor before releasing the transient launch
  copy and transferring to fatal FILE/EXIT. The snapshot is made before
  destructive commit. No new owner registry or per-process fields.
- Retain v1706's exact `mm_free` refusal stop for transient launch-copy
  scratch. No forced unpin/free or allocation on the failure path.

## Source-linked native regressions

`daimos-testkit/tests/validation/daimos-k02-exec-lowmem-owner-v1720.sh`
extracts the *actual* `exec_replace_current_lowmem` assembly and runs it
under native PDP-6, with mocked external services that return distinct
ownership states. It checks seven cases: old-VM release refused before
commit; backing detach refused after VM_NONE; early loader refusal;
loader refusal after a simulated metadata scrub; successful replacement;
replacement startup refusal and final cleanup; and launch-copy allocation
refusal. Status, stable process META, VM state, mapping hold, launch-copy
release and helper call counts are asserted. Two independently modified
production excerpts remove the VM_NONE guard and metadata restoration;
neither is allowed to pass.

Initial native fixture failed because its mock allocator read the
PUSHJ return PC at `(17)` as the fifth argument instead of `-1(17)`.
Correcting this **test-only stack ABI error**, and writing the startup
stack output at `3(4)` rather than `5(4)`, restores deterministic passing
native execution. No simulator/assembler defect or production change was
inferred from the faulty fixture.

The following independent native regressions also pass with v1720:
v1719 staged EXEC owner, v200 EXEC transaction (8 cases), v20 PURE
mount/rollback, and v1706 launch-copy MM refusal with negative control.

## Exact measured footprint

Matched GCC lowmem/disk-root links against committed v1719:

| Quantity (decimal 36-bit words) | v1719 | v1720 | Delta |
| --- | ---: | ---: | ---: |
| KCORE | 10,454 | 10,461 | +7 |
| KCORE plus installed MRES | 17,179 | 17,186 | +7 |
| Permanent last | 17,226 | 17,233 | +7 |
| Newly fixed RAM data | 0 | 0 | 0 |

Both full links assemble and link, but neither satisfies the independent
strict permanent-address boundary `040000` octal. v1720 shows -850
free-word accounting for SINGLE_ROOT. This is not a new accepted 32K
lowmem link or a 48K physical-boot claim.

## Remaining K02 work

Source-linked mock VM results are not a full physical fault injection.
The next gates must prove this same old-VM-after-free backing-refusal
path in a real 256K PDP-6 guest, along with finalizer recovery, no
resumed user PC and continuing independent process progress. Audit EXIT,
unpublished RUN and FILE/PIPE/FIFO reference lifetime under real release
refusals. K03 automatic OOM termination stays disabled. Completed-feature
documentation in `pdp10-doc` is unchanged until K02 closes.
