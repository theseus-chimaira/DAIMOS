# Inline codec + packed transaction v076

The real C99 inline codec v068 has now exercised the registry-aware
`proc_packed_evict_transaction_v45` and `proc_packed_restore_transaction_v46`
consecutively under daimos-testkit's host MM mock, in both dense and sparse
registry modes. A 128-word zero suffix encodes to three 36-bit words;
subsequent restore decodes the actual packed data, reconstructs the PURE
prefix and retires MM ownership, registry entry, PACKED mode and TRANSITION
correctly. No identity or synthetic pair codec is used in this test.

A true v45 rollback bug was discovered and fixed: allocation failure after
pinning the resident source returned directly without unpinning it. The
cleanup labels now release that pin exactly once on every precommit path.
A preceding failing allocation no longer poisons a subsequent successful
eviction. The original v45 and v46 regressions continue to pass.

This remains **host-MM simulation**: scheduler hooks, live KCORE linkage,
production VFS/TSFS 1.3 mount, native simulator eviction/recovery and the
mandatory below-020000 octal resident endpoint are NOT yet verified.
The native v073 encoder/decoder ABI is still staged. Keep production
identity codec and disk swapping active until full native acceptance.
No resident optimization is attempted at this checkpoint.
