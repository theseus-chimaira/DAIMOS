# Proposal B v434: linked KCORE layer census and next redesign target

Status: source/map audit only; no production change or RAM saving.

Using `build/system/boot/pdp6/kcore.map` at the accepted v432/v433
baseline, address differences (all numbers below decimal):

| Map span | Start octal | End octal | Span words |
|---|---|---|---:|
| `vfs_lookup` -> `fs_provider_reg_call` | 004274 | 005763 | 823 |
| `file_node_from_fd` -> `pipe_fifo_unlink_object` | 006154 | 010206 | 1050 |
| `exec_native_syscall` -> `native_sys_getchar_call` | 015675 | 017320 | 787 |
| `fs_provider_reg_call` -> `fs_memfs_service_jump` | 005763 | 006021 | 30 |

These are **contiguous linker address spans**, not exact per-subsystem
instruction counts: they can include constants, helpers, trampolines
or unrelated neighbors; the syscall region overlaps execution and
device-bridge responsibilities. Therefore do not add them to claim an
exact total or a recoverable saving. They do establish an order-of-
magnitude distinction versus the 30-instruction provider bridge.

The next architecture experiment should target the FILE/VFS boundary
and syscall dispatch as a *whole*: classify each operation's duplicated
path validation, provider number decoding, result publication and
syscall marshalling, then design one compact register ABI and ownership
contract that deletes a complete resident layer. Since v399 explicitly
allows caller/provider ABI changes, historical register conventions
must not disqualify a beneficial alternative. Preserve essential
filesystem capabilities and the tested resource invariants. First
implement one end-to-end vertical slice in a reversible versioned
patch; reject it unless post-fold linked KCORE shrinks and native
FILE/VFS/provider/process lifecycle tests PASS.

Current accepted KCORE baseline: **8990 words**. MRES-only reductions
are not a success criterion. Strict 32K acceptance stays deferred.
