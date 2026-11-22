# DLINK unused EXTERN and P10RUN SIMH expiry correction v1812

Date: 2026-11-22. Scope: host source correctness, native target compile
and link. Native in-guest execution remains unverified.

## Reproduced defects

Host DLINK treated every DOBJ1 UNDEF symbol as an external dependency.
An assembler source may declare OPTIONAL without using it. In the
source-linked v1812 fixture OPTIONAL is retained as an UNDEF symbol,
but only NEEDED has a symbol relocation. Original DAIMOS DLINK fails
with `dlink: undefined symbol: OPTIONAL` even when NEEDED is provided.
Scanning relocation use before unresolved/archive tests fixes both
false failure and unnecessary archive extraction. RH18 and LH18
symbol references remain mandatory; missing NEEDED still fails.

Host P10RUN previously classified a matching zero exit word as PASS
even when SIMH reported `Step expired` or `Time expired`: the guest
had not executed its completion path. The source-linked v1812
transcript test explicitly reproduces that false PASS with old P10RUN
and now requires FAIL for both conditions. A normal completed-zero
transcript continues to pass; wrong-exit and low-core failure controls
remain effective.

## Implementation and RAM accounting

- `tools/host/dlink.c` and standalone `pdp10-tools/dlink.c` scan the
  existing object relocation array, without a persistent index.
- `tools/host/p10run.c` and standalone `pdp10-tools/p10run.c` reject
  explicit SIMH step/time budget exhaustion.
- Native `userland/toolchain/dlink.c` streams DOBJ1 relocations to
  build a three-word 36-bit use bitmap on its stack, once per object
  during an archive resolution pass. It does not add static storage,
  and never performs a disk scan per undefined symbol.

The production KCC/DAS compilation of native DLINK was accepted by
the current host compiler and assembler. Independent KCC/DAS native
object comparison against the previous source produced:

| Native DLINK DOBJ | TEXT words | BSS words |
|---|---:|---:|
| Previous source | 2650 | 8438 |
| v1812 source | 2800 | 8438 |
| Delta | +150 | 0 |

The v1812 native DLINK linked successfully into a DXR with image
`011506` octal words and BSS `020443` octal words. These are userland
tool costs, not fixed KCORE growth. The three-word stack bitmap is
temporary, not BSS. No physical guest or PDP-6 full system acceptance
is claimed from the compile/link check.

## Verification

The source-linked DAIMOS testkit suite is
`tests/host/toolchain-extern-expiry-v1812/run-v1812.sh`, with real
DAS-produced DOBJ fixtures and P10RUN evaluate-source controls.
It passed against the updated DAIMOS `tools/host` sources and the
standalone pdp10-tools source checkout. The original DAIMOS linker
and original P10RUN each failed their deliberately targeted control.
The real simulator `real-simh-v1812.sh` verifies that an unchanged
v014 test program completes under PDP-6 GO and that a forced one-step
stop is reported as an expiry failure. A fresh default-OFF DAIMOS
`make build` also passed after the source changes.
Rebuilding old P10RUN and running the same one-step SIMH case confirms
the original false PASS in real simulator execution, independently of
the direct transcript mutation controls.

The remaining acceptance gate is a guest-native DLINK run on a
source-matched DAIMOS root containing the newly built native DXR,
including a truly absent provider, optional archive member, and both
relocation halves. Preserve current accepted KCORE, RAW PURE default
OFF and historic guest results until that gate is performed.
