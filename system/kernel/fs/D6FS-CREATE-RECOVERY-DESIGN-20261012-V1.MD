# D6FS V2 create and publication recovery design — 2026-10-12 v1

Status: **design and audit only; no format or kernel implementation yet**.
Phase-one crash-atomic HISTFILE remains blocked on a recoverable commit mechanism.

## Existing sequence and proven problem

`d6fs_provider_create_object` first chooses a free FCB, initializes a 16-word
FCB in its existing stack workspace, and calls `d6fs_reader_put_fcb`.
Only after success (and any symlink payload work) does it publish a directory
entry with `d6fs_provider_write_dirent`. A negative FCB write result branches
directly to `d6fs_create_fail`. Thus a fully written FCB with an ambiguous
error response becomes an unreachable object, as observed in the testkit.
The fault image contained unreferenced type-1 zero-length FCBs 0451/0452;
the no-fault control did not.

`d6fs_reader_commit_cache` invalidates the shared clean BCACHE tag before a
backing write and only restores it on reported success. This prevents the
old cached contents from being silently treated as durable after failure,
but does not establish media durability and does not make two writes atomic.

## Recovery contract

1. FCB allocation is an **uncommitted intent** until a recoverable directory
   reference or equivalent transaction commit record exists. Do not expose
   an unreferenced intent as a committed file.
2. Never reuse an FCB or free any extent when the latest durable reference
   state is uncertain. An I/O return code does not establish whether an
   FCB, directory entry, allocation bitmap or commit record reached media.
3. Make recovery idempotent. Replaying a committed intent must not create a
   duplicate name/FCB; discarding an uncommitted intent must not free blocks
   referenced by a committed directory entry.
4. On boot, replay/resolve incomplete intents **before writable mounts**.
   Merely reporting a superblock CLEAN flag does not prove absence of
   unreachable FCBs.
5. The published commit decision must survive power loss independently of
   the caller's return value. An immediate readback may aid diagnosis but
   is not an atomic commit marker and cannot replace crash recovery.
6. Normal creates, symlinks, unlink, rename, and HISTFILE append must have
   explicitly defined interaction with the same recovery mechanism; avoid
   a special-case HISTORY workaround.

## Candidate low-RAM design, subject to format and ordering audit

Use a bounded **single in-flight metadata intent** serialized under the
existing D6FS provider lock. Store durable intent/commit state in reserved
filesystem space with a sequence and checksum, using an A/B selection rule
so a torn update cannot be mistaken for committed metadata. An intent names
the FCB index, parent, name/dirent identity, and any affected extent/map
state. Transaction stages must be ordered: durable intent, prepare new FCB
and data, publish directory entry, durable commit state, retire intent.
The precise sequence must be established by modeling *all* metadata writes
and failure points; the order listed here is **not yet proven safe** for the
existing in-place directory-block update. In particular, interrupted
overwrite of a directory or bitmap block may require COW or equivalent
before the protocol can be crash-safe.

One global in-flight transaction would avoid per-file or per-open resident
buffers, but can serialize metadata operations and must not hold a lock
across process sleep. The existing 128-word shared block workspace is the
budget. On-disk reserved space and boot recovery code size must be measured.
No new format, implementation, or RAM cost is approved by this note.

## Required acceptance before enabling

- Identify target FCB index and hit count for post-callback error injection.
- Capture a clean no-fault control and a faulted post-callback image.
- Power-cut at each relevant FCB, bitmap, data, directory and commit write.
- Boot-replay cloned disks, run read-only `d6fsck`, and verify no orphan,
  duplicate name, lost committed data, extent reuse or false CLEAN state.
- Run concurrent creators and HISTFILE writers; verify exactly-once outcome
  or explicitly reported *unknown* status without unsafe automatic retry.
- Run full PDP-6 SIMH and testkit regressions and measure resident words.

## Write-order audit addendum (2026-10-12)

The current `d6fs_provider_write_dirent` path has **multiple** publication
boundaries, not just child-FCB and dirent. If a slot extends beyond the old
directory EOF, `d6fs_provider_resize_fcb` stages growth and can update
extent/allocation metadata. It then writes six directory-entry words using
`d6fs_reader_write_words`, followed by an mtime update through
`d6fs_reader_put_fcb`. An existing reusable directory slot can take a
shorter path. A symlink additionally has data and extent writes before
entry publication. Therefore every test scenario needs both an existing-slot
and an extend-directory case, as well as normal-file versus symlink creation.

Testkit `daimos-d6fs-create-write-order-model-20261012-v1.py` (`adf1e65`)
enumerates conservative crash prefixes. It demonstrates that an intent
record without replay is insufficient, and an additional commit bit cannot
undo an already durable unreferenced FCB. The model deliberately excludes
sector tearing and device write reordering, so it is **not** a proof of
transaction correctness. The on-disk format and recovery choices remain
unimplemented. No extra resident workspace is approved.

## Two-copy record selector experiment (2026-10-12)

Testkit commit `ac9495d` prototypes a host-only fixed-size intent selector
with two independent 128-word slots. All 129 word-prefix interruptions of
one slot, with the other intact, and a same-sequence conflict case passed.
The 12-word record schema is experimental; its rotation/XOR digest is too
weak to deploy, and this test does not model word tearing, storage ordering,
flush durability, simultaneous failure of both copies, or replay. It proves
no safety of the existing D6FS create protocol and reserves no disk blocks.
Before integration, evaluate a suitable 36-bit integrity scheme and define
how to persist/advance the intent without losing the only recoverable copy.
