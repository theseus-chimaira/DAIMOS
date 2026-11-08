# Grouped D6CODEC Stage-1 continuation contract v1360

Date: 2026-11-08. Status: tested *host state-machine reference*, not
linked or bootable PDP-6 assembly. Production still runs D6LZ36.

## Existing native deployment constraints

`system/stand/pdp6/common/decompressor.inc` installs a resumable
D6LZ36 core at executive address 000060 and is reused by KCORE.
All five Stage-1 loaders (`dsk`, `drm`, `dtc`, `mtc`, `ptr`) install
that same fixed image. The kernel VFS adapter
`system/kernel/core/d6lz_pdp6.s` currently reserves a 64-word VFS
refill window plus eight stack words of saved context; it preserves
the core state in registers across NEED_INPUT. The grouped v053
decoder is currently a separate complete-buffer engine and is not
binary-compatible with this continuation or relocation contract.

## Proposed grouped continuation

The staged v1360 reference tests an input-window callback which can
suspend at **any compressed-word boundary**. It retains a control
word, token index, count and flags; up to 15 36-bit descriptor words;
current descriptor-half index; output cursor/origin/end; and exact
compressed words consumed. It does not buffer up to 46 complete
group words. The decoder's output is its word-copy history; kind-2
halfword distances are restricted to even values as in native v053.

Each group is parsed as control, packed descriptors, then literals
interleaved with match/zero output. The machine consumes all matched
descriptors before requesting literal words, and can suspend during
descriptor ingestion or between literal tokens. A normal return of
+1 means NEED_INPUT, 0 means the requested output and the declared
compressed input both completed exactly, and -1 means malformed or
inconsistent input. A successful return is never inferred solely
from the output count while compressed words remain.

The 15-word descriptor bound is exact: 30 possible match tokens per
group, two 18-bit descriptors per 36-bit word. A 30-match all-zero-run
group exercises that bound; a 46-word whole-group staging buffer is
unnecessary. This bounds persistent decoder scratch, not the complete
caller input window, stack frame or whole KCORE footprint.

## Reference acceptance

`daimos-testkit/tests/validation/d6codec-stream-v1360` runs both a
bounded-reader implementation and an independently resumed state
machine against 27 current linked kernel/MRES DXRs and six refill
widths (1, 2, 3, 7, 16 and 64), yielding 162 successful combinations.
Ten malformed cases per refill width, a 15-descriptor stress group,
and empty stream behavior also PASS. This is **host simulation only**;
it does not establish native PDP-6 register preservation or stack.

## Required next native implementation gates

1. Assign the control, descriptor cursor, output cursor and compressed
   count to real PDP-6 ACs or bounded private stack. Respect AC10..15
   preservation under compiler and VFS calls.
2. Implement the staged decoder with explicit NEED_INPUT return and
   exactly 15 descriptor scratch words; check physical address limits
   and make all partial output unpublishable on failure.
3. Measure the installed low-core image size and relocation; do not
   assume the complete-buffer v053 engine fits the 000060 contract.
4. Validate split-group, all-match, boundary, short-window, corruption
   and rollback paths on a native PDP-6 with 1-word through 64-word
   windows and real VFS/provider call nesting.
5. Only then move all five Stage-1 loaders, KINIT production, VFS,
   EXEC/PURE and TSFS to the new format in a single synchronized
   rebuild. No permanent dual-codec fallback or mixed-format image.

## v1361 native assembly implementation checkpoint (2026-11-08)

`system/kernel/core/d6codec_stream_v1361.s` implements the first
actual PDP-6 grouped-format resumable decoder, staged and unlinked.
Its assembly-only ABI is AC1 = caller-owned 26-word state, AC2 =
compressed input window, AC3 = window words, and AC0 = 0 exact complete,
+1 NEED_INPUT, or -1 malformed. AC2 and AC3 advance to the unread
source position and remaining window words; AC4..16 are volatile.
The state block includes exactly 15 packed descriptor words, plus
control/group flags, progress, output geometry and source count.

The native testkit v1361 simulator suite PASSes one-word refill
windows for a literal, a split zero descriptor, a 30-match/15-word
descriptor stress group, invalid group count, invalid physical output
endpoint and an independently host-encoded 128-word KCORE DXR prefix.
Native object inspection reports **230 text words, 0 data, 0 BSS and
82 relocations**. No KCORE link or Stage-1 production boot was made.

This engine is intentionally not claimed to fit the fixed `000060`
installation region. Its 230-word text and 26-word caller state need
measured address placement, position-independent relocation or linked
absolute targets, validated stack and refill/provider behavior, plus
much more malformed-stream acceptance. Stage-1 installation and all
five boot devices remain OPEN; do not activate the new format yet.

## v1362: current-build native streaming matrix

The v1361 native assembler also PASSes fifteen additional PDP-6
decoding regressions against independently host-generated compressed
DXR prefixes. KCORE, D6FS, MonitorFS, SWAP and tape are each decoded
with 1-, 7- and 64-word refill windows, reconstructing all 128 source
words and checking exact completion. The v1362 testkit regenerates
fixtures from the current linked build, not stale historical paths.
This does not establish low-core relocation, full-image runtime or
Stage-1 boot acceptance; production D6LZ36 is unchanged.
