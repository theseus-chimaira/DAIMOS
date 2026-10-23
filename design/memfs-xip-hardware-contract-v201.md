## v216 non-writing native user-mode lifecycle

The testkit now assembles a raw XIP fixture whose user-mode instructions
are only `MOVEI 2,01234` and monitor UUO `040`. The latter enters a PDP-6
executive-only handler, which verifies the MEMFS image remains unchanged,
calls the production `vm_space_destroy` body, and checks final unpin/VM
reset and release of the backing extent. This passes on SIMH and AAP.
The dedicated SIMH runner intentionally removes its default `break 20`
sentinel, since `020` is a *legitimate* user entry PC in this test.
The fixture stubs storage/scheduler behavior: real process scheduling,
EXEC/exit syscalls and a privilege-gated executable trust model remain open.
## v213 VM teardown integration

KCORE `vm_space_destroy` uses the existing FS register gateway (MEMFS op 026)
before process-owned MM free. An owned XIP base unpins through the movable
MEMFS MRES; unknown/unavailable/error cases reach only a type/owner-checked
process MM free, so they cannot free MEMFS's allocation. Native dispatch tests
pass and the lowmem KCORE grows nine words. Actual process-XIP startup remains
disabled until no-writing and rollback invariants are established.
## v212 exact-base ownership probe

`memfs_xip_owns_base(base)` reuses the pinned-node reference table to
recognize an exact live MEMFS physical base. It is read-only, incurs no new
resident data words, and returns false after final unpin. VM provenance and
safe KCORE-to-MRES release dispatch are still to be integrated.
## v211 reverse-release primitive

MEMFS MRES now offers `memfs_xip_unpin_base(base)`, which scans the existing
four-entry file reference table for an exact pinned physical base. It releases
one reference without requiring VM to retain a vnode. A last release unpins
the MM-owned allocation; invalid or repeated releases fail. The routine adds
no additional persistent state and does not itself change a process VM.
## v210 file admission mode restriction

`memfs_xip_admit` additionally rejects file modes that contain any write
permission bits (0222) or lack all execute bits (0111). The test image is
staged while writable and then changed to mode 0555 before admission.
This reduces accidental XIP admission and does not make user instructions
read-only: APR provides no write protection for shared image storage.
# MEMFS XIP hardware contract v201 (2026-10-23)

Status: MEMFS pinning foundation implemented and native-tested in v202;
the production EXEC loader is unchanged and XIP launches remain disabled.

## v207 explicit admission milestone (2026-10-23)

MEMFS MRES now exports `memfs_xip_admit(fs, vnode, &base, &entry)`.
This is **not an EXEC hook** and does not start a process. It requires the
existing large-file 02000-aligned allocation and checks a separate raw-image
format: word 0 is octal `054506000000`, word 1 is a full 18-bit logical entry
at or beyond `020`, within the file's logical extent. Generic DXR files are
rejected. Validation occurs while pinned; all rejected candidates are
un-pinned, and a successful caller owns the pin until explicit release.
The native testkit fixture checks both cases on SIMH and AAP PDP-6.

The header is **not a certificate of immutability or non-writing code**.
No user process may be mapped from it until a privileged executable trust
contract, no-write startup, swapping exclusion, and VM unpin-on-teardown
exist. EXEC/RUN and KCORE still have no XIP launch code.

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

## v202 MEMFS pinning milestone

The PDP-6 MEMFS MRES now exports `memfs_xip_pin`, `memfs_xip_unpin`,
`memfs_xip_is_pinned`, and `memfs_xip_any_pinned`. The reservation API is
kernel-only and accepts only a resident regular file with a dedicated large
chunk whose physical base is aligned to `02000` words. It does **not**
declare any DXR image safe to execute.

Four packed `(slot+1),,refs` words track the four possible dedicated MEMFS
chunks; multiple reservations for the same file consume only one MM pin.
The final release drops that MM pin before retiring the reference entry.
Pinned files reject writes and truncation, and their chunks resist reclaim;
unmount also refuses to free the metadata while any XIP reference exists.
`memfs_data_init` resets the reference table at mount initialization.

These routines reuse MM's existing physical pin mechanism; they do not
create a new independent memory-owner system. The ordinary mutable MEMFS
allocator did not originally guarantee `02000` alignment for large files.
The v203 allocator now requests `02000` alignment for new large dedicated
extents (including fault-in); small packed chunks retain their prior policy.
Executable admission and a non-writing contract remain prerequisites.

## v203 aligned large-file allocation

Large MEMFS file allocations now use MM's six-argument aligned allocator,
with `02000` alignment, `MM_TYPE_KERNEL_DYNAMIC` and MEMFS owner `011`.
Both SIMH and AAP native MEMFS pin tests pass. Fragmentation under pressure,
VM ownership and actual XIP execution remain untested.

Focused PDP-6 SIMH tests under daimos-testkit's `target-memfs-pdp6` fixture
exercise rejected small-file pins, aligned dedicated-file pins, repeated
references, blocked writes/truncation, reclaim exclusion, and final unpin.
The FIFO fixture also links and passes. The same MEMFS target fixture passes
on AAP using `run-aap-v202.sh`; neither test constitutes an XIP process launch.
