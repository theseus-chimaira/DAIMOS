# v111 — Share repeated `mm_compact` gap-search setup

Baseline: production v110, MM `.text` 485 words and `.bss` 70 words.
`mm_compact` calls `mm_find_fit(words,alignment,LOW,&result)` initially
and after every successful process relocation. Both call sites formerly
repeated the same four AC argument setup instructions plus `PUSHJ`.

The v111 implementation uses `PUSHJ 17,mm_compact_find` at each site.
The private tail-call helper loads the four arguments and `JRST`s to
`mm_find_fit`. The helper takes the return address pushed by `PUSHJ` into
account: `MOVEI 4,-1(17)` points to the caller's existing fit-result slot.
This keeps all prior success, exhaustion, restart and error semantics.

A first experimental helper used `(17)` for the output slot; this was
**incorrect** because the extra return address changes the stack index.
The v099 relocation native test hung and timed out. The address was corrected
to `-1(17)`; this is a genuine test-detected candidate defect, not a
production baseline failure. The candidate was not committed until fixed.

| Assembled MM measurement | v110 | v111 | Change |
| --- | ---: | ---: | ---: |
| `.text` | 485 | **482** | **-3** |
| `.bss` | 70 | 70 | 0 |
| Combined words | 555 | **552** | **-3** |

Measurement: `das -F -C -L/-O`, `pdp10-objdump -h`.
After correction the PDP-6 emulator passed `alloc-v090`, `free-v092`,
`gap-v094`, `noreclaim-v095`, `relocation-v099`, and `vm-live-v102` through
`vm-live-v106` (except no separate v100/v101 replay in this checkpoint).
GCC KCORE/KINIT DXR link PASS. The helper adds one return-address stack
word transiently during gap search; the helper's tail jump requires no
second return address. Runtime cycle and whole-KCORE post-fold impact are
not measured. Full-system boot and scheduler/PI concurrency remain open.

ITS-138/JOSS have different block memory maps, so this is a measured local
PDP-6 instruction-sequence optimization rather than a claim of direct
historical algorithm equivalence.
