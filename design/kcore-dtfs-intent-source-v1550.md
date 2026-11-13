# KCORE P0-B v1550: DTVOL2 format and native record validator

Date: 2026-11-13. Incremental source implementation, no live-mount
enablement. P0-B remains OPEN; this report does not supersede the
required durable transaction in v1549.

## Implemented

`system/kernel/fs/dtfs_intent_v1550.h` centralizes the fixed
36-bit DTVOL2 magic `0446466575422`, exact WRITE_INTENT state
`0552255225522`, seal salt `0246134572610`, and six used words.
The four-word identity checksum remains the complement of
magic XOR the two nonzero ID halves; the new word 5 seal is the
complement of magic XOR both ID halves XOR state XOR salt. All
122 remaining physical words must be zero.

`system/kernel/fs/dtfs_intent_validate_v1550.s` is native PDP-6
code with ABI `AC1=pointer to RBN0[0200]`; returns `0` CLEAN,
`1` DIRTY or `-1` INVALID in AC1. The helper executes the
actual complete full-word checks and is independently testable.
It does not allocate RAM, alter owner state or call disk I/O.
**It is deliberately not yet linked into the mounted provider**.

The host `tools/host/mkdtfs.c` preserves its normal DTVOL1
`mkdtfs -o new-image` output for compatibility. An opt-in
`mkdtfs --durable-v1550 -o new-image` creates an isolated
DTVOL2/CLEAN image using the existing strong random 72-bit ID
generator and otherwise unchanged native DTFS directory/owner
layout. The opt-in prevents new-format media from becoming the
default before runtime write-ahead, admission and clean-unmount
semantics have been physically qualified.

## Tests executed

Testkit `daimos-dtfs-intent-native-v1550.sh` compiles the host
formatter, decodes real 18-bit-half DECtape image bytes, and
feeds 13 clean/dirty/invalid, torn, legacy, duplicate-format
and replacement-identity records to genuine PDP-6 instructions.
**13 native cases PASS. Seven unsafe source mutations rejected.**

An initial harness failure was identified as a generated `.word`
missing its required leading octal zero: bare large numeric words
were interpreted as decimal by the assembler. After correcting
that generated fixture, all 13 and all seven mutants pass. No
production validator logic changed in response.

The existing complete v1520 testkit formatter/probe/native mount
identity regression still PASSes, including five native source
mutations. Existing default DTVOL1 output and target MKFS reset
are unchanged. KCORE fixed code, installed optional DTFS MRES,
runtime memory, syscall ABI and production media are unchanged.

## Next P0-B feature gates

Before enabling DTVOL2 for real mounts: update the mount-control
physical RBN0 validation, all formatter and target probe users,
and the low-level write path as a coherent transaction. A
provider-serialized raw readback of newly written WRITE_INTENT is
required **before the first native DATA/DIRECTORY write**. This
must use independent MM scratch, never corrupt the caller's
`fs_block_workspace` or `dtfs_dir`. Only after a checked final
owner-safe unmount may CLEAN be committed and verified. Reject
old ambiguous DTVOL1/legacy media under the new integrity policy
instead of silently upgrading them. Run native Type-551 partial
write and full-guest reinsertion/cold-boot tests before closing
P0-B. P0-A is complete; P1-C and optimization are unrelated.

Details: `daimos-testkit/tests/validation/daimos-dtfs-intent-native-v1550.md`.
VERSION 0.5, strict 32K/optimization HOLD.
