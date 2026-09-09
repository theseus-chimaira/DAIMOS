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
074..077 remain reserved.  Low PDP-6 opcodes 001..037 remain user LUUOs and do not enter the monitor-UUO
dispatcher.  DAIMOS allocates no kernel service through that path.

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

## Finding 3: bulk console text is worth one bounded resident primitive

UUO 043 was an unused direct PUTCHAR selector; normal character output already
uses WRITECHAR 062.  It is now `WRITE_CHARS`: AC1 is stdout/stderr, AC2 is a
9-bit user byte pointer, and AC3 is a character count.  The handler validates
and translates the user pointer once, checks each touched word against the
physical user limit, and feeds the existing console output leaf.  It uses only
caller-scratch ACs, so it needs no save/restore frame.

`u_puts()` and `u_crlf()` use 043 for console output.  Regular-file output keeps
its existing WRITECHAR semantics.  `CAT` uses STAT + READ_WORDS in 32-word
chunks and WRITE_CHARS for regular-file-to-console copies, with the old
READCHAR/WRITECHAR path retained for pseudo-files and devices.

Trap-count effect, excluding OPEN/CLOSE:

- N-character `u_puts`: N traps -> 1;
- CRLF: 2 traps -> 1;
- regular-file `CAT`: about 2N traps -> `1 + 2*ceil(N/128)` (the one is STAT).

For a 128-character regular file this is 256 character-I/O traps versus three
classification/bulk-I/O traps.

### Memory cost and rejected over-batching

Relative to the preceding executive-UUO-removal tree:

- `__kcore_low_init_end`: 013076 -> 013122;
- `__kcore_low_end`: 013621 -> 013645;
- `MEMSTAT RESIDENT`: 10413 -> 10433;
- fixed resident cost: **20 words**;
- `dsh` `__main`: 005650 -> 005776, **86 user words**.

An experiment batching SIXBIT and numeric formatters too grew `dsh` by about
200 words total.  That was rejected because RAM consumption remains the primary
optimization goal.  Only the high-payoff literal/CRLF and CAT paths are kept.

## Next optimization tranche

1. Add a static/dynamic trap-frequency benchmark for representative commands so
   further batching is accepted only when its RAM/runtime trade is measured.
2. Prototype `dlink` direct-call relaxation for exact native-ABI libc veneers:
   `PUSHJ 17,dsys_*` -> the corresponding UUO.  This reduces user text and the
   extra PUSHJ/POPJ without adding kernel resident code.  Address-taken/indirect
   calls must continue to use the real veneer.
3. Census whether small hand-written libc assembly for hot formatting helpers can
   recover user words without reintroducing per-character traps.
4. Keep interactive `dsh_getline()` character-oriented.
5. Allocate new UUOs only when they reduce privilege transitions or substantial
   dispatch; 074..077 remain reserved for process/self-hosting work.

## Regression rule

Resident kernel assembly must contain no executive-mode `UUO` instruction.
Userland syscall veneers are intentionally excluded from this rule.
