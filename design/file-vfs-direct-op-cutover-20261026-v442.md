# Proposal B v442: rejected FILE/VFS direct-op cutover

Status: full linked experiment rejected and reverted; no production
change accepted.

The experiment replaced FILE's indirect `vfs_mkdir`/`vfs_mkfifo`
function-pointer selection with a stored provider opcode and a single
VFS entry `vfs_make_node_v442`. The shared entry decoded MKDIR versus
CREATE-as-FIFO, checked eligible FIFO providers, and reused the VFS
create policy. This preserved the conceptual FILE/VFS ownership split
but added an extra opcode decoder.

Full linked GCC PDP-6 lowmem comparison:

| Metric | accepted v441 | experimental v442 | regression |
|---|---:|---:|---:|
| KCORE | 8987 | 8990 | +3 |
| KCORE+MRES | 15534 | 15537 | +3 |
| PERMANENT_LAST | 15581 | 15584 | +3 |

Rejected immediately under the resident-RAM acceptance policy and
reverted both `system/kernel/fs/vfs.s` and
`system/kernel/fs/file_runtime.s`. No runtime test is claimed for the
rejected candidate; linked-size regression alone disqualifies it.

Architectural conclusion: a shared entrypoint which preserves both
existing FILE and VFS decoding layers cannot save code. Future
cutovers must remove a complete ownership/dispatch layer together,
not add an opcode-based wrapper. KCORE remains 8987. Strict 32K
acceptance remains deferred.
