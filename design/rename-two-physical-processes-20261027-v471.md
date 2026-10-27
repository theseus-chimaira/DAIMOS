# Proposal B v471: two native D6FS RENAME transactions under scheduling

The full PDP-6 native v471 scheduler regression extends v470 so that
two separate user processes each create a distinct private file in
`/CONFIG`, write a distinct 36-bit value, rename the file within that
directory, verify that the source name disappeared and that the
destination contains the expected data, and remove their destination.
The original failed-RENAME case and multiprocess PI6/disk I/O checks
are preserved. Full-system SIMH result: PASS.

The test exercises two physical-filesystem RENAME transactions in one
multi-process run, but does not instrument whether either transaction
was suspended while awaiting a provider lock or disk operation. It
therefore does not yet prove a PI6 context switch *inside* RENAME.

Architectural scope: FILE, VFS, D6FS/DTFS and provider internals can
all be redesigned or consolidated to save permanent RAM, provided
filesystem semantics and the externally documented ABI remain intact.
The source ownership boundaries in the v460 proposal are candidates
for optimization, not immutable implementation requirements. Measure
each accepted edit in the fully linked image; defer speculative growth.

No production edit in v471. Accepted linked KCORE 8962,
KCORE+MRES 15509 and PERMANENT_LAST 15556; strict 32K deferred.
