# DSH redirection sidecar — PDP-6 text and RAM measurement

Date: 2026-10-12. Base: v10 lazy-history optimization (committed in
`dsh-lazy-history-20261012-v12`). Shell grammar, limits and functionality
are unchanged.

## Change

The former `struct dsh_node` stored two complete seven-word SIXBIT
redirection paths even for empty, list, pipeline, loop and control-flow
nodes. Only simple-command nodes actually consume these fields.

Both fields have moved into `struct dsh_node_payload`, and the fixed node
descriptor holds two pointers. Temporary argument-bearing payloads reserve
14 additional words after their variable-width argument/mask area.
Persisted functions deep-copy the optional redirection fields into their
own independently owned payloads, rebinding the pointer fields during
function-node array compaction.

## Measured changes (octal 36-bit words)

| | v10 | v13 |
| --- | ---: | ---: |
| DSH image | 035673 | **035726** |
| Static BSS | 004034 | **003374** |
| Image + BSS + origin/stack | 043747 | **043342** |
| Aligned initial VM | 044000 | **044000** |

Savings: **0440 octal = 288 decimal BSS words**, with image growth of
0033 octal = 27 decimal words. Net unaligned core demand falls by
0405 octal = 261 decimal words. The ordinary image limit 036000 is met.
The rounded initial VM extent has not crossed the next 02000-word boundary,
so 32K boot acceptance must be checked separately.

## Target acceptance

- Fresh full PDP-6 boot image: PASS under normal DSH image limit.
- Full DSH conformance: **115/115 PASS** via console terminal (CTY).
- Focused input/output/append/function-persistence and redefinition
  redirection tests in `daimos-testkit`: **7/7 PASS**.

An earlier DCS0 full-conformance run lost characters in HELP DAS, an
existing long-output transport issue. A fresh image driven by CTY passed
all 115 cases. The focused tests used another independent clean image.

## Next source investigations

- Find a safe compact representation for the 64-entry variable table;
  the hand-written `dsh_var_pdp6.s` assumes its current 13-word layout.
- Reduce runtime argument and substitution copies without overlapping
  temporary lexing storage and persistent functions.
- Establish the exact memory allocation failure during 32K ROOT LOGIN
  EXEC. Do not infer bootability from image/BSS size alone.
