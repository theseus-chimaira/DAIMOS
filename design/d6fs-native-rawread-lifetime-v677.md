# D6FS raw-read syscall mapping lifetime correction v677

Audit of v676 revealed two inconsistent failure branches in
`native_sys_d6fs_read_block`: the buffer-bounds rejection released an
acquired VM mapping before falling through the common mapped-return helper,
which released it again; the RO-policy rejection reached the same common
helper despite not having acquired a mapping. Both cases are unsafe.

The v677 correction separates pre-mapping and post-mapping failures.
Pre-mapping rejects discard the two saved arguments and return -1 directly.
Post-mapping rejects discard arguments and pass through the single standard
`native_sys_mapped_return` release. Failed `native_sys_map_one` has already
released its own failed hold and therefore takes the unmapped return.
No new storage, syscall, ABI, or recovery privileges were introduced.

Testkit `daimos-d6fs-rawread-lifetime-v677.sh` and existing v676 source
checks PASS. PDP-6 assembler object build PASS. Native runtime fault
injection and full-system D6FSCK integration remain pending.
