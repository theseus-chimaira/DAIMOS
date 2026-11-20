# K02 physical RUN inheritance and ordinary EXIT PIPE ownership v1723

Date: 2026-11-20. Status: 256K PDP-6 acceptance PASS; K02 remains OPEN.

## Purpose

Native source-linked v1722 tests established the order of RUN FD-map
validation, PIPE reference acquisition, FILE close, and retry under
provider refusal. v1723 tests these same stages on a **booted 256K
PDP-6** with actual, independently scheduled parent and child processes,
a real MM extent, and the production RUN, FILE, PIPE and WAIT paths.

Ordinary `SYS_EXIT` must return an error to the original process when
its FILE close fails. Unlike a fatal APR/EXEC error, this path does not
enter `proc_fatal_close_current_v879` and cannot transfer ownership to
the slot-zero finalizer until FILE close succeeds. This is deliberate:
it lets the child retain and retry its exact outstanding descriptor.

## Deterministic multi-process scenario

The testkit's `daimos-k02-run-pipe-exit-v1723.c` is compiled as C89.
INIT (process slot 1) creates an anonymous data PIPE and a second,
independent control PIPE. Its first four newly allocated descriptors
are data reader FD3, data writer FD4, control reader FD5, and control
writer FD6. The real `SYS_RUN` maps standard FD0..FD2 and the data
reader, data writer and control reader into the child.

The child blocks reading control FD5. Meanwhile the parent closes its
data reader, data writer, and control reader, then writes byte `G` to
control FD6 and closes that descriptor. Consequently no parent-owned
data-PIPE reference survives when the child begins its first EXIT.
The child then calls `dsys_exit(065)` twice. On the first call the
production FILE close-all closes inherited data reader FD3, but its
last writer FD4 cannot free the physically pinned PIPE. `SYS_EXIT`
returns -1 to the child; FD4 must still contain the same vnode. The
second EXIT retries only the outstanding FD and completes. INIT waits
for `SYS_WAIT_EXITED` with status 065 and checks that process slots,
process words and parent FILE occupancy returned to their baseline.
INIT emits an exact `P` line and halts the guest.

## Physical failure injection and assertions

`daimos-k02-run-pipe-exit-v1723.sh` copies the current production
kernel into an isolated directory and performs one anchored test-only
modification of `ipc/pipe.s` at `pipe_close_free_v870`, plus read-only
diagnostic counters around its real `mm_free`. The hook selects the
first final-close object and verifies that its owner is a RUN child,
not INIT. On both attempts it checks that FD3 is closed, FD4 still
points at the same PIPE object, and the object's original counts remain
`readers=0, writers=1`. On the first attempt it calls the real `mm_pin`;
the real `mm_free` returns MM_ERR_BUSY (-5). On the second attempt it
calls the real `mm_unpin`, allowing the same physical release to commit.
No simulated MM free, synthetic success, altered production RUN code,
or replacement process scheduler is used.

The complete guest and strict post-run verifier passed:

| Recorded guest condition | Value |
| --- | ---: |
| Chosen physical PIPE base (decimal) | 21,567 |
| Child process slot | 2 |
| Final-close attempts for that PIPE | 2 |
| Real `mm_pin` calls accepted | 1 |
| Real `mm_unpin` calls accepted | 1 |
| Actual MM_ERR_BUSY returns | 1 |
| Successful final `mm_free` results | 1 |
| Owner/descriptor/count errors | 0 |
| Parent WAIT / MEMINFO baseline | PASS |
| Exact parent `P` line and final HALT | PASS |

The SIMH run halted at the user-requested halt rather than a fatal MM
failure. Kernel DXR SHA-256 (test-only instrumented image):
`ee80fbbf1ccdca85652bd5f136b62b9850767d93d83bb83cc189cd01372a498e`.
SIMH transcript SHA-256:
`a24b81e1625a010b848caa0a63f2597d6920e492a6057af6d315853d6744f314`.
After the guest completed, the stricter verifier was replayed against
its preserved transcript, and all conditions passed.

## Independent regressions and remaining work

The source-linked v1722 native RUN/PIPE/FILE test with two source
mutants, the v892 unpublished-RUN failed-release/reaper test, and the
v1099 typed PIPE/FIFO final-MM-close test with four unsafe mutants
all passed against the same production checkout.

KCORE text delta: **0 words**. Permanent data delta: **0 words**.
No new per-process descriptors, RAM allocations, interface changes,
or KCORE workarounds. `pdp10-doc` remains unchanged until K02 closes.

K02 is still OPEN: not all provider-originated permanent refusal,
remote EXIT/HUP, filesystem/provider, PI interrupt, PIPE/FIFO and
unpublished-RUN interleavings are accepted. K04 recovery remains a
separate task, and K03 automatic OOM termination stays disabled.
