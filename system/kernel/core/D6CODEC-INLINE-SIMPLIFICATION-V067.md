# D6 inline v067: redundant literal guard removal

The v066 inline fixed-address decoder checks output remaining before its
first token and branches to completion after each token that consumes the
last output word. The extra literal-only zero-remaining check is therefore
unreachable for a legal entry/resumption. V067 removes that single check;
all other guards, descriptor dispatch, and match-copy operations are unchanged.

Measured native PDP-6 image: v066 88 words, v067 87 words, both zero BSS
and decoder stack. With the unchanged 56-word VFS, v067 costs 143 words,
+47 over original D6LZ36 decoder plus VFS (96). No production format switch.

The daimos-testkit v067 valid, address-bound, malformed-descriptor and VFS
native tests passed. These tests do not establish input/output alias safety,
exact trailing-data rejection across all interfaces, or an 8K KCORE link.
The single-word saving is not sufficient to address the architectural budget.
