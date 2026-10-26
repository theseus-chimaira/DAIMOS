# Proposal B v454: RENAME ownership and namespace regression

Strengthened daimos-testkit coverage without changing production code.
The namespace test now explicitly checks rejection of each of twelve
single-bit changes to every one of the 4096 provider/mount identities,
as well as independence from local vnode bits. A new FILE RENAME source
guard checks both parent lookups, both write/search permission checks,
the 14-word frame, argument handoff to VFS, and stack cleanup.

Both static tests PASS. Native PDP-6 FILE and pathname-walk fixtures
PASS; extracted native DTFS RENAME PASS (ITS/TENEX, destination-exists,
cross-mount). This is not a complete integrated syscall transaction
test, and a source guard cannot prove runtime rollback or concurrency.

Neither the old/new parent records nor the mapped-path ownership can
be dropped merely by reusing a one-parent mutation adapter. No
architectural source change has yet demonstrated net resident savings
under the current lifetime constraints. Accepted fully linked v452
measurements remain KCORE 8961, KCORE+MRES 15508,
PERMANENT_LAST 15555. No new RAM reduction is claimed.

Next attempt should build an integrated native FILE/VFS RENAME test,
then evaluate replacement of a complete pathname-wrapper boundary,
not the introduction of another trampoline. Strict 32K deferred.
