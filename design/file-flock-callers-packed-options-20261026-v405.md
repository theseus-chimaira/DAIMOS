# Proposal B v405: production FLOCK callers and packed-format options

Status: source audit only. No kernel ABI or production source edit.

## Callers and scope

Repository search finds userland FLOCK exposed by libc (`dsys.h`,
`syscall.s`), toolchain linkage (`dlink.c`) and native syscall routing,
but no direct production userland call to `dsys_flock` was found.
Tests in daimos-testkit exercise lock families and conflicts. This search
does not establish that external applications never use the API.

The current FLOCK conflict/update loops traverse only `file_table`, the
current process's 16 descriptor records. They therefore cannot directly
detect a second process's locks. RUN copies packed family identifiers into
the child's independent table; this does not establish system-wide lock
ownership or cross-process exclusion. Do not silently document FLOCK as
an interprocess lock implementation.

## Packed representation versus canonical representation

The packed format already puts nine nominal metadata bits into unused
vnode positions; the high mount bits are separately reconstructed.
Removing four family bits without migrating those positions back to mount
identity could simplify FLOCK but does not by itself eliminate the map,
unless the vnode/metadata bit assignments are redesigned together.

An alternative packed layout should first allocate the full mount width,
then fit only proven necessary permission/type/append/lock bits in the
remaining canonical-vnode-zero positions. Calculate the exact overlap
using 36-bit masks and audit all OPEN, DUP, RUN, FIFO, syscall and CLOSE
consumers. The source currently rejects mount bit 5 in `file_new_fd`;
the planned hardware-dependent maximum of 26 slots fits in five mount
bits, while future expansion needs a separate design. A temporary
reassignment must not quietly prevent future mount growth.

## Next acceptance gates

1. Confirm whether a per-process-only FLOCK contract fulfills the OS's
   necessary capability, with explicit behavior for independent OPENs,
   DUP, RUN, CLOSE, failed CLOSE and unmount.
2. Produce bit-exact 36-bit masks and a size model for a smaller packed
   layout, including use of former family bits for mount identity.
3. Only then prototype against a measured full link. All new regression
   tests belong in daimos-testkit. Preserve the existing v388 image as
   the measured reference; no v405 link or runtime test is claimed.

Reference: KCORE 9109, KCORE+MRES 15657, PERMANENT_LAST 15704.
