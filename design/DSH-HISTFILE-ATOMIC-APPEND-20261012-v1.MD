# DSH HISTFILE atomic append: D6FS write-order audit (v1)

Status: audited source paths, **not** an implemented transaction protocol.
Scope: DSH Phase 0.  Never infer crash durability from a successful return
or a serialized provider call without testing storage interruption boundaries.

## Current write path and its boundaries

1. `dsh_history_add` builds one S6REC of at most 31 words, opens the file
   with `O_APPEND`, and calls `u_write_words_all`. A short successful write
   can cause subsequent, independent append calls; each is serialized, but
   the complete S6REC is not necessarily one indivisible operation.
2. `d6fs_provider_write_words` reloads the FCB inside provider serialization
   and resolves `-1` to the current EOF. For growth it invokes
   `d6fs_provider_resize_fcb` with a private staged-growth marker.
3. `d6fs_provider_resize_fcb` can zero physical blocks, update freemap state,
   and modify staged extent descriptors. A new run may reserve eight or more
   blocks, exceeding the logical file growth. Staging avoids publication of
   the new logical size, **not** all physical allocation writes.
4. `d6fs_reader_write_words` copies payload into the shared transfer cache
   and commits each touched 128-word logical block separately via
   `d6fs_reader_commit_cache`. A bounded history record may straddle blocks.
5. Only after all payload words are reported written does the provider call
   `d6fs_reader_put_fcb`, which replaces one FCB in a cached 128-word FCB
   block and commits that block. Data and the size/extent metadata do not
   share one crash-atomic commit operation.
6. On a failed payload write the provider attempts to abort staged growth.
   The rollback itself can fail, leaving cleanup for offline recovery. A
   failed or interrupted FCB block commit may have an ambiguous outcome.

## Phase-zero invariant

After any interruption, replay or repair must expose either the previous
committed EOF or an entire valid new S6REC, never a partially published new
record. Allocations/extent metadata must remain consistent, or be recoverable
without guessing whether another writer committed the same record.

## Required implementation and acceptance gates

- Define a bounded record-level request that serializes each whole S6REC,
  rather than retrying separate append calls. The DSH record ceiling is 31
  words; avoid per-open or per-file resident transfer buffers.
- Audit physical freemap, block, FCB and superblock write ordering and the
  reader's exact cache flush semantics. Select a durable commit marker and
  recovery algorithm *after* checking these paths and available on-disk space.
- Return distinct committed / definitely-not-committed / indeterminate
  results from the record transaction. Never auto-retry an indeterminate
  record at a different EOF.
- Inject interruptions at every newly allocated block, freemap update,
  first/crossing/last payload block, FCB commit and recovery intent write;
  test both append within existing capacity and append requiring allocation.
- Reboot and verify retained prefix, whole-record visibility, no duplicated
  records, no leaked/shared allocations, and simultaneous independent writers.
- Keep fault and target integration tests in `daimos-testkit`. Do not mark
  Phase 0 closed based on host models or successful ordinary append alone.

## Implementation decision

Do **not** add a transaction-shaped wrapper around existing write_words:
its per-block cache commits and earlier allocation writes would remain
separate crash boundaries. A recoverable on-disk commit/replay mechanism must
precede the DSH-facing record API. The current DSH safe-prefix loader remains
unchanged until that mechanism has been tested.

## Second audit: backing writes and boot recovery (2026-10-12)

The write-through path is:

    d6fs_reader_commit_cache
      -> bcache_workspace_invalidate
      -> fs_backing_write
      -> configured storage callback (BLOCKSET / DSK270 / DRM236)
      -> bcache_store only on reported success

Each 128-word block is a separate backing write. There is no resident
write-back queue in this D6FS path. However, d6fs_provider_sync returns
success without a separate storage flush. Callback completion alone has not
been proven to enforce power-loss durability barriers or write ordering on
physical devices. A/B superblocks mark filesystem CLEAN/DIRTY, not records.

The boot superblock selector refuses a DIRTY generation; normal boot does
not replay an interrupted filesystem. A journal therefore requires boot
recovery before writable mount, including recovery of interrupted replay.

Superblock D6FS_SB_LOG_RESERVATION is configured as LOGSTORE storage and
must not be reused as filesystem transaction space. V2 does not define a
record transaction format. Any incompatible journal must be version-gated
to prevent older readers from interpreting transactional media as V2.

## Bounded undo candidate: testkit model, not implementation

DAIMOS-TESTKIT commit e971aeb (make test-daimos-d6fs-undo-cuts) passes
5,805 host C99 interruption checks. It records full 128-word before-images
of the old tail data, allocation bitmap, free summary and FCB block, writes
PREPARE, mutates media and writes COMMIT to the alternate header.
Recovery after PREPARE verifies before-images and restores them, then
publishes EMPTY. Interrupted recovery can be restarted. Scenarios cover
growth, existing allocated capacity and a block-aligned EOF. Corrupt
undo records and conflicting or missing markers are rejected.

Four images are NOT proven sufficient for every append. Growth can touch
multiple bitmap blocks; a bounded implementation must preflight all overwritten
blocks before any mutation and refuse an operation exceeding journal capacity.
The example digest is not a production-strength 36-bit integrity check.

An explicit test counterexample makes COMMIT persistent before the final
FCB write. The journal then accepts incomplete data as committed. Durable
write ordering / flush semantics are mandatory; software call ordering
alone is not enough. The current model cannot prove this hardware property.

### Implementation gates

1. Establish backing-write durability semantics for BLOCKSET, DSK270 and
   DRM236, including interrupted writes, failures and possible reordering.
2. Specify separately reserved versioned journal blocks, strong validation
   and the maximum possible preflight overwrite set.
3. Test idempotent boot replay including allocation reconciliation and
   interruption during recovery, before enabling writable mount.
4. Define DSH record return states: committed, definitely uncommitted and
   unknown; test serialization with multiple writers.
5. Measure permanent code and runtime RAM before deploying this protocol.
