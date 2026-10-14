# v126: External MM descriptor-table ABI audit

## Current verified checkpoint

The v125 capacity control keeps all 32 descriptors resident. The matching
PDP-6 emulator baseline passed all 15 native MM fixture scripts, including
real VM relocation, pinned obstacles, repeated compaction, pressure and
31/32 capacity transitions. Host `make mm-core` also passed.

## ABI inventory and hard constraints

The production PDP-6 `mm_pdp6.s` directly addresses `mm_extents` in 26
instructions (not counting its export and definition). The production VM
relocator `proc/vm_pdp6.s` has five direct address uses, including an
address-forming MOVEI; the staged packed VM variant has five more. Portable
MM and packed-process C consumers use an exported array symbol as well.

The exported `mm_extents` array is **not** a pointer. Replacing its first
word with a pointer without converting *all* clients corrupts descriptors.
Direct PDP-6 addressing currently needs no extra pointer load. An external
table may require base-register setup and changed register allocation in
multiple hot routines. Such a conversion is not accepted on source-level
word counting: measure real linked KCORE and KINIT before and after folding.

The permitted target is no executable growth, with eight extra resident
words as an *experimental upper bound*, not automatic acceptance. Net
permanent KCORE must shrink, including the service pointer/capacity words.
Total physical RAM includes the externally reserved table and must also be
reported. Do not make the table a normal MM allocation: KINIT must reserve
it before publishing managed free arenas, and its lifetime lasts the boot.

## Next implementation gate

Implement and test a complete table-address ABI in one change spanning
`mm_pdp6.s`, both VM assembly variants, `mm.c`, `mm_boot.c`, `mm.h`, packed
C consumers and KINIT initialization. Preserve the original 32-entry table
until every consumer is converted. Reject unmeasured runtime instruction
patching or partial pointer conversion; neither is a correct ABI migration.

New testkit cases must cover boot physical-memory sizes 32K, 64K, 128K,
256K, failed reservation and physical overlap, capacity transitions,
relocation/compaction under an external descriptor base, and failure
rollback. Keep the existing 15 native emulator regressions mandatory.

No external-table change is claimed in v126.
