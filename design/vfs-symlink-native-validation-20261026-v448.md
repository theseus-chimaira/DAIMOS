# Proposal B v448: PDP-6 native SYMLINK dispatch validation

New daimos-testkit standalone PDP-6 executable links actual resident
`vfs.s` against a recording provider. It invokes `vfs_symlink` with an
unmounted provider vnode and verifies the shared CREATE policy's
SYMLINK opcode 010, the zeroed unused provider E argument, the
counted-target argument pointer and successful nonzero vnode publication.
The test passes under p10run native PDP-6 instruction execution.

The first experimental fixture failed during unrelated `vfs_mount`
setup; an unmounted provider vnode exercises the requested policy
directly without imposing a mount prerequisite. No production source
or filesystem policy changed. This is provider-dispatch integration,
not full backing-filesystem SYMLINK lifecycle coverage.

KCORE remains 8972 linked lowmem words, the accepted v447 baseline.
Strict 32K acceptance remains deferred.
