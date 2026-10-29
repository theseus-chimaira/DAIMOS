# KINIT 256-word bootstrap stack reserve v652

The KINIT bootstrap stack reserve is reduced from octal `0600`
(384 decimal words) to octal `0400` (256 words). The default is
consistent across the PDP-6 boot Makefile, standalone kernel Makefile,
and KINIT header. Existing explicit build overrides remain supported.

Previously accepted native fatal-path watermark tests v640 and
v648–v651 measured at most octal `0172` (122 decimal words) for root
selection, D6FS mount validation, idle-stack MM allocation and
BLOCKSET read errors. The new reserve retains 134 words of margin
over the highest **measured** stack use; unmeasured paths are not
claimed to have been exhaustively bounded.

Fresh GCC 32K lowmem tape-root image with the new `0400` reserve and
watermark enabled built and reached `proc_boot_init` at octal `061437`
in native PDP-6 SIMH. Image end `077260` plus reserve `0400` gives
`077660`, below the 32K core boundary `0100000`. A native invalid-root
HALT on that image recorded octal `0164` (116 decimal words).

No new full-profile replays were required by the reserve change.
No other production boot logic or permanent KCORE residency changed.
