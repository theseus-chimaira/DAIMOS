# v1660: bounded PURE bitmap FULL decode preflight (test-only)

Date: 2026-11-18. Parent: v1659 commit
`ad64849f1bc6c416905533c0e90f2409616f4552`.

## Source audit before changing mode3

The current permanent storage-mode ASM checks and publishes only
0=RESIDENT, 1=DISK, 2=RAW PACKED; it rejects mode3. This is a
necessary safety barrier: the permanent `pdp6_pure_detach_v1609`
expects an exact RAW suffix length, and the v1384 PREFIX restore
requires an in-place RAW extent. The existing v1391 FULL restore
copies RAW data after successful PURE loading. Neither path can
handle an encoded bitmap. The old `proc_packed_v1` APIs are staged,
not connected to this fixed-KCORE terminal path. Simply permitting
mode3 in `proc_storage_runtime_v30.s` would allow unhandled EXIT.

## Bounded decoder proof

The new opt-in `proc_pure_bitmap_decode_v1660.c` performs a complete
two-pass decode. First, with no output writes, it validates disjoint
physical geometry, exact encoded length, every mask, every present
nonzero literal, short final-mask padding, and complete stream
consumption. Only then does it invoke the original v1451 PDP-6
30-word decoder, whose assembly has no bounds checks. Its ABI is
AC1=source, AC3=destination, AC4=decoded words; the C call explicitly
passes an unused AC2 argument. No scratch RAW-sized buffer is used.

The preflight is a format/geometry check, NOT a replacement for the
still-required MM exact owner/type/pin scan. Caller must hold
TRANSITION and two disjoint pinned, source-authenticated MM extents.
No permanent kernel or MRES linkage and no process-mode publication.

The actual PDP-6 KCC/native test executes a 73-word RAW image with
three nonzero words across two full masks and a final one-word mask:
three masks plus three literals, six encoded words. It validates
exact output and an untouched destination after malformed literal,
poisoned trailing mask, extra presence bit, truncated stream,
trailing encoded words and overlapping source/destination. An
independent compiled source mutant removes the exact-consumption
check; its native oracle correctly rejects the changed behavior.
The v1659 original MM allocator and bitmap baseline is also run.
The unlinked KCC PDP-6 decoder preflight object is 144 TEXT words,
zero DATA and zero BSS. The unchanged shared codec is 30 TEXT words.
These are not installed MRES or KCORE costs; do not count a net saving.

## Unfinished mode3 transaction

To publish compressed storage, first implement a fully source-linked
terminal path accepting a single exact compressed owner and verified
encoded length; implement bounded FULL restore and all PI6/MM
uncertainty and rollback cases. Commit must never free the sole RAW
owner before a recoverable compressed owner and terminal policy are
guaranteed. Reuse the existing two-bit map, with no new permanent
registry. Only after terminal/restore coverage may mode3 be admitted
by the permanent runtime. No 48K/64K compressed physical guest or
net RAM savings are claimed; PURE remains OFF by default and strict
32K acceptance remains deferred.
