# Proposal B v430 — READDIR projection experiment

Accepted small MRES reduction. In TSFS READDIR the child record pointer
was redundantly copied into AC5 solely to read type bits and then form
the three-word name source. Using AC1 directly leaves the same type
bits in AC011 and copies the name to the caller-owned dirent before
the next provider operation. It does **not** change the global provider
lock or free the shared 128-word workspace earlier.

Full linked GCC PDP-6 lowmem sizes (v429 baseline -> v430):

- KCORE: 8995 -> 8995 (unchanged)
- KCORE+MRES: 15543 -> 15542 (one word saved)
- PERMANENT_LAST: 15590 -> 15589 (one word saved)

Native TSFS multiextent and D6LZ regression PASS. The v429 source
lease guard and new v430 READDIR projection guard PASS. This is not a
concurrent-filesystem stress test. No additional process storage was
allocated. Strict 32K acceptance remains deferred.
