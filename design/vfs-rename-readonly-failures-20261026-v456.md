# Proposal B v456: VFS RENAME readonly and failure behavior

Extended the native PDP-6 production `vfs_rename` test in
`daimos-testkit` to cover an active readonly source mount,
a readonly destination mount, writable mount dispatch and negative
provider result propagation. Rejected calls must not reach the
provider, while a provider error is returned to the caller.

The native VFS fixture and native DTFS RENAME regression PASS.
The test is not a complete FILE pathname or syscall transaction
test, nor does it exercise a real block-device failure under load.

No production code or ABI change. The last full linked measurement
remains v452 KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. No new RAM reduction claimed.
Strict 32K remains deferred. Future structural optimization must
replace an existing FILE/VFS ownership boundary rather than add a
layer, and must be measured on the fully linked image.
