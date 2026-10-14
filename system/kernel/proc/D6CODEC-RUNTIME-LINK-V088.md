# Packed runtime permanent link v088

The active PDP-6 KCORE build now links the native v078 inline encoder,
its no-allocation sizing and decode ABI, the relocatable v088 inline decoder,
the v077/v046 MM transactions, lifecycle gate v082, slot-0 mode dispatcher
v083, and storage-aware destruction v084. The active swap assembly source
selection is v083 and the active VM destroy leaf is the v088 derivative of
the tested v084 assembly path. The original DISK swap-in remains available.

The separate low-core D6LZ bootstrap decoder is **not** replaced. The KCORE
inline decoder uses its own relocatable symbol and 30 relocated internal
branches; it cannot collide with the Stage-1 fixed decoder at 000060.
The real KCORE and KINIT DXR links PASS. A standalone native PDP-6 test
executes measure/encode/decode through that relocatable decoder, PASS.

NOT YET ACCEPTED: actual KINIT boot, scheduler eviction policy for selecting
a sleeping resident candidate, real MM allocation under system pressure,
TSFS 1.3 mount/read, and device matrix (minimal 32K TSFS; minimal 32K
D6FS; fully attached maximum-memory PDP-6). The KCORE endpoint/8K limit
is deferred as explicitly requested. Never infer boot success from link.
