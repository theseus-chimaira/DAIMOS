# Shared ED/EX line core for DSH (v1, 2026-10-21)

## Boundaries and ownership

`userland/libc/edcore.{h,c}` owns **only operations on an existing bounded
packed-SIXBIT line**.  Its caller owns the word buffer and supplies its
capacity; the primitive never allocates, performs I/O, or maintains globals.
Four operations are provided: character read, insertion, deletion and
replacement.  They preserve fixed six-characters-per-word packing.

DSH now uses this core to modify its 180-character interactive input line,
while DSH remains responsible for TTY mode, cursor redraw, Ctrl-C, completion
and script execution.  The direct host tests live in `daimos-testkit`.

Later ED and EX should reuse the same byte/word-editing core, not import
DSH's lexer, shell-specific expansions or TTY logic.  Their next layer
should own a dynamically bounded *collection of lines*, addressing and
file I/O.  EX then adds a command and interactive-mode frontend over the
same storage.  No multi-line ED/EX features are claimed here yet.

## Scripts vs interactive facilities

The 32K configuration must retain DSH script parsing, `SOURCE`, functions,
loops, redirection, jobs and background processes.  History is optional;
completion and advanced terminal interaction may later be separately gated.
This shared primitive is deliberately small enough to remain present even
when those optional UI layers are excluded.

## Future parameter syntax (reserved, not implemented here)

The proposal `$<NAME>` conflicts *lexically*, not semantically, with `<`
and `>` redirection.  It can be recognized only on the immediately adjacent
`$<` sequence before generic operator tokenization.  Quoted/escaped text
must retain its existing literal semantics.  `$ < FILE` must continue to
mean separate word and redirection tokens.  Any `>` inside a default value
requires a defined escape/quote rule; nested `$<...>` must be designed
explicitly rather than acquired accidentally.  Existing `$NAME` and
`$[NAME]` must not change.  No parser/lexer implementation is included in v1.

## Follow-up gates

1. Verify the DSH interactive editor, both host and target, with full-line
   editing and redrawing at word boundaries.
2. Build a shared ED line-store and reusable command dispatcher only after
   its memory and low-RAM lifetime requirements are measured.
3. Test BREAK/CONTINUE (including multi-level and function-local use) as
   an independent scripting change before extending loop-state semantics.
4. Keep implementation and completed documentation in sync, and measure
   GCC image, BSS and stack independently.  No overlays.
