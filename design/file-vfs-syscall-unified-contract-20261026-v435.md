# Proposal B v435 — unified pathname request: first vertical slice

Status: interface design and source-grounded call-chain inventory only.
No accepted code size change, ABI cutover, or test success claimed.

## Existing OPEN chain

1. `proc/syscall_dispatch_pdp6.s:%L67` maps the user path under
   `native_sys_map_one`, which calls `vm_user_mapping_hold` and
   `vm_user_words`. It also normalizes public OPEN flags in eight
   instructions before calling `file_open`.
2. `fs/file_runtime.s:file_open` resolves a path and handles access,
   descriptor publication, append state and FIFO ownership.
3. `fs/vfs.s` routes namespace operations to built-ins or providers;
   `fs_mres.s:fs_provider_reg_call` serializes sleeping physical
   providers around the shared 128-word workspace.
4. The syscall veneer releases the user mapping through
   `native_sys_mapped_return`, even if I/O blocked.

## Proposed first replacement contract

Introduce a *single pathname request ABI* with `mapped_path`, `opcode`,
`flags/arg` and caller-owned result, rather than creating another
generic provider trampoline. Its entry owns exactly one VM mapping
lease; it normalizes user flags once, invokes the operation policy,
then releases the mapping on every return. Provider code must not
receive raw user pointers or retain the mapped pointer after return.
The provider-wide I/O lease remains separate: it protects mutable
backing state, the shared block buffer, and blocking device waits.

The proof obligation is to replace, not supplement, old branches:
delete the corresponding `exec_native_syscall` cases, mapping cleanup
veneers and FILE operation wrappers *atomically* for the selected
operation(s). A new entry added on top of the old ones would increase
resident RAM and is unacceptable.

Start with `OPEN` and a paired error path, since it crosses all three
layers and has observable resource ownership. Track successful OPEN,
already-existing file, FIFO, denied access, invalid user pointer,
descriptor exhaustion and failed provider OPEN. Preserve process
stability across blocking I/O and RUN descriptor inheritance.

The next coding step needs a disposable build with the one-operation
cutover, native tests in `daimos-testkit`, and full linked PDP-6 GCC
lowmem KCORE measurement. Discard if KCORE is >= 8990 words or the
new contract does not demonstrably improve the measured total RAM.
MRES-only reductions do not count. Strict 32K acceptance is deferred.
