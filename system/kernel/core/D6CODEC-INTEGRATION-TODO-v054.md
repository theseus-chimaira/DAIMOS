# D6 codec v054 — complete integration TODO

**Status:** integration specification, NOT implemented. The v053 encoder and
 decoder are already staged as inactive DAIMOS assembler sources. Execute the
cutover in a **new isolated workspace**. Do not modify production KCORE or
its active build in this staging checkpoint.

## Required outcome and non-negotiable invariants

- [ ] Replace D6LZ36 with the new grouped-control, paired-18-bit-descriptor
  codec everywhere. **No permanent legacy decoder, format auto-detection,
  compatibility switch, or mixed-format images.** Keep the old system bootable
  only until the atomic change passes all regressions.
- [ ] Both **compressor and decompressor must be resident in KCORE**. The
  provisional 129+99 = 228 code-word baseline excludes wrappers and safety
  checks. Optimize combined **code + stack + persistent workspace**, not
  isolated object sizes. No additional permanent buffers without evidence.
- [ ] Defer shared encoder/decoder routines, unrelated refactoring, and
  generic streaming frameworks until after production integration.
- [ ] Preserve the existing disk-swap fallback until complete PACKED
  lifecycle, scheduler and restore regression acceptance. Replacing a codec
  does not imply enabling process packing or deleting swapping.
- [ ] Keep all tests in `daimos-testkit`, production documentation in
  `pdp10-doc` **after successful integration**, and update PROJECT-STATE.MD
  only after the corresponding integration milestone genuinely passes.

## Phase A — establish format and compatibility-free cutover

- [ ] Read `PROJECT-STATE.MD`, `TODO.MD`, the staged
  `D6CODEC-STAGING-v053.md`, native codec sources and relevant `pdp10-doc`
  design material before editing production paths.
- [ ] Record the current complete KCORE linked endpoint, object list,
  low-core memory map, pre-cutover boot results and all producer formats.
- [ ] Define one canonical, versioned compressed-stream specification:
  36-bit control word (token count 1..30 and matching bitmap), paired
  18-bit descriptors before literals, match kinds and lengths, distances,
  overlap semantics, maximum history, expected output size, exact consume,
  reserved flags, noncanonical padding, empty stream and failure behavior.
- [ ] Reconcile the current decoder's **even-distance-only kind-2** handling
  with any host producer supporting odd-distance halfword matches. Either
  restrict every producer to the native-supported set or implement and test
  full decoding; do not ship an unsupported descriptor.
- [ ] Decide whether format markers/versions belong in DXR, TSFS, boot or
  container headers. No runtime fallback to D6LZ36. Rebuild every stored
  artifact at cutover; reject stale/mismatched compressed artifacts.
- [ ] Establish a single host/native golden stream set and bit-exact tests;
  use the 81-DXR **host corpus** for aggregate regression, representative
  full native images instead of 81 redundant simulator tests.

## Phase B — production encoder and decoder ABI

- [ ] Keep both `system/kernel/core/d6codec_encode_v053.s` and
  `d6codec_decode_v053.s` resident and linked exactly once. Give the final
  production entry symbols stable names without conflating old and new
  calling conventions.
- [ ] Verify the actual PDP-6 compiler ABI and assembly call sites (not
  merely the testkit prototype). In particular preserve callee-saved
  AC10..AC15, maintain AC17 pushdown stack and define AC0..AC7 clobbers.
  Inspect generated compiler calls to confirm argument placement, pointer
  halfword representation, unsigned lengths and full-word -1 return.
- [ ] Define encoder arguments (source, source words, destination, capacity)
  and decoder arguments (compressed source, exact compressed words,
  destination, exact decompressed words); ensure source/destination are
  disjoint and pointer arithmetic cannot wrap 18-bit address space.
- [ ] Reject invalid pointers/lengths, truncated groups, invalid counts,
  lengths, kinds/distances, backreferences before output origin, output
  overflow, trailing input and partial/unknown output geometry. Specify
  what partial buffer modification on error means to every caller.
- [ ] Add an upper bound for compressed output sizing (including control
  and paired descriptors); avoid writing a control header if capacity is
  zero. Exercise exact-capacity and capacity-minus-one boundaries.
- [ ] Measure **wrapper-inclusive** encoder and decoder stack high-water
  separately on real DXR, incompressible, zero-heavy, overlapping-reference,
  capacity failure and malformed-input paths. The six-word v051 measurement
  covered one complete darc test, **not** all production call depths.
- [ ] Maintain zero codec BSS if possible; reject increases in persistent
  RAM without quantified net benefit. Record encoder, decoder, wrappers,
  stack peak, any scratch and compressed-data savings in 36-bit words.
- [ ] Make every caller treat decoded data as invalid until successful
  full-length, exact-consumption completion. Do not execute or publish a
  partially reconstructed image.

## Phase C — Stage-1, low-core and boot-image migration

- [ ] Replace `system/stand/pdp6/common/decompressor.inc` and its fixed
  low-core installation contract (000060) with a **new-format** decoder.
  Inspect relocation, decoder size, boot-stage scratch placement, KINIT
  target placement and overlaps; do not assume the current 99-word decoder
  is a drop-in for the old compact resumable fixed-address core.
- [ ] Update **all five** Stage-1 implementations and their Makefiles:
  `dsk`, `drm`, `dtc`, `mtc`, `ptr`. Review D6LZ36-specific control setup,
  refill mechanics, BLT installation, entry symbol, buffer geometry and
  exact output count. The v053 decoder expects a complete in-memory
  compressed stream, unlike the old resumable decoder.
- [ ] Update `system/boot/pdp6/Makefile` and the top-level `Makefile`
  (currently uses `D6LZ_HOST`, `IMAGE_COMPRESSED_WORDS`, `kinit.d6lz.words`,
  compressed DXR targets and installer manifests). Ensure KINIT and any
  compressed boot payload is produced using precisely the new format.
- [ ] Check the required low-core image agreement between Stage-1 and
  linked KCORE. If fixed address 000060 remains necessary, guarantee
  identical resident code/ABI and no overwrite of architectural vectors.
- [ ] Rebuild boot images from scratch and boot from each supported device.
  Include bad/truncated compressed KINIT image rejection, exact-size and
  memory-map consistency; don't deploy a boot format change that only
  passes native in-memory decoder tests.

## Phase D — resident runtime consumers and storage

- [ ] Replace the old VFS decoder adapter in
  `system/kernel/core/d6lz_pdp6.s`, and both `d6lz_fixed.s` / `d6lz.s`
  construction in `system/kernel/Makefile`. The current adapter includes a
  64-word VFS refill window and a resumable old-codec core; port or replace
  the **behavior**, not just the symbol names. Quantify stacked VFS frame
  plus codec stack and choose a bounded solution that actually fits KCORE.
- [ ] Adapt `system/kernel/proc/exec_load.s` for the new DXR payload format:
  framing, compressed input offsets/lengths, executable validation,
  protection of pure/text regions, publication only after success.
- [ ] Adapt `system/kernel/proc/proc_pure_backing_pdp6.s` and verify lazy
  PURE-page rereads and failure handling with new compressed backing.
- [ ] Adapt `system/kernel/fs/tsfs_runtime.s` for restart extents and
  immutable compressed contents; verify exact consume/decoded length,
  bounded memory and seek/refill behavior.
- [ ] Review `system/kernel/proc/proc_swap_pdp6.s` and every reference to
  `d6lz36_decode_vfs`, `d6lz36_decode_buffer` or old fixed decoder ABI.
- [ ] Replace the temporary identity implementation
  `system/kernel/core/kpack_codec_v1.c` (and its header/interface) with the
  **real** encoder/decompressor path when hooking up PACKED process state.
  Never claim compression on an identity-copy path; the allocator must
  reject nonshrinking results and maintain transactional ownership.
- [ ] Integrate with the existing inactive packed lifecycle code only after
  current PROJECT-STATE gates pass: registry, transition discipline,
  pin/free failure injection, scheduler suppression, eviction/restore
  rollback and disk-swap fallback. Avoid turning on a partially wired
  scheduler path.

## Phase E — migrate every format producer and checker

- [ ] Replace `tools/host/d6lz.c` and `d6lz-codec.c/.h` with a new-format
  host compressor/decompressor (C99), preserving determinism and exact
  36-bit packing. Update `tools/host/Makefile` accordingly.
- [ ] Update `tools/host/mktsfs.c`, `tsfscheck.c`, `tsfs-format.h` and any
  format marker, block-size, CRC, index or compression-capacity assumption.
- [ ] Update `userland/d6lz/d6lz.c`, its `userland/Makefile` wiring and
  `userland/tsfs/tsfs_scan.c` if they produce, decode, inspect or report
  compressed data. Rename user-visible tools and manual references as
  appropriate; don't leave a misleading D6LZ command that emits new codec
  bytes without an explicit migration decision.
- [ ] Audit generated `.d6lz.dxr` naming, KINIT compressed files, image
  manifests, TSFS images, filesystem installers, scripts and build caches;
  force regeneration to avoid old-format artifacts accidentally surviving.
- [ ] Verify every host producer's output decodes in the *native* PDP-6
  decoder, especially alignment, group boundaries, incomplete final
  descriptor pair and exact compressed-word count.

## Phase F — tests, benchmarks and release gates

- [ ] All new tests go in a **new versioned `daimos-testkit` directory**;
  keep shared testkit Makefiles and concurrent changes untouched. Extend
  targeted v048 corruption/guard tests with 18-bit address wrap, aliasing,
  near-end buffers, noncanonical descriptor padding and input-length fuzz.
- [ ] Run existing v045/v047/v048/v049/v051 native tests, plus independent
  host corpus parity/round-trip. Stress largest `dsh.dxr`, incompressible
  words, many control groups, zero-heavy and overlapping matches.
- [ ] Test compiler-generated calls and callee-saved ACs in the **linked**
  kernel. Validate exact stack peaks under nested VFS and syscall paths.
- [ ] Assemble/link the complete KCORE, report all changed text/data/BSS,
  endpoint and folding effects, and compare net resident RAM to baseline.
  Do not quote only the 228-word isolated assembler size as kernel growth.
- [ ] Full simulator boot smoke: all Stage-1 devices, KINIT, root selection,
  disk/TSFS filesystems, login, shell, EXEC of compressed applications,
  PURE backing page-in, invalid executable rejection, restart/extents,
  scheduler/packed tests and disk-swap fallback.
- [ ] Failure injection: corrupted compressed boot payload, truncated
  executable, short VFS read, malformed TSFS extent, no memory for packed
  payload, encoder insufficient capacity, decoder output mismatch,
  interrupted eviction/restore. No ownership leaks or partially published
  process state.
- [ ] Confirm every code path is new-format-only via repository search
  (D6LZ36 identifiers, `.d6lz` artifacts and old decode entry symbols),
  while distinguishing legitimate historical documentation.
- [ ] On **successful complete integration only**, update `pdp10-doc`,
  DAIMOS `PROJECT-STATE.MD`, `TODO.MD` and in-tree manual pages. Document
  measured resident RAM, speed, format, error behavior and acceptance tests.
- [ ] Produce versioned SHAR patch archives including git commit/push steps,
  commit as `Theseus <theseus@chimaira.net>`, push the integration branch,
  and provide a reproducible final report. Do not silently push changes
  into another agent's in-progress kernel checkout.

## Cutover order and rollback boundary

1. Implement/validate the new codec's native ABI, bounded stream behavior,
   host producer and unit tests **without activating the new image**.
2. In an isolated integration branch, switch the boot payload producer,
   Stage-1 decoder, KCORE resident routines, runtime consumers, and image
   builders together. Build **all** compressed artifacts anew.
3. Execute complete boot/EXEC/TSFS/packing failure-injection regressions.
4. Merge/deploy only after the new-format boot image and kernel agree.
   Rollback means selecting the **entire previous image/artifact set**, never
   a mixture of old and new compressed objects. There is no in-system
   dual-codec fallback.
5. After stabilization, separately examine optional shared encoder/decoder
   subroutines and optimize only if actual code+stack+time measurements
   demonstrate positive net resident savings.

## State of the staged sources

Staged commit `de85d60`: `d6codec_encode_v053.s` 129 text words,
`d6codec_decode_v053.s` 99 text words; no linked KCORE behavior changes.
These sources were assembled and passed full native PDP-6 darc roundtrip
before staging. Their existing calling convention clobbers callee-saved
registers: they are **not ready to link directly as C kernel functions**.
The separate experimental v052 wrapper was reverted in daimos-testkit;
future integration should validate its own ABI implementation rather than
assuming the reverted wrapper is still part of the tree.
