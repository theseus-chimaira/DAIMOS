# v118 — Restore legacy real-MM host pressure and swap regression

Base: DAIMOS v117 and daimos-testkit v117. The `make mm-core` target
previously failed at link: the portable host fallback lacked definitions
of the PDP-6 assembly-owned `proc_storage_publish_v30` and
`proc_swap_storage_invariant_v30` entry points.

The failure was deeper than missing symbols. Portable C swap-out and
swap-in in `vm_pdp6_swap.c` omitted the two-bit process storage map
publication performed by the production PDP-6 assembler. During a
reclaim-triggered swap-out, the resident MM extent could be freed and a
disk swap record published without changing storage mode. Later detach
correctly required DISK->RESIDENT and would reach an invariant violation.

The correct fix adds the same checked RESIDENT->DISK publication after the
portable C swap-out commit, and DISK->RESIDENT publication after a
successful portable C swap-in commit, before clearing TRANSITION. A mode
mismatch invokes the pre-existing fatal invariant handler; no permissive
fallback or silent state repair is introduced. These are correctness fixes
for the portable swap path, **not kernel optimization beyond MM**. The
PDP-6 production swap assembler and MM routines are unchanged.

The testkit now links the actual `proc_storage_map_v28.c` implementation
and a C99 host fixture implementing the two assembly-owned publication
symbols through `proc_storage_change_v28`, plus a hard invariant failure
handler. The existing host mm-v1 test explicitly checks the DISK state
after swap-out, RESIDENT after successful slot-0 service swap-in, and
unchanged DISK after forced swap-in read failure. The full legacy
`make mm-core` host suite, including MM pressure, fragmentation,
compaction, swap reclaim and rollback, **PASS**. GCC PDP-6 KCORE/KINIT
DXR link **PASS**.

This does not prove the production PDP-6 swap state transitions under a
booted kernel, or all real-allocator native reclaim/retry paths. Those
remain open in KCORE.MD. The MM assembly footprint is unchanged at
**482 text + 70 BSS PDP-6 words**. No BLT optimization was accepted.
