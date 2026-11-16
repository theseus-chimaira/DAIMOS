# P1-E v1634: original PURE MRES compiler flag sizing

Date: 2026-11-16. Base: validated v1633 48K fixed-parent APR
corridor bound. The PURE service is optional, installed in an MRES,
and must remain capable of DROP, prefix in-place restore, FULL
relocated restore and failure-atomic PI/MM ownership rollback.

## Reproducible baseline

The testkit v1634 experiment compiles the original seven-object
removable PURE MRES under GCC 3.2 `-std=gnu99 -march=pdp6
-mcpu=pdp6`, preserving the original assembler objects, original
`kcore.map`, link export contract and `p10fold -m 20` plan.
Every candidate gets separate versioned intermediate outputs.

An initial experimental harness mistakenly omitted `das -F`,
producing a FOUR-word size increase despite identical generated
assembly. This was a harness error, not a production assembler or
GCC defect. Using the exact original `das -F -C -O` flags restores
the baseline DXR **byte-for-byte**, SHA-256 equal to the original
physical v1629 source-backed image, at `003336` octal (1758
decimal) resident words.

| Compiler options | Resulting MRES decimal words | Change |
|---|---:|---:|
| Original `-Os` | 1758 | 0 |
| `-O2` | 1806 | +48 |
| `-O1` | 1797 | +39 |
| `-Os -fno-inline` | 1758 | 0 |
| `-Os -fomit-frame-pointer` | 1758 | 0 |
| `-Os -fno-cse-follow-jumps` | 1771 | +13 |
| `-Os -fno-strength-reduce` | 1757 | -1 |
| `-Os -fno-builtin` | 1758 | 0 |

The one-word `-fno-strength-reduce` improvement has NOT undergone
full physical child execution, rollback, or source admission tests,
and cannot be treated as an accepted toolchain option. No tested
compiler flag closes the minimum **389-decimal-word** resident
gap identified in v1633. Therefore the standard `-Os` production
build is unchanged.

## What this rules out and what remains

The failure to reach the threshold is a negative result for the
specific tested compiler flags, not proof that consolidation or
another correct memory layout cannot improve usable memory.
Inspect overlapping C helper routines, KCORE/MRES folding with
verified donors, and any optional source-independent allocation
that can actually be eliminated without disabling FULL restoration
or sacrificing rollback. A future reduction must be measured in
the linked installed MRES and validated with genuine PDP-6 48K
process admission and MM owner snapshots, not estimated from C
lines or compressed disk size.

P1-E remains experimental and OFF by default. The strict 32K
whole-KCORE size pass remains HOLD; no production source changes
in this version. Reproduction:
`daimos-testkit/tests/diagnostics/pure-mres-size-v1634`.
