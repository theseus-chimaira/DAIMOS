# D6FS append commit: format and recovery decision (2026-10-11, v1)

## Constraints verified in source

- D6FS V2 explicitly excludes data/metadata journaling, checksums, and
  continuation FCBs (`pdp10-doc/KERNEL/D6FS.MD`, sections 2, 17, 24).
- V2 section 17 requires allocation, complete data writes, allocation-map
  publication, then FCB extent/size publication. Present source contradicts
  this: `d6fs_provider_write_words` invokes `d6fs_provider_resize_fcb`,
  whose `d6fs_resize_publish` calls `d6fs_reader_put_fcb`, before
  `d6fs_reader_write_words` writes the payload.
- FCBs have seven inline extents; writes use a single shared 128-word transfer
  workspace and synchronous block I/O. The A/B superblock records filesystem
  CLEAN/DIRTY state, not a file transaction. `d6fsck` rebuilds free-map and
  summaries and can reclaim uniquely owned orphan FCBs, but has no record
  commit journal or record checksum.

## Two separate engineering tasks

1. **Correct D6FS V2 ordered growth**: stage/initialize data and allocations
   before publishing a new readable FCB size, preserving the prior FCB on
   detected errors. Add simulated power cuts at allocation, data, map and
   FCB transitions. This restores the documented V2 ordering but does **not**
   make appends fully crash-atomic (including writes inside existing blocks).
2. **Crash-atomic HISTFILE records**: requires an independently recoverable
   per-record commit protocol, not just moving an FCB write. Do not add a
   journal or checksum to frozen V2 without an explicit version transition.

## Candidate V3 bounded transaction contract (not yet accepted or coded)

- Version-gated feature: older V2 readers must reject transactional media,
  not interpret new metadata as ordinary file contents.
- Support records of at most 31 words and no more than one file per operation.
  All writers acquire the existing serialized filesystem provider entry.
- Keep the old committed file extent/size reachable until a new snapshot is
  fully initialized. Allocate shadow blocks for every modified data block;
  never overwrite the old committed partial tail block in place.
- Persist new data, map ownership, and a self-validating descriptor referencing
  the new root/FCB state. The active commit selector needs a defined whole-
  block durable-write guarantee and redundant generations; do not assume a
  lone 36-bit word store is power-loss atomic.
- On mount, select a fully verified committed generation; report and reclaim
  unreachable staging allocations offline. A failed commit may be
  *uncommitted* or *outcome unknown*; DSH must not automatically retry an
  ambiguous write or it may duplicate a command.
- Prove correctness under failure after each backing write, after each cache
  invalidation, and during publication, including multiwriter reboot recovery.

## Size/complexity gate

Before implementation, measure kernel resident words, request dispatch
cost, disk metadata and recovery complexity. A whole-file COW design can
consume extra extent entries; seven inline extents may make it infeasible.
Avoid a general-purpose journal added solely for DSH unless the cost is
justified. Keep code and tests separate: DAIMOS runtime, daimos-testkit
fault injection, and completed design documentation in pdp10-doc.

**Status:** design investigation complete; no protocol selected, no crash-
atomicity claim, no disk format or kernel mutation in this change.
