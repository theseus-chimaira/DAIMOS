# D6FS native raw inspection routed through removable provider (v678)

Full-image linking found a correctness/integration defect in v676-v677:
the resident syscall dispatcher directly referenced `blockset_read_block`,
which is exported by the removable BLOCKSET MRES and is not a fixed KCORE
symbol. The previous assembly-object checks did not detect the missing link.

The syscall now dispatches private operation 026 (octal) to the already
serialized D6FS provider MRES, which reads one logical block via the mounted
reader's existing `fs_backing` descriptor. No new permanent BLOCKSET mapper
or filesystem state is introduced. Privilege, RO policy, user buffer bounds,
and VM mapping lifetime checks remain on the syscall entry path.

Testkit v678, v677, v676 source checks PASS. A 32K lowmem build progressed
past the formerly undefined symbol, but failed its existing stack-safe KINIT
image-size gate (image end 104642 octal > 77400 octal). That is NOT a 32K
boot success. A larger-core build must be assessed separately.

The raw interface remains read-only. The userspace checker, exclusive repair
session, metadata revalidation and idempotent privileged remount are still
required before the unconditional INIT recovery sequence is enabled.
