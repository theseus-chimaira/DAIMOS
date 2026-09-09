# DAIMOS UUO / Trap-Frequency Optimization Sweep

Date: 2026-09-09

## Goal

Treat a PDP-6 programmed-operator trap as an expensive privilege-boundary event,
not as a free replacement for PUSHJ/JRST.  Minimize permanent resident words and
trap frequency without changing DAIMOS semantics or adding feature-specific
shortcuts.

## Current syscall ABI

User mode uses monitor UUOs 040..073.  The effective address supplies argument
0, AC2..AC4 carry remaining arguments, and AC1 carries the result.  Opcodes
074..077 remain reserved.  Low PDP-6 opcodes 001..037 are deliberately not
allocated to user services and are rejected by the native syscall dispatcher.

The libc syscall layer is a set of two-word veneers (UUO + POPJ).  This is
source-compatible and compiler-independent but leaves a PUSHJ/POPJ pair around
every unavoidable user->kernel trap until a linker relaxation facility exists.

## Finding 1: executive filesystem UUOs were pure overhead

Before this sweep, eight resident VFS leaves issued private UUOs while already
executing in executive mode:

- READDIR
- STAT
- UNLINK
- TRUNCATE
- CHMOD
- READ_WORDS
- WRITE_WORDS
- SYNC

The trap merely decoded the opcode/provider and called `fs_provider_reg_call`.
Each leaf already ended in `UUO; POPJ`, so replacing those two words with
`MOVEI AC6,op; JRST fs_provider_reg_call` is word-for-word neutral at the leaf
and removes the hardware programmed-operator trap.

With no executive UUO callers left, the private branch in `mach_user.s` is
unnecessary and has been removed.

### Size result

Production KCORE before:

- `__kcore_low_init_end = 013110`
- `__kcore_low_end = 013633`
- runtime `MEMSTAT RESIDENT = 10423`

After direct provider tail calls:

- `__kcore_low_init_end = 013076`
- `__kcore_low_end = 013621`
- runtime `MEMSTAT RESIDENT = 10413`

Net permanent resident saving: **10 words**.

This optimization therefore improves the hot VFS path while also reducing
resident code.

## Finding 2: userland trap multiplication is now the largest obvious target

Static userland call sites are already mostly bulk-oriented (`READ_WORDS` and
`WRITE_WORDS` exist), but the small output library expands text into one
`WRITECHAR` syscall per character:

- `u_puts()` loops through `u_putc()`;
- `u_put_s6()` loops through `u_putc()`;
- decimal/octal formatting loops through `u_putc()`;
- `u_crlf()` takes two traps;
- `CAT` reads one character and writes one character at a time.

Interactive `dsh_getline()` should remain character-oriented because it needs
per-character editing and echo semantics.  Bulk file/text paths should not.

## Next optimization tranche

1. Add packed 9-bit text batching helpers in user libc around `WRITE_WORDS`.
   Keep `u_putc()` for true character devices and interactive echo, but make
   `u_puts`, `u_put_s6`, numeric formatting and CRLF emit bounded blocks when
   the destination is a regular file or when a generic bulk path is safe.
2. Rewrite `CAT` to use `READ_WORDS` + `WRITE_WORDS` for regular files while
   preserving character-device behavior.
3. Add a trap-frequency regression/benchmark that counts or predicts traps for
   representative shell commands (ECHO, CAT, LS, MEMSTAT) so optimization is
   measured rather than inferred only from source.
4. In `pdp10-tools/dlink`, prototype direct-call relaxation for known libc UUO
   veneers: direct `PUSHJ 17,dsys_*` -> equivalent UUO when the veneer ABI is
   exactly the native syscall ABI.  Indirect/address-taken calls continue to use
   the real veneer.  This is mainly a user-text reduction plus a small speed
   improvement; it does not remove the unavoidable privilege trap.
5. Only allocate additional UUO opcodes when they reduce trap count or remove
   substantial resident dispatch.  Do not create tiny specialized syscalls just
   because opcode space exists.

## Regression rule

Resident kernel assembly must contain no executive-mode `UUO` instruction.
Userland syscall veneers are intentionally excluded from this rule.
