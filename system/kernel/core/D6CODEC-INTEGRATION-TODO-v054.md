# D6 codec v054 — integration TODO for the next workspace

**Status:** executable integration checklist, not an implementation. Source baseline: DAIMOS staging commit `de85d60` on `d6codec-staging-v053-20261013`. The new assembler routines are `system/kernel/core/d6codec_encode_v053.s` (129 words) and `d6codec_decode_v053.s` (99 words). They are currently *not linked*. Integration must replace D6LZ36, not preserve a second format. **Both compression and decompression must reside in KCORE.** Keep sharing/deduplication optimization for after integration.

## 0. Workspace and safety gates

- [ ] Create a new **isolated DevSpace worktree** from the *latest* applicable DAIMOS commit, merge/cherry-pick `de85d60` if absent, and first read `PROJECT-STATE.MD` and the codec/boot sections of `pdp10-doc`. Check concurrent changes before every merge. Avoid editing another agent's checkout.
- [ ] Do not enable PACKED scheduler transitions or remove disk swapping merely because the codec compiles. `PROJECT-STATE.MD` at the staging base says the v45 eviction/v46 restoration are inactive and warns against premature activation. Validate their integration gates separately.
- [ ] Record a known-good boot, KCORE resident size, KINIT image size, DXR execution, TSFS and swap results *before* changing any format; preserve the old commit/tag as rollback, **not** as a compatibility codec in the new image.
- [ ] Add test coverage **only** in versioned `daimos-testkit` directories; don't modify shared testkit Makefiles or other agents' dirty files. Create early versioned SHAR patches containing the final git commit/push instructions; use `Theseus <theseus@chimaira.net>`.

## 1. Freeze the single on-disk codec format

- [ ] Record the bit-exact framing: each 36-bit control has token count 1–30 in its top six bits and flags in its bottom 30; follows `ceil(matches/2)` descriptor words, each holding two 18-bit descriptors (high half first), followed by literals in token order. Each descriptor has kind bits 17–16, length bits 15–9 (1–127 words), distance bits 8–0.
- [ ] Define the legal kinds: 0 = overlapping word-history copy, 1 = zero-run with distance 0, 2 = even-aligned halfword history copy (distance even; converts to word distance /2), 3 = invalid. Native decoder currently **rejects odd kind-2 distances**. Ensure every producer emits only the native-supported subset. Word-reference history is at most 128 words; count and length fields are exact.
- [ ] Specify whether unused flags, the odd trailing descriptor half and any trailing compressed words must be zero/rejected. Implement and test the chosen strictness; do not silently diverge between host and PDP-6 decoders. Define a version/magic or container-level format identifier sufficient to reject stale D6LZ36 artifacts, without introducing dual-format decoding.
- [ ] Fix zero-length input/output semantics, exact decoded-size check, compressed-length accounting and error codes in the formal format contract. Define bounds including input and destination address wraparound and buffer overlap; ensure nonzero truncated input is rejected before each read.
- [ ] Keep an independent host reference encoder/decoder and test vectors from `daimos-testkit` v044–v049 to detect byte-level divergence during the migration.

## 2. Harden the resident compressor and decompressor

- [ ] Assemble the staged encoder and decoder unchanged first and confirm 129 + 99 text words, 0 data/BSS, using `$PDP10_PREFIX/bin/das` and `pdp10-objdump`.
- [ ] Confirm actual compiler-generated kernel ABI: AC0..AC7 volatile, AC10..AC15 callee-saved per existing `system/kernel/core/d6lz_pdp6.s`; verify AC17 stack and frame conventions in relevant C callers. Supply register-save wrappers with accurately tested size, not assumed size.
- [ ] Expose bounded **one-shot** encode/decode entries first. Explicitly define AC1 input pointer, AC2 input word count, AC3 output pointer, AC4 encoded capacity/exact decoded size, return AC1 = words written or -1 for encode and 0/-1 for decode (or a deliberately revised and consistently applied kernel ABI). Avoid additional streaming state unless a real consumer needs it.
- [ ] Before touching memory, validate pointer ranges and 18-bit address arithmetic (input, output, end pointers, scratch/stack and compressed payload length). Reject wraparound, inadequate capacity, impossible overlap and malformed descriptors; no writes beyond capacity. Do not invent an allocation-heavy workaround.
- [ ] Ensure the output is not published, executed, or swapped in until decode completes successfully. Partial output after failure is permitted only if the caller discards it safely.
- [ ] Repeat PDP-6 canary measurement for encoder, decoder, both wrappers and nested caller paths. v051's **six-word high-water mark** is only for the unwrapped `darc` test; do not treat it as a worst-case stack bound. Test incompressible, all-zero, long-overlap, full-capacity, capacity-minus-one, corrupted and large `dsh` streams.
- [ ] Probe descriptor-boundary insertions, maximum-length matches, 30-token groups, exact output capacity, long inputs, guard words and error exits. Run v048's 22 corruption cases, v047 complete DXRs and v049 largest DXR. Add deterministic and randomized host fuzz vectors whose native decode can be verified.

## 3. Integrate KCORE as the **sole** codec

- [ ] Update `system/kernel/Makefile` (`KCORE_FIXED_ASM`, object rules), `system/boot/pdp6/Makefile` (`KCORE_FIXED_ASM/OBJS`, dependencies) and any folding/link/address scripts to include **both** native routines plus required wrappers, and remove superseded resident D6LZ36 objects when no callers remain.
- [ ] Audit `system/kernel/core/d6lz_pdp6.s` and `system/stand/pdp6/common/decompressor.inc`: the current D6LZ36 fixed decoder lives at low core 000060, shared with Stage-1; new decoder's whole-buffer ABI is **not** interchangeable with its resumable VFS decoder ABI. Implement real adapters or refactor callers and remove the obsolete machinery. Do not leave an unresolved old symbol or allocate a duplicate full decoder.
- [ ] Confirm KCORE_BASE / low-core 000060 placement and Stage-1 installed-image geometry. The new decoder must not overwrite boot code, control words or destination during relocation. Measure the actual final linked KCORE endpoint and load address, not only 228 assembler words.
- [ ] Replace provisional `system/kernel/core/kpack_codec_v1.c/.h` identity-copy behavior with the real codec at its eventual integration point, after establishing ABI and transaction safety. Preserve failure guarantees of v45 packed eviction and v46 restoration; no nonshrinking image should be published as PACKED.
- [ ] Keep the compressor resident and callable from KCORE even if boot image production uses a host compressor. Verify stack/caller-owned workspace and no unintended permanent buffers; minimize total RAM rather than disk size.

## 4. Migrate all boot paths atomically

- [ ] Rewrite fixed decoder installation/entry and control-block handling for `system/stand/pdp6/{dsk,drm,dtc,mtc,ptr}/stage1.s`; update each device Makefile dependency on `common/decompressor.inc`. Audit all references to `d6lz_fixed_base`, `d6lz_image_start/end`, hard-coded call to 000060, and low-memory relocation/overlap.
- [ ] Update compressed KINIT generation in `system/boot/pdp6/Makefile` (`D6LZ_HOST`, `IMAGE_COMPRESSED_WORDS`, `kinit.d6lz.words`, compression recipes) to emit the new stream and correct sizes; ensure Stage-1 understands the new stream **in the same commit**. Do not boot a mixed-format image.
- [ ] Test boot from all supported devices (DSK, DRM, DTC, MTC, PTR) or explicitly document which are unavailable and why. Include cold boot, failed/truncated image, bad length/header, compressed and permitted raw paths, KINIT-to-KCORE handoff and reboot.
- [ ] Require actual PDP-6/SIMH smoke logs showing KINIT and KCORE initialized, with output/registry/filesystem behavior unchanged except format.

## 5. Replace executable, PURE and filesystem decode paths

- [ ] Update `system/kernel/proc/exec_load.s` and `system/kernel/proc/proc_pure_backing_pdp6.s` from the old `d6lz36_decode_vfs` API; preserve vnode ownership, exact payload consumption, error reporting and safe non-publication on failure.
- [ ] Decide whether VFS needs bounded refill streaming. Unlike v053's complete-buffer decoder, current `d6lz_pdp6.s` uses a **64-word refill window** and resumable state. Compare (a) complete compressed buffer supplied by caller, (b) minimal group-atomic resumable decoding; choose based on **net resident RAM + stack**, buffer lifetime and actual VFS constraints. Do not assume one-shot decoding can replace refill without a memory analysis.
- [ ] Update `system/kernel/fs/tsfs_runtime.s` direct-buffer decoder and its expected length/format handling; test compressed TSFS restart extents and failure rollback.
- [ ] Audit `system/kernel/proc/proc_swap_pdp6.s` references and all remaining `d6lz36_*` call sites. Don't remove disk swap until a separate complete PACKED scheduler/VM transaction is tested and approved.
- [ ] Verify dynamic DXR execution, reload of PURE backing, repeated eviction/restoration and malformed/truncated compressed executable rejection without corrupting process state.

## 6. Replace every host and userland producer/consumer

- [ ] Replace stream engine in `tools/host/d6lz.c` and `tools/host/d6lz-codec.c/.h` or rename them consistently to versioned new-codec names; update `tools/host/Makefile`, `mktsfs.c` and `tsfscheck.c`. Ensure host tools write the same 36-bit word format and reject old streams.
- [ ] Update `system/boot/pdp6/Makefile` DXR recipes and variables (`*.d6lz.dxr`, `D6LZ_DXR`, system library and TSFS command lists, `D6LZ_HOST`, KINIT compression). Also update `userland/d6lz/d6lz.c`, `userland/Makefile`, installer references and build scripts if the utility stays available; avoid stale-format tools with unchanged names.
- [ ] Rebuild **all compressed artifacts**, boot images and TSFS images from uncompressed sources. Treat existing D6LZ36 `.dxr`, compressed KINIT payloads and TSFS extents as incompatible; define an explicit regeneration procedure instead of silent fallback.
- [ ] Cross-check host-produced streams with the actual native decoder and native-produced streams with the independent host decoder; compare lengths, control words, descriptors and full round trips. Run all 81 DXRs in the host corpus; native regression may use representative complete images and largest `dsh` rather than all 81.

## 7. Verify whole-system safety, size, performance

- [ ] Build the real KCORE/KINIT and all Stage-1 targets; examine assembled and **linked** resident text/data/BSS, displacement in memory layout, stack depth, boot image size and boot stage relocation. Record before/after net RAM, not just 129+99 words.
- [ ] Boot test compressed KINIT; execute compressed DXRs and TSFS programs; exercise filesystem refill, PURE reload, swap path and expected failure paths. Compare simulator logs to the pre-cutover baseline.
- [ ] Complete isolated packed-process lifecycle integration tests first (v45 eviction -> v46 restoration, v38 registry, v43 release, v44 publication, MM pin/free failure injection) before enabling PACKED transitions; do not silently alter active scheduler or swap policy.
- [ ] Measure PDP-6 decode and encode steps/cycles on representative incompressible, match-heavy and normal datasets; document runtime impact, worst-case stack and net KCORE growth. Fix assembler/compiler defects immediately if encountered, not by working around them.
- [ ] Test cold, warm and recovery boots, malformed KINIT/DXR/TSFS images, image integrity and correct rollback after failures. Require explicit evidence that no stage produces legacy-format payloads.

## 8. Finalize only after a successful cutover

- [ ] Update `PROJECT-STATE.MD`, DAIMOS TODO and design status with measured production facts, including any remaining inactive packed functionality. Update definitive codec/boot/runtime documentation in **`pdp10-doc`**, not the obsolete `MANUAL-PAGES`; keep DAIMOS in-tree manual pages where they belong. Documentation goes into pdp10-doc only after the feature is finished.
- [ ] Retire obsolete D6LZ36 code, symbols, tests tied solely to the obsolete codec and unused recipes when replacement boots and consumers have all passed. Keep historical source in git rather than shipping compatibility paths.
- [ ] Commit/push small versioned patches early and often; final change set must contain a SHAR archive with instructions for `git commit` and `git push`. Do not modify unrelated concurrent KCORE work.
- [ ] Defer cross-routine code sharing until after integration. Then reassess combined resident size, call overhead and PDP-6 speed; retain changes only for measured net RAM benefit.

## Explicit acceptance criteria

**Not integrated:** staged `.s` files assembled; tests pass; no runtime callers changed.

**Integration candidate:** all producers and consumers of the new format implemented, native ABI/bounds tested, boot tools regenerated, no legacy codec calls in the active link, all compressed streams version-validated.

**Integrated:** each required boot path and normal runtime path passes end-to-end with the new format; both encoder and decoder are KCORE resident; system boot and executable tests pass; actual resident RAM, stack and CPU numbers are recorded; `PROJECT-STATE.MD` is updated. PACKED scheduler enablement and removal of disk swap remain separate decisions.
