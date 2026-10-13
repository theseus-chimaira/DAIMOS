# D6 grouped TSFS staging v061

The v061 `mktsfs-v061`/`tsfscheck-v061` host programs are deliberately
separate from production tools and not part of the installed TOOLS list.
They link the v060 grouped codec library. The staged extent flag is 2,
and the staged descriptor/TDIR format version is 1.2 rather than 1.1.
No legacy D6LZ compressed extents may be accepted by v061, nor may an
old TSFS checker accept the new image.

The testkit `tests/host/d6codec-tsfs-v061/run-v061.sh` constructs a 256-word
zero-heavy source, creates an image, checks it, builds legacy media using
the unchanged producer, and verifies both cross-format checker rejections.
This is host-format validation only, not runtime TSFS restart decoding.

Resident code budget: the old decoder begins at 000060 and the next KCORE
entry appears at 000130, a 40-word span; the new fixed v058 decoder is 129
words, an 89-word increase prior to folding and VFS adapter changes. No
new permanent data or stack is needed. The user's no-extra-RAM constraint
therefore remains UNMET at the whole-resident-image level. It is necessary
to reduce combined code usage before Stage-1 activation; do not treat the
zero-BSS/stack result as proof of no resident growth.
