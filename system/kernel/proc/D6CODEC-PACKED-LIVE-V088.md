# PDP-6 packed-RAM runtime development link v088

The `PACKED_RUNTIME_V088=1` build now links exact-size compression, MM-owned
packed extents, packed registry publication/retirement, native inline decode,
slot-0 restore dispatch, memory-pressure reclaim, and the PACKED-aware VM
teardown into one KCORE. Default `PACKED_RUNTIME_V088=0` preserves the v087
production disk-swap path and other codec consumers unchanged.

Crucially, the new resident v088 inline decoder is fully **relocatable** and
uses private labels. It does not overwrite or replace the legacy D6LZ36
fixed decoder at 000060, which remains necessary for Stage-1, EXEC and TSFS
1.1 until the separate media-format cutover. The v088 KPACK adapter invokes
the relocatable decoder through its own buffer frame.

The v088 swap-service assembly tries `proc_packed_try_evict_v088()` before
its existing DISK fallback, only if the reclaim loop selected a resident
victim. It computes the omitted executable PURE prefix from the unchanged
backing helper. The compressed path uses the real no-reclaim MM allocator;
the owner and storage discriminator change together at the commit point.
On a precommit failure, disk swap remains usable. A postcommit failure is
**not** allowed into disk swap: the transition remains held, and the
storage-state invariant branch halts rather than double-freeing the image.
The scheduler slot-0 service dispatches PACKED to native RAM restoration
and DISK to the existing backstore; the v084 VM destructor retires PACKED
MM ownership and executable backing without freeing a disk span.

Validation so far: native v088 relocated decoder + v088 packed public API
PASS with test transaction stubs; opt-in v088 full KCORE DXR link PASS; full
256K PDP-6 disk/auto-root simulator reaches `INIT V1` with the v088 reclaim,
restore and VM-destroy variants compiled in. These establish real linkage
and boot, **not** yet actual scheduler-triggered packing: a live memory
pressure/eviction/restoration test is still required. The user has requested
additional acceptance boots for a truly minimal 32K DECtape/TSFS system,
a minimal 32K single-disk/D6FS system, and a fully attached 256K PDP-6.
These are tracked in daimos-testkit v089. The 8K resident-size acceptance
limit is temporarily deferred, not removed.
