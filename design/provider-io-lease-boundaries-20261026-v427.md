# Proposal B v427: I/O-buffer lease boundaries

Status: existing production remains unchanged; focused source-path guard
and native provider-lock baseline PASS. No memory reduction claimed.

The global serialized provider call cannot be shortened merely by
unlocking immediately before device I/O. Concrete examples:

- D6FS `d6fs_reader_get_block` supplies `fs_block_workspace` to
  `fs_backing_read`, then publishes the returned contents into BCACHE.
- D6FS `d6fs_reader_commit_cache` gives the same buffer to
  `fs_backing_write`, then retags the successful write-through.
- TSFS `tsfs_table_record` and word-read paths pass the buffer to
  `dtfs_dtc_read`; they interpret its contents after the call returns.
- DTFS uses the buffer with `dtfs_dtc_write`; `dtfs_dtc_read` runs
  through `dtfs_dtc_read_jump` and then `bcache_store`.
- DSK, DRM and tape physical drivers all contain `proc_wait_event`.

Therefore the buffer is live across potentially sleeping calls. A
second request cannot borrow the same physical block while the first
is pending. Nor is early release safe between I/O completion and cache
publication or before the consumer finishes decoding the transferred
record. A per-provider lock would not fix the inter-provider collision.

New testkit source guard `daimos-provider-io-lifetime-v427.py` PASS:
asserts those producer/consumer boundaries and the driver wait paths.
Prior native lock wait/retry test v327 and inventory v426 also PASS.

**Next concrete experiment:** inventory the read-only consumers that can
copy just the selected record to process-private stack storage after
the transfer completes. If this permits releasing a workspace lease
before the remainder of computation, prototype one isolated call path
and measure total stack high-water plus the post-fold linked image.
The alternative of private in-flight 128-word buffers must be costed
for worst-case concurrent processes and should be rejected unless the
removed resident bridge outweighs their *physical* RAM usage. Retain
the global lock until complete coverage and native interleaving tests.

Current reference: KCORE 8995, KCORE+MRES 15543,
PERMANENT_LAST 15590. The strict 32K acceptance remains deferred.
