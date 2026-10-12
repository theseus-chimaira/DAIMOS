# DSH easy image/CPU optimizations — source audit (2026-10-12 v1)

Scope: keep the frozen DSH language and PDP-6 KCC/DAS behavior. Prioritize
permanent RAM, then execution speed, then disk usage; avoid features and
unmeasured savings claims. This stage does not change D6FS or the kernel.

## Measurement method and results

Fresh isolated `make -C userland` DSH DXR builds, KCC `-Pgnu99 -O -x=pdp6
-m=gas`, DAS, DLINK and the existing hand-assembler-only P10FOLD plan.
All sizes are **octal 36-bit words** as reported by DXRCHECK.

| Step | Image | Increment saved | BSS |
| --- | ---: | ---: | ---: |
| Unmodified HISTFILE-repair baseline | 036722 | — | 015241 |
| Move unused u_text_fields/key to libc/text_config.c | 036356 | 0344 | 015241 |
| Share expansion quotation-mask marking in dsh_expand_quoted | 036233 | 0123 | 015241 |
| Move unused u_s6_eq into libc/u_compare.c | 036100 | 0133 | 015241 |
| Remove redundant dsh_line_ch lexer wrapper | 036065 | 0013 | 015241 |
| Remove trivial dsh_space wrapper | **036054** | 0011 | **015241** |

Total saving: **0646 octal = 422 decimal words**, with unchanged BSS.
Normal hard image limit: 036000; remaining excess **0054 octal = 44 decimal
words**. The image meets only the temporary 037000 development limit.

The libc functions remain linked into other programs where required. The
source lists for normal command builds, toolchain executables and D6LZ
were updated accordingly. A full PDP-6 boot disk build succeeded, followed
by **115/115 DSH target conformance probes**. The separate testkit size
guard verifies that the three unused helpers no longer appear in the
DSH map and that image/BSS have not regressed from this stage.

## Next simple candidates, not yet implemented or measured

1. **History traversal CPU cost** (`dsh.c:dsh_history_remember`):
   every new command moves up to 15 complete 31-word history entries before
   copying the newest one. Once full this is up to 465 copied words plus
   31 for the new line per entry. A ring of 16 cached entries could avoid
   most copying, but affects Ctrl-P/Ctrl-N indexing. Benchmark code/BSS
   effect before accepting it, especially at the 036000 size gate.
2. **Builtin dispatch** (`dsh_exec.c:dsh_builtin`): a long sequence of
   `dsh_s6_eq_text` character-by-character comparisons is executed before
   external command dispatch. A first-six-SIXBIT-character/length test could
   make common rejection paths faster. Do not blindly replace it with a
   resident table: measure both code and data cost, and keep diagnostics.
3. **SIXBIT character extraction** (`dsh_s6.c` and lexer/parser callers):
   repeated get-character calls compute divisions/modulo by six. Test a
   local word/slot cursor where characters are scanned sequentially; avoid
   duplicating a second tokenizer or changing quotation semantics.
4. **AST node clearing** (`dsh_parse.c:dsh_node_clear`): clears all word,
   mask, and redirection fields even for small nodes. Lazy initialization
   could improve speed but may expose stale-state bugs on parser error and
   node reuse. This is **not** a first-choice easy change.

Priority: find at least 0054 more octal image words by similarly small,
measured refactors **before** larger CPU optimizations that might raise
permanent storage. Do not include generated KCC objects in P10FOLD's fold
plan; earlier testing showed that can corrupt foreground execution.

CPU speed changes above are inferred from fewer code paths/calls, not
quantified with a target-cycle benchmark. Preserve existing conformance
coverage and add targeted history/lexer tests for future refactors.
