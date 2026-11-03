# Historical RAM-shrink, backup, and KCC diagnostics (2026-11-03 v1059)

This archival branch preserves the original commits from three diverged
DAIMOS development tips without reintroducing their old source trees:

- `backup/pre-push-20261005-v1` (79b90a71): early Type 340 display
  scrolling, local picture staging, and kernel optimization checkpoint
- `checkpoint/shrink-20261006-v1` (02fa024e): early assembler MM/MEMFS,
  D6FS namespace, module bindings, EXEC and resident-size accounting
- `experiment/kcc-swap-in-diag-20261008-v1` (f9997c55): exploratory
  two-word resident swap-in diagnostics and a KCC MAKE ALL failure study

Each original branch tip is an ancestor of this archive via an `ours`
merge. This keeps exact historical files and ancestry available, but
does not put superseded assembly or instrumentation into main.

## Reconciliation and remaining uncertainties

Main already packages local pictures under `/PICTURES` and provides
Type 340 scrolling and CLEAR. The native MM, MEMFS and D6FS code and
fixed-binding count were developed further after the shrink checkpoint.
`design/historical-branch-audit-v1006.md` (sections v1008 and v1010)
documents the comparisons for allocator accounting and low-memory EXEC.
Old size numbers cannot be treated as modern linked resident size.

The KCC diagnostic investigation recorded decimal status 131073
(octal 0400001) while running historical MAKE ALL, but did **not**
prove a swap-in fault. Its resident `proc_swap_diag_slot` and
`proc_swap_diag_reason` words and failure-path changes are not a kernel
repair. In the historical experiments `MAKE -N ALL` failed while
shorter MAKE targets completed, implicating graph traversal, compiler
code generation or stack use rather than definitively swap-in.

DAIMOS main now includes lazy recursion-depth scratch allocation in
native MAKE (v1054). Its 256K six-depth/cycle probes passed, but a
full fresh-image `MAKE ALL` self-host run has **not** been repeated.
Therefore do not claim that v1054 resolves the historical failure.
The original plan, sources and earlier diagnostic measurements remain
recoverable by following the archive's ancestry.

The historical diagnostic source's labels and cleanup must be audited
if the experiment is ever repeated. Do not enable it simply to record
two more resident words. No KCORE optimization or 32K-specific test
was performed as part of this branch consolidation.
