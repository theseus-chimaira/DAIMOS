# v440: KCC DSH image fits the unchanged EXEC size gate

One helper in `userland/dsh/dsh_s6.c` now clears the fixed-size SIXBIT
and input-line buffers. Both public functions preserve their original
length reset, null handling and complete buffer-zeroing semantics.
The helper introduces no new persistent storage.

Controlled KCC/PDP-6 clean-build comparison, same sources and flags
apart from this edit, both using the same linker and fold configuration:

| Build | DXR image (octal) | BSS (octal) |
|---|---:|---:|
| Original | `036013` | `012751` |
| Shared word-clear helper | `035757` | `012751` |

The new image is 034 octal (28 decimal) words smaller and now fits
the unchanged `036000` limit with 021 octal words of headroom.
No compiler change, GCC substitution or gate override was used for
this size comparison. The FIFO fixture is restored to an explicit
`COMPILER=kcc` selection to enforce KCC validation. The full native
PDP-6 FIFO regression PASS confirms MKFIFO, OPEN rendezvous, shared
runtime, rename, unlink reuse, mount-busy and D6FS persistence.

The separately tracked KCORE size is unaffected by DSH source edits.
