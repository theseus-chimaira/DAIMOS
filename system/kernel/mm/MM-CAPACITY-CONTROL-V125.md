# v125: MM descriptor capacity control

This is a strictly bounded intermediate step toward boot-sized MM backing.
The physical table remains the original 32-entry KCORE array; the active
capacity defaults to 32 and MUST NOT exceed physical backing capacity.

The PDP-6 insertion comparison now reads `mm_extent_capacity` using one
memory compare in place of the old immediate compare. No new permanent
allocator subroutine or repeated descriptor-loop load is introduced.
The portable C reference uses the same active ceiling. The new initialized
capacity word increases static MM state by one word until the fixed table
is replaced; the overall linked footprint has not yet been measured.

Regression evidence: all 15 v090-v122 native PDP-6 MM fixtures pass after
the correction of the compare skip condition; the v125 capacity fixture
additionally exercises a lower active ceiling (31) with 31 live descriptors
and checks atomic failure, unchanged count and unchanged output. Host
`make mm-core` runs successfully and reports `mm-v1: PASS`.

Remaining work: make the capacity writable only by boot initialization;
reserve a permanently valid descriptor table outside the KCORE image;
migrate *all* C and assembly users together (including VM and packed VM);
test memory-size selection, KINIT reservation exclusions and full system
boot; record real linked text/BSS and permanent RAM. Do not raise the active
capacity above 32 before the table migration is complete.
