# SUPERVISOR: one PDP-6 APR block (2026-10-12 v11)

**Status:** validated on independent fresh disk images with physical
AUXSTORE persistence verification, strict maximum-service stress, and
full 115-probe DSH conformance. The matching daimos-testkit v8/v9/v11
checks are retained. All addresses/sizes below are octal
36-bit words, except where explicitly marked decimal.

## Resident PID 1 allocation

| Component | v7 | v8 |
| --- | ---: | ---: |
| SUPERVISOR executable image | 001210 | **001177** |
| BSS | 000051 | **000051** |
| User origin | 000020 | 000020 |
| User stack reservation | 002000 | **000400** |
| Unrounded allocation requirement | 003301 | **001670** |
| PDP-6 APR-rounded allocation | 004000 | **002000** |

The SUPERVISOR now occupies one hardware relocation/protection block,
saving **02000 octal / 1024 decimal words** in the process extent.
Nothing changes the machine's 02000-word relocation granularity.

The increased kernel footprint is measurable and smaller than the
process saving. Comparing fresh v7/v8 boot builds:

- `__kcore_low_end`: `023610 -> 023702`, **0072 octal / 58 decimal**
  additional KCORE words.
- `logstore-mres.dxr`: 800 -> 1096 container bytes, an additional
  **37 decimal 36-bit words** in the LOGSTORE MRES image.
- Net standing resident saving is approximately **929 decimal words**,
  excluding unrelated runtime MM allocations or descriptor changes.
  During an event write, an additional 0200-word dynamic scratch is
  temporarily allocated, then freed.

## Stack-size DXR2 metadata

The existing DXR1 two-word and DXR2 `TX2` formats are unchanged.
Plain, non-PURE, uncompressed DXR2 may use a third header word
`stack_words,,SIXBIT/ST2/` instead of `text_words,,SIXBIT/TX2/`.
`stack_words` must be between `0400` and `02000` inclusive; the
default for all old executables remains `02000`. Compressed or PURE
images retain the `TX2` text-prefix extension and cannot use `ST2`.

Both host and on-target DLINK recognize `--stack-words OCTAL` (the
on-target spelling is `--STACK-WORDS`). The DXR verifier and filesystem
image builders check the extension and reject invalid combinations.

`exec_load_process` allocates `image+BSS+origin+stack_words` (rounded
by the PDP-6 APR VM backend) and writes the validated stack size into
protected logical origin word `017`. This internal value belongs to
the **new** process VM, not to any persistent kernel descriptor.
Both RUN and the normal/low-memory EXEC paths use the same
`vm_space_startup` code, which reads word `017`, checks every counted
SIXBIT startup record before copying data, and requires `0100` words
of free stack space beyond the startup records. On failure the existing
loader transaction destroys the incomplete VM. This protects adjacent
process extents; runtime APR protection remains the ultimate guard.

## Compact LOGSTORE event transport

Only the SUPERVISOR build switches from `SYS_LOGCTL_APPEND` to the new
root-checked `SYS_LOGCTL_EVENT` suboperation `011` (MRES operation 7).
The new operation receives four contiguous user words: timestamp,
packed severity/source/payload-count, code, and value. It allocates a
temporary 0200-word kernel-dynamic buffer (owner `015`), zeroes it,
copies those four words into record offsets 2..5, invokes the existing
transactional `logstore_append`, then frees it on every completion
path. The existing APPEND and on-disk LOGSTORE record format remain
unchanged. The dynamic allocation is not permanent KCORE, and its
failure propagates as a failed log call.

The LOGSTORE transient allocation owner `015` is deliberately separate
from DPY's owner `014`. The syscall holds the current process mapping
while the four-word user record is read. No 0200-word user-stack buffer
or permanent BSS block is needed for SUPERVISOR logging.

## AUXSTORE persistence regression and correction (v11)

The initial v8/v9 persistence runs uncovered a real kernel error shared
by APPEND and EVENT. The AUXSTORE branch of `logstore_minit` installed
`MODULE_SERVICE_DRM_READ_BLOCK` / `WRITE_BLOCK` (or the matching DSK
raw services) as the backend jump target. Those relocated MRES entries
take `AC1=hardware_address, AC2=buffer`, but `logstore_io_direct`
generates `AC1=unit, AC2=physical_block, AC3=buffer`. This silently
misdirected writes into physical drum block zero using a code-address
buffer instead of the actual record. The LOGSTORE sequence advanced
despite the lack of a correct durable write.

MINIT now checks raw service availability and instead patches the
validated KCORE `drm236_read_block` / `drm236_write_block` or
`dsk270_read_sector` / `dsk270_write_sector` bridges, whose
unit/block/buffer ABI matches the LOGSTORE direct path. This fixes
the cause rather than changing callers, record format, or recovery
policy. Host inspection of the drum's AUXSTORE descriptor and the
physical LOGSTORE slots independently confirms the repair.

## Acceptance and remaining coverage

- Full PDP-6 production disk-image build; DXR size and ST2 checks.
- Standard CTY LOGIN -> DSH conformance, 115 shell probes.
- Two independent fresh-image `LOGD; LOGDUMP` persistence probes PASS,
  with valid INIT_READY, LOGIN, and LOGD records on media.
- Strict eight accepted INITTAB entries (TTY0 LOGIN plus seven distinct
  supported TTYs, seven 102-character invalid paths) PASS: seven distinct
  launch-failure events and the LOGIN event are recovered from physical
  AUXSTORE slots. The descriptor in physical block zero remains intact.
- Full standard shell conformance: 115/115 PASS on a third fresh image.
- Negative DXR checks: ST2 below 0400, above 02000, bad tag, and
  forbidden PURE/compressed combinations.

Repeated PID 1 respawn exhaustion, target-side user-stack high-water
instrumentation, and full 32K LOGIN-to-DSH boot remain separate tests.
Do not infer their results from the one-block size gate alone.
