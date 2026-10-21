# DSH bounded angle parameter expansion v27 (2026-10-21)

## Syntax and lexical ownership

The native SIXBIT shell does not admit `{`/`}`; DSH now accepts `$<...>`
instead of POSIX `${...}`.  The prefix `$<` has precedence over `<` input
redirection **only when adjacent and unescaped**.  A matching `>` ends the
expression; the following `>` or `<` remains the normal output/input
redirection operator.  Single quotes or an escaped dollar preserve literal
`$<...>` text.  Double quotes permit expansion without changing the
existing quote mask and globbing semantics.  The lexer still preserves
the unmodified `$NAME` and `$[NAME]` forms.

| Form | Result |
| --- | --- |
| `$<NAME>` | Value or empty if unset |
| `$<NAME:-LITERAL>` | Value if nonempty, otherwise literal |
| `$<NAME:=LITERAL>` | Value if nonempty, otherwise assign literal and use it |
| `$<NAME:+LITERAL>` | Literal only when value is nonempty |
| `$<NAME:?LITERAL>` | Value if nonempty, otherwise diagnostic and status 2 |

An empty `:?` diagnostic prints the variable name.  The `:=` setter
preserves the previous exported flag, including when an exported variable
was empty.  A newly assigned variable is not exported by default.

The name grammar matches existing DSH uppercase variable names and `_`:
letters/underscore initially, then letters/digits/underscore.  Neither
the expression token nor the resulting SIXBIT value may exceed 36
characters.  Literal defaults may contain spaces and ordinary operator
characters, including `&`, in one token.  The first `>` ends an expression;
embedded closing brackets, quote marks, backslashes, `$`, and recursive or
nested `$<...>` defaults are **not** supported in v27.  Invalid forms fail
without silently falling back to redirection or truncating the value.

## Resource and safety decisions

The implementation reuses `dsh_expand_quoted` and the existing two-word
variable descriptor/heap, with no new persistent buffer.  The local
fallback is a bounded SIXBIT record to avoid aliasing the assembler
variable getter's global scratch record during `:=` updates.  The error
path writes a bounded diagnostic to stderr and returns the shell status 2.

No overlays or private parameter AST were added.  GCC image size and BSS
are measured in `PROJECT-STATE.MD`.  This addition consumes executable
text, so the strict `<030000` octal code target remains a separate gate.

Host lexer tests and 25 dedicated SIMH parameter regression probes live in
`daimos-testkit`; all pass.  The legacy suite passes 113/115, with exactly
the two compact HELP format failures already seen on v26 (unrelated to
angle expansion).  Fix/accept those failures independently before treating
the *entire* DSH conformance suite as green.
