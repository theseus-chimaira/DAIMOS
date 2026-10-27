# Proposal B v479: RENAME optimization and correctness closure

## Disposition

RENAME is complete for the present KCORE optimization pass. Keep the
tested syscall -> FILE -> VFS -> physical-provider ownership boundary.
The speculative coordinated wrapper-removal design is **not accepted**:
no verified reduction in fully linked permanent RAM was established,
and removing ownership across blocking I/O without direct PI6 evidence
would create unnecessary risk. This does not prohibit a later
cross-layer redesign when measurable savings justify it.

## Accepted implementation

- Compact provider/mount namespace XOR comparison, retaining the required
  provider ID in AC7 before `fs_provider_reg_call` (v466 correctness fix).
- Three-word VFS RENAME save frame preserving AC4 (v472, -2 linked words).
- FILE parent lookup and permission checks, VFS readonly/namespace checks,
  serialized physical-provider transaction and one mapped syscall lease.

## Validation

- Reexecuted on this source checkout: v450 exhaustive namespace guard
  (4096 identities), v468 PI6/VM/provider source contract, native PDP-6
  v455 VFS RENAME, v461 integrated FILE/VFS, v466 actual provider gateway
  with synchronous deterministic wait/retry: **all PASS**.
- Earlier full-system PDP-6 SIMH v469/v470/v471 runs **PASS**, including
  error RENAME and successful create/rename/read/verify/unlink on D6FS;
  v471 uses two separate user processes alongside PI6 scheduling and
  disk sleep/wakeup. These tests were not rerun for this closure.

## Boundaries and next priority

The tests do **not** prove that a single RENAME was interrupted and
resumed across a real PI6 switch within its own provider operation.
This remains a targeted future concurrency test, not evidence of a
known failure. The existing synchronous test covers the gateway's
save/restore and lock wait; full-system runs cover successful operations
and multiprocess scheduling, but not forced within-RENAME interruption.

Keep existing filesystem ABI, error behavior and stack ownership.
Do not spend permanent RAM on instrumentation or wrappers. For further
KCORE reduction, prioritize the existing P0 strict-32K boot gate,
which remains failing. Current accepted linked GCC PDP-6 lowmem
baseline from v474: KCORE **8960**, KCORE+MRES **15507**,
PERMANENT_LAST **15554**. No production code change in v479.
