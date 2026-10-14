# v124 — MM replacement boundary and KCORE bootstrap dependency

Architectural requirement: if MM is to be replaceable, its implementation
must not remain linked permanently into KCORE. Its publicly visible state
and calls must have stable lifetimes across relocation or replacement.

## Existing dependency cycle

KINIT currently calls `mm_boot_init`, publishes managed arenas, and uses
`mm_alloc` to install MRES images (`mres_install`). Moving all MM allocation
code into an MRES without a bootstrap strategy would make that MRES depend
on its own installer. Existing MRES are boot-installed permanent resident
images, not yet dynamically hot-swappable services.

## Required separation before boot-sized table conversion

1. Keep only the minimum bootstrap physical-region reservation mechanism in
   KCORE/KINIT. It must reserve the MM implementation and its metadata before
   normal managed-core allocation, without calling the MM that it installs.
2. Keep a stable, minimal KCORE MM service/dispatch ABI (or equivalent stable
   entry indirection) so process, VM, filesystem and swap code never retain
   addresses inside an implementation that could be replaced. This stable
   indirection is a deliberate memory cost requiring measurement.
3. Keep descriptor ownership and boot-sized table backing independent of the
   replaceable implementation. A replacement must adopt the existing state;
   it must not zero extents, release occupied memory or change descriptor
   representation without an atomic migration protocol.
4. Move the full normal allocator, gap scanner, descriptor helpers and
   compactor out of permanent KCORE only after auditing direct calls and
   direct `mm_extents` array references in `mm_pdp6.s`, `vm_pdp6.s`, packed
   VM, C/boot and process code. KINIT installs and publishes the MM service
   before any consumer requires normal allocation.
5. Define safe quiescence for any **future** live replacement: no active MM
   calls, blocked reentry and PI/VM activity, validated state ABI, and rollback
   to the original service on failure. This is *not* a request to introduce
   dynamic load/unload support ahead of the KA10/DAIMOS v1.1 deferral.

## Implications

The v123 direct `mm_extents` pointer rewrite cannot be applied independently
without settling this service/state ownership boundary. A boot-sized table
must be created by KINIT and remain permanently valid, while the MM code is
separately placeable. There are two compatible implementation milestones:
(a) boot-time relocated MM with stable service addresses, and (b) optional
hot replacement deferred to v1.1 unless explicitly reprioritized.

This is a **design gate only**. No MM assembly, KINIT or linker conversion has
been performed in v124. The working production baseline remains 482 MM text
words and 70 BSS words, as measured prior to this architectural change.
