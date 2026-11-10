# v1371: full normal DAIMOS distribution on a grouped D6CODEC root

Date: 2026-11-08. Status: **native PDP-6 full-distribution grouped
boot, LOGIN, DSH, multiple RUN commands and LOGIN respawn PASS**.
Production remains D6LZ36. No resident decoder optimization.

## Why a second full boot after v1370?

v1370 verified a complete grouped kernel and all 27 MRES packages
against a small 15-node root with three grouped user executables.
That did not prove the real distribution's additional programs and
data could be built, grouped, packed or accessed using actual VFS.

The v1371 testkit builds the **normal root manifest directly from the
current `system/boot/pdp6/Makefile`**, not a separately maintained
copy. It stages native DAS, KCC source and headers, bundled manual
pages, ASMUTILS sources and system configuration in an isolated
`$TMPDIR/d6codec-fulldist-v1371/normal-build` tree. Its auxiliary
Makefile is read only after the production Makefile so the variables
expand correctly. The original DAIMOS source and binary build
outputs are not modified.

The expanded `D6FS_ROOT_ARGS` currently specifies:

| Item | Count |
|---|---:|
| Directories | 29 |
| Files | 237 |
| DXR executables | 89 |
| Legacy D6LZ36-compressed DXR inputs converted to grouped DXR2 | 71 |
| DXR executables retained uncompressed | 18 |
| Other `binwords` files | 148 |

Every original compressed input is replaced by a newly encoded
grouped DXR2 generated from the corresponding uncompressed linked
DXR. The independent host grouped reference checks decoded content;
the DXR producer verifies entry, image/TX2 and relocation metadata.
The inventory refuses missing sources, already compressed DXR
inputs, invalid output paths and duplicate filesystem destinations.
Original uncompressed programs and non-DXR data retain their
existing representation. A versioned manifest includes SHA-256
hashes of all 237 final files. No legacy compressor output is used
to populate the new test filesystem.

The regular host `mkdsk`, `mkd6fs`, `logstore` and `d6fsck` tools
generate four DSK270 member images. The resulting real D6FS checks
**CLEAN with 336 FCBs**, compared with 80 FCBs in v1370's reduced
image. Its new grouped stream output totals 904704 host bytes
across the 71 grouped DXR files (including DXR headers/relocations).

## End-to-end native test results

The complete v1371 runner first repeats the independent v1370
full grouped kernel boot and old-codec negative control. Then it
boots the same grouped KCORE/KINIT and all 27 matching MRES packages
from a fresh clone of the **237-file normal D6FS root**. The boot
uses actual PDP-6 Stage-0, guarded Type-136 DECtape Stage-1, DSK270,
D6FS/VFS, INIT and the installed grouped-format executables.

An interactive ROOT login reaches the grouped DSH. The following
commands execute under the actual guest RUN/syscall path and their
specified outputs are verified (input echo is not sufficient):

- `ECHO V1371FULL`: standalone `V1371FULL` output line.
- `LS /SYSTEM/EXEC`: LOGIN, DSH, GREP, WC and MAKE entries visible.
- `CAT /CONFIG/FSTAB`: correct `# DAIMOS FSTAB V1` header.
- `GREP MEMFS /CONFIG/FSTAB`: `MEMFS:/TEMP:4096` line.
- `WC /CONFIG/FSTAB`: `3 7 55` result.
- `STAT /SYSTEM/EXEC/DSH`: executable type and mode 0555.
- `LS /OPTION/BASE/EXEC`: native DAS, DARC, DLINK, OBJDUMP and KCC
  packages visible in the filesystem (their native execution is
  **not** established by this listing test).
- `EXIT`: a new `LOGIN: ` prompt appears via INIT respawn.

Every phase **PASSes** in native PDP-6 SIMH. Full-root tests always
boot from a separate copy of a pristine CLEAN filesystem, since
normal RW mount operations can modify superblock generation/state.
No filesystem safety check or codec error was bypassed to obtain
the results.

The standalone full regression is:

    DAIMOS_REPO=/path/to/staged/DAIMOS sh \
      tests/validation/d6codec-fulldist-v1371/run-v1371.sh

It does not overwrite or install any production image. All outputs
are isolated under `$TMPDIR/d6codec-fulldist-v1371` and the v1370
and v1368 versioned test workspaces. The grouped kernel cost remains
v1368's +256 text words with reserved low-core geometry; its BSS
and the 27 MRES resident sizes are unchanged.

## Remaining cutover blockers

Even with a full normal grouped image, **production D6LZ36 stays
enabled** until the following are resolved and tested:

1. The existing `D6LZ` user executable is a legacy-format writer;
   it remains present as a normal uncompressed file. Generating a
   new `.d6lz.dxr` would be incompatible with grouped-only KCORE.
   Replace it with an actual grouped producer or remove it in the
   synchronized production conversion; do not install dual decoders.
2. Native DAS/KCC execution and build-toolchain workflows are not
   proven by their presence in the filesystem alone.
3. Full TSFS root/restart media, real PURE evict/swap reload and
   nested runtime VFS/device stack high-water are not yet accepted.
4. Error injection across actual device/media and VFS/EXEC cleanup,
   and subsequent recovery after an interrupted write, are still
   independent required release gates.
5. Host producers, media/boot selection and the production build
   need one atomic grouped-format switchover with no persistent
   legacy fallback; DXR's compressed flag has no codec identity.

The v1371 milestone prioritized actual operation and completeness,
not code-size optimization or premature production enablement.
