# Proposal B v436: OPEN argument normalization

Accepted measured KCORE reduction: `exec_native_syscall` OPEN flags
are normalized directly in AC2, using AC3 for preserved option bits.
Old and new transformations are exhaustively equivalent for all
262144 unsigned 18-bit flag values. `file_open` and VFS APIs are not
changed; user mapping hold/release and provider ownership are intact.

Full linked PDP-6 GCC lowmem image: KCORE 8990 -> **8989** words;
KCORE+MRES 15537 -> 15536; PERMANENT_LAST 15584 -> 15583.

New daimos-testkit exhaustive flag-ABI guard v436 PASS and existing
`test-daimos-exec-abi` PASS. No stack allocation or other persistent
RAM growth. This is a one-word resident improvement, **not** completion
of the larger cross-layer OPEN redesign. Strict 32K acceptance deferred.
