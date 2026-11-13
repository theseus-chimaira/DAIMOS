# KCORE P0-B v1552: pinned MM scratch for native DTFS write intent

Date: 2026-11-13. Tested standalone provider-lifecycle building
block; **not yet enabled in the production DTFS MRES**.

## Source and correctness constraints

`system/kernel/fs/dtfs_intent_owned_arm_v1552.s` now wraps the
v1551 raw dirty-before-data service with an MM-owned physical
buffer. The entry accepts AC1=Type-551 unit, AC2/3=expected
nonzero 72-bit ID halves, and AC4=pointer to caller-retained
scratch base initialized to zero. The caller holds the exclusive
DTFS provider lease. It must refuse a new operation whenever
that base is nonzero from a previous incomplete cleanup.

The wrapper allocates exactly 0200 contiguous words using the
production `mm_alloc_aligned_noreclaim` six-argument interface,
MM_TYPE_KERNEL_DYNAMIC=3, MM_ALLOC_HIGH=1, and owner 015 octal.
The low kernel MM-owner ledger already uses owners through 014
(display text); 015 is distinct from the existing BCACHE, DTFS
directory, MEMFS, D6FS, exec, PIPE and BADMAP owners.

The temporary extent is pinned by `mm_pin` **before** calling
the complete v1551 Type-551 WRITE_INTENT RBN0 physical write
and fresh readback. This is mandatory because raw controller
DMA may sleep while MM compaction can move an unpinned extent.
After the actual arm returns, the wrapper unpins and frees
the original extent with exact type and owner. The original
arming result is returned only if that release fully succeeded,
and the caller's basep is cleared only when `mm_free` confirmed
the ownership was relinquished.

No failed allocation, pin or cleanup is converted to success.
A refused pin releases only an unpinned extent; a refused unpin
or MM free retains the exact still-owned physical base in the
caller's provided word and returns -1. Reentry with that pointer
nonzero refuses instead of losing ownership or allocating another
extent. A completed dirty physical marker with a refused scratch
release is still reported as a failed transaction and may only
be reconciled by the eventual exclusive owner-aware provider.

No permanent DATA/BSS memory, new syscall ABI, media format or
KCORE fixed instruction is added by this standalone wrapper.
At most one 128-word scratch per independently serialized
in-flight provider operation is allocated from managed high
memory; it is released on all accepted success/failure paths.

## Tests

Testkit `daimos-dtfs-owned-scratch-v1552.sh` executes the exact
wrapper instructions on PDP-6 under strict MM boundary stubs:
11 allocation/pin/unpin/free/error/retention cases PASS.
Ten actual-source unsafe mutations are rejected, including
omitted pin/unpin/free, wrong MM owner or size, missing physical
arm, losing a handle after cleanup refusal, and bypassing a
necessary pin verification.

Testkit `daimos-dtfs-owned-scratch-real-mm-v1552.sh` extracts
and executes **unchanged actual MM allocator/find-fit/extent,
pin/unpin and free source** together with the actual wrapper,
using a physical managed arena. It allocates/pins/frees the
0200-word buffer twice, checks the native descriptor type,
owner 015, exact size and pin state during the simulated I/O,
verifies both allocations leave no extent, and then checks a
real no-free-arena shortage refuses to begin I/O. PASS.
Only the lower physical Type-551 service is substituted in this
real-MM test; independent native v1551 already exercises the
unmodified real intent service with controlled physical I/O.

## Remaining P0-B feature gates

**The wrapper is deliberately not linked into production DTFS.**
The provider still lacks a per-mounted-volume MM-owned scratch
handle, DTVOL2/CLEAN admission, exactly-once first-write intent
barrier and exclusive owner-safe CLEAN/unmount transaction.
The persistent handle must remain reachable if MM cleanup fails,
and no alternate DTFS data/directory write may bypass an unverified
WRITE_INTENT. Only after the real provider is integrated may the
full Type-551 partial write, reinsertion and cold-boot matrix be
run and P0-B considered complete.

See testkit `tests/validation/daimos-dtfs-owned-scratch-v1552.md`.
P0-A complete, P0-B open, P1-C untouched. VERSION 0.5 and
strict 32K/optimization HOLD unchanged.
