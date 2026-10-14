# v133: Direct-addressed descriptor backing outside linked KCORE

The native PDP-6 `mm_extents` symbol is now a zero-sized label in the final
KCORE linker object, coincident with `__kcore_low_end`. Removing the old
0100-octal-word BSS allocation keeps **every existing direct MM and VM
memory-reference instruction unchanged**. The linker resolves the physical
table address normally; neither self-modifying code nor a runtime pointer
register is needed.

KINIT now calls the validated `mm_boot_low_table()` calculation, zeroes the
reserved `2*mm_extent_capacity` words, publishes its low free arena starting
after the table, and makes MRES installation start after the reservation.
Capacity remains 32 descriptors at this checkpoint. The C reference
allocator retains its own host-backed array (same exported API), so native
tests supply standalone fixture storage when not linking the full KCORE.

The tested KCC disk-root KCORE decreased from 12463 to 12399 decimal words
exactly (64 words). The KCORE linker map confirms both boundary symbols at
030237 octal. Host `mm-core` passed, 16 native PDP-6 MM/VM regressions
passed, and the linker-address check passed. KINIT compiles.

**Boot acceptance is not complete.** The tested full KCORE already extends
past the KINIT 030000 boundary; the lowmem build ends at 027131, failing
the stricter 020000 KCORE gate. The new KINIT layout deliberately refuses
descriptor reservations that overlap KINIT. No real system boot or valid
linked full-image acceptance is claimed yet. The external table consumes
the same 64 physical words outside linked KCORE; total RAM is not saved.

Next: produce an image satisfying the relevant KCORE and KINIT boundaries;
verify MRES install/boot under PDP-6, test 32K/64K/128K/256K boot memory
configurations and descriptor ceilings beyond 32 before enabling tiered
capacity selection.
