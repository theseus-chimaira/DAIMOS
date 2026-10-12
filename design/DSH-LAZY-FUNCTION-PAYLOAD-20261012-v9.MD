# DSH lazy function pool and variable-width AST payloads — 2026-10-12 v9

The four-function/eight-node and twenty-argument limits remain unchanged.
Previously, eight complete 180-word function payloads were reserved in BSS
before interactive DSH started, even when no function was ever defined.

DSH now reserves that same bounded pool **once**, on the first successful
function definition, using the existing BRK allocator. The pool never moves:
function redefinition/removal reuse the existing eight slots, and node
compaction still rebinds payload pointers. Recursive and SOURCE execution
continue to use their own LIFO temporary AST allocations. If the pool is
first reserved inside a live parse, BRK unwinding respects the new persistent
floor rather than freeing the function bodies. This can leave a bounded
one-time gap below the function pool; it cannot grow on later redefinitions.

Active parser AST payloads are also sized by the count of lexer tokens
available in that parse, capped at the unchanged 20-word-node limit.
Each slot needs seven SIXBIT record words and two mask words. For a
two-token simple command this reserves 18 instead of 180 temporary
words. A full 20-slot FOR construct still receives all 180 words.
The parser stores no token pointers past evaluation, and permanent function
copies remain independent of the token or transient heap lifetimes.

## Measured PDP-6 results, octal 36-bit words

| Measurement | Compact AST v7 | Lazy pool v9 |
| --- | ---: | ---: |
| DXR image | 035552 | **035671** |
| Static BSS | 007651 | **005013** |
| Image + BSS + 02020 stack/origin | 047443 | **044724** |
| Rounded initial VM extent | 050000 | **046000** |

Permanent BSS decreases by 1,438 decimal words. Initial process VM
allocation decreases by 02000 octal (1,024 decimal) words. A function
definition may add up to 1,440 words of persistent heap storage thereafter;
the worst-case language capacity has **not** been reduced.

The ordinary 036000 image limit passes. Fresh PDP-6 full-image builds
pass. The combined v9 image passed **115/115** DSH conformance via
SIMH CTY, and **16/16** focused tests on an independent clean boot
image, including the full 20-slot FOR and nested SOURCE function
redefinition. Long HELP/MAN output on DCS0 suffered dropped characters;
this was reproduced without the variable-width allocator and was not
reproduced through CTY.

The separate 32K LOWMEM INIT -> LOGIN -> DSH acceptance remains OPEN:
previous low-memory builds still reported LOGIN: EXEC FAILED.
