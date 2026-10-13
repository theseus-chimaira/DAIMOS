; monitorfs_dispatch_v14.s -- staged VFS-to-MonitorFS MRES entrypoints.
;
; NOT linked into KCORE yet. Once the MonitorFS MRES is installed, MINIT
; must patch each one-word JRST to its relocated implementation address.
; The package must remain pinned for as long as MonitorFS is reachable.
;
; Every entry has the same register/stack calling convention as the old
; resident routine. Before installation, every entry safely fails.

        .text
        .globl  kret_neg1

        .globl  mfsdev_lookup
        .globl  mfsdev_readdir
        .globl  mfsdev_stat
        .globl  mfsdev_readchar
        .globl  mfsproc_lookup
        .globl  mfsproc_readdir
        .globl  mfsproc_stat
        .globl  mfsproc_read_words
        .globl  mfsproc_format_slot

; These labels are also the writable jump words patched by MINIT.
mfsdev_lookup:
        jrst    kret_neg1
mfsdev_readdir:
        jrst    kret_neg1
mfsdev_stat:
        jrst    kret_neg1
mfsdev_readchar:
        jrst    kret_neg1
mfsproc_lookup:
        jrst    kret_neg1
mfsproc_readdir:
        jrst    kret_neg1
mfsproc_stat:
        jrst    kret_neg1
mfsproc_read_words:
        jrst    kret_neg1
mfsproc_format_slot:
        jrst    kret_neg1
