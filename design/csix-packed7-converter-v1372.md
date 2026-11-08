# v1372: native packed ASCII7 -> canonical C-SIX/S6REC conversion

Date: 2026-11-08. Status: **host and actual PDP-6 native full-root
converter tests PASS**. There is no change to the resident DAS,
grouped KCORE or production D6LZ36 image.

## Canonical DAIMOS source representation

Use the documented reversible **C-SIX** conversion rules, not a
second case-folding syntax. Native C source lives in SIXBIT S6REC
records and can be inspected and edited by normal native SIXBIT text
utilities. In C-SIX, raw `A..Z` encode logical lowercase `a..z`;
`@A..@Z` encode logical uppercase; `@@` is literal `@`; `@'`,
`@<`, `@!`, `@>` and `@-` encode backtick, `{`, `|`, `}` and `~`.
S6REC records delimit lines. CRLF and tab conversion follow the
existing host CSIX encoder. This is syntax-blind and lossless for
the documented repertoire.

The old `s6filter` performs *assembly-specific* uppercase SIXBIT
normalization and rejects lowercase within semantic literals.
It is not a replacement for the reversible C-SIX contract and
must not be used to convert arbitrary C source. Likewise, native
DAS still reads ordinary uppercase SIXBIT/S6REC **assembly**, not
escaped C-SIX. Keeping these representations distinct prevents
silently changing C source or adding a permanent DAS ASCII parser.

## Changes and RAM budget

The canonical `tools/host/csix.c` host converter gains an optional
`-e -p` packed-ASCII7 source mode. It reads the normal 8-byte host
container for 36-bit words, extracts five seven-bit characters per
word, checks NUL padding, and feeds the *existing* C-SIX encoder.
Ordinary `csix -e` and `csix -d` behavior is unchanged. This mode
is useful for cross-host inspection of actual native KCC outputs.

The isolated native C89 `userland/toolchain/csix_v1372.c` accepts
`CSIX INPUT.ASCII7 OUTPUT.S6REC` with counted native SIXBIT paths.
It calls `dsys_read_words` in 16-word chunks and streams source
characters directly through `u_text_sink_attach`/`u_putc`, rather
than retaining an entire source file or constructing an alternate
format. Uppercase, punctuation and text-line boundaries follow the
documented C-SIX rules. Malformed or incomplete input is rejected,
and the output file is removed on failure. Native image measurements:

| Artifact | Octal words |
|---|---:|
| Native CSIX DXR image | 002700 |
| Native CSIX BSS | 000106 |

The host tool and native converter are built by a **test-only**
v1372 runner, without adding either new image to the production
filesystem or changing DAS's 033244-word first-phase budget.

## Verification

`daimos-testkit/tests/validation/daimos-csix-packed7-v1372/run-v1372.sh`
reuses the accepted full grouped KINIT/KCORE/MRES plus the normal
237-file D6FS distribution on native PDP-6 SIMH. It first proves
byte-identical S6REC output from host CSIX's ordinary input and new
packed-7-bit mode, then decodes back to the original mixed-case C89
source. It rejects trailing nonzero data after packed NUL padding,
files without final newline and truncated packed words. A second host
fixture verifies exact CRLF normalization and eight-column tab stops.

The guest boots the complete grouped kernel on a fresh root clone;
the native CSIX program converts the packed fixture to S6REC.
The actual 36-bit output words match the host CSIX output using
the real guest `CMP` command (exit status zero). Guest `CAT`
displays the C-SIX escapes. Native KCC reads and compiles the
converted source; the invalid-padded source is rejected, and
its output file is absent afterward. A second packed CRLF/tab fixture
matches its host reference at the full 36-bit-word level as well.
All eight native probes PASS.

## Compiler issue discovered: separate blocker

Native KCC currently produces malformed assembly from a very small
valid C89 `return 012345` fixture, with e.g. `FMPL 017,0` where the
host compiler generates `MOVEI 1,012345` followed by `POPJ 17,`.
The malformed native output is present in the **packed ASCII7
file before conversion**, and appears both with and without native
`-O`. This is a KCC compiler/emitter defect, not a CSIX encoding or
DAS parser failure. A zero KCC exit status by itself does not prove
correct compilation. The native compiler -> assembler -> linker ->
executable RUN chain remains an OPEN gate. Compiler correctness has
priority over any KCORE size optimization or production cutover.

The earlier experimental DAS ASCII7 reader and non-reversible
`S6CONV` converter were discarded. The new path reuses the existing
C-SIX representation and leaves resident DAS unchanged.
