# Native PDP-6 inline-token encoder v071 (staged)

This assembly encoder derives its word-match and zero-run selection from
v053, but emits interleaved full-word descriptors under the 36-token
inline control word used by the v067 resident decoder and v068 host codec.
Unlike the grouped encoder, it neither shifts literals nor packs pairs of
18-bit descriptors. The original grouped encoder remains unchanged.

Input ABI: AC1 source cursor, AC2 raw word count, AC3 destination cursor,
AC4 destination capacity in words; AC1 returns encoded word count or -1.
The encoder uses the AC17 stack for its search bookkeeping and is not yet
wired into the KPACK C ABI. A standalone PDP-6 test exercises a source
with a literal, a zero run and repeated words, and successfully roundtrips
through fixed-address v067. The test exposed a three-bit control flag
mistake during development; the corrected build passes.

The encoder has NOT yet passed the full 81-DXR parity or KPACK MM ownership
and allocation tests, and has not been linked into active KCORE. The 8K
resident ceiling remains mandatory at eventual activation. Optimization
is explicitly deferred while integration continues.
