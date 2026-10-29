# D6FS RO-first recovery bootstrap — v671 first increment

Accepted boot contract (implementation in progress):

1. KINIT initially mounts D6FS read-only, regardless of CLEAN/DIRTY state,
   but only when safe traversal of the bootstrap files is established.
2. The existing copied-image loader finds `/SYSTEM/INIT`; INIT executes
   `/SYSTEM/D6FSCK` and waits for successful validation/repair on **every**
   D6FS boot.  It then executes `/SYSTEM/MOUNT` to remount root RW, and
   only afterwards starts ordinary services.
3. If either command fails or critical metadata cannot be safely traversed,
   startup stops with the filesystem read-only.  An interrupted repair must
   never be published CLEAN without revalidation.
4. ROOT remains mounted throughout; open file descriptors and the resident
   copied INIT process do not require an unmount.  The RW transition must
   invalidate/rebuild any metadata cached before raw D6FSCK writes.

RAM constraint: full D6FSCK is a bounded-memory standalone userspace program,
not linked into KINIT or permanent KCORE.  Do not introduce a second
resident metadata allocator or automatic root write access.

First code increment v671: `d6fs_super_select_recovery()` reports the
structurally valid newest DIRTY generation separately from the existing
`d6fs_super_select()` CLEAN-only production mount contract.  The existing
selector remains unchanged to callers; there is **no** recovery RO mount,
userspace checker, syscall, or RW remount authorization in this increment.
Testkit `daimos-d6fs-recovery-select-v671.sh` PASS, alongside original
`daimos-d6fs-super-recovery-v1.sh` PASS.

Next implementation steps before enabling RO-first production boot:

- Validate restricted DIRTY traversal including both required executables
  and fail closed on invalid directory, FCB, extent or block references.
- Implement a native block-device D6FSCK with exclusive repair access and
  bounded workspace; preserve the host checker's ambiguous-corruption policy.
- Expose privileged, idempotent explicit-target remount with media and cache
  revalidation; prevent unchecked DIRTY->RW or accidental RO->RW toggles.
- Supply native MOUNT command, stage it and D6FSCK in boot images, and
  invoke both synchronously before INIT activates ordinary services.
- Native testkit: CLEAN/RO, DIRTY/RO, broken essential executables,
  interrupted repairs, multi-member BLOCKSET, and 32K memory high-water.

Do not relax the existing `d6fs_super_select()` policy to make an image boot
before those gates are implemented.
