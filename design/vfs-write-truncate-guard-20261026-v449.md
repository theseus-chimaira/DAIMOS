# Proposal B v449: shared VFS TRUNCATE / WRITE safety guard

The resident VFS TRUNCATE and WRITE_WORDS paths duplicated read-only
mount enforcement, swapped executable backing protection and provider
dispatch. Both now use the same four-argument transient stack frame,
`vfs_write_guard_v449` and provider call continuation. Operation codes
remain distinct (TRUNCATE=13, WRITE_WORDS=16, decimal).

The PIPE fast path in WRITE_WORDS still bypasses vnode file checks.
The readonly and swapped-backing checks remain in the original order;
both see the same vnode in AC1. AC6 survives these guards. On success,
all four arguments are restored before provider dispatch. On failure,
the shared unwind returns -1. TRUNCATE now saves two additional
process-private stack words; no permanent allocation is introduced.

Full linked GCC PDP-6 lowmem: **KCORE 8972 -> 8966** (6 words saved),
KCORE+MRES 15519 -> 15513, PERMANENT_LAST 15566 -> 15560.

New testkit static shared-frame guard PASS; native PDP-6 provider-lock,
VFS STAT frame, FILE/PIPE direct mount and UMASK tests PASS. A full
TRUNCATE/WRITE syscall-path runtime regression remains outstanding.
Strict 32K acceptance remains deferred.
