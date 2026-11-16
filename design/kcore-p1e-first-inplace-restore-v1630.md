# P1-E v1630: original physical in-place PURE restore path verified

Date: 2026-11-16. Base: v1629 true 48K five-PACKED-child
physical PIPE/WAIT/EXIT acceptance. Testkit-only read-only SIMH
breakpoint replay, original installed PURE MRES and original
source-built guest. No kernel, MM, MRES, ABI or media modification.

## Exact original module and original physical branch

The original v1629 PURE MRES map and DXR2 image identify these
original offsets and loaded PDP-6 addresses in octal:

| Original symbol | MRES offset | Installed address |
|---|---:|---:|
| `proc_pure_restore_v1384` | 001442 | 033471 |
| `proc_pure_ram_restore_v1391` | 002207 | 034236 |
| `mm_pure_claim_prefix_v1384` | 002561 | 034610 |
| `mm_pure_release_prefix_v1384` | 002563 | 034612 |
| `mm_pure_commit_prefix_v1384` | 002565 | 034614 |

Physical machine memory at 034610 is `400000000000` (`SETZ 0,0`)
and at 034614 is `201000000002` (`MOVEI 0,2`), both exactly matching
the same built PURE DXR. These verify the module was installed at
the expected base, not a debugger address guessed from source.

The replay first stopped at the ORIGINAL `mm_pure_claim_prefix_v1384`
instruction, with AC1 = 117000 (retained suffix), AC2 = 2 (child),
AC3 = 001000 (omitted PURE prefix) and AC4 = 010000 (complete APR).
After continuing with the original compiler output, it next reached
the ORIGINAL `mm_pure_commit_prefix_v1384` instruction at 034614,
with AC1 = 116000 (original complete APR base), AC2 = 2, AC3 =
001000, AC4 = 010000. The original full-APR fallback entry 034236
was NOT reached before this commit. The user then printed all five
`w` markers after original PIPE/mutable checks, the parent passed
all original WAIT statuses (`Z`), and the simulator HALTed normally.

This is source-linked physical proof that **the first restored child
uses the fast v1384 in-place prefix path and reaches MM COMMIT**.
Earlier v1620 separately proved the actual type/pin/READY_ONCE
publication following such a commit; v1629 proves complete user
execution, final EXIT/WAIT and owner cleanup under five PACKED
processes. No duplicate full mutable suffix is needed on this path.

The v1630 physical auditor PASSes and rejects two independently
falsified witnesses (incorrect suffix AC1 and a forged FULL fallback
breakpoint). Current-source native v1395/v1397 MM rollback and
v1612 RAW service testkit suites also PASS. The original replay
runner initially required contiguous `LwwwwwZ` despite debugger
insertion between `L` and subsequent `wwwwwZ`; its output-only
terminal assertion was corrected, and the actual trace was already
source-valid. No guest instruction was changed.

## Still open

The v1630 trace classifies only the FIRST restored child. Other
children may use different paths; neither relocated FULL v1391
nor its physical rollback was triggered here. Immutable source
lifetime/forced errors, concurrent PI6 activity, source-matched
PURE-disabled RAM comparison and transient physical peak remain
OPEN. PURE default OFF and strict 32K optimization HOLD.
