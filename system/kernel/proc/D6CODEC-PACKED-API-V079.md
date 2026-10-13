# Native inline codec + packed-process API v079

The staged `proc_packed_inline_v079.c` binds the exact-size eviction
transaction v077 to native v078 size/encode callbacks and the registry-aware
v46 restore transaction to native v078 decode. The API exposes two calls:
`proc_packed_inline_evict_v079(slot, retained_offset)` and
`proc_packed_inline_restore_v079(slot, backing, pure_reloader)`.

Host-MM tests execute the actual v068 inline stream through this API,
including both dense/sparse registry layouts, variable compressed lengths,
PURE reconstruction and packed registry/MM retirement. Native PDP-6 linkage
test executes both public entry points with transaction stubs that invoke
the actual v078 native callbacks, decode and compare restored words. Both
pass. The transaction stubs are test-only: full native MM ownership is
validated by the host C mock, not by the PDP-6 simulator yet.

A PDP-6 register-numbering bug in the original v078 count-only encoder was
fixed in this checkpoint. It incorrectly used a nonexistent ninth octal AC
as independent mode state; this aliases AC11 match flags. The corrected
encoder keeps its measure/write flag in a balanced stack local below the
saved cursor and uses memory SKIPN on that local. Failure branches in
native tests now always return nonzero, preventing a previous false PASS
when inspecting a zero stream word.

Production scheduler, TSFS, boot and disk-swap defaults are unchanged. A
native kernel-linked MM transaction test, full simulator boot, and mandatory
KCORE exclusive endpoint <=020000 octal remain required before activation.
No resident-code size optimization is attempted.
