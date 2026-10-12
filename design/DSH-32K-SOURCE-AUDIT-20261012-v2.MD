# DSH source audit and 32K-word residency: second shrink stage

Date: 2026-10-12. Base: `dsh-easy-shrink-20261012-v5`.
No shell feature was removed, no runtime limit was reduced, and no kernel
source or filesystem format was modified.

## Measured code-size savings

KCC/DAS/DLINK PDP-6 builds; all counts are **octal 36-bit words**:

| Change | DXR image | Increment saved |
| --- | ---: | ---: |
| Existing source-cleanup baseline | 036054 | -- |
| Remove redundant AST clear and 32-token reset loop | 036012 | 000042 |
| Remove uncalled internal dsh_expand convenience wrapper | 036007 | 000005 |
| Share builtin classifier for ordinary/launch-only execution | 035675 | 000112 |
| Reuse packed parser keyword decoder for multiline depth | 035550 | 000125 |
| Compare builtin names as packed SIXBIT constants | **035545** | 000005 |

Total: **0307 octal (199 decimal) words** saved from this stage.
BSS remains **015241 octal (6817 decimal) words**.
Normal production `DSH_IMAGE_LIMIT=036000` is met without an override.
The full PDP-6 image build and **115/115 shell target conformance** passed.

These are source/structural reductions. Fewer repeated linear string scans
should also help speed, but no PDP-6 cycle-level benchmark was performed.

## Critical finding: 32K core is still insufficient

From the actual `dxrcheck`, `kcore.map`, `exec_load.s`, and VM allocator:

| Component | Octal words |
| --- | ---: |
| DSH image | 035545 |
| DSH BSS | 015241 |
| User origin + reserved stack | 002020 |
| Minimum unrounded VM size | 055026 |
| VM allocation after 02000-word alignment | **056000** |
| Linked fixed KCORE end | **023610** |
| KCORE + DSH VM lower bound | **0101610** |
| Physical machine capacity | **0100000** |

The static lower bound is already **01610 octal (904 decimal) words over
32K**, even before dynamic kernel/process allocations and resident filesystem
packages. The 32K test `daimos-low-memory-boot-v2.sh` reaches INIT and LOGIN
but ROOT ends with `LOGIN: EXEC FAILED`; this remains an open regression,
consistent with insufficient available memory. The exact EXEC failure path
has not yet been instrumented to rule out secondary causes.

Reducing image + BSS by **01026 octal (534 decimal)** words will lower the
rounded VM from 056000 to 054000, but this would leave only 0170 octal
(120 decimal) words after KCORE, still insufficient for realistic resident
packages. The real acceptance target is a clean 32K INIT -> LOGIN -> DSH
boot, plus command execution and memory accounting. Expect to need **multiple
02000-word VM allocation steps**, not merely the final 036000 image limit.

## Major fixed-memory hotspots

The `dsh_exec.s` BSS declarations and C structures give:

| Storage | Decimal 36-bit words | Cause |
| --- | ---: | --- |
| `dsh_nodes[16]` | **3,200** | Each AST node contains 20 full argument records and both mask arrays |
| `dsh_function_nodes[8]` | **1,600** | Same large AST structure for persistent functions |
| `dsh_main_state` | **1,158** | Variable, alias, argument and job tables |
| `dsh_history[16]` | **496** | Existing 16-entry, 180-character bounded history |
| `dsh_run_block` | **192** | EXEC V2 argument/environment transfer |

Active and function ASTs together occupy **4,800 words, approximately 70%
of DSH BSS**, before any AST nodes are actually used. The fixed 200-word
`struct dsh_node` is the dominant design inefficiency: list, pipe and
control-flow nodes reserve the same 20 argument records and masks as a
maximal simple command.

### Required next structural refactor (not yet implemented)

Separate small fixed-size node metadata from variable-length word and mask
payloads. Allocate payloads only for nodes that use them. Preserve:

- all 16 active AST node slots, 8 stored function-node slots,
  20 arguments, full SIXBIT lengths, and both quotation masks;
- persistent function definitions and nested SOURCE/function evaluation;
- bounded stack usage and deterministic allocation failure semantics.

A compact node header plus payload arena is preferable to reducing any
capacity. A simple, unconditional `dsys_brk` allocation is **not** safe:
the existing large-argument fallback allocates and releases a temporary
LIFO brk extent, and a nested SOURCE could define a persistent function
inside it. An arena design must separate persistent function allocations
from temporary nested evaluation/scratch lifetimes; do not silently free
persistent nodes when unwinding temporary argv storage.

Prototype in an isolated branch. Measure actual KCC image, static BSS,
peak VM extent, parser/function execution cost, and 32K target boot.
Do not change interpreter node, argument, or function capacity to make
an artificial size number.

## Other verified source inefficiencies and candidates

- Parser formerly cleared the entire AST arena and cleared each allocated
  node again; now only one full clear remains.
- The multiline script reset formerly cleared all 32 token records despite
  both lexer token constructors initializing their destinations.
- `dsh_builtin` and `dsh_is_builtin_name` formerly carried independent
  17-name comparisons; both now share one classifier.
- `dsh_script_depth` formerly carried its own 11 keyword string
  comparisons, even though the parser already had a packed-SIXBIT
  keyword decoder; it now shares the decoder.
- `dsh_expand` was an uncalled three-argument convenience wrapper.
- History insertion still shifts up to 15 * 31 words per new command;
  a ring would improve CPU time but preserve roughly the same BSS and
  therefore is not the priority for 32K residency.
- Character-by-character SIXBIT processing repeatedly divides by six;
  sequential word/slot cursors deserve isolated measurement but likely
  do not remove enough static RAM on their own.
- Parser diagnostic wrappers and keyword metadata remain linked even
  where the interactive interpreter passes null diagnostics. Audit all
  external/test consumers before attempting a build-time split.

The correct priority is **compact AST storage**, followed by a measured
low-memory boot, not weakening the shell's language or static limits.
