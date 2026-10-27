# Proposal B v463: native dual-path mapping through FILE and VFS RENAME

Added a native PDP-6 integration regression to daimos-testkit combining
the actual `native_sys_two_paths` mapping helper, actual `file_rename`
frame, actual resident `vfs_rename`, and a recording provider. A small
test entry drives precisely the syscall's map/rename/release sequence;
it is not the complete monitor-UUO dispatcher.

Verified under native PDP-6 execution: successful RENAME; failure of
either translated user pointer; failure at either parent pathname
lookup and either parent permission check; and provider failure.
Every invocation acquires/releases exactly one VM hold, translates
both pointers, and the provider is never called without a live hold.
Each case returns the expected status. Test PASS.

The provider does not sleep; integration with actual syscall dispatch,
deterministic provider blocking/wakeup and full cross-mount checks
remain necessary before any destructive wrapper cutover. No production
change; accepted linked lowmem baseline KCORE 8961,
KCORE+MRES 15508 and PERMANENT_LAST 15555. Strict 32K deferred.
