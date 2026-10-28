# KINIT D6FS superblock-validation failure watermark v649

Native PDP-6 SIMH GCC 96K full-profile disk-root image with
`KINIT_STACK_WATERMARK=1` has KINIT image end `107501` octal and the
unchanged `0600`-octal (384-word) reserved stack.

Testkit `daimos-kinit-d6fs-mount-failure-v649.sh` first checks that
the original D6FS diskset is CLEAN, copies all four disk members and
invalidates only the magic words of both D6FS superblocks in the
copies. The DBOOT member descriptors, selected disk root and other
filesystem contents remain intact. Native SIMH boots, initializes
D6FS, then emits `?RT` and executes HALT. Its fatal-path watermark
records `0172` octal (122 decimal words), strictly below the `0600`
reserve. Test PASS. No production or permanent RAM changes.

This tests mount rejection after root selection, unlike earlier
selector failures. It does not prove other errors (allocation,
I/O failure or missing INIT) have bounded stack; they remain open.
