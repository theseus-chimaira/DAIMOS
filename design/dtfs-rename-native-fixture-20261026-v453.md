# Proposal B v453: native DTFS RENAME fixture recovery

The existing daimos-testkit extracted production `dtfs_rename`
fixture failed to link because its standalone assembly omitted the
shared `kconst_1_1` stack adjustment constant defined by the kernel
in `core/ret.s`. The testkit fixture now supplies that same `1,,1`
word, following the previously repaired DTFS UNLINK fixture pattern.

The native PDP-6 DTFS RENAME test passes its ITS, TENEX,
destination-exists, and cross-mount-rejection cases. This tests the
provider body, not the complete FILE-to-VFS syscall transaction.

No production code, ABI, or persistent RAM changed. Full linked
lowmem baseline remains v452 KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. No new size reduction claimed.

FILE RENAME still needs both parent paths validated and kept live;
the provider requires old and new names and the VFS must reject
cross-namespace operations. Do not combine it with one-parent
mutation wrappers until the whole ownership boundary is replaced,
measured and regression-tested. Strict 32K remains deferred.
