# Exact-size packed eviction v077 (inactive)

`proc_packed_evict_transaction_v077` adds a two-pass codec contract:
while the resident MM source is pinned, a pure length-measuring callback
returns the exact number of compressed 36-bit words; MM then allocates
that exact number of words, the real encoder writes into that extent,
and the transaction verifies the written count equals the measured count
before committing ownership. Errors return the source to an unpinned,
resident state through the corrected v076 cleanup paths.

Unlike v45, the caller does not need to know a payload's exact size in
advance. A nonshrinking or invalid output is refused before allocation.
This avoids padding and unused retained packed storage. The testkit v077
host-MM cycle uses the real v068 inline codec for both measurement and
encoding, testing zero-dominated and mixed-prefix payloads with the dense
and sparse registries. Eviction, MM publication, real decoding, PURE reload,
registry retirement, and cleanup pass. Real PDP-6 encoder count-only mode
and scheduler integration remain pending. The test uses host malloc inside
the measurement callback; this is not acceptable in KCORE and must be
replaced by a no-allocation native length calculator.

The existing v45/v46 and production VM paths remain unchanged. No
performance/size optimization is attempted. Resident KCORE must ultimately
fit below 020000 octal before production activation.
