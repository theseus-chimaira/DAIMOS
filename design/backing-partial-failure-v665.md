# BACKING multi-block failure contract — v665

The unchanged production `fs_mres.s` BACKING transfer routine was
executed under native PDP-6 instruction emulation. In a three-block
direct DEVICE write, an injected failure on block two makes
`backstore_write` return -1 after exactly two device calls; block
three is never attempted. Testkit native v665 PASS.

Source transaction audit (testkit v665 PASS) verifies process swap-out
branches to rollback before publishing its DISK record and MEMFS
eviction branches to free its newly allocated extent before publishing
the per-node backing pointer. Both callers retain their original
in-memory authoritative data upon this write failure.

Important limitation: BACKING writes are **not physically atomic**.
Earlier blocks may have reached the device, and any arbitrary failure
after process image deallocation requires separate lifecycle recovery.
The guarantee tested here is fail-stop at the first failed block plus
nonpublication of a newly allocated backing extent. Live full-system
in-flight faults, MM unpin/free errors, and persistent snapshot
recovery remain unverified. No production code or size optimization.
