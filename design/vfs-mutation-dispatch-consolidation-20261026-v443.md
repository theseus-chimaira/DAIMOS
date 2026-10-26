# Proposal B v443: unify VFS mutation checks and provider dispatch

Accepted cross-operation resident code consolidation. VFS had two
separate paths checking readonly mounts and redispatching to the
filesystem provider: the two-argument UNLINK/CHMOD path and the
three-argument common setattr path (CHOWN/UTIME). These now use one
three-register stack frame and `vfs_mutate3` continuation. The
operation opcode in AC6 remains unchanged by `vfs_readonly`.

Full linked GCC PDP-6 lowmem measurement:

| Measurement | v442 accepted | v443 | Saved |
|---|---:|---:|---:|
| KCORE | 8987 | 8982 | 5 |
| KCORE+MRES | 15534 | 15529 | 5 |
| PERMANENT_LAST | 15581 | 15576 | 5 |

The two-argument mutations now save one additional caller-scratch AC3
word on the process-private stack. There is no new permanent allocation.
This is an explicit RAM-versus-execution trade: two-argument operations
execute extra PUSH/POP while the resident dispatch duplicates disappear.

Testkit v443 static call/frame test PASS. Existing native provider-lock
PDP-6 test, FILE/PIPE directmount, UMASK and OPEN lifetime tests PASS.
Full mutation-specific native filesystem runtime validation remains a
follow-up before treating the whole FILE/VFS project as complete.
Strict 32K acceptance remains deferred.
