# Proposal B v445: common CREATE/SYMLINK provider call

VFS CREATE and SYMLINK already shared result-node publication, but
each emitted an independent call/branch sequence for the same
`fs_provider_reg_call` request. Both now tail-branch to one call site,
which falls through to the existing vnode publication logic. The
provider operation and request arguments are unchanged.

Full linked PDP-6 GCC lowmem KCORE 8982 -> **8981** (one saved word);
KCORE+MRES 15529 -> 15528; PERMANENT_LAST 15576 -> 15575. No
permanent data or runtime stack allocation added.

Focused FILE/PIPE direct-mount, UMASK, native provider lock, and new
static shared-dispatch contract tests PASS. Dedicated CREATE/SYMLINK
native mutation regression remains desirable. This is a small
cross-operation consolidation, not completion of Proposal B.
