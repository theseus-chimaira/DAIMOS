# KCORE v1519: direct DTFS mount must validate the physical native directory

Date: 2026-11-12. KCORE filesystem **feature correctness**, not MRES
optimization. Native DTFS media format unchanged. Root TSFS
critical-device loss semantics from v1515-v1517 are unaffected.

## Problem found in the kernel source sweep

The normal `MOUNT.DTFS` userland path invokes `DTFSPROBE`, which
reads the directory and checks native magic plus required reserved
allocation-map tags. The actual kernel `dtfs_mount_unit` did not
repeat those checks: once a direct `SYS_DTFS_MOUNT` caller supplied
syntactically valid flags and unit number, it could proceed to
`vfs_mount_prevalidated` and publish a mount without proving that
the currently attached physical DECtape was native DTFS.

Even if the caller first ran DTFSPROBE, the tape could have been
changed between userland validation and kernel publication. An
old clean cache entry on the same physical DTC unit/block does
not establish the contents of the *current* medium.

## Implemented production source

The new gate is placed at `dtfs_mount_unit_media_ready` in
`system/kernel/fs/dtfs_runtime.s`, before any VFS mount slot is
allocated and before the MM-backed media owner is patched.
For native format it:

1. Invalidates the two clean BCACHE identities without MM freeing.
2. Clears the single shared `dtfs_cache_mount` directory identity.
3. Reads physical DTC block 0144 directly through the MINIT-patched
   `dtfs_dtc_read_jump`, into the existing `fs_block_workspace`.
   This intentionally bypasses `dtfs_dtc_read` cached hits.
4. Refuses a negative physical transfer result.
5. Checks directory word 0177 against native magic `0446446632021`.
6. Checks allocation owner 036 for physical block 0 and block 0144.
7. Checks that the terminal three allocation owner tags are 035.
8. Only after all checks may `vfs_mount_prevalidated` publish a
   root mount, `dtfs_patch_media` publish its packed physical unit
   and owner, and `dtfs_cache_mount` remain invalid until the next
   actual directory load.

All these format fields already exist on every native DTFS image
created by the host `mkdtfs` or the userland `MKFS.DTFS`. The
same owner/magic checks exist in the short-lived userspace native
probe. This update does not change native on-tape format or the
MM-backed VFS/media/TSFS per-mount layouts, and does not introduce
a separate probe buffer, generation counter, quarantine state or
resident data. Disabled TENEX/ITS builds are unaffected; if their
optional foreign feature gates are enabled, the new checks only
run for the native media personality.

The directory validation is an admission guard: malformed or
unreadable physical media cannot be mounted directly by bypassing
userspace validation. It is **not** a unique-volume authentication
mechanism and cannot make previously open files safe after an
unreported physical tape exchange.

## Source-linked native and regression acceptance

Testkit `daimos-dtfs-mount-native-verify-v1519.{s,sh}` links the
actual PDP-6 `dtfs_mount_unit`, saved-register restore epilogue
and production BCACHE invalidate leaf. The only test-controlled
boundaries are the raw DTC read callback and VFS publication:
both represent the existing production interfaces. The test
verifies the physical unit/block/buffer register ABI, a valid
mount published exactly once, and six deliberate refusal cases:
wrong magic with valid stale shared workspace, bad block-zero
owner, bad directory owner, bad terminal native tags, physical
I/O failure, followed by successful fresh read after the original
tape becomes available again. Every refusal preserves the caller
output, leaves VFS/media ownership unpublished, clears stale
cache tags and forces physical I/O on another attempt.

Six executed source mutations are all rejected: bypassing physical
I/O, bypassing BCACHE invalidation, reversing magic, skipping either
reserved owner, and accepting absent native terminal owner tags.
Baseline and six negative PDP-6 runs PASS.

Existing v1518 DTFS I/O error/cache tests, v1490 BCACHE ABI and
refused-MM-free tests, DTFS chain-walk/create/unlink/rename/
directory fields and other source-linked DTFS regressions PASS.

Both isolated 256K KCC full and 32K GCC lowmem tape-root KINIT
with optional DTFS/TSFS MRES LINK. The separate strict **32K
fixed-KCORE size gate still FAILS** at `023241` octal against
the required `<020000` limit; this is not a successful 32K
deployment. Controlled old/new optional-MRES links use the same
fixed-KCORE map and other objects, with independent fold plans:

| Symbol | Before (octal) | After (octal) | Delta |
| --- | --- | --- | ---: |
| `dtfs_mount_unit` | `001620` | `001620` | 0 |
| `dtfs_lookup` | `001704` | `001734` | +24 words |
| `tsfs_mres_dispatch` | `002443` | `002476` | +27 words |
| `tsfs_root_owns_dtc_unit_v1516` | `002455` | `002510` | +27 words |

Net v1519 cost: **+27 optional DTFS/TSFS MRES instruction words,
+0 permanent data words, +0 fixed KCORE instruction words**.

## Remaining safety and feature gates

Native DTFS has common filesystem magic and allocation-map tags
but **no unique on-tape volume identifier**. Two separately
formatted DTFS tapes can pass this test. A future replacement-media
protocol must define a persistent identity with compatibility and
format migration considered, plus checked live FILE/vnode/dirty-data
rules. Revalidation must not turn old open handles into authority
to write to a physically replaced tape. Until that exists, v1519
does not claim online replacement recovery.

A tape that silently vanishes while requests hit valid cache entries
can still escape immediate detection. The v1517 Type-551 status
investigation did not provide a reliable actual hardware removal
signal; confirmed root TSFS member loss and v1515 fatal dispatch
remain P0. Non-root device-loss state and checked replacement are
still separate P1 tasks; neither is solved by mounting a valid
native directory or by retrying after a generic negative read.
