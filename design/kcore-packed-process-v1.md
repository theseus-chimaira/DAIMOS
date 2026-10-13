# KCORE compressed-process split: staged implementation v1

Status: provisional codec interface only; process eviction is NOT enabled.

Target: PDP-6, CTY and four DECtapes, native TSFS root, no external swap.
Eviction must discard PURE text and compress remaining valid writable state.
Packed state need not be APR-aligned; restoration must reconstruct a complete
aligned extent before returning to user mode. Regions may be omitted only if
VM/ABI can prove no valid process state remains there.

The kpack codec v1 is a lossless identity transform. It consumes no dynamic
scratch RAM, but saves no writable-state words, so it must not be mistaken for
the final compressor or used to enable compressed eviction.

Implementation phases:
1. Separate executable backing and PURE reload from disk-backed swap.
2. Implement packed-process MM descriptors and transactional relocation.
3. Implement bounded-space packing, rollback and restore; preserve current
   full-swap behavior until the no-swap configuration is verified.
4. Replace identity codec following real process-state compression benchmarks
   with instruction counts and PDP-6 timing-table estimates.
5. Place all behavioral and failure-injection tests in daimos-testkit.

Acceptance requires no swap I/O, no hot-path context-switch overhead, correct
backing pinning, exact restoration, and safe operation under fragmentation.
