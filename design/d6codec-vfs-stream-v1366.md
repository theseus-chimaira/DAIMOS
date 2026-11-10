# v1366: staged grouped D6CODEC VFS refill interface and RAM cost

Date: 2026-11-08. Status: **native PDP-6 fake-provider acceptance**,
not a production KCORE/VFS link and not a runtime format cutover.

## Runtime requirements from actual callers

The production `system/kernel/core/d6lz_pdp6.s` implements two
entries for the fixed-low-core D6LZ36 decoder: VFS streaming and
complete in-memory buffer. `exec_load.s` supplies an exact compressed
length in AC3; `proc_pure_backing_pdp6.s` uses AC3=0 to request
trusted immutable source backing when the original exact compressed
length is not available. `tsfs_runtime.s` uses the exact complete
buffer entry. The new grouped codec must preserve these distinct
source-count contracts, not silently treat zero as an empty stream.

## Versioned native implementation

The unlinked `system/kernel/core/d6codec_stream_v1366.s` is the
staged v1361 grouped decoder extended with `DS_REMAIN=-1` as an
explicit **trusted immutable VFS** sentinel. The output word count
and valid token termination determine completion in that mode;
already-prefetched compressed words may remain unused. Nonnegative
compressed lengths still require exact total consumption. Every
other negative or out-of-range length is rejected in the frontend.

`system/kernel/core/d6codec_vfs_v1366.s` provides:

- `d6codec_decode_vfs_v1366`: the current AC1 vnode, AC2 source
  file offset, AC3 exact source length (or zero for trusted), AC4
  `decoded_words,,destination` calling convention;
- `d6codec_decode_buffer_v1366`: an exact disjoint in-memory stream;
- status 0 for success, -1 for error, preservation of AC10..AC16.

The adapter allocates all of its working memory on the active
process-private kernel stack. Its **68-word frame** is seven saved
ACs, three persistent VFS parameters, 26 grouped decoder state
words (15 packed descriptor words), and a 32-word VFS input window.
No heap allocation or global BSS is introduced. The output already
contains its own backwards-match history. Input, output and the
entire reserved adapter stack frame are checked for illegal overlap
before decoding, including saved ACs and the VFS buffer.

This is not an instruction-copy replacement for the old fixed
`000060` decoder. The staged engine and wrapper are fully relocatable
normal assembly objects; no production loader, kernel or caller uses
them yet.

## Native PDP-6 acceptance

Testkit `tests/validation/d6codec-vfs-v1366` builds a native fixture
from a host-golden current KCORE DXR 128-word prefix and tests:

- Exact 32-word-window VFS decoding and 7-word short provider reads.
- Trusted immutable VFS backing with surplus encoded words.
- Exact complete-buffer decoding with all 128 output words compared.
- Premature provider EOF, malformed first control, exact-length
  trailing data, truncated buffers and forbidden zero-length direct
  input, invalid negative/19-bit lengths and source/output overlaps.
- Source/output memory overlapping the adapter's reserved stack
  frame and all seven callee-saved ACs' values after success/failure.

All native tests PASS. The native stack canary reports `0110`
**octal words (72 decimal)** including the test CRT main call and
the engine's nested calls. The VFS stub is a leaf function, so
this does **not** bound real cold-read VFS/provider nesting or the
process's syscall stack usage. Full nested stack canary tests remain
mandatory before any production link.

## Permanent RAM and code-size tradeoff

Measured standalone native text sizes (36-bit instruction words):

| Component | Legacy D6LZ36 | Staged grouped v1366 |
|---|---:|---:|
| Decoder engine | 40 | 237 |
| Runtime VFS/buffer adapter | 57 | 96 |
| Combined text | 97 | 333 |
| Static data/BSS | 0 | 0 |
| Per-call VFS frame | 72 | 68 |

The grouped frontend **saves four temporary stack words** but
**would add 236 permanently resident instruction words**, before
linker folding or full KCORE layout effects. These figures are
independent object measurements, not an accepted linked KCORE
size or a claim of whole-system RAM savings. They are a significant
memory-cost regression under the project's RAM-first optimization
goal. No production decoder format switch is authorized by v1366.

## Remaining gates

Reduce the resident grouped code cost, prove actual nested provider
stack headroom on a 0315-octal-word process kernel stack, then test
a full temporary KCORE link with all consumers and exact format
compatibility. Check source-provider failure cleanup, PURE backing
immutability, TSFS restart, and swap/EXEC loads. Finally rebuild all
grouped host producers, media images, kernel consumer objects, KINIT
and Stage-1 loaders atomically. No permanent double-codec fallback.
