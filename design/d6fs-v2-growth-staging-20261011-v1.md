# D6FS V2 growth staging implementation notes (2026-10-11, v1)

## Existing source and safety invariant

`d6fs_provider_write_words` currently invokes `d6fs_provider_resize_fcb`
**before** `d6fs_reader_write_words`. The resize helper modifies extent
allocation, writes allocation maps, updates the FCB size in memory and calls
`d6fs_reader_put_fcb`, publishing the larger logical file before its payload
has reached storage. This contradicts section 17 of `pdp10-doc/KERNEL/D6FS.MD`.
`d6fs_file_block` uses extents only (not logical size) when mapping a write.
This makes a staged mapping possible without early size publication.

## Correct fix requires paired operations

The existing resize helper has many other callers: truncate, symlink create,
regular-file growth, directory growth, and namespace mutation. Therefore do
not globally defer its FCB publication. Instead introduce a *growth-only*
staging interface for regular-file writes, keeping existing resize semantics
for other callers until each is audited.

1. Keep the original FCB and decoded info available throughout staging and
   writing. Preserve original **allocated capacity**, not merely original
   logical size: extents reserve at least eight blocks and freeing from old
   logical EOF would accidentally release old reserved blocks.
2. Under `fs_provider_serialized`, reserve and zero any newly needed blocks,
   set their freemap ownership and build the *new extent mapping in memory*.
   Do not persist a changed FCB extent list or `size_words` yet.
3. Write payload using the staged mapping, checking the *full requested word
   count*. A short transfer or failed backing commit must not publish EOF.
4. Write through the final FCB once, with the new extent mapping, size and
   modification time. This is the first point at which additional bytes may
   become readable. Keep the existing one-block FCB publication property.
5. On any pre-publication failure, release only newly allocated blocks,
   restore the original FCB and decoded info in memory, and report failure.
   A block-commit error may already have changed data inside a previously
   allocated old tail block; this is **not crash-atomic record append**, and
   V2 cannot claim otherwise. If cleanup itself fails, surface the error and
   require fsck; do not silently treat it as completed.
6. If FCB publication fails ambiguously, do not reuse/free staging blocks
   until the last durable FCB generation can be determined. Avoid a cleanup
   operation that frees blocks referenced by a possibly committed FCB.

## Fault-injection acceptance

Exercise (a) growth solely inside preallocated capacity; (b) first extent
allocation; (c) extension of existing extent; (d) cross-block writes; (e)
allocation and data-write errors; (f) a failed FCB commit; (g) reboot after
each write boundary. Check old EOF before commit, new EOF after successful
commit, absence of reused live blocks, and d6fsck of all interrupted images.

## Memory and status

Reuse `fs_block_workspace` (128 words) and the provider's process-local
stack. Keep no resident per-file buffers. The rollback snapshot may need
transient stack space for an FCB (16 words) and decoded info (12 words):
measure before implementation. No format version change is needed for
correcting V2 write ordering, but it **does not** establish full crash-atomic
history records. The latter remains a separate V3 design gate.

**This file is an implementation specification, not a code fix.** Do not
mark the V2 source-order guard complete until staged mapping and rollback
are actually implemented and tested.
