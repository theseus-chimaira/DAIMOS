# PURE swap backing: 26-mount extension without resident RAM growth (v20)

## Representation

The existing one-word resident swap record remains exactly 36 bits:

| Location | Contents | Width |
|---|---|---:|
| Swap record bits 0..17 | Executable vnode index | 18 |
| Swap record bits 18..21 | Zero-based mount slot bits 0..3 | 4 |
| Swap record bits 22..35 | Immutable PURE text length | 14 |
| Stable u-area word `066`, bit 27 | Zero-based mount slot bit 4 | 1 |

The fifth slot bit is a spare credential-word bit above UID9, GID9 and UMASK9.
There is still only one swap-record word per process and no extra word in the
stable u-area. Mount slots 0..25 cover public IDs 1..26. A 26-slot maximum
also retains the full 18-bit vnode index and all 14 PURE-text length bits.
The provider is derived from the live VFS mount root; only a mounted provider
can be reconstructed. The unpacker checks `vfs_mount_limit` **before** using
the published MM slab base, and rejects missing roots and invalid providers.

## RUN staging and lifecycle

RUN deliberately loads and attaches the executable **before** allocating the
child's u-area. It cannot publish the fifth bit into credential word 066 at
attach time. Instead, while the child is `PROC_SIDL` and does not yet own an
u-area, `proc_swap_attach()` stages the one bit in scheduler RH bit 0. This
bit is otherwise unused until SRUN starts using scheduler RH as its intrusive
run-queue link. After RUN allocates and initializes the u-area, it copies
UID/GID/UMASK credentials from the parent **without** copying the parent's
PURE backing extension, then transfers the staged bit into child word 066.
The final SRUN scheduler word is initialized without any staging bit.

Attachment for an existing process with a stable u-area updates bit 27
directly, leaving UID/GID/UMASK and all other credential bits unchanged.
`proc_swap_detach()` clears the attached bit or, if SIDL has no u-area,
the staging bit. A staged attach outside SIDL or a missing backing mount is
rejected. Non-PURE backing sets a zero text length, so swap always transfers
the entire image regardless of its mount ID.

## Swap, EXEC, and protection

Swap-out keeps the complete resident backing record in the stable u-area
while its one-word per-process swap record temporarily holds the BACKSTORE
span. The credential extension bit survives. Swap-in and the optional PURE
contract verifier pass the **owning u-area** to the native reload decoder;
no global current-slot assumption is permitted. Native `proc_swap_backing_busy`
and `proc_swap_mount_busy` compare all five mount bits and skip process slots
whose metadata does not indicate a valid stable u-area.

Transactional EXEC snapshots the old and newly attached extension bits in
two transient kernel-stack words, alongside its existing old/new swap
records. Success publishes the new bit even after `vm_space_destroy()`
detaches the old backing. Any load, startup, or old-image-destruction failure
restores the original backing record **and** the original extension bit.
This does not change resident kernel/process descriptor size. Destructive
low-memory EXEC continues to call detach before loading its new executable,
which acquires the appropriate new extension through the normal attachment.

## Verification and pending work

Native PDP-6 tests in daimos-testkit verify mount 26 versus mount 10 (same
four low slot bits); maximal vnode index and text length; a missing backing
mount; published runtime mount-limit bounds; immutable-text busy protection;
native C99 attachment and SIDL staging; RUN credential inheritance; and
EXEC success/rollback through its actual assembly transaction. The complete
raw/compressed PURE swap-out/in tests and the diagnostic contract verifier
also run with `TEST_MOUNT_ID=26`. The diagnostic disk-root SIMH ROOT login
regression passes after the SIDL staging conversion; its DSH size override
is diagnostic only.

**Production capacity remains four mounts.** No claim of 26 live mounted
volumes is made until remaining provider-local bounds and optional foreign
DTFS personality indexing are converted and boot capacity selection is
activated. The strict 32K KCORE size gate is also still not satisfied.

## Resident-size and boot verification (v20 final)

Relative to v19, the fully checked fifth-bit lifecycle costs **116 linked
permanent words**, adding **zero per-process words**. PDP-6 disk-root
single/multi permanent ends are `037355`/`037416` octal, with 274/241 words
free below the `040000` gate; tape-root permanent last is `036537`, with
672 words free. GCC lowmem 32K KCORE end is `024142`, 2,146 decimal words
above its strict `020000` threshold. Disk/tape boot image ends are
`073042`/`074001`, leaving 1,502/1,023 decimal words after the reserved
KINIT stacks. Measurements do not require a shell; these are **not** 32K
runtime boot acceptance.

The diagnostic disk-root PDP-6 reaches ROOT login and executes ECHO under
the existing test-only DSH size override. The first attempted implementation
failed this boot because RUN attaches the executable before acquiring the
child u-area; staging the bit in SIDL fixed that genuine regression. The
native tests exercise mount IDs 10/26, maximum record fields, bounds before
slab indexing, full RAW/D6LZ PURE swap and verifier, RUN staging/inheritance,
and EXEC rollback/commit. The older host `mm-core` test currently cannot
link because its fixture lacks pre-existing `vfs_mount_limit` and native
storage-state service symbols; this is an existing test-harness limitation,
not a new target-side compiler or assembler failure.
