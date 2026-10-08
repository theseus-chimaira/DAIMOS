# Native MAKE recursive-frame experiment (2026-10-08 v1)

Status: isolated experiment, NOT merged to production.

With the production MAKE, a fresh 256K native `MAKE ALL` built directory B,
then returned `STATUS:131073` before the first KCC command. Direct INSTALL
and MAKE B passed. Traces established RUN/WAIT and B returned success and
MAKE advanced into a five-level dependency chain toward `CC.C`.

The generated MAKE `make_build` recursive frame was octal 0121 words (81
words), partly because it held two 104-character implicit-rule buffers.
Relocating those buffers into depth-indexed storage reduced the frame to
035 octal words (29 decimal). The static 33-frame test advanced to KCC but
would permanently cost about 1749 user-process words of BSS.

The improved version instead lazily allocates a buffer with `malloc()` the
first time each recursion depth is reached, retaining it for subsequent
targets. It requires native `stdlib.c` and `string.c` in the MAKE link.
Native `string.c` lacked `memcpy`, required by `realloc`; this branch adds
a correct C99 implementation, operating in target C characters (9-bit).

Full image: linked successfully. Fresh `MAKE ALL` reached `MAKE: BUILD
B/CC-CPP-V1.S` and launched the KCC CC.C compiler command. The harness
then timed out at 95 seconds; there was no return status for the full KCC
compile. Standalone host memcpy test passed in DAIMOS-testkit.

This does not conclusively identify the root cause of the earlier TERM, and
is not yet an accepted production kernel or MAKE fix. Next: test deep
recursive dependency and heap/BRK exhaustion behavior, verify compiler
outputs, measure heap peak and complete self-hosting before merge.
