# v120 — Native PDP-6 successful reclaim and descriptor saturation

The versioned daimos-testkit `tests/native-kernel/mm/pressure-v120` suite
executes the unmodified production MM PDP-6 assembler under p10run. Only
external FS/swap reclaim services are instrumented callbacks.

**PASS:** A first failed allocation invokes `fs_memory_reclaim`, which frees
an actually allocated extent through the production `mm_free` entry point.
The ordinary allocator then retries and succeeds at the freed base, with a
consistent descriptor count, exactly one FS callback and no swap callback.
The original v119 failure-only and no-reclaim callback tests remain active.

**PASS:** A second test constructs 32 actual sorted, valid, one-word extents
with gaps in a managed physical arena and requests a further allocation.
Although physical space is available, the descriptor table is full.
`mm_alloc_aligned` returns MM_ERR_DESCRIPTORS (-3), leaves the output and
extent count unchanged, and performs its bounded pressure callbacks: two
FS and one swap. This retry policy is observable behavior, not a claimed
optimization. A follow-up may consider early descriptor exhaustion return
only after the fixed-vs-boot-sized table design is settled.

The first saturation fixture was invalid (count 32 with only one actual
descriptor); it was replaced with a fully initialized 32-entry table.
The first assertion expecting no reclaim on descriptor exhaustion was also
inconsistent with current `mm_alloc_aligned` semantics; the accepted test
checks the actual bounded retry sequence rather than silently changing code.

This suite does not exercise the production filesystem reclaimer or process
swapper, full scheduler concurrency or complete OS boot. Production MM is
unchanged at 482 text + 70 BSS PDP-6 words. No BLT variant was introduced.
