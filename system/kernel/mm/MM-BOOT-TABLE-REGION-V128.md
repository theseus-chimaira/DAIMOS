# v128: Validated disposable MM table placement calculation

`mm_boot_table_region` now checks the physical memory limit, KINIT image
boundary, bootstrap stack reservation and the requested descriptor storage
without allocating anything. It returns the table base immediately above
the KINIT stack and the first subsequent managed-core address. Invalid or
insufficient layouts leave both outputs unchanged.

This is a boot-only prerequisite for external MM descriptor backing; it
does not change the resident table symbol, MM capacity policy, or existing
KINIT arena publication. In particular, the result is not yet consumed by
KINIT and must not be considered a working dynamic table.

The host MM test exercises 32-, 48- and 64-descriptor layouts, including
rejected short and out-of-range memory sizes and output rollback; `make
mm-core` passes. All 15 existing production PDP-6 MM/VM emulator fixtures
pass without modifications. The helper belongs to disposable `mm_boot.c`;
no permanent MM assembly code or KCORE BSS was altered.

Before enabling the result, convert all C and native assembly descriptor
consumers atomically, exclude this reservation from KINIT-managed arenas,
verify KINIT late-stack reclamation boundaries and linked footprints,
and run complete native boot and low-memory acceptance tests.
