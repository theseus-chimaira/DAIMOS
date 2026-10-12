# DSH variable allocator shrink: phase-zero v16 experiment

Status: investigation only.  No experimental code retained.  No overlays.

## Reproducible KCC/DLINK measurements

The clean current `main` checkout (not the historical v13 image) built with
the PDP-6 KCC toolchain in `build/system/boot/pdp6/userland-build` reports:

| Implementation | DXR image (octal) | DXR BSS (octal) |
| --- | ---: | ---: |
| Current fixed 13-word variable slots | 036523 | 015241 |
| Experimental shared allocator with C variable implementation | 037213 | 013742 |
| Difference | +000470 | -001277 |

The prototype saved 0607 octal words in combined image plus BSS, but broke
the current image acceptance gate (temporary 037000, production 036000).
These are build measurements, not tested runtime or peak physical allocation.
They must not be presented as an accepted shrink.

## Root cause and proposed solution

The historical lazy-payload prototype was not intrinsically wrong: its
failure was caused by two independent users of the process break.  The
20-argument fallback in `dsh_exec_simple_large()` directly grew and then
restored BRK.  A variable stored during that interval could outlive the
temporary argument array, so the restore invalidated persistent storage.

The v16 experiment used a shared, word-oriented first-fit allocator for
both variable payloads and the large argument fallback.  It kept free
blocks in address order and coalesced adjacent blocks.  It never restored
BRK over live allocations.  The variable table was reduced to two words
per slot (metadata and pointer) and each packed name/value allocated only
its used SIXBIT words.  Reassignment allocated replacement storage before
freeing the old value; unset released storage.  The implementation compiled
and linked, but its C-generated code increased the DXR image too much.
The untested experiment was reverted in full.

## Next implementation gate

1. Keep the existing compact assembler variable entry points, preferably
   modifying their slot addressing and payload-copy loops instead of
   replacing them with generic C.
2. Build a small, shared assembly allocator, or compare against a carefully
   measured dedicated allocator, with distinct persistent and temporary
   lifetimes.  No permanent BRK floor and no unreclaimable leaked blocks.
3. Preserve all 64 variables, 36-character names and values, export rules,
   and full 20-argument fallback.
4. Test assignments made *inside* a large expansion, repeated set/grow/unset,
   all-slots-full, memory exhaustion, functions, scripts, and nested SOURCE.
   Put new regression fixtures in `daimos-testkit`.
5. Compare both image gate and image+BSS+live heap high-water, with a clean
   KCC build.  Reject an implementation that cannot meet the image gate or
   demonstrably worsens peak resident memory.

Do not update `pdp10-doc` with experimental behavior.  The user-visible
variable contract is unchanged until a tested implementation is accepted.
