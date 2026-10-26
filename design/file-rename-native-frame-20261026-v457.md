# Proposal B v457: native FILE RENAME frame and failure test

Added a native PDP-6 testkit fixture that extracts and executes the
production `file_rename` assembly body, with controlled parent path,
permission and VFS RENAME callbacks. It verifies success with distinct
parent/name records, lookup failures on both sides, permission failures
on both sides, and provider error propagation. The test checks that
failed intermediate steps do not call later stages. Native FILE,
VFS RENAME and DTFS RENAME fixtures all PASS.

This does not yet constitute a single native integrated FILE/VFS/
provider transaction or real blocking-I/O lifecycle test. Future
architectural attempts should remove an existing pathname boundary
without adding parallel wrappers, preserving call ownership and
testing allocation and failure cleanup.

No production code, ABI or physical RAM consumption changed. The
last linked GCC PDP-6 lowmem measurement remains v452: KCORE 8961,
KCORE+MRES 15508, PERMANENT_LAST 15555. Strict 32K deferred.
