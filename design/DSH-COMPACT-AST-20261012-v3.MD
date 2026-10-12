# DSH compact AST payloads — measured implementation (2026-10-12, v3)

The frozen DSH feature set is unchanged. This follows
`design/DSH-32K-SOURCE-AUDIT-20261012-v2.MD` and the versioned
`dsh-32k-source-audit-20261012-v6` branch.

## Implementation

The former 200-word AST node embedded 20 full seven-word SIXBIT argument
records and two 20-word quotation-mask arrays. Sixteen active AST nodes
plus eight stored-function nodes therefore reserved 4,800 static words,
even for operator and control-flow nodes with no arguments.

Each `struct dsh_node` now holds its fixed metadata and pointers to its
argument records and masks. Only argument-bearing node kinds allocate a
temporary 180-word payload, through the existing user BRK facility.
Temporary parse payloads are released at the evaluator boundary, *after*
recursive evaluation. Nested SOURCE allocations remain LIFO. Standalone
command substitution explicitly initializes its destination word; ordinary
arguments are copied completely from initialized lexer tokens. No unfilled
argument slot may be read.

Stored function AST nodes instead copy argument payloads into their own
permanent eight-slot BSS pool. Replacing or removing a function compacts
both node metadata and payloads, rebinding their pointers. This separation
avoids dangling references to the reused script token buffer and avoids
freeing stored functions when temporary evaluation unwinds.

No node, function, argument, SIXBIT-length, quotation, SOURCE, or history
limit was reduced. Whole-record SIXBIT copy/swap and common radix-number
parsing replace duplicate loops. Unused libc path packing and path-based
S6REC open routines were split into standalone objects so the shell no
longer links them; other consumers retain the original interfaces.

## Measurements (octal 36-bit words)

| Metric | Previous v6 | Compact v7 |
| --- | ---: | ---: |
| Executable image | 035545 | **035552** |
| Static BSS | 015241 | **007651** |
| Image + BSS + 02020 origin/stack | 055026 | **047443** |
| Rounded user-VM extent | 056000 | **050000** |
| Linked KCORE end | 023610 | 023610 |
| KCORE + user-VM minimum | 0101610 | **073610** |

BSS saves **2,808 decimal words**; the initial aligned user extent saves
**03072 decimal words** (06000 octal). The image remains below the ordinary
036000 octal acceptance gate. The static lower bound now fits inside 32K,
but does **not** include resident kernel packages, process metadata,
other live user extents, or allocator fragmentation.

## Acceptance and limits

- Full PDP-6 disk-image build under the normal image gate: PASS.
- DSH conformance 115/115: PASS on the exact DXR binary; the final
  standalone rebuild's SHA-256 digest matched the previously fully tested
  DXR byte for byte.
- Focused AST lifetime test, 10/10: PASS (function copy, replacement,
  compaction, CASE arms, and function persistence through SOURCE).
- 32K low-memory PDP-6 boot: **NOT PASSED**. It reaches LOGIN but ROOT
  reports `LOGIN: EXEC FAILED`. The kernel needs further free physical
  core beyond the static KCORE + DSH lower bound.
- A reused disk image after a test boot can fail earlier with `?RT`
  because the D6FS root requires recovery. Never classify this as a DSH
  regression; use a freshly built disk image for each boot test.

## Next optimizations without feature changes

1. **Short temporary argument payloads.** The present heap allocation
   reserves all 20 argument records even for a one-word command. Safely
   pre-count the maximum number of arguments from token boundaries and
   allocate only the necessary records and masks. Preserve full 20-word
   capacity when needed. Benchmark allocation overhead and peak BRK.
2. **Persistent function payload storage.** Eight 180-word static
   records are still reserved. A compact persistent arena would save
   BSS, but must handle on-demand growth, redefinition compaction,
   recursive calls, and nested SOURCE without ever freeing a live
   function from temporary LIFO BRK storage. Do not replace this with
   an unsafe shared scratch buffer.
3. **Measure actual 32K available extents.** Determine why DSH EXEC
   cannot obtain the 050000-word extent after INIT/LOGIN. Account for
   resident packages and other processes; a simple numeric KCORE
   lower bound is not enough. Do not claim a 32K boot until the live
   test enters DSH and runs representative commands.
