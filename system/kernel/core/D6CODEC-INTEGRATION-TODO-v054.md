# v1464 production decision (2026-11-10)

The explicit request to delete D6LZ36 supersedes the historical v1395
deferral below. The canonical v1421 block codec now replaces D6LZ36 in
Stage-1, KCORE, EXEC/PURE, TSFS and all host executable producers.
The obsolete decoder, encoder and userland utility have been removed;
there is no mixed-format runtime fallback. The new source-matched v1464
host build and v1461 TSFS producer/checker are selected. The native
fixed-core/VFS tests, five Stage-1 assemblies, and 256K DSK/D6FS ROOT,
DSH and compressed external ECHO have passed; TSFS cold boot, complete
physical pressure and 32K/48K memory gates remain pending. The older
design and deferral notes below are preserved as historical context.

## 2026-11-08 v1371 full normal D6FS distribution checkpoint

The full normal D6FS Makefile manifest is now booted in an isolated
native grouped KCORE/KINIT environment: 237 files, 29 directories,
89 executable inputs, with all 71 prior `.d6lz.dxr` programs
converted and independently decoded as grouped DXR2. The resulting
336-FCB CLEAN D6FS root boots to INIT, grouped LOGIN/DSH and native
guest ECHO/LS/CAT/GREP/WC/STAT; EXIT respawns LOGIN. This is true
real-provider boot testing, not a small three-executable test root.
Installed native toolchains have not yet run, and the included
uncompressed D6LZ tool still *writes* obsolete D6LZ36; it must be
replaced or removed as part of the synchronized format cutover.
Physical PURE/swap, TSFS root, nested stack, media fault/recovery and
atomic production switchover remain unaccepted. There is no change
to production D6LZ36 or the existing KCORE memory budget.
See `design/d6codec-full-distribution-v1371.md`.

## 2026-11-08 v1370 complete grouped minimal-system native boot

The full testkit `d6codec-fullboot-v1370` runs grouped Stage-1 and
an actually grouped KCORE (using non-executable legacy low-core
reservation), all 27 re-linked MRES packages, an actual DSK270/D6FS
root and real INIT -> grouped LOGIN -> grouped DSH -> grouped ECHO.
The ECHO output and LOGIN respawn are verified interactively under
PDP-6 SIMH. The negative control uses old D6LZ36 KCORE and fails
to RUN grouped LOGIN as expected. Each boot uses independent pristine
media; the normal D6FS newer DIRTY superblock is never suppressed.
Only a small 15-node root is exercised; full distribution and all
runtime TSFS/PURE/swap/stack/failure gates remain to be completed.
The compiled production DAIMOS KCORE/host producers/media are still
unchanged. See `design/d6codec-fullboot-v1370.md`.

## 2026-11-08 v1369 functional grouped runtime checkpoint

Versioned test-only grouped DXR2 writer now preserves original
executable metadata and relocation bitmaps. Native PDP-6 actual
EXEC and PURE-reload code links to the staged grouped VFS adapter;
compiled marker execution, 390-word linked DXR short refills,
corrupt streams and provider-failure rollback PASS. Actual TSFS
read/copy/free integration decodes a one-block grouped restart
and frees transient memory on valid and invalid input. These tests
use deterministic VFS/VM stubs, not real boot-media providers.
Full grouped KCORE/KINIT/root filesystem cold boot and synchronous
host/runtime production cutover remain OPEN. No D6LZ36 changes.

# D6 codec v054 — complete integration TODO

**2026-11-10 v1395 scheduling supersession:** The grouped D6 codec format
is under discussion. Defer its production cutover until AUXSTORE/BACKSTORE,
restricted raw MEMFS XIP and swapless PURE RAM reclamation are all finished
and accepted. This is a sequencing change, not abandonment of the codec or
permission to mix formats. Existing D6LZ36 producers/loaders stay unchanged;
plain PURE_PREFIX and bounded legacy FULL restoration may be developed in
the meantime, without adding a second permanent decoder. Compressed
PURE_PREFIX remains unsupported pending a selected bounded codec.

**Status:** integration specification, NOT implemented. The v053 encoder and
 decoder are already staged as inactive DAIMOS assembler sources. Execute the
cutover in a **new isolated workspace**. Do not modify production KCORE or
its active build in this staging checkpoint.

## Required outcome and non-negotiable invariants

- [ ] Replace D6LZ36 with the new grouped-control, paired-18-bit-descriptor
  codec everywhere. **No permanent legacy decoder, format auto-detection,
  compatibility switch, or mixed-format images.** Keep the old system bootable
  only until the atomic change passes all regressions.
- [ ] The runtime decompressor must be available to active KCORE consumers.
  A native encoder is needed only for the separately admitted **swapless
  opt-in mutable-RAM compression** feature; it is not a prerequisite to
  the basic executable/boot codec cutover. Do not install an unused encoder
  in swap-enabled KCORE. Optimize linked code + stack + workspace together.
- [ ] Defer shared encoder/decoder routines, unrelated refactoring, and
  generic streaming frameworks until after production integration.
- [ ] Preserve existing **raw** disk swapping. Compressed disk-swap records
  are explicitly cancelled (v1375). The independently planned swapless
  PURE-dropping and opt-in mutable-RAM compression must not gate the
  executable/boot grouped-codec conversion.
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

- [ ] Link `d6codec_decode_v053.s` exactly once in the required runtime
  paths. Retain `d6codec_encode_v053.s` as separately qualified code for
  swapless-only opt-in RAM compression, not mandatory KCORE residency.
  Give final entry symbols stable names and retain the ABI.
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

**v1363 design correction:** Every current Stage-1 loader already
stages the entire compressed KINIT payload. A resumable grouped
decoder is therefore not required *inside Stage-1*. Test-only v1363
variants build and exercise the smaller direct-linked full-buffer
v053 decoder without BLT copying it into the old `000060` slot.
Runtime VFS still needs resumable refill handling. No production
Stage-1 or KINIT format cutover has occurred.

**v1364 staging geometry gate:** A dedicated PDP-6 assembler helper
and test-only Stage-1 alternatives now validate decoded KINIT plus
stack reserve and decoded KINIT plus compressed staging input against
the configured physical core limit, before the first payload store.
Twelve native header-guard tests and the independent helper tests
PASS; PTR also rejects an overlong first-tape segment before copying.
No production image or decoder selection has been changed. Physical
device boot and synchronized runtime cutover are still OPEN.

**v1365 actual staged boot acceptance:** All five media paths now
run the staged guarded direct-linked grouped decoder on a complete
real KINIT: DSK270, DRM236, DTC, MTC and two-volume PTR (5/5
KINIT-entry PASS). Two corrupt DECtape image cases halt before KINIT
without writing 030000. These proofs stop at KINIT entry and do not
activate grouped runtime consumers. The original D6LZ36 production
images, callers and KCORE residence remain unchanged.

**v1366 staged VFS runtime:** Versioned PDP-6 grouped VFS/buffer
entries now handle exact input and trusted immutable AC3=0 backing.
The 32-word VFS input window plus 26-word state gives a 68-word
temporary frame. Native fake-provider and leaf-provider stack tests
PASS, but real nested provider stack remains unmeasured. New decoder
and frontend are 237+96=333 text words, compared with legacy 40+57=97.
The resulting +236 resident text words are a RAM-first cutover blocker
until reduced or justified by full linked KCORE measurements. No
production D6LZ36 interface, boot image, producer or consumer changes.

**v1367 bounded native RAM optimization:** The staged grouped engine
and VFS adapter now total 225+92=317 text words, reducing v1366 by
16 words, with 25 continuation state words and a 67-word temporary
VFS frame. The native fake-provider, max-descriptor, refill corpus
and leaf-stack canary acceptance use the versioned v1367 testkit.
The codec still costs +220 resident text words against legacy; full
KCORE link/cold-provider stack and synchronized format cutover remain
OPEN. No production format or resident memory has changed.

**v1368 temporary full-KCORE link:** The exact production link
rebuild (9,601 text/269 BSS) matches byte-for-byte. Replacing the
legacy VFS frontend with staged grouped v1367 yields 9,817 text
(+216) when the fixed-lowcore slot is reclaimed, 9,857 (+256) with
its old physical range reserved. Five real MRES packages relocate
cleanly against either test map with unchanged resident sizes. RIM
and initialized KCORE stream generation PASS; the resulting image
has NOT been run through KINIT or native cold VFS. The low-core
relocation and nested provider stack remain release blockers.


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
- [ ] For swapless PURE in-place RESTORE, support rebuilding only the
  omitted immutable prefix without writing the retained mutable RAM.
  A single geometry word can encode uncompressed text size (LH18) and
  live mutable suffix size (RH18), but these are already derivable from
  DXR2 TX2, PURE backing and process VM length. The old D6LZ core
  rejects partial MATCH tokens crossing a requested output limit, and
  ordinary compressed reload overwrites all initialized DATA. Choose a
  bounded-prefix grouped decoder or independently framed text segment,
  with native boundary-crossing tests, as part of the atomic cutover.
  Do not add a permanent size word or retain legacy codec support
  solely to solve this problem.

## v1386-v1388 shared executable parser staging (not linked)

- [x] Native standalone DXR1/2 FULL and RAW PURE_PREFIX parse/read
  engine implemented and tested; existing D6LZ36 compressed FULL
  delegation retained, but compressed PURE_PREFIX refuses safely.
- [x] Native real PURE MM restore and a generated disposable EXEC
  caller both use this common parser/reader and PASS focused tests.
- [ ] Grouped-codec bounded-prefix cutover, legacy full-reload
  unification, and permanent-code size reduction still required.
  The current isolated EXEC FULL refactor adds 105 pre-fold words:
  do not link the candidate into fixed KCORE without net RAM benefit
  and full 32K/48K guest acceptance.

## v1389/v1390: ONE common FULL/PREFIX parser, reduced, still cost gated

- [x] BLT register-save optimization: common DXR module 225 -> 212
  target text words, zero static RAM. v1386-v1388 native tests PASS.
- [x] Real native v1390 FULL callback reuses that same parser/loader:
  plain/compressed DXR2, mount 18 and rollback PASS; disk-swap
  production code unchanged. v1390 size regression reports candidate
  EXEC plus FULL replacement +98 pre-fold text words.
- [ ] Before any production cutover, add an explicit full destination
  length to the loader contract and reject oversized initialized
  images before decoding; native oversized mutation tests required.
  The grouped codec must independently support bounded PURE_PREFIX
  output, including cross-boundary MATCH tokens.

## v1391/v1392: bounded FULL callback accepted in native test only

- [x] Introduce v1391 four-argument FULL callback carrying the real MM
  APR capacity and reject oversize before plain/compressed payload.
  Native exact-fit, corrupt, high-mount and oversized tests PASS.
  The unsafe three-argument shared FULL adapter is removed.
- [x] Resolve the independent KCC symbolic ADDI folding bug at the
  optimizer source. The v1392 PDP-6 fixed-slot regression PASSes;
  the full compiler matrix and a small old-failing repro remain open.
- [ ] Before production link, reduce the v1391 common DXR and FULL
  transaction cost: 235 TEXT in the common assembler (74 for backing
  adapter) and 359 TEXT in its standalone C transaction. EXEC plus
  FULL replacement is +121 pre-fold words even before that C code.
  Test the actual folded KCORE and swapless 32K/48K RAM high-water.
- [ ] Adapt `system/kernel/fs/tsfs_runtime.s` for restart extents and
  immutable compressed contents; verify exact consume/decoded length,
  bounded memory and seek/refill behavior.
- [ ] Review `system/kernel/proc/proc_swap_pdp6.s` and every reference to
  `d6lz36_decode_vfs`, `d6lz36_decode_buffer` or old fixed decoder ABI.
- [ ] Treat `kpack_codec_v1.c` as an **inactive identity-copy experiment**,
  not a functional compressor. Do not activate it or the old PACKED
  disk-swap lifecycle as part of executable codec conversion. Future
  swapless-only RAM compression must obey the admission, MM ownership
  and no-net-RAM-growth requirements of the swapless memory design.

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
  scheduler tests and unchanged raw disk-swap behavior.
- [ ] Failure injection: corrupted compressed boot payload, truncated
  executable, short VFS read, malformed TSFS extent, decoder output
  mismatch and interrupted PURE restore. Future swapless-only mutable
  RAM compression has its own encoder/eviction/restore fault tests and
  must not be conflated with this executable format migration.
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
3. Execute complete boot/EXEC/TSFS and raw-swap failure-injection regressions.
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
