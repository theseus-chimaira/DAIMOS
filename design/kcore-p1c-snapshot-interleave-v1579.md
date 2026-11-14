# KCORE P1-C v1579: native MEMFS snapshot and BACKSTORE interleaving

Date: 2026-11-14. Accepted *native source-linked* P1-C transaction
slice. Overall P1-C remains OPEN; P0-A and trusted-operator P0-B are
CLOSED. VERSION 0.5, strict 32K/optimization HOLD.

## Tested production path

The versioned testkit executes original `memfs_snapshot_pdp6.s` and
`backstore.s` together on the PDP-6 native execution engine. The
source-copy modification only exports the normally private
`snapshot_span` symbol to the test oracle. All test-only sector I/O,
MEMFS nodes, counters and callback scheduling stay outside DAIMOS.

The source-linked snapshot mount reserves the last 38 of 512 BACKSTORE
blocks (474..511) for a 4,096-word MEMFS pool. An older process owns
block zero. Actual snapshot shutdown writes an invalid commit header,
metadata, then a checked final commit header. During the *first* test
physical I/O completion a real `backstore_alloc` obtains a 90-block
process run (1..90); during the metadata completion it obtains an
additional 18-block run (91..108). No second bitmap is introduced.

The oracle validates all 512 allocation bits, physical output block
addresses, source pointers, commit order and version/range/header
fields. It requires 147 allocated sectors with both new processes,
then 39 after their independent checked releases. The original
process and all snapshot sectors remain allocated. The test passes.

Three source-linked changes are each compiled/executed as independent
negative tests and rejected by the exact `__test_exit` oracle: erase
the snapshot reservation mark, skip the final commit header, or treat
occupied allocator bits as free. Non-oracle test failures do not count
as accepted mutants.

Testkit: `tests/validation/daimos-p1c-snapshot-interleave-v1579.sh`,
`.s` and `.md`. The runner copies its input source to a unique
`$TMPDIR` workspace and preserves the original production checkout.

## Strict limits and next gate

The I/O callbacks are controlled native test doubles, not Type-236
or Type-270 physical transfers. They interleave actual allocator
code *during* actual snapshot code, but no independent scheduled
process is swapping and there is no simultaneous PI-driven I/O or
natural media failure. This cannot close v1578's full-guest P1-C
concurrency requirement. Next: one instrumented physical guest with
real process swap ownership across persistent MEMFS snapshot/HALT,
independent LOGSTORE operations and exact MM/BACKSTORE/BCACHE owner
proof, then mapped DEVICE/VOLUME controller-error/return isolation.

No new KCORE/MRES instructions, permanent RAM, process metadata,
on-media format, installed utilities or ABI changes. Fixed low end
remains `024364` octal. P1-D alternate inventory, P1-E uncompressed
PURE and final feature freeze remain separate open tasks.
