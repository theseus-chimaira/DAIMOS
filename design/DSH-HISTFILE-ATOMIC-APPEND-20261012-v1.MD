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
