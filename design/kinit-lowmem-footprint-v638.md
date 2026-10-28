# KINIT 32K footprint recovery v638 (2026-10-28)

The v637 lowmem TSFS+AUXSTORE bootstrap had an image end of octal
`077236`, exceeding the 32K stack-safe end of `077200` by `036` octal
(30 decimal) words. The stack reserve remains octal `0600`.

The transient PDP-6 KINIT diagnostics now use three compact native
assembly services in `kinit_io_pdp6.s`, replacing the corresponding
C-generated routines without changing the public call contract:

- `kinit_probe_word`: same two complementary bit-pattern tests, same
  original-word restoration, one shared pattern literal.
- `kinit_put6_spaces`: one countdown loop, including zero-count support.
- `kinit_put_memory`: two positive-range `IDIVI` operations encode the
  detected 32..256 Kword count into the original decimal SIXBIT field.

The first two C-only memory-probe restructuring experiments did not save
linked words (the second grew by one) and were discarded. Native assembly
was selected only after source and generated-object examination.

The linked KINIT image end is now `077200`, a reduction of exactly
30 decimal words. `mkimage` passes with 32K core and the unchanged
octal `0600` stack reserve. There is zero additional linked-image
headroom at this limit; failure-path stack watermark testing and other
boot profiles remain separate acceptance tasks. Permanent KCORE is not
changed by this patch.

Native PDP-6 SIMH 32K TSFS+AUXSTORE v493 PASS, and an independent
native boot prints `MACH PDP6`, `MEM 32 K` and reaches `INIT V1`.

The complete image build also revealed a v636 host-tool interaction:
`mkdsk` writes a known one-member clean DBOOT descriptor in sector
zero before `mkd6fs` formats the protected D6FS partition. The
boot-image build now passes explicit `--convert-dboot` and
`--confirm-destructive` to `mkauxstore`. Conversion only accepts a
fully validated known DBOOT header with an exact matching D6FS/tail
layout and empty backing ranges; it still rejects unknown or nonempty
media. Existing D6FS contents are preserved, and no automatic
formatting occurs during DAIMOS boot. Testkit v638 conversion safety
and the existing v636 host safety suite pass.
