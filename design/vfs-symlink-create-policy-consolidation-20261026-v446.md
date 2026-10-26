# Proposal B v446 — SYMLINK joins the existing VFS CREATE policy

The separate `vfs_symlink` read-only check, four-argument register
save/restore, provider result allocation, and vnode publication path
are replaced by a short argument mapping into `vfs_create_op`.
SYMLINK retains its opcode 010, counted-target argument, non-null
output requirement, and zeroed unused provider request argument e.
`vfs_create_op` already validates the output pointer, checks the
directory's read-only mount and publishes the returned vnode with
inherited mount identity. The mode/UMASK path remains specific to
CREATE/MKDIR and is not applied to SYMLINK.

This replaces the old operation implementation rather than adding
another wrapper around it. The transient one-word fifth-argument
stack allocation is released before returning, with no new permanent
data.

Full linked PDP-6 GCC lowmem: KCORE 8981 -> **8973** (8 saved),
KCORE+MRES 15528 -> 15520, PERMANENT_LAST 15575 -> 15567.

Testkit FILE/PIPE direct-mount, UMASK, native provider-lock,
DTFS CREATE success/rollback and new SYMLINK mapping contract PASS.
Dedicated full native SYMLINK runtime verification is still pending.
Strict 32K acceptance work remains deferred.

Rejected early v446 trials: eliminating a CREATE branch changed no
linked words (linker already relaxed it). Sharing four-word VFS error
cleanup increased KCORE from 8981 to 8983 and was reverted.
