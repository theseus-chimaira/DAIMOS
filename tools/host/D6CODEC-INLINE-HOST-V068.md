# D6 inline-token host codec v068

Experimental C99 producer, decoder and DXR CLI for the v067 PDP-6 inline
wire layout. It uses one 36-bit flag word for up to 36 interleaved 36-bit
literal/18-bit descriptor tokens and the native v053 word/zero matching
policy. The reusable API allocates only on the host; it is not linked into
resident KCORE.

The daimos-testkit `tests/host/d6codec-v068/run-v068.sh` validates a
10,000-word roundtrip, odd-halfword overlap, strict trailing/truncated input,
word-width rejection, and guarded output. Actual CLI output matches the
independent v065 repacking model bit for bit on all 81 native-policy DXRs:
159,973 source words become 135,534 compressed words.

The production tools, TSFS 1.1 format, Stage-1 boot loaders, and all KCORE
codec consumers remain on D6LZ36. No attempt was made to meet the final
8K KCORE acceptance budget during this implementation checkpoint; the
limit remains mandatory before activation. Next: stage an explicitly
versioned TSFS 1.3 producer/checker with v068, then coordinate Stage-1,
EXEC/PURE, KPACK and runtime consumers before changing production defaults.
