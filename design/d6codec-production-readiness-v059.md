# D6 codec production readiness v059

Status: staged native decoder correction only. No production cutover.

The active system uses the D6LZ36 resumable decoder installed at 000060.
The v053 grouped-token codec decoder was complete-buffer-only and rejected
odd kind-2 halfword match distances, despite the broader format proposal.
It must not replace the live decoder or be used to produce boot images yet.

## First staged correction

`system/kernel/core/d6codec_decode_v059.s` adds an odd-distance halfword
copy path to the staged v053 decoder. It writes the high output half first
and then reads the second source half, preserving overlapping distance-one
semantics. No permanent data, heap buffer or scratch BSS is introduced.
Native DAS compilation: PASS; 115 text words, zero data and BSS. This
is not a native functional test or evidence of safe decoding.

## Mandatory validation before replacing anything

1. Put native tests in daimos-testkit for distances 1, 3, maximum supported,
   overlap, destination/source boundaries, truncated and malformed groups,
   input/output alias rejection and exact consumption. Compare with an
   independent host reference model. Fix all discovered bugs at source.
2. Implement the grouped-stream decoder as a resumable fixed-000060 engine,
   retaining caller state and using the existing 64-word VFS refill window.
   Group size is at most 31 words; compact incomplete groups in the window.
3. Validate compiler calling conventions, saved ACs, 18-bit address bounds,
   encoder capacity and full worst-case stack high-water with native tests.
4. Update ALL five Stage-1 loaders, kernel VFS/EXEC/PURE/TSFS consumers,
   host image and filesystem producers, host validation tools and userspace
   tools in a single new-format-only migration. Rebuild all encoded images.
5. Run exact-format corpus tests, 32K GCC KCORE/KINIT size/boot tests,
   corruption/restart tests and packed-process failure injection; retain
   disk-swap fallback until its own acceptance is complete.

The detailed producer/consumer inventory and cross-subsystem acceptance
criteria remain in `system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md` and
`TODO.MD` (v057 plan). Do not enable a mixed-format runtime or claim KCORE
savings from codec replacement before actual linked measurements.
