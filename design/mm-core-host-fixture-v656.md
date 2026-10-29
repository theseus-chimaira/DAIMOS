# daimos-testkit mm-core host-fixture repair v656

The v656 `make mm-core` host regression builds, links, and ends in
`mm-v1: PASS` against the current DAIMOS source. Production kernel
source and permanent KCORE RAM are unchanged.

The old testkit Makefile still linked the deleted PDP-6 VM C file
`proc/vm_pdp6.c`. The live VM functions now reside in
`proc/vm_pdp6.s`. Testkit `tests/system/mm-v1/vm_host_v656.c`
provides a C99 reference for the fixture's exercised MM-backed VM
lifecycle: process extent movement, pin-sensitive swap eligibility,
and process VM release/detach. This reference is test-only and does
not claim complete MEMFS XIP/provider teardown coverage.

The host test also had two stale assumptions: the MM descriptor table
is runtime sized (`mm_extent_capacity`, not `MM_MAX_EXTENTS`), and
`proc_slots_for_core` uses 8 slots at 32K, two more per additional
8K bank, capped at 64. Tests now use the production policies.

Result: `make -s mm-core DAIMOS_REPO=... PDP10_PREFIX=...` PASS.
Further target-native PDP-6 assembly testing remains authoritative.
