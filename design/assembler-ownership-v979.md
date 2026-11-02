# Single authoritative implementation per kernel operation (v979)

The PDP-6 DAIMOS kernel has one implementation of each operation. If the
operation is implemented in assembly, do not add a second C implementation,
a compile-time C fallback, or a matching alternative algorithm. Tests belong
in daimos-testkit; tests should exercise the actual production implementation.

## Removed duplicate C bodies

- `proc.c`: stable u-area release, TTY session release, rollback discard,
  child adoption and finish, event apply, wait status, scheduler selection,
  and swap victim selection. The PDP-6 assembler already implements them.
- `vm_pdp6_swap.c`: swap transfer, transaction, service, and reclaim
  alternatives. The assembler swap implementation is authoritative.
- `file.c`: alternate `file_open`. The PDP-6 assembly OPEN implementation
  is authoritative.

The kernel Makefile no longer passes `*_PDP6_ASM` feature switches to hide
these alternative bodies; their source is deleted. The C implementation of
`proc_exit_finish` remains because it is shared exit policy calling the
assembly owner finalizer, not a duplicate implementation of that finalizer.
Similarly, the remaining C filesystem, TTY buffer, and swap-detach helpers
implement distinct services.

Do not reintroduce conditional duplicate bodies. If another machine target is
introduced, explicitly design its assembly or platform-specific mechanism
rather than silently adding fallback algorithms to common kernel C files.

The source deletion alone does not guarantee a smaller linked kernel because
these C bodies were excluded by previous build switches. Verify actual image
sizes and regressions independently of source line savings.

## Whole-file cleanup v980

The disabled C-only files `mm/mm.c`, `fs/memfs.c`, and
`fs/d6fs_provider.c` were also deleted, along with their dependency-only
Makefile entries. Their native PDP-6 implementations are respectively in
`mm/mm_pdp6.s`, `fs/memfs_runtime.s`, and
`fs/d6fs_provider_namespace_pdp6.s` plus its mutation assembly.
The disabled C `dtfs_resize` body was removed from the otherwise active
`fs/dtfs.c`. The assembler `fs/dtfs_resize.s` remains authoritative.
Do not restore these source-only fallbacks. Existing active C functions
that provide *distinct* services remain.
