# Proposal B v429: value-returning TSFS record lease cost gate

Status: read-only source and testkit analysis PASS; no kernel change or
memory saving accepted.

`tsfs_file_record` returns a pointer into the shared 128-word transfer
workspace, using 16 eight-word records/block. `tsfs_extent_record`
returns a four-word record, 32/block. `tsfs_parent_common` passes the
file record pointer to the parent-name consumer. `tsfs_readdir` reads
a parent record and then fetches a child record, so the first pointer
cannot be assumed valid after the second read. The compressed-word
path also consumes data from `fs_block_workspace` in its decoder.

The proposed value-returning API therefore needs at least eight
additional words of per-call storage per copied file record, or four
per copied extent record, beyond saved registers and call linkage.
Copying only a projected subset may cost fewer words, but requires
per-operation ABI analysis and cannot simply replace the existing
record pointer with an unowned address. Meanwhile sleeping physical
I/O still needs buffer ownership until completion, independent of how
later metadata is represented. A blanket value-copy transformation
does not itself remove the global provider lock and is NOT accepted
without an end-to-end total-RAM reduction.

New `daimos-testkit` guard
`tests/validation/daimos-tsfs-record-lease-v429.py` PASS: verifies
record widths, workspace pointer lifetime, parent/readdir consumers,
and D6LZ input dependence. This is a source-level contract guard,
not a real multi-process race test.

Next action: find an operation-specific projection that is already
copied to a caller-owned result (e.g., stat/name), calculate its full
resident and stack impact, and benchmark a reversible implementation
*with* ownership shortening. Reject code growth without a proven
runtime benefit. Global provider locking remains mandatory.

Reference full linked image: KCORE 8995, KCORE+MRES 15543,
PERMANENT_LAST 15590. Strict 32K acceptance is deferred.
