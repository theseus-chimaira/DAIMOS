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

## v1353: staged decoder safety milestone (2026-11-08)

The isolated implementation now rejects out-of-range 18-bit input and
output address geometry, high address bits, and overlapping nonempty
source/destination buffers before accessing either buffer. It also
rejects nonzero unused control flags and a nonzero unused right
descriptor halfword. The existing complete-stream, exact-output check
and failure-on-partial-output contract are unchanged.

The full-word octal `01000000` endpoint check must use `CAMLE` with a
literal, not an 18-bit `CAILE` immediate: the initial implementation
was caught by its native regression and corrected before acceptance.

Testkit branch `test-d6codec-bounds-v1353-20261108`, commit `97e222d`:
native `d6codec-bounds-v1353` PASS, existing native
`d6codec-kcore-abi-v055` PASS. The negative-control test fails against
the original v1351 decoder with code 1 (unused control flag accepted).

This is **only staged decoder hardening**. It is not production KCORE
integration, not a full malformed-input fuzz campaign, not an encoder
bound guarantee, and not a 32K or complete-image boot acceptance.

## v1354: staged encoder physical geometry milestone (2026-11-08)

The staged `d6codec_encode_v053.s` encoder now checks its source and
maximum destination extents against the physical 18-bit word address
boundary, refuses nonzero high pointer/count bits and nonempty source /
destination overlap before reserving a control word. Empty input and
zero-capacity output remain valid when combined; nonempty input with
zero output capacity is rejected without modifying the destination.

Testkit `1d2b15c` adds native `d6codec-encode-bounds-v1354`: PASS on
the modified encoder, expected code-1 negative witness against the
unmodified v1353 encoder. The staged full-DXR v055 native corpus PASSes
with the change. These tests do not cover complete host/encoder maximum
encoded-size arithmetic, full malformed-input fuzz or linked stack use.

Neither encoder nor decoder is currently linked into production KCORE;
the complete atomic grouped-format cutover remains OPEN.

## v1355: conservative encoder buffer bound (2026-11-08)

The staged encoder provides `d6codec_v053_max_encoded_words`:
AC1 input word count, AC1 returned worst-case output count or -1;
clobbers AC0/AC2/AC3, no stack or BSS. For N=0 the bound is zero;
otherwise it is `N + ceil(N/30)` 36-bit words. The helper rejects
counts with high address bits, negative counts and a result exceeding
the physical 18-bit word address range. A caller still must validate
the destination start plus its chosen capacity before compression.

Proof: all complete groups contain 30 tokens, so group count is at
most `ceil(N/30)`; each token consumes one or more source words and
each descriptor pair fits in one word, so the output payload does
not exceed N source words. There is no persistent scratch buffer.

Testkit v1355 checks exact/one-word-short capacities for 30/31
distinct literals, one-word zero-run capacity, native roundtrip,
sentinel preservation outside the advertised destination region,
and helper boundaries including maximum allowed N=253687. The
combined v1355, v1353, v1354 and full-DXR v055 native PDP-6 tests PASS.
No production linker, image or installed MRES has been changed.

## v1356: first wrapper-inclusive native stack measurement

Testkit `d6codec-stack-v1356` executes the complete v051 darc DXR
roundtrip with the staged v055 encoder/decoder ABI adapter and
saved-register probes. The existing 512-word canary CRT observes
`__v051_first_changed=016` octal: 14 decimal stack words measured
including `PUSHJ main`, the v055 save/restore frame, codec internals
and the encoder's search call. The reproducible v1356 test PASSes.

This is a verified representative workload, **not** a proved maximum
across all failures or nested KCORE consumer paths. No code or
persistent RAM is added by the test. Continue explicit error-path
and nested VFS/EXEC/TSFS stack measurement before activation.

## v1357: failure-path wrapper stack regression (2026-11-08)

Native testkit `d6codec-stack-fail-v1357` exercises malformed and
truncated decoder control input, invalid physical input geometry,
encoder short-capacity literal and zero-run failures, and a
repeated-word match-search path that later exhausts output capacity.
All calls return -1, and the 512-word CRT stack-canary observation is
again 016 octal (14 decimal words) including the main-call frame and
v055 wrapper. v1357 PASS and the existing v1356 full-DXR stack PASS
with the same staged sources.

No actual worst-case nested VFS/EXEC/TSFS production stack has yet
been measured; these results must not be promoted to a universal
stack bound or a release gate for the atomic format conversion.

## v1358: independent host-golden/native codec parity (2026-11-08)

The native PDP-6 testkit `d6codec-parity-v1358` reuses three original
v045 real-DXR, 128-word input and host-generated encoded-golden
fixtures. Only their entry names were changed to invoke the staged
DAIMOS v053 engines through the v055 register-saving ABI. Configctl,
darc and d6lz all PASS bit-for-bit comparison of the native encoded
words with host-generated expected streams, followed by exact native
decoded roundtrips. Production D6LZ36 is unaffected.

The historical 81-DXR host manifest references an older absolute
build tree and is not directly reproducible from that inventory.
The three accepted samples are not a production-wide host format
migration, nor proof of the current full 81-executable corpus.
Full host/native format parity, current artifact regeneration and
all stage-1/runtime consumers remain OPEN.

## v1359: present-build DXR corpus and native prefix parity

`daimos-testkit` v1359 reads the actual linked DXR artifacts under
this worktree's disk-root build, avoiding the historical 81-DXR
manifest's unavailable absolute paths. Its independent host reference
completes 27/27 full-format encode/decode roundtrips: 19,270 input
words and 17,899 grouped compressed words. Five generated native
PDP-6 fixtures compare the first up-to-128 input words and their
encoded host-golden streams bit-exactly through the staged v053/v055
ABI, with full native roundtrip of each prefix. KCORE, D6FS,
MonitorFS, SWAP, and tape all PASS.

This validates the current linked kernel/package artifacts, not the
81 userland DXRs. Native parity of full-length 27-member streams,
present-day host producers, Stage-1 decompression and all runtime
consumer interfaces remain OPEN. No production KCORE or format switch.
