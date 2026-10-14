# v132: KCORE, MM descriptor backing, then MRES

The required permanent low-memory order is **KCORE / MM table / MRES**.
The MM table begins at `__kcore_low_end`, spans exactly two words per
boot-selected descriptor, and the first MRES begins immediately after it.
This leaves the rest of the low KINIT gap contiguous and managed by MM.

`mm_boot_low_table()` checks this layout against the KINIT base and
physical memory without modifying state. It reports the descriptor base
and MRES starting address on success; failures preserve output values.
The testkit host regression passes for 32K, 64K, 128K and 256K sizes,
including capacity and KINIT overlap failures. The v131 active-capacity
native fixture and all fifteen earlier MM/VM PDP-6 fixtures pass.

**Not activated:** `mres_init()` still starts at `__kcore_low_end` and MM
still contains its fixed array. The layout must only become active once
the exported array symbol is converted atomically across MM, VM, packed
assembly and C consumers. Activating it earlier would allocate duplicate
descriptor RAM and could invalidate MRES placement. The activation must
initialize external backing before adding the low arena, then publish only
`[mres_base, KINIT_IMAGE_BASE)` for the low managed range. The existing
`mm_boot_reserve()` edge-trimming operation must still succeed for MRES.

The changed helper is disposable KINIT/MM boot code. No KCORE size saving
or linked-size acceptance is claimed.
