# Swap-out MM unpin failure must fail closed — v667

The v666 late-rollback audit exposed a missing failure check in the
PDP-6 production `proc_swap_out` transaction. If `mm_unpin` rejected
the pinned resident extent, the ordinary cleanup code could clear
TRANSITION and return with an unresolved pin/ownership inconsistency.
That is unsafe: a scheduler-visible resident process might retain a
hidden pin, and the cleanup path cannot legitimately assume the MM
metadata is intact.

v667 changes both calls to `mm_unpin` in swap-out: after successful
BACKING write and on rollback following allocation/write failures,
an unexpected failure goes to the existing
`proc_swap_storage_invariant_v30` halt path. It cannot rejoin the
scheduler. When unpin succeeds but `mm_free` fails, the original
resident extent remains authoritative, the provisional BACKING span
is released, and TRANSITION is cleared as before.

This fixes the silent failure path at the ownership boundary, without
adding a workaround, new resident state, or storage layer. The v667
source regression and adjusted v666 ordering test pass. Native PDP-6
execution of both actual unpin-failure branches also PASSes: rejected
unpin enters the existing invariant handler. A fresh GCC 32K disk-root
image builds and boots through `proc_boot_init` with BSET/D6FS loaded.
Full-system MM fault injection remains open. Unpin failure is
intentionally fatal rather than recoverable.
