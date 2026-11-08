# v1367: PDP-6 grouped runtime decoder RAM reduction

Date: 2026-11-08. Status: tested native staged decoder, **not** a
production KCORE object or a grouped-format runtime cutover.

## Objective and preserved behavior

The v1366 grouped VFS adapter passed native PDP-6 correctness tests
but measured 333 text words versus 97 for the resident D6LZ36
decoder plus VFS adapter. The +236-word cost blocks a RAM-first
cutover. v1367 reduces both text and per-call continuation state
without changing the grouped v053 wire format, the v1366 trusted
immutable-backing interpretation, or any production file.

New staged sources:

- `system/kernel/core/d6codec_stream_v1367.s`
- `system/kernel/core/d6codec_vfs_v1367.s`

Decoder changes use physical-address `TLNE` checks rather than
register copies and 18-bit shifts, a compact conditional phase
dispatch, a shared compressed input-fetch body for bounded and
trusted input, and indexed descriptor addressing. The previous
descriptor-ingestion count and decoded-token descriptor index are
never live simultaneously, so they now share one state word. This
reduces the continuation from 26 to 25 words, including the fixed
15-word descriptor payload. All match/literal/zero-run and error
semantics remain unchanged.

The adapter keeps the same 32-word refill window and AC1..AC4
calling convention and preserves AC10..AC16. It now has a 67-word
per-call frame rather than 68 words. Shorter comparisons and the
physical count `TLNE` check reduce its text without weakening the
source/destination overlap guard against the *entire* adapter frame.

## Measured native PDP-6 memory cost

All numbers are 36-bit words except where otherwise noted.

| Component | D6LZ36 production | Grouped v1366 | Grouped v1367 |
|---|---:|---:|---:|
| Stream engine text | 40 | 237 | 225 |
| VFS/buffer adapter text | 57 | 96 | 92 |
| Total text | 97 | 333 | **317** |
| Static data/BSS | 0 | 0 | 0 |
| Grouped continuation state | n/a | 26 | **25** |
| Per-call VFS frame | 72 | 68 | **67** |
| Leaf-provider canary depth | unmeasured here | `0110` | **`0107`** |

The optimization saves **16 text words** and **one temporary stack
word** compared with v1366. The new combined grouped engine and
adapter would still occupy **220 more permanently resident text
words** than D6LZ36 before full linking and folding. There is no
claim of an accepted KCORE size or overall RAM improvement.

The last linked KCORE artifact examined at this checkpoint has
9,601 text words and 269 BSS words; these values are the unchanged
production baseline, not a new temporary grouped KCORE link.

## Testkit acceptance

`daimos-testkit/tests/validation/d6codec-ram-v1367/run-v1367.sh`
uses the independent v1359 host golden streams and v1366 fake VFS
provider interface. Its native PDP-6 cases cover the complete VFS
exact/trusted/short-read/error/overlap/callee-save matrix; a real
15-descriptor maximum-size group; and the five current-build DXR
categories KCORE, D6FS, MonitorFS, SWAP and tape under 1-, 7- and
64-word refill windows (15 representative native reconstructions).
The versioned stack canary reaches `0107` octal (71 decimal) words
including its CRT call and fake leaf-provider stack usage.

This stack depth is **not** the actual EXEC/PURE/TSFS cold-provider
and syscall nested worst case. The existing per-process kernel stack
is only `0315` octal words, so the complete call graph must still
be measured before any production link. The 15 DXR checks reconstruct
128-word prefixes, not all full current DXR images.

## Remaining mandatory release gates

1. Reduce or justify the remaining +220 resident text words in a
   temporary fully linked KCORE, with complete folding/layout report.
2. Test real VFS/FS-provider nested stack watermarking, provider
   failures and partial output rollback before allowing publication.
3. Verify EXEC, PURE immutable reload, TSFS restart and swap callers
   with grouped-format data and correct lifetime/ownership semantics.
4. Rebuild all host producers, grouped KINIT media, KCORE runtime
   objects and filesystem artifacts atomically, without a permanent
   dual-codec fallback. Production D6LZ36 stays unchanged until then.
