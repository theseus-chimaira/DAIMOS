# DSH image-size failure in PIPE regression: v418 resolution

Status: fixed in daimos-testkit; full PDP-6 simulator regression PASS.

The PIPE word-FIFO v2 fixture built its INIT with PDP-6 GCC but did
not select a compiler for the full boot-image build. The boot Makefile
then selected its full-profile default KCC for userland. The produced
DSH DXR image exceeded its unchanged EXEC size limit:

    Incorrect full-profile/KCC DSH: 036013 octal
    Production DSH image limit:   036000 octal

The correct existing compiler policy permits full-profile kernels with
GCC userland and requires GCC for the smaller DSH needed at this
limit. The testkit now explicitly requests `BOOT_PROFILE=full
COMPILER=gcc` from its boot-image submake. This is not a raised limit,
new compiler configuration or reduced shell capability. Trying
`BOOT_PROFILE=lowmem COMPILER=gcc` instead selected an unnecessarily
small kernel layout for this test and failed the independent KINIT
stack-reserve check; that experiment was discarded.

Verified GCC DSH via `dxrcheck`:

    DXR image=031313 bss=012751 entry=000000 reloc=000552

The full `make test-daimos-pipe-wordfifo` build then booted PDP-6
and completed:

    daimos-pipe-wordfifo-v2: PASS
    (opaque word FIFO, wrap, EOF, EPIPE, dup refs,
     RUN refs, blocking wakeup)

The testkit fix is commit 4e531d6. KCC DSH remains supported for
configurations not requiring this strict image size; its executable
was not reduced or recompiled under this patch. No kernel memory or
DSH_IMAGE_LIMIT adjustment was made, and this does not establish the
deferred strict 32K runtime acceptance.
