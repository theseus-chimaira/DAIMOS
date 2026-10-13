# Staged KPACK inline codec adapter v073

The v073 assembly adapter connects the native v071 inline encoder to the
four-register packed-process encoder interface. The paired decoder adapter
translates AC1 source, AC2 compressed words, AC3 destination, AC4 expected
output words into the v070 direct-buffer ABI (AC2 source, AC3 compressed
words, AC4 count,,destination). The decoder returns the restored word count
on success and -1 on failure. The expected count is saved on the stack
because the direct-buffer frontend overwrites AC4 during decoding.

Both adapters preserve AC10..AC16, allocate no global state, and remain
outside production KCORE. A native PDP-6 daimos-testkit test verifies
encode/decode round trip and register preservation, PASS. The production
`kpack_codec_v1.c` identity implementation and packed process lifecycle
are intentionally unchanged. The v073 implementation is not yet linked to
registry-aware eviction/restore, has no full MM transaction regression,
and is not yet accepted for production process compression.

The 8K KCORE limit remains mandatory before final activation; resident size
optimization is deferred until implementation is otherwise complete.
