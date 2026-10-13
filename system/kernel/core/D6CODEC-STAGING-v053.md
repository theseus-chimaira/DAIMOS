# D6 codec v053 — source staging only

This checkpoint brings both native PDP-6 routines into the DAIMOS source
repository **without linking or activating them**. The integration workspace
will perform the actual codec format cutover, replacing D6LZ36 producers,
consumers, boot loaders and packed-process codec wiring together.

- `d6codec_encode_v053.s`: grouped control/paired-descriptor encoder,
  128-word backward match search, two-word matches and zero runs.
- `d6codec_decode_v053.s`: complete-buffer decoder for 36-bit word-copy,
  zero-run and *even-aligned* halfword-copy descriptors.
- Source lineage: validated `daimos-testkit` v051, based on v045 algorithm.
  Reference native regressions: v045, v047, v048, v049 and v051.

Experimental assembly calling convention for both: AC1 source address,
AC2 compressed/input word count, AC3 destination, AC4 output capacity
(encoder) or exact decoded word count (decoder); AC17 pushdown stack.
Encoder returns AC1 = compressed word count or -1. Decoder returns AC1=0
or -1. AC0..AC16 may be overwritten. **These are NOT yet kernel ABI-safe**;
in particular AC10..AC15 need preservation through the final wrappers.
Caller buffers must not overlap. Bounds/address-wrap validation, ABI and
stack measurement with wrapper remain required before integration.

No changes to `system/kernel/Makefile`, stage-1 loaders, KCORE link,
D6LZ36, any host image compressor or existing runtime call site occur in
this staging commit. No dual-format support is intended: the old codec
is retained solely until an atomic format migration in the next workspace.

The baseline measured code is 129 encoder + 99 decoder = 228 36-bit words,
static data/BSS zero. The last standalone v051 PDP-6 canary measurement
recorded six stack words for a full `darc.dxr` round trip, inclusive of
its test CRT call depth (not yet a proven worst case).
