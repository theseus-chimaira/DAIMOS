# SUPERVISOR source-by-source and PDP-6 APR allocation review — v6

Date: 2026-10-12. Base: v5, production `supervisor.c` 152 lines;
measured executable **001442**, BSS **000261**, resident VM **004000**
(all octal 36-bit words). The measured selective-veneer change below
is the only production implementation arising from this audit.

## Implemented selective syscall veneers (v6)

The universal syscall object links many unused UUO entry points into
SUPERVISOR. A new `init/supervisor_syscalls.s` contains precisely the
eight original, ABI-identical veneers called by SUPERVISOR:
`dsys_exit`, `dsys_writechar`, `dsys_run`, `dsys_wait`, `dsys_getpid`,
`dsys_gettime`, `dsys_logctl`, and `dsys_ttyctl`.

| Property | v5 | v6 |
| --- | ---: | ---: |
| Executable image | 001442 | **001236** |
| Static BSS | 000261 | **000261** |
| VM at unchanged stack | 004000 | **004000** |

This saves **0204 octal (132 decimal) executable words** with no
kernel ABI change or new monitor entrypoints. Absent required syscall
symbols must remain hard link failures, not silent fallbacks.

## Resident implementation, line by line (logical blocks)

`userland/init/supervisor.c` was compared with the actual generated
KCC PDP-6 assembler in a fresh standalone userland build.

| Source lines | Purpose and necessary behavior | Optimization assessment |
| --- | --- | --- |
| 1–12 | Includes, limits, `init_entry` and eight-entry resident table | Four metadata words + eighteen SIXBIT path words per entry: **22 words per entry / 0260 octal total**, plus `init_count`. The pathname dominates BSS. Keep eight entries; consider a *fixed-size bounded compact path arena*, with each entry referencing a counted variable-length path. Do not discard or overwrite records needed to respawn children. |
| 14–31 | `spawn_entry`: RUN buffer, TTY output selection | `ttyctl` must be retained for DPY entries. KCC frame starts with two saves and `ADD 17,[072,,072]`, a **0074-octal-word** local stack cost. Inlining the RUN request builder in PDP-6 assembler may save text and a few frame words, but requires exact syscall layout and register preservation. |
| 33–43 | Count SIXBIT path and encode 0–20 TTY | Bounded and feature-complete. TTY conversion uses C division/modulo by 10; generated sequences are a candidate for replacing with a short hand-coded 0–20 conversion, but measure before accepting an assembler rewrite. |
| 44–65 | RUN V2 offsets, header, duplicate path argument, TTY, fdmap | The pathname is deliberately stored *twice*, once as path and once as argv[0], as required by RUN V2. Duplication is transient stack storage, not permanent BSS. The fdmap includes all three standard descriptors; do not omit the mappings without checking RUN inheritance semantics. Fixed headers/TTY/fdmap might be emitted more compactly in assembler. |
| 66–71 | `dsys_run`, returned child PID | Essential supervision state; do not remove return-value/error check. |
| 73–99 | Validate bounded handoff from startup INIT; decode metadata; copy path | Required because SUPERVISOR receives user-supplied EXEC arguments. Direct extraction already avoids a generic SIXBIT decoder. The current loop copies all eighteen path words and zeros unused trailing words; compact service-table design would replace this with exact-count copies and explicit capacity checking. Generated `read_entry` saves four ACs and reserves five local words. |
| 101–121 | PID 1 and argc checks, validate up to eight entries | Keep PID 1 restriction, argc range and even-count validation. Error strings cost text but are valuable for diagnosing PID 1 startup failure. Discarding diagnostics is not a sound memory fix. |
| 122–131 | READY log and initial service launches | Essential log and ONCE/RESPAWN behavior. A helper consolidating launch-failure reporting with respawn may save some duplicate code; verify output/error code identity. |
| 132–151 | `wait`, reap, match and conditional respawn | Essential minimal PID 1. Loop is already direct; preserve stopped/continued filtering, PID comparison, and original exit status behavior. Tiny savings in a full hand-written rewrite are possible but should follow structural savings. |

## Linked helper routines

- `crt0.s` has only `PUSHJ main; PUSHJ dsys_exit; HALT`, so replacing
  `main` with a native assembler `_start` is a negligible saving.
- The universal `syscall.s` object contributes many *unused* syscall
  veneers to the image. A SUPERVISOR-specific syscall-veneer source
  containing only `dsys_getpid`, `dsys_ttyctl`, `dsys_run`, `dsys_wait`,
  `dsys_logctl`, `dsys_gettime`, `dsys_writechar`, and `dsys_exit`
  is a promising **measurable** code reduction. Include any additional
  symbols actually referenced by the relevant syscall helper object.
  Treat missing symbols as a hard link failure, not a workaround.
- `ulog_event` has a `0205`-word frame because its mandatory LOGSTORE
  APPEND record is `0200` words, already moved into the reserved stack.
  Its record cannot be shortened under the existing `SYS_LOGCTL_APPEND`
  ABI, which maps `0200` user words unconditionally.
- `u_puts` and `u_crlf` are small direct-console utilities. Removing
  error reporting to shrink text is not recommended.

## Should all of SUPERVISOR be converted to assembler?

**Not yet.** The best sequence by expected RAM savings and review risk:

1. Link only the syscall veneers actually required by SUPERVISOR;
   retain original syscall ABI and assembler definitions.
2. Try the fixed-size eight-entry compact pathname arena, calculating
   the worst-case case where *all eight paths use their full allowed
   length*. If savings occur only for typical short paths, account for
   separately allocated persistent storage and its safety.
3. Measure `spawn_entry` as isolated, well-documented assembler against
   C with identical RUN V2 records, including longest path and TTY 20.
4. Keep the event loop and boundary-validation logic in C unless
   compiled measurements justify further assembly.

Compiler bug evidence must take precedence over code-level tricks:
do not camouflage a miscompile by rewriting source around it.

## Can the process allocation quantum be less than 02000?

The generic `mm_alloc_aligned()` implementation supports arbitrary
power-of-two word alignment; **02000 is not a generic MM minimum**.
The PDP-6 process backend specifically requests 02000 and rounds
extent sizes to that boundary:

- `vm_pdp6.s:426–450`: `vm_space_create` adds `01777`, masks to a
  `02000` boundary, and passes a `02000` alignment to MM.
- `vm_pdp6.s:140–160`: `BRK` resize uses the same quantum.
- `vm_pdp6.s:98–105,740–760`: the APR control value encodes
  user-space size minus `02000` and the relocated physical base.
- `proc_swap_pdp6.s` and `mm_pdp6.s` also use `02000` as the
  movable-process alignment; `vm.h` defines `VM_EXTENT_ALIGN_WORDS`.

This is hardware-constrained, not just an inefficient software choice.
The PDP-6 simulator implementation at `pdp6/emu/apr.c:690–718`
states that the address is split into **eight upper relocation bits
and ten low address bits**. The upper portion is adjusted by RLR and
checked against PR. Thus relocation/protection is in `2^10 = 1024`
decimal word, or **02000-octal-word**, units. Existing PDP-6 processes
cannot safely be relocated/protected independently on smaller units
by changing these constants alone.

Potential advanced approach: suballocate tiny tasks *inside one shared
02000-word hardware mapping*, using trusted kernel-supervised scheduling
and explicitly shared protection. This is a **different process model**:
it loses current independent per-process relocation/protection and
requires a complete design of stack, ownership, faults and context
switching. It is not an appropriate low-risk optimization for PID 1.

For *non-process* kernel allocations the generic MM may already allocate
with smaller alignment. That is separate from the PDP-6 user-VM unit.

## Recommended next experiment

Selective-syscall-veneer linking is implemented in v6. Next, add
testkit coverage of all eight INITTAB
slots, two-digit TTYs and repeated RESPawn/ONCE behavior before merging.
Do not change user-stack ABI or PDP-6 hardware allocation quantum in
the same patch.
