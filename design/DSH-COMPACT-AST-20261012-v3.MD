# DSH compact AST storage: third low-memory stage (2026-10-12)

Status: code built at the normal 036000 image limit; 115/115 general and
10/10 dedicated AST lifetime target probes passed. **32K low-memory boot
still fails at LOGIN: EXEC FAILED.**

## Allocation layout

Previously every one of the 16 active AST nodes and 8 saved function nodes
contained an entire 20-word-argument, quote-mask and literal-mask payload,
whether the node needed arguments or not. The combined static node arrays
occupied 4800 decimal PDP-6 words.

Now the node descriptor retains type, links, flags and redirection records,
with three pointers into a separately allocated `dsh_node_payload`.
The full 20-argument capacity and both 36-bit quotation masks are intact.
The eight saved function nodes own independent fixed payload records in
BSS; definition copies every active argument/mask by value and compaction
rebinds all payload pointers. No function depends on ephemeral parse data.

Parsing/evaluation uses one 180-word static fast-path payload for the common
single argument-bearing node. Its ownership is checkpointed and restored
across nested SOURCE and script execution. Further argument-bearing nodes
obtain 180-word transient buffers from BRK, exclusively for the current
parse/evaluation lifetime. Nested execution retains its parent buffers until
return and restores the BRK in reverse order. The full-size argument
expansion already uses LIFO BRK scratch above those allocations.
Persisted function payloads **never** reside inside a transient BRK extent.

Each ordinary word uses a full seven-word SIXBIT copy and initializes both
masks. A standalone substitution clears its initially empty record before
appending characters. No uninitialized word/mask is read within argc;
avoiding unconditional clearing of 180 words improves common-case speed.

## Measured final-size candidate

KCC/DAS/DLINK PDP-6 result, all measurements octal:

| | Prior v6 | Compact v7 |
| --- | ---: | ---: |
| DXR image | 035545 | **035603** |
| BSS | 015241 | **010136** |
| Image + BSS + 02020 stack/origin | 055026 | **047761** |
| 02000-aligned user VM | 056000 | **050000** |

BSS falls by **2627 decimal words** while image rises 30 decimal words.
The user VM's allocation quantum falls from 056000 to 050000:
**3072 decimal (06000 octal) words less physically reserved**.
The 036000 normal image gate passes; no feature/capacity limits changed.
This is *per process* because current PDP-6 executable text is not
physically shared across processes.

One additional libc object split moves u_text_open and u_s6_pack into
text_open.c/u_pack.c, retaining both for programs that call them but
excluding them from DSH's image. The SIXBIT record copy/swap uses native
structure assignment; EXIT/RETURN decimal number and UMASK octal number
parsing share a single number conversion algorithm.

## 32K correctness and unresolved work

With the current fixed KCORE end 023610, the static lower bound for
KCORE + a 050000-word DSH extent is 073610 octal, leaving only 04370
octal words for resident packages, process tables and kernel allocations.
The final cached-payload version's live 32K lowmem test still reaches
LOGIN but reports LOGIN: EXEC FAILED. A successful ordinary
image/conformance test cannot substitute for low-memory acceptance.

Potential next improvements include variable-sized payload allocation
instead of always reserving 180 words per additional argument-bearing
node, and compact persistent function payload storage. The latter must not
introduce permanent allocations inside temporary BRK arenas: SOURCE and
functions can nest while transient scratch is live.

Do not claim 32K support until INIT -> LOGIN -> DSH and meaningful commands
pass the dedicated low-memory SIMH test with measured kernel headroom.
