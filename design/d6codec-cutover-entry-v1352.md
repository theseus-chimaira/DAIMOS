# D6CODEC cutover entry audit v1352 (2026-11-08)

Status: pre-cutover source and native ABI baseline; no production-format
switch, image migration or resident kernel change is claimed.

## Source baseline and accepted scope

- DAIMOS base: `fa776f44` (v1351). The previous four uncommitted
  documentation inserts were stale v1286 snapshots and were discarded
  before creation of this isolated worktree.
- `d6codec_encode_v053.s`, `d6codec_decode_v053.s`, and
  `d6codec_kcore_abi_v055.s` remain staged and not part of the active
  compressed-image or KCORE link.
- The existing testkit `d6codec-kcore-abi-v055/run-v055.sh` passes on a
  native PDP-6 execution using this source checkout. This covers the
  staged codec round trip and saved-register ABI, not linked kernel
  calls, low-core relocation, nesting, or malformed-image acceptance.
- v1351's accepted 256K process-swap/owner test and 38/38 grouped codec
  staging baseline are distinct; neither authorizes a boot-format change.

## Fresh unchanged-code linked-size baseline

Executed `make -C system/boot/pdp6 permanent-size BOOT=dsk ROOT=disk`
in the isolated v1352 worktree, using the existing production D6LZ36
image path. The link/size stage ran and reported:

| Measure | Octal | Decimal 36-bit words |
| --- | ---: | ---: |
| KCORE endpoint | 023216 | 9870 |
| KCORE plus *all packaged* MRES | 046426 | 19734 |
| Single-root permanent last | 046505 | 19781 |
| Multi-root permanent last | 046546 | 19814 |

The make target exits 2 because the full optional package inventory
exceeds octal 040000 (single-root by 3398 words; multi-root by 3431).
This is neither the simultaneously installed MRES footprint nor a
runtime RAM peak. Do not attribute the package overrun to D6CODEC:
the new codec was **not** linked. This pre-cutover baseline must be
compared against the same profile after the complete format switch.

## Current v053 bit layout (observed from native sources)

- All stream elements occupy 36-bit words. A group begins with one
  control word. Its highest six bits are a token count from 1 through
  30 (octal 036). The next 30 control bits select literal (0) or
  descriptor (1) tokens, consumed from the most-significant end.
- Following the control word are `ceil(matches / 2)` packed descriptor
  words, followed by one word per literal token. Each descriptor uses
  an 18-bit halfword; the first occupies the left half.
- Within a descriptor, the low nine bits encode distance, the next
  seven bits encode decoded word count (1..127), and the high two
  bits encode kind. Kind 0 copies preceding 36-bit words; kind 1
  emits zero words with distance zero; kind 2 copies 36-bit words from
  an **even** halfword distance converted to word distance. Kind 3
  and odd halfword distance in kind 2 are rejected by the decoder.
- Matches may overlap prior output. The encoder emits word matches
  and zero runs; it does not currently emit halfword kind 2. Host
  producers must not emit unsupported odd-halfword matches.
- Input is complete-buffer, not refill/resumable. Success requires
  exact output length and exact input exhaustion. Zero input succeeds
  only with zero requested output. The output is invalid on failure,
  because the decoder can already have written a prefix.

## Unclosed safety and canonicality conditions

The current decoder does **not** establish these as accepted invariants:

1. Pointer validation and 18-bit address-wrap avoidance for source,
   destination, or group payload pointer arithmetic.
2. Source/destination disjointness; exact limits for the caller-owned
   buffers before any stores.
3. Unused control-flag bits and trailing descriptor halfword must be
   zero; otherwise distinct encodings of the same data can be accepted.
4. Full kind-2 odd-distance halfword semantics, should a producer ever
   require them. Prefer an explicitly restricted format unless measured
   benefits justify the extra decoder code and tests.
5. Exact worst-case stack high-water through the v055 saved-register
   wrapper and actual nested VFS/EXEC/TSFS calls.
6. Host producer parity and bounded worst-case encoder capacity.

These are test/design gates, **not** claims of a newly discovered
production defect: the staged decoder is not production-active.

## Production dependency graph and atomic boundary

- All five stage-1 loaders (`dsk`, `drm`, `dtc`, `mtc`, `ptr`) install
  the existing resumable decoder at octal 000060. The v053 decoder
  needs a whole input stream and cannot simply replace this binary.
- `system/boot/pdp6/Makefile` still generates `kinit.d6lz.words` and
  `.d6lz.dxr` images; `tools/host/d6lz.c` and `d6lz-codec.c` are
  active producers. Rebuilding *all* images is part of one cutover.
- `proc/exec_load.s` and `proc_pure_backing_pdp6.s` call
  `d6lz36_decode_vfs`; `fs/tsfs_runtime.s` calls
  `d6lz36_decode_buffer`. VFS's resumable refill contract needs an
  explicit bounded replacement, not a symbol substitution.
- Keep the present D6LZ36 boot image functional until new producers,
  low-core loading, resident codec, consumers, verification and rollback
  pass together. No mixed-format fallback should be linked afterward.

## Next implementation gate

Add source-linked native negative tests in `daimos-testkit` for the
six unclosed conditions, particularly address overflow and
noncanonical padding. Then implement bounded production wrappers and
validate their stack/ABI. Only then begin the synchronized producer,
stage-1 and KCORE consumer switch in this isolated worktree. Measure
both KCORE endpoint and total loaded permanent/dynamic words.
