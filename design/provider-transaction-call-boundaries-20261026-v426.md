# Proposal B v426: transaction boundary inventory, baseline gates

Status: baseline native provider-lock tests PASS; **no production change**.

Existing daimos-testkit native PDP-6 serialized-provider regression
`test-daimos-provider-lock-native-v327` PASS (wait/retry, provider IDs
4..7 and invalid IDs). Static `test-daimos-provider-lock-contract-v325`
PASS. New testkit `daimos-provider-transaction-inventory-v426.py`
enumerates shared-workspace source references and checks that the current
lock path remains intact. It reports 60 references among seven named
files: D6FS runtime 15, DTFS runtime 5, DTFS resize 18, TSFS runtime
12, VFS 2, BCACHE 7, AUXSTORE boot 1.

Direct sleep-reference counts in the provider assembly are zero because
calls traverse I/O and cache wrappers. This **does not prove** that the
workspace can be released. Drivers that call `proc_wait_event` include
`drivers/dsk_io.s`, `drivers/drm236_io.s` and `drivers/tape_io.s`.
TSFS calls `dtfs_dtc_read` while using `fs_block_workspace`; other
provider -> driver paths still require a full call-path inventory.

Next implementation gate: trace source-level continuation state across
device waits and classify every buffer's live interval; then add a
native multi-process interleaving test in daimos-testkit. Only after
that may the global bridge lock be replaced. The 128-word transfer
workspace is already shared with BCACHE and is not free savings.

Accepted pre-experiment baseline stays KCORE 8995, KCORE+MRES 15543,
PERMANENT_LAST 15590. No new RAM saving or source mutation claimed.
