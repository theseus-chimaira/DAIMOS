# TSFS 1.3 inline runtime v074 — inactive

The staged `tsfs_runtime_inline_v074.s` corrects the v070 experimental
runtime extent dispatch to accept `TSFS_EXT_D6INLINE=2`, matching the actual
v069 TSFS 1.3 producer and checker. A stored extent (0) remains supported;
legacy grouped/D6LZ36 flag 1 is rejected rather than fed into the inline
v070 decoder. The active production runtime (`tsfs_runtime.s`) is unchanged.

The testkit `tests/host/d6codec-tsfs-v074` uses the actual C99 TSFS block
reader to inspect a generated v069 image. It confirms that the producer
**actually emits** an extent tagged 2, and that the v069 checker accepts
it; the previous version-only compatibility test did not prove this.

LIMITATION: this does not constitute live PDP-6 TSFS runtime acceptance.
The transient TSFS mount metadata validator still needs an explicit
format-1.3 gate (the assembly runtime itself does not parse the volume
version); a full mount+read test and malformed-media rejection remain open.
The five Stage-1 boot tests and whole-KCORE 8K acceptance remain open.
No production format cutover or decoder optimization is authorized here.
