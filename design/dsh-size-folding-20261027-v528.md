# DSH no-feature-change linked-image reduction (v528, 2026-10-27)

Goal: restore the existing `036000`-octal DSH EXEC image-size gate without
removing shell functionality, relaxing that limit, or rewriting concurrent
work in `userland/dsh/dsh_exec.c`.

## Linked KCC measurement

Compiled the current checkout with KCC into a versioned separate build root:

- Original DSH folding plan: `p10fold -m 4` examined assembler-only inputs
  `$(DSH_S_OBJS)`; linked DXR image `036157` octal.
- Updated plan: same four-word threshold, but both `$(DSH_S_OBJS)` and
  `$(DSH_C_OBJS)` are examined. Linked DXR image `035725` octal.
- Net linked reduction: `0232` octal = **154 decimal 36-bit words**.
- Original image was `0157` octal = 111 decimal words above the `036000`
  EXEC limit; new image is `0053` octal = 43 decimal words below it.
- Both builds retain DXR BSS `001763` octal. The optimization changes only
  folded instruction runs; no shell source, data structures or runtime
  storage layouts are changed.

The build retains `-m 4`: lowering this to three was explored but is not
necessary. The key was including C objects in the fold-plan dependencies,
which also ensures they invalidate the plan when recompiled.

## Validation

- Separate KCC DSH builds and linked image-size comparison: PASS.
- Existing `daimos-apr-user-protection-v1.sh` full-image native SIMH boot:
  PASS; previously blocked before SIMH by the oversized DSH executable.
- `daimos-dsh-parser-direct-20261011-v1.sh`: PASS (host parser).
- `daimos-dsh-bracketed-expansion-20261011-v1.sh`: PASS (host expansion).
- `daimos-dsh-kcc-command-limits-20261009-v1.sh`: PASS.

The full interactive DSH conformance suite has not been rerun in this
checkpoint, so treat the existing runtime evidence as boot/APR acceptance,
not a complete command-level acceptance claim. No KCORE change or RAM
high-water reduction is claimed; this only reduces DSH's linked executable.
