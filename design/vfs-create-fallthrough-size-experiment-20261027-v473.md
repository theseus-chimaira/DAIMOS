# Proposal B v473: rejected source-level CREATE dispatch cleanup

Inspected the shared VFS CREATE/SYMLINK dispatch after v472. The source
contains a `jrst vfs_create_dispatch_v445` directly before its target
label, with no other references. Removing the jump and allowing the
provider call to fall through is semantically straightforward.

The VFS shared-create and SYMLINK contract tests and the native DTFS
CREATE regression passed after updating the source-layout assertions
for the fallthrough form. Nevertheless the **full linked** GCC PDP-6
lowmem measurements remained KCORE 8960, KCORE+MRES 15507,
PERMANENT_LAST 15554: no permanent-word savings. The experiment was
reverted in both DAIMOS and daimos-testkit. No production change was
accepted, and no test contracts were left altered.

Follow-on optimization should prioritize shared FILE/VFS/provider
responsibilities and fully linked RAM changes, not source instruction
counts alone. Filesystem implementations may be restructured as long
as ABI, on-disk and error semantics remain correct. Strict 32K
acceptance remains deferred.
