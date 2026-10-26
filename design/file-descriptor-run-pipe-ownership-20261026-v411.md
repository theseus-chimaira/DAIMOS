# Proposal B v411: RUN construction and PIPE ownership

Status: read-only architecture audit. No production change approved.

## Actual RUN acquisition/rollback order

`proc_run.s` validates the requested explicit descriptor map, copies
selected parent descriptor pairs and mount-extension bits into the
child u-area, then calls `pipe_add_refs` once over the completed child
descriptor table. Successful code next marks the child SRUN, queues
it, and returns. The presently identified FD-map rejection branches
all execute before `pipe_add_refs`, through `proc_run_claimed_bad`.

`proc_slot_discard` in `proc_pdp6.s`/alternative `proc.c` destroys VM,
releases terminal/session state and the child's u-area, removes it
from the run queue, and zeroes the process record. It does NOT walk
the child's FILE table or call `file_close_all`. This is correct for
the observed construction-failure sequence because no child PIPE
references have been acquired yet. Do not describe this cleanup as
general descriptor reference rollback.

`pipe_add_refs` in `ipc/pipe.s` scans all 16 child descriptor slots,
uses the child's mount-extension map to distinguish mounted TSFS from
PIPE, and calls `pipe_add_ref`. It has no ordinary failure return.
Thus the current path does not provide a post-increment rollback
facility and must not acquire more references before fallible checks.

Normal fatal-event/exit paths call `file_close_all` for the relevant
process before tearing down resources. `file_close_all` uses the
per-descriptor `file_close` operation; this is separate from initial
unpublished RUN rollback.

## Invariant required by a new descriptor encoding

1. Validate entire FD map and perform all other fallible child-setup
   steps before taking PIPE references. No new failure branch may be
   placed after acquisition without an explicit matching release.
2. Child FILE words may be copied speculatively while unpublished,
   but their ownership of PIPE counts starts only at `pipe_add_refs`.
3. Use the full canonical vnode to distinguish PIPE provider 7,
   mount 0, stream kind 1 from TSFS provider 7 mounted identities.
   A TSFS vnode must never increment or decrement PIPE references.
4. On successful RUN, the new process owns its acquired PIPE counts;
   fatal/exit cleanup must run in its process context and close them.
5. DUP/DUP2 continue their separate immediate PIPE ref accounting.
   Failed destination CLOSE in DUP2 must preserve the original record.

## Required test matrix (all tests in daimos-testkit)

- RUN mapping rejects duplicate child FD, invalid source FD, or
  invalid destination FD before reference acquisition.
- Successful RUN mapping duplicates PIPE endpoints accurately,
  including two child descriptors from one parent source.
- Child exit/forced termination releases acquired PIPE references.
- Failed RUN construction leaves parent and PIPE reference counts
  unchanged; a separate test confirms process slot/u-area cleanup.
- TSFS roots with PIPE-like low provider/kind bits on mounts 8, 16,
  24, 26 and 63 never enter PIPE reference management.
- Failed DUP2 close leaves both old and new descriptors and reference
  counts unchanged.

## Implementation gate

This confirms the current ordering rather than authorizing a rewrite.
Prototype only after the vnode-wide FLOCK semantics and the narrow
provider/local-kind contract have been decided. Check the complete
full-linked image, total physical memory and runtime cost; revert
negative results. Strict 32K and JRST folding remain deferred.

Historic measured baseline v388: KCORE 9109, KCORE+MRES 15657,
PERMANENT_LAST 15704. No new build or test pass claimed.
