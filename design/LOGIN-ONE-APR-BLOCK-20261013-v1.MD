# LOGIN: smaller PDP-6 APR allocation (v1, 2026-10-13)

All sizes below are octal 36-bit words. LOGIN uses the existing DXR2
ST2 header extension, supported by the SUPERVISOR v11 loader. No kernel
ABI or storage interface changes are required.

| Component | Before | Now |
| --- | ---: | ---: |
| Executable | 006133 | 006133 |
| BSS | 000255 | 000255 |
| Origin | 000020 | 000020 |
| Reserved stack | 002000 | 001000 |
| Unrounded requirement | 010430 | 007430 |
| APR-rounded extent | 012000 | 010000 |

The one-block reduction saves 02000 octal (1024 decimal) resident
physical words per active LOGIN image. A 00400 stack would produce
the same rounded extent, so the larger 01000 reserve is preferable
until an instrumented worst-case high-water study exists. LOGIN has
large local EXEC argument arrays and nested account-file parsing;
do not assume the SUPERVISOR's smaller 00400 stack is sufficient.

The Type-6 disk build uses the uncompressed LOGIN DXR. D6LZ currently
rejects DXR2 ST2 metadata rather than preserving its special stack
reservation. This is a storage-size tradeoff only: replacing ST2 with
the normal stack default would silently erase the RAM savings and is
not acceptable. The general compression format is unchanged.

Acceptance must include real PDP-6 LOGIN -> DSH and the full shell
conformance regression, DXR ST2 metadata/extent bounds, and physical
LOGSTORE integrity on a fresh image. Future targeted tests should
measure LOGIN stack high-water under maximal account/password/path
lengths, failed authentication, successful login, and EXEC failure.
The unrelated 32K system boot remains separate from this allocation
proof.
