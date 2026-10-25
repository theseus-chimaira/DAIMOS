; monitorfs_dispatch_v14.s -- staged VFS-to-MonitorFS MRES entrypoints.
;
; Linked into KCORE as ABI jump slots only; the complete MonitorFS runtime
; and swapped-process metadata reader reside in the always-loaded MRES.
; MINIT patches each one-word JRST to its relocated implementation address.
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
        .globl  mfsdev_lookup_entry_v16
mfsdev_lookup_entry_v16:
mfsdev_lookup:
        jrst    kret_neg1
        .globl  mfsdev_readdir_entry_v16
mfsdev_readdir_entry_v16:
mfsdev_readdir:
        jrst    kret_neg1
        .globl  mfsdev_stat_entry_v16
mfsdev_stat_entry_v16:
mfsdev_stat:
        jrst    kret_neg1
        .globl  mfsdev_readchar_entry_v16
mfsdev_readchar_entry_v16:
mfsdev_readchar:
        jrst    kret_neg1
        .globl  mfsproc_lookup_entry_v16
mfsproc_lookup_entry_v16:
mfsproc_lookup:
        jrst    kret_neg1
        .globl  mfsproc_readdir_entry_v16
mfsproc_readdir_entry_v16:
mfsproc_readdir:
        jrst    kret_neg1
        .globl  mfsproc_stat_entry_v16
mfsproc_stat_entry_v16:
mfsproc_stat:
        jrst    kret_neg1
        .globl  mfsproc_read_words_entry_v16
mfsproc_read_words_entry_v16:
mfsproc_read_words:
        jrst    kret_neg1
        .globl  mfsproc_format_slot_entry_v16
mfsproc_format_slot_entry_v16:
mfsproc_format_slot:
        jrst    kret_neg1
