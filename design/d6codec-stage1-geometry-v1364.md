# v1364: PDP-6 Stage-1 pre-write memory geometry

Date: 2026-11-08. Status: validated staged implementation; no
production Stage-1, boot media or D6LZ36 format change.

## Source-derived physical-memory contract

The boot image builder (`system/boot/pdp6/image/mkimage.sh`) already
accepts a configured core word count and KINIT stack reserve. The
standard `system/boot/pdp6/Makefile` profiles use `0100000` octal
(32K) for lowmem or `01000000` octal (256K) for full, with a default
KINIT stack reserve of `0400` words. The compressed-image producer
also validates the staging end against the configured core size.

Until v1364, Stage-1 did not independently validate the combined
image and compressed payload staging extent before its first payload
store. This matters for corrupted or forged headers, even when the
normal host producer generates correct images. A 36-bit PDP-6 word
stores two 18-bit header lengths; checking only their individual
lengths does not establish that both regions fit in installed RAM.

## Validated staged geometry helper

The unlinked `system/stand/pdp6/common/stage1-geometry-v1364.inc`
accepts AC1=uncompressed KINIT word count U (>0), AC2=compressed
word count C (zero for raw), AC3=configured physical core size M,
and AC4=KINIT stack reserve S. Return AC0 is 0 on success or -1
on failure; AC1..AC16 are unchanged. One PUSHJ return word uses AC17.

The checks are, using full-word arithmetic:

    030000 + U + S <= M
    030000 + U + C <= M

The first guarantees the decoded KINIT can begin its stack safely;
the second guarantees the entire compressed payload can be staged
immediately after the decoded region. They also accept the raw
`C=0` paths used by DSK, DRM and DTC/TSFS hybrid. Existing loader
code checks the DAIMON magic, nonzero decoded length, entry offset
and media mode separately. The helper is called **after those header
fields are extracted, before the first payload-copy or tape-write
operation**.

For two-tape PTR boot, the staged header path now additionally rejects
Tape 1's compressed segment if its count exceeds the total compressed
payload count, before writing any of that first segment. The existing
Tape 2 exact-total check remains in place.

The staged test generator takes explicit octal core and stack-reserve
parameters. Production migration must pass actual `BOOT_CORE_WORDS`
and `KINIT_STACK_RESERVE_WORDS`, not silently impose a fixed 32K/256K
limit or default stack configuration.

## PDP-6 native acceptance

`daimos-testkit/tests/validation/d6codec-stage1-geometry-v1364`
checks exact-fit and one-word-overrun cases for 32K and 256K,
zero decoded length, and preservation of input ACs. The runner
produces and assembles ten Stage-1 alternatives: five devices for
each normal memory profile. Native PDP-6 tests extract and execute
the actual staged header guard for every profile/device combination
(ten PASS), including PTR's additional first-segment rejection.

Two more tests (DTC/PTR) run with 64K `0200000` physical memory and
`02000` stack reserve, confirming configured limits are honored.
All **12 native header-guard cases and the separate helper test PASS**.

Measured text increase over the v1363 direct-linked Stage-1 version:

| Loader | +words, 32K | +words, 256K | New BSS |
|---|---:|---:|---:|
| DSK | 20 | 21 | 0 |
| DRM | 20 | 21 | 0 |
| DTC | 22 | 23 | 0 |
| MTC | 22 | 23 | 0 |
| PTR | 24 | 25 | 0 |

The extra word in the 256K builds reflects the full-word literal
handling needed by this assembler; the 18-bit-immediate form cannot
represent the physical endpoint `01000000`. Both profile families
assemble within the low Stage-1 footprint below stack address
`020000`, and the inspector verifies no BSS growth.

## Still open

This is a test-only source and generated test read-in images. The
five real device bootloaders have not been switched or booted with
grouped KINIT streams. Actual device failure, premature EOF, header
corruption during media transfer and rollback still require tests.
The grouped format must ultimately be enabled atomically across
host producers, all Stage-1 loaders, KINIT, KCORE, VFS, EXEC/PURE
and TSFS without a production mixed-format fallback. Permanent KCORE
RAM and production D6LZ36 are unchanged by v1364.

## v1365 subsequent boot validation

The grouped v1364 pre-write guards and direct-linked v053 decoders
are now exercised by five **actual SIMH PDP-6 controller boot paths**
using a real complete linked KINIT image. DSK, DRM, DECtape, magnetic
tape and two-volume paper tape all enter KINIT at 053524. Two corrupt
DECtape media tests (oversized decoded length and zero-token control)
halt before KINIT and leave 030000 untouched. See
`design/d6codec-stage1-real-boot-v1365.md`. No production format
selection or resident KCORE code has changed.
