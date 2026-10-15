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

## Native regression checkpoint v060 (2026-10-15)

`daimos-testkit/tests/validation/d6codec-native-v060/run-v060.sh` now
tests odd-distance halfword copies of distances one (overlapping) and
three (across two literal source words). Both pass on the PDP-6 simulator.
The runner also rebuilds and executes all 21 historical v048 decoding
fixtures against the v059 entry point, including malformed descriptor,
overrun, invalid kind, truncation, empty stream and even-halfword tests:
23/23 PASS. The existing v055 kernel ABI/golden-corpus regression also
passes (using the older v053 decoder).

The initial distance-one test failed due to an incorrectly emitted
decimal assembler descriptor; adding a leading octal zero fixed the
fixture without changing the decoder. No v059 implementation failure
was established by this test. Native v060 tests are **not** an end-to-end
streaming, full-boundary or kernel-ABI acceptance test; remaining checks
below still gate production use.

## Mandatory validation before replacing anything

## Group-boundary streaming prototype v061 (2026-10-15)

`system/kernel/core/d6codec_decode_group_v061.s` implements a staged
group-boundary resumable decoder without a static buffer or persistent
workspace. Every refill must supply one or more **complete** groups.
AC1/AC2 identify the compressed window; AC3 is the retained output cursor;
AC4 is the absolute exclusive output end; AC5 is the fixed output origin.
AC1 returns 0 for exact completion, +1 when an entire group was consumed
and further input is required, and -1 on malformed input. This is a new
experimental ABI, **not** a replacement for fixed address 000060.

Native DAS object size: 122 text words, zero static data/BSS. Each group
requires at most 46 compressed words (control + 15 paired descriptors +
30 literals), so the existing 64-word VFS stack refill window can hold a
complete group with headroom for carry; the VFS adapter must implement
incomplete-group compaction and must not hand an incomplete group to this
engine. The new core does not allocate or copy its own window.

`daimos-testkit/tests/validation/d6codec-native-v061/run-v061.sh`:
3/3 native PDP-6 fixtures PASS (two-group resume with overlapping odd
halfword copy; reject truncated descriptor group; reject trailing input
after exact output completion). This is **not** full ABI, refill, stage-1,
address-wrap, or stress acceptance; production D6LZ36 remains unchanged.

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
