# Proposal B v459: rejected XOR operand-order experiment

Swapped the operand order in the resident VFS RENAME namespace XOR.
The namespace identity comparison is mathematically unchanged, but
the full linked PDP-6 GCC lowmem image stayed at KCORE 8961 words
(`__kcore_low_end` 021461). The experiment was size-neutral and was
reverted. No ABI, semantic or persistent RAM change was accepted.

The v452 accepted baseline remains KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. Small syntax-level changes at this boundary
are unlikely to justify further work; the next useful effort is a
whole-layer FILE/VFS pathname redesign measured across the linked
image and accompanied by integrated lifetime tests.

Strict 32K acceptance remains deferred.
