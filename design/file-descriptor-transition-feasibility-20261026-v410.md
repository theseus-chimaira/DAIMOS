# Proposal B v410: transition and ownership feasibility

Status: design only; production layout and FLOCK semantics remain unapproved.

## OPEN invariant cost

Under v409's proposed process-local vnode lock model, regular OPEN
must inherit the mode of an existing descriptor for the same full vnode.
This requires a bounded scan (at most 16 entries) on successful OPEN.
The pathname, STAT, access and CREATE/TRUNC failure paths must not
publish an inconsistent descriptor. This is additional OPEN-time work;
no claim is made that it is free or that it is offset by the removed
lock-family machinery in the final linked image.

## RUN and pipe reference correction

`proc_run_map_loop` copies the two FILE words and the mount-extension
field per selected descriptor. `pipe_add_refs` then traverses the
child table, distinguishes PIPE from mounted TSFS using the child's
extension map, and increments the references. The assembly helper
returns normally; it does not expose a fallible allocation/error
interface. v409's reference to failed reference acquisition and
rollback is therefore only a hypothetical migration requirement,
NOT a description of the currently present routine.

The child-discard path after unsuccessful RUN mapping is separate.
Inspect actual `proc_slot_discard`/close behavior before claiming
which references or descriptors must be undone, especially when
mapping stops before `pipe_add_refs` executes.

## Exact transition proposal, subject to approval

* OPEN first completes failure-prone policy steps, determines the
  existing vnode-wide lock mode, and publishes new descriptor state.
  An existing lock is inherited without copying other descriptors'
  permissions, APPEND mode or independent offsets.
* FLOCK changes matching local descriptor states in a bounded scan;
  the old separate OPEN-family conflict behavior is intentionally lost.
* RUN copies the chosen parent state to child before publication.
  Child and parent locks subsequently diverge. PIPE references are
  adjusted once for successfully mapped descriptors, with the actual
  discard and cleanup implementation validated, not assumed.
* CLOSE and DUP2 retain existing failure atomicity; a failed sync
  must not delete or mutate the descriptor being replaced.
* Provider-7 PIPE vs TSFS identity includes full mount bits; the new
  format must not permit a TSFS mount to be treated as a PIPE stream.

## Cost ledger to measure after approval

Potential savings: remove `file_node_from_fd` mount extraction,
`file_mount_ext_store/copy/update`, repeated FLOCK mount-extension
decodes, unmount reconstruction, RUN map transfer, and PIPE child-map
lookup. Potential additions: bounded OPEN lock inheritance, new
metadata masks, changed FLOCK update logic and all rewritten callers.
No net KCORE saving or physical RAM improvement has been proved.

The 0410-octal process u-area is unchanged even if the extension word
067 becomes unused; do not count the word as physically reclaimed.

## Remaining decision gate

Explicit approval is still needed for vnode-wide process-local FLOCK
semantics and for limiting FILE provider/local-kind to 0..7 while
generic VFS accepts 0..63. A final audit of process discard and PIPE
ownership plus daimos-testkit failure cases is required before an
implementation experiment.

Measured reference remains v388 KCORE=9109, KCORE+MRES=15657,
PERMANENT_LAST=15704. No v410 production build or test is claimed.
