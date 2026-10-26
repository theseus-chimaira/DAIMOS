# Proposal B v425 — new provider transaction-ownership redesign

Status: NEW architectural candidate, source-audited only; no production
implementation or RAM saving claimed.

## Observed existing cost and coupling

`system/kernel/fs/fs_mres.s:fs_provider_reg_call` has a resident
serialized dispatch path for DTFS/D6FS/TSFS and a separate direct
MEMFS path. The serialized path saves seven request registers on the
process-private stack, waits on the single `fs_provider_ready` word,
restores the arguments, dispatches through the provider service slot,
then wakes waiters. This costs resident code plus up to seven temporary
kernel-stack words per blocked invocation; those stack words are not
permanent allocations and must not be counted as a permanent saving.

`fs/vfs.s` allocates a permanent 0200-octal (128-decimal) word
`fs_block_workspace`. DTFS, D6FS and TSFS depend on it. BCACHE slot 0
overlays the same workspace, with a tag invalidation protocol in
`storage/bcache_pdp6.s`. Direct users include storage and boot paths.
Consequently removing the lock without redesigning the workspace and
cache transaction protocol is unsafe. Three independent provider locks
alone would *not* be safe with the single shared workspace.

## Proposed architecture to measure (not an accepted patch)

Use an explicit *short-lived filesystem transfer transaction* that
owns the workspace and invalidates BCACHE slot 0 at the same boundary.
Separate persistent provider/mount state from transfer workspace state.
Make provider code that can sleep either suspend without holding the
workspace (saving only required small continuation state) or keep an
explicit workspace lease whose owner survives rescheduling. The latter
still needs serialization, so prefer the former **only after** proving
all callers can release the workspace before a potentially blocking
device operation. No release-before-sleep may expose an unfinished
write or half-published mount/provider state.

Candidate acceptance sequence:

1. Inventory each use of `fs_block_workspace` and all sleep points in
   DTFS/D6FS/TSFS and direct storage/BCACHE callers; classify borrowed
   versus owned transfer state and where it must remain stable.
2. Write a native PDP-6 concurrent provider/BCACHE test in
   `daimos-testkit` that interleaves calls while the device sleeps,
   including failure, rollback and remount; require a baseline pass.
3. Prototype one provider path in a reversible build, leaving the
   global lock intact until all shared mutable state is accounted for.
4. When all providers are converted, remove the now-unused seven-AC
   lock/wait/restore bridge and compare the **full post-fold link**,
   per-process stack high-water, dynamic RAM and elapsed I/O time.
5. Reject if total resident RAM grows or concurrency/boot tests fail.

The 128-word workspace is already *shared*, so counting it as a free
128-word saving would be incorrect. The plausible benefit is deleting
the resident serialization bridge and reducing stack pressure while
preserving a single transfer buffer. Savings are unmeasured; this is
an architectural experiment, not further single-word instruction churn.

Current accepted reference (v424): KCORE=8995,
KCORE+MRES=15543, PERMANENT_LAST=15590.
