# v444: restore native DTFS UNLINK test linkage

The native standalone `daimos-dtfs-unlink-v1.sh` previously failed
before entering its test code with `dlink: undefined symbol:
kconst_2_2`. Its extracted `dtfs_unlink` procedure now uses the
ordinary shared kernel stack constant from `core/ret.s`, but the test
only linked the extracted procedure and its own stubs.

The testkit fixture now defines `.word 2,,2` under `kconst_2_2`,
matching the real kernel constant. This is a fixture linker dependency
repair, not a workaround or kernel change. With the fixture fixed,
`test-daimos-dtfs-unlink` **PASS** (native/ITS, TENEX chain,
corruption and rollback). The earlier v443 FILE/PIPE, UMASK, OPEN
and provider tests were also PASS.

The v443 VFS mutation consolidation itself was not modified.
Accepted KCORE remains 8982 words. Full mutation syscall-level
validation remains outstanding, as the DTFS test exercises provider
unlink directly rather than the VFS mutation bridge.
