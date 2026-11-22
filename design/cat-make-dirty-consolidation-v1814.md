# CAT/MAKE dirty-worktree source consolidation v1814

Date: 2026-11-22. Source: retained dirty worktree
`~/.devspace/worktrees/DAIMOS-a3c86a4c` on the older
`daimos-native-selfhost-v1414-20261109` branch. This promotion
imports only the userland memory-saving CAT and MAKE changes.

## CAT correctness and resident code cost

The historical CAT patch removed its 259-byte automatic line array
but still wrote long records through `u_text_sink`, which has a
258-character capacity. The original patch therefore could not
correctly emit long records. The accepted source correction keeps
one `u_text_reader` (16 words) and splits output by destination:

- For files/pipes, forward S6REC header and payload verbatim, in
  chunks already in the reader buffer, through `u_write_words_all`.
- For a controlling terminal, decode the source SIXBIT characters and
  use WRITECHAR, since TTY WRITE_WORDS requires a complete S6REC
  record in a single call and cannot stream partial records.
- Do not create a second sink in CAT; `standalone.c` handles the normal
  redirected-output sink, which remains empty on the raw-word path.

The CAT-only `libc/cat_stream_v1814.c` is an independent member of
`libcommand.darc`, extracted only for CAT. Embedding it in shared
`libc/text.c` would have increased shared object TEXT from 621 to
712 words (+91), an unwanted cost to unrelated userland programs.
The final shared `libc/text.c` source is unchanged. The separate CAT
module occupies 201 TEXT words and adds no BSS. No line-sized buffer
or fixed KCORE allocation was added.

The source-linked host regression exercises empty, 7, 258, 259 and
1000-character S6REC records with deliberately short word reads and
writes, byte-identical redirected output, 1000-character TTY output,
EOF, invalid types and truncated records. All pass. A native CAT DXR
was successfully linked from the current KCC/DAS and host DLINK.

## MAKE RAM reduction

MAKE formerly allocated `goals[16][104]` in `main`, including unused
goal slots. It now stores stable string-arena offsets in a 16-entry
automatic array and reuses its argument buffer for the optional `-J`
count. The default rule already exists in the string arena, avoiding
another copy. All offsets survive parser arena growth and shrink.

The v1814 testkit compares original source at DAIMOS `dbdc403f`
against the promoted source using the actual KCC and DAS:

| MAKE main KCC frame | Octal | Decimal words |
|---|---:|---:|
| Source before promotion | 01113 | 587 |
| v1814 source | 0172 | 122 |
| Saving | 0721 | 465 |

Native MAKE DOBJ TEXT is 7876 -> 7748 words (-128); BSS remains 839
words. The production MAKE DXR linked successfully. The goal strings
use the pre-existing arena rather than a new static array.

## Explicit acceptance limit and preserved history

The source-linked host CAT and native KCC/DAS/MAKE frame tests PASS.
An isolated current-source MAKE guest test was attempted using
`daimos-native-make-common-simh-20261011-v1.sh`. Its fresh boot-image
build stopped before simulator runtime because the DSH executable
measures `036133` octal, exceeding the configured `036000` limit by
`0133` octal (91 decimal words). The test is NOT a guest PASS.
The source-linked CAT-only archive member is not part of DSH; its
addition cannot explain this DSH limit. Do not raise the EXEC limit
as a workaround. Investigate DSH source and folding against its actual
build inputs before making a boot-policy change.

The full historical v1413/v1414 self-hosting boot Makefile, optional
staging script and native build Makefiles remain deliberately unmerged.
In particular, the old DSH limit increase from 036000 to 037000 is
not accepted. The untouched dirty source worktree and v1810 archival
snapshot remain available for follow-up review.
