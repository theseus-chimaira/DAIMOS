# MEMFS XIP hardware contract v201 (2026-10-23)

Status: design verification only; the production EXEC loader is unchanged.

## Hardware cross-check

The PDP-6 has one user relocation base and one upper protection bound.
In SIMH, `sims/PDP10/kx10_cpu.c`, PDP6 `page_lookup`, every user access
within `Pl` maps to `(addr + Rl) & RMASK`, regardless of read, write or
instruction fetch. APR DATAO uses 02000-word relocation granularity.

The independent AAP model in `pdp6/emu/apr.c`, `relocate`, splits the
logical address into upper eight bits and lower ten bits, adds RLR to the
upper field, and checks PR. It likewise cannot mark individual words or
regions read-only. There is no independent private-data relocation base.

An ordinary DXR program cannot safely run out of a shared MEMFS file:
the stack, globals and arbitrary stores would change the file for every
other reader. An immutable-file flag or kernel pin does not block stores
through the user-space relocation hardware. Do not implement XIP by only
substituting a file physical base into `proc.vm`.

## Existing ownership constraints

* `vm_space_create` allocates and zeroes an aligned MM_TYPE_PROCESS extent.
* `vm_space_destroy` frees that extent as MM_TYPE_PROCESS and detaches swap.
* MEMFS data uses four MM_TYPE_KERNEL_DYNAMIC chunk allocations with
  0200-word sectors; MEMFS eviction can spill and free a whole chunk.
* `memfs_resize` can relocate a file; unlink and shutdown can free it.
* The DXR header and relocation map live alongside the image, and DXR2
  compressed payload is not an in-place executable image.

## Minimum viable trusted XIP class

A future **privileged** XIP class must be explicitly distinguished from
ordinary DXR: the program must avoid *all* user-memory writes (including
user stack writes), require no private BSS, and require no write-producing
user ABI facilities. Its code must reside at the correct logical origin
inside one 02000-aligned MEMFS-owned allocation. The kernel must pin that
allocation and retain its MEMFS ownership until the last process exits.
Unlink, truncate, resize, spill, and shutdown must reject or defer changes
while pinned. Process teardown must release the pin and must never call
MM_TYPE_PROCESS free on MEMFS-owned storage. Normal EXEC remains fallback.

This class is **not** yet validated: simply claiming that a binary is
read-only is not proof that it cannot issue a store. User programs that
need a writable stack cannot qualify. A regular DXR executable is never
implicitly admitted as XIP. Shared mutable XIP is not supported by the
PDP-6 hardware and cannot be made safe by a MEMFS-only change.

## Required acceptance gates before enabling

1. Define the explicit privileged image format and loading contract.
2. Add a non-reclaimable aligned MEMFS allocation and pin/refcount.
3. Add VM provenance and teardown distinct from MM_TYPE_PROCESS.
4. Specify how process swapping handles a pinned shared image.
5. Test hardware write behavior and relocation on both SIMH and AAP.
6. Add native program fixtures and concurrency/lifetime tests to
   daimos-testkit; report RAM and resident word effects separately.

No XIP code path is enabled by this document.
