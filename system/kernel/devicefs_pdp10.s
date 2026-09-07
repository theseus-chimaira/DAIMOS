; devicefs_pdp10.s -- compact resident DEVICEFS primitives.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  devicefs_io_in
        .globl  devicefs_io_out
        .globl  devicefs_storage_errors
        .globl  devicefs_mtc_words_read
        .globl  devicefs_mtc_words_written
        .globl  devicefs_d6set_reads
        .globl  devicefs_d6set_writes
        .globl  devicefs_d6set_blocks_read
        .globl  devicefs_d6set_blocks_written
        .globl  devicefs_swap_reads
        .globl  devicefs_swap_writes
        .globl  devicefs_swap_blocks_read
        .globl  devicefs_swap_blocks_written
        .globl  devicefs_swap_errors
        .globl  devicefs_log_reads
        .globl  devicefs_log_writes
        .globl  devicefs_log_errors
        .globl  devicefs_d6set_members

; Full DEVICEFS runtime operations.  MINIT freezes detected device names
; into devicefs_names; a zero slot means that device is absent.
        .globl  devicefs_names

; Capability masks indexed by device id.  TTY0 has output accounting at the
; logical terminal layer in addition to the physical backend accounting.
; IN:  CTY0 PTR0 CR0 DCS0 GE0 WCNSLS OCNSLS DTC0 MTC0 DSK0
; OUT: CTY0 PTP0 CP0 DCS0 GE0 DPY0 TTY0 WCNSLS DTC0 MTC0 DSK0

; Derive 3/4/6-character device name length from trailing SIXBIT blanks.
; input AC5=name word, output AC6=chars.
devicefs_name_length:
        movei   6,3
        trnn    5,0777777
        popj    17,
        movei   6,4
        trnn    5,07777
        popj    17,
        movei   6,6
        popj    17,

; AC4=device id.  Skip next instruction when input accounting exists.
devicefs_skip_if_in:
        movei   5,1
        lsh     5,0(4)
        tdne    5,[076325]
        aos     (17)
        popj    17,

; AC4=device id.  Skip next instruction when output accounting exists.
devicefs_skip_if_out:
        movei   5,1
        lsh     5,0(4)
        tdne    5,[073751]
        aos     (17)
        popj    17,

; Validate AC4 as a present device id.  Return 0/-1 in AC1.
devicefs_validate_id:
        cail    4,021
        jrst    pdp10_ret_neg1
        skipn   5,devicefs_names(4)
        jrst    pdp10_ret_neg1
        jrst    pdp10_ret_zero

; int devicefs_lookup(vnode_t dir, const struct vfs_name *name,
;     vnode_t *nodep)
        .globl  devicefs_lookup
devicefs_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        hlrz    4,1
        caie    4,020001               ; DEVICEFS root
        jrst    devicefs_lookup_dir
        movei   4,0
devicefs_lookup_scan:
        cail    4,021
        jrst    pdp10_ret_neg1
        skipn   5,devicefs_names(4)
        jrst    devicefs_lookup_next
        came    5,1(2)
        jrst    devicefs_lookup_next
        pushj   17,devicefs_name_length
        came    6,(2)
        jrst    devicefs_lookup_next
        move    5,4
        tlo     5,020003               ; device directory
        jrst    devicefs_lookup_store

devicefs_lookup_next:
        addi    4,1
        jrst    devicefs_lookup_scan

devicefs_lookup_dir:
        caie    4,020003
        jrst    pdp10_ret_neg1
        hrrz    4,1
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        move    5,(2)
        caie    5,2
        jrst    devicefs_lookup_stats
        move    5,1(2)
        came    5,[0515700000000]      ; IO
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020002
        jrst    devicefs_lookup_store

devicefs_lookup_stats:
        caie    5,5
        jrst    devicefs_lookup_d6extra
        move    5,1(2)
        came    5,[0636441646300]      ; STATS
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020004
        jrst    devicefs_lookup_store

devicefs_lookup_d6extra:
        caie    4,020                  ; D6SET0 only
        jrst    pdp10_ret_neg1
        caie    5,7
        jrst    devicefs_lookup_swap
        move    5,1(2)
        came    5,[0554555424562]      ; MEMBER
        jrst    pdp10_ret_neg1
        move    5,2(2)
        came    5,[0630000000000]      ; S
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020005
        jrst    devicefs_lookup_store

devicefs_lookup_swap:
        caie    5,4
        jrst    devicefs_lookup_log
        move    5,1(2)
        came    5,[0636741600000]      ; SWAP
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020006
        jrst    devicefs_lookup_store

devicefs_lookup_log:
        caie    5,3
        jrst    pdp10_ret_neg1
        move    5,1(2)
        came    5,[0545747000000]      ; LOG
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020007

devicefs_lookup_store:
        movem   5,(3)
        jrst    pdp10_ret_zero


; Store ent name/type and return 1. AC4=ent, AC5=word, AC6=chars, AC7=type.
devicefs_readdir_store:
        movem   6,(4)
        movem   5,1(4)
        setzm   2(4)
devicefs_readdir_store_tail:
        setzm   3(4)
        setzm   4(4)
        movem   7,5(4)
        movei   1,1
        popj    17,

; Set AC7 to the VFS dirent type for an IO endpoint.  AC0=device id.
devicefs_io_type:
        caige   0,014
        jrst    devicefs_io_type_char
        caile   0,016
        jrst    devicefs_io_type_maybe_mount
        movei   7,4                    ; block
        popj    17,
devicefs_io_type_maybe_mount:
        caie    0,020
        jrst    devicefs_io_type_char
        movei   7,5                    ; mount source
        popj    17,
devicefs_io_type_char:
        movei   7,3                    ; char
        popj    17,

; int devicefs_readdir(vnode_t dir, unsigned int off,
;     struct vfs_dirent *ent)
        .globl  devicefs_readdir
devicefs_readdir:
        jumpe   3,pdp10_ret_neg1
        move    4,3
        hlrz    5,1
        caie    5,020001
        jrst    devicefs_readdir_dir
        movei   5,0
        movei   7,0
devicefs_readdir_scan:
        cail    5,021
        jrst    pdp10_ret_zero
        skipn   6,devicefs_names(5)
        jrst    devicefs_readdir_next
        camn    7,2
        jrst    devicefs_readdir_found
        addi    7,1
devicefs_readdir_next:
        addi    5,1
        jrst    devicefs_readdir_scan

devicefs_readdir_found:
        move    5,6
        pushj   17,devicefs_name_length
        movei   7,1                    ; directory
        jrst    devicefs_readdir_store

devicefs_readdir_dir:
        caie    5,020003
        jrst    pdp10_ret_neg1
        hrrz    0,1
        move    4,0
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        move    4,3
        jumpe   2,devicefs_readdir_io
        caie    2,1
        jrst    devicefs_readdir_d6extra
        move    5,[0636441646300]      ; STATS
        movei   6,5
        movei   7,2                    ; regular
        jrst    devicefs_readdir_store

devicefs_readdir_d6extra:
        caie    0,020                  ; D6SET0 only
        jrst    pdp10_ret_zero
        caie    2,2
        jrst    devicefs_readdir_swap
        movei   6,7
        move    5,[0554555424562]      ; MEMBER
        movem   6,(4)
        movem   5,1(4)
        move    5,[0630000000000]      ; S
        movem   5,2(4)
        movei   7,2
        jrst    devicefs_readdir_store_tail

devicefs_readdir_swap:
        caie    2,3
        jrst    devicefs_readdir_log
        move    5,[0636741600000]      ; SWAP
        movei   6,4
        movei   7,2
        jrst    devicefs_readdir_store

devicefs_readdir_log:
        caie    2,4
        jrst    pdp10_ret_zero
        move    5,[0545747000000]      ; LOG
        movei   6,3
        movei   7,2
        jrst    devicefs_readdir_store

devicefs_readdir_io:
        move    5,[0515700000000]      ; IO
        movei   6,2
        pushj   17,devicefs_io_type
        move    4,3
        jrst    devicefs_readdir_store


; int devicefs_stat(vnode_t node, struct vfs_stat *st)
        .globl  devicefs_stat
devicefs_stat:
        jumpe   2,pdp10_ret_neg1
        hlrz    3,1
        hrrz    4,1
        caie    3,020001
        jrst    devicefs_stat_not_root
        movei   5,1
        movei   6,0555
        jrst    devicefs_stat_store

devicefs_stat_not_root:
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        cain    3,020003
        jrst    devicefs_stat_dir
        caie    3,020002
        jrst    devicefs_stat_file
        caige   4,014
        jrst    devicefs_stat_char
        caile   4,016
        jrst    devicefs_stat_maybe_mount
        movei   5,4
        jrst    devicefs_stat_device_mode
devicefs_stat_maybe_mount:
        caie    4,020
        jrst    devicefs_stat_char
        movei   5,5
        jrst    devicefs_stat_device_mode
devicefs_stat_char:
        movei   5,3
devicefs_stat_device_mode:
        movei   6,0600
        jrst    devicefs_stat_store

devicefs_stat_file:
        caige   3,020004
        jrst    pdp10_ret_neg1
        caile   3,020007
        jrst    pdp10_ret_neg1
        cain    3,020004
        jrst    devicefs_stat_file_ok
        caie    4,020                  ; extra files exist only on D6SET0
        jrst    pdp10_ret_neg1
devicefs_stat_file_ok:
        movei   5,2
        movei   6,0444
        jrst    devicefs_stat_store

devicefs_stat_dir:
        movei   5,1
        movei   6,0555

devicefs_stat_store:
        movem   5,(2)
        movem   6,1(2)
        setzm   2(2)
        setzm   3(2)
        jrst    pdp10_ret_zero


; int devicefs_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  devicefs_readchar
devicefs_readchar:
        jumpe   3,pdp10_ret_neg1
        hlrz    6,1                    ; validate_id clobbers AC5
        hrrz    4,1
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        caie    6,020002
        jrst    devicefs_readchar_not_io
        move    1,[-3]                 ; VFS_DEVICE_IO
        popj    17,

devicefs_readchar_not_io:
        cain    6,020004
        jrst    devicefs_stats_device
        caie    4,020                  ; remaining files are D6SET0 only
        jrst    pdp10_ret_neg1
        cain    6,020005
        jrst    devicefs_members_readchar
        cain    6,020006
        jrst    devicefs_stats_swap
        caie    6,020007
        jrst    pdp10_ret_neg1
        movei   5,2                    ; LOG stats subtype
        jrst    devicefs_stats_readchar

devicefs_stats_swap:
        movei   5,1                    ; SWAP stats subtype
        jrst    devicefs_stats_readchar

devicefs_stats_device:
        setz    5,                     ; ordinary device stats subtype

; Fixed-width statistic lines keep readchar offset handling compact.
; AC4=device id, AC5 subtype (0 device, 1 swap, 2 log).
devicefs_stats_readchar:
        setz    6,                     ; line number
devicefs_stats_line_loop:
        caile   2,022                  ; 023 chars per line
        jrst    devicefs_stats_next_line
        jrst    devicefs_stats_select
devicefs_stats_next_line:
        subi    2,023
        aoja    6,devicefs_stats_line_loop

devicefs_stats_select:
        jumpe   5,devicefs_stats_select_device
        cail    6,5
        jrst    pdp10_ret_zero
        caie    5,1
        jrst    devicefs_stats_log_value
        move    1,@devicefs_swap_stat_ptrs(6)
        jrst    devicefs_stats_sub_label
devicefs_stats_log_value:
        move    1,@devicefs_log_stat_ptrs(6)
devicefs_stats_sub_label:
        move    5,devicefs_sub_stat_labels(6)
        jrst    devicefs_stats_emit

devicefs_stats_select_device:
        cain    6,0
        jrst    devicefs_stats_device_reads
        cain    6,1
        jrst    devicefs_stats_device_writes
        caige   4,014
        jrst    devicefs_stats_device_simple_error
        caile   4,016
        jrst    devicefs_stats_device_maybe_d6
        move    0,4
        subi    0,014
        jrst    devicefs_stats_device_native
devicefs_stats_device_maybe_d6:
        caie    4,020
        jrst    devicefs_stats_device_simple_error
        movei   0,3
devicefs_stats_device_native:
        caie    6,4
        jrst    devicefs_stats_device_native_value
        move    1,devicefs_storage_errors-014(4)
        move    5,[0456262630000]      ; ERRS
        jrst    devicefs_stats_emit
devicefs_stats_device_native_value:
        caige   6,2
        jrst    pdp10_ret_zero
        caile   6,3
        jrst    pdp10_ret_zero
        lsh     0,1                    ; class * 2
        move    5,6
        subi    5,2                    ; read/write selector
        add     5,0
        move    1,@devicefs_native_stat_ptrs(5)
        move    5,devicefs_native_stat_labels(5)
        jrst    devicefs_stats_emit

devicefs_stats_device_simple_error:
        caie    6,2
        jrst    pdp10_ret_zero
        setz    1,
        caige   4,014
        jrst    devicefs_stats_error_label
        move    1,devicefs_storage_errors-014(4)
devicefs_stats_error_label:
        move    5,[0456262630000]      ; ERRS
        jrst    devicefs_stats_emit

devicefs_stats_device_reads:
        cain    4,020
        jrst    devicefs_stats_d6_reads
        setz    1,
        pushj   17,devicefs_skip_if_in
        jrst    devicefs_stats_read_label
        move    1,devicefs_io_in(4)
devicefs_stats_read_label:
        move    5,[0624541440000]      ; READ
        jrst    devicefs_stats_emit
devicefs_stats_d6_reads:
        move    1,devicefs_d6set_reads
        jrst    devicefs_stats_read_label

devicefs_stats_device_writes:
        cain    4,020
        jrst    devicefs_stats_d6_writes
        setz    1,
        pushj   17,devicefs_skip_if_out
        jrst    devicefs_stats_write_label
        move    1,devicefs_io_out(4)
devicefs_stats_write_label:
        move    5,[0676251640000]      ; WRIT
        jrst    devicefs_stats_emit
devicefs_stats_d6_writes:
        move    1,devicefs_d6set_writes
        jrst    devicefs_stats_write_label

; Subsystem stats share the same five line labels.  LOG block counts alias
; its one-block READ/WRITE requests, avoiding two extra resident counters.
devicefs_sub_stat_labels:
        .word   0624541440000          ; READ
        .word   0676251640000          ; WRIT
        .word   0425453620000          ; BLKR
        .word   0425453670000          ; BLKW
        .word   0456262630000          ; ERRS
devicefs_swap_stat_ptrs:
        .word   devicefs_swap_reads
        .word   devicefs_swap_writes
        .word   devicefs_swap_blocks_read
        .word   devicefs_swap_blocks_written
        .word   devicefs_swap_errors
devicefs_log_stat_ptrs:
        .word   devicefs_log_reads
        .word   devicefs_log_writes
        .word   devicefs_log_reads
        .word   devicefs_log_writes
        .word   devicefs_log_errors

; Native transfer units by class: DTC blocks, MTC words, DSK sectors,
; D6SET blocks.  DTC/DSK reuse request counters because one request is one
; native block/sector; only MTC and aggregate D6SET need separate volume.
devicefs_native_stat_ptrs:
        .word   devicefs_io_in+014
        .word   devicefs_io_out+014
        .word   devicefs_mtc_words_read
        .word   devicefs_mtc_words_written
        .word   devicefs_io_in+016
        .word   devicefs_io_out+016
        .word   devicefs_d6set_blocks_read
        .word   devicefs_d6set_blocks_written
devicefs_native_stat_labels:
        .word   0425453620000          ; BLKR
        .word   0425453670000          ; BLKW
        .word   0676244620000          ; WRDR
        .word   0676244670000          ; WRDW
        .word   0634543620000          ; SECR
        .word   0634543670000          ; SECW
        .word   0425453620000          ; BLKR
        .word   0425453670000          ; BLKW

; Emit one fixed-width line: LABEL4 SP 12-octal-digits CR LF = 023 chars.
; Native octal formatting is deliberately used here: it avoids a second
; decimal formatter in KCORE and makes every line fixed-width.
devicefs_stats_emit:
        caile   2,3
        jrst    devicefs_stats_after_label
        move    6,2
        imuli   6,6
        subi    6,036
        move    0,5
        lsh     0,0(6)
        andi    0,077
        addi    0,040
        jrst    devicefs_stats_store
devicefs_stats_after_label:
        caie    2,4
        jrst    devicefs_stats_octal
        movei   0,040
        jrst    devicefs_stats_store
devicefs_stats_octal:
        caile   2,020                  ; columns 5..16 are 12 octal digits
        jrst    devicefs_stats_eol
        move    6,2
        subi    6,5
        imuli   6,-3
        addi    6,041                  ; bit shift 33..0
        move    0,1
        lsh     0,0(6)
        andi    0,7
        addi    0,060
        jrst    devicefs_stats_store
devicefs_stats_eol:
        caie    2,021
        jrst    devicefs_stats_lf
        movei   0,015
        jrst    devicefs_stats_store
devicefs_stats_lf:
        caie    2,022
        jrst    pdp10_ret_zero
        movei   0,012
devicefs_stats_store:
        movem   0,(3)
        movei   1,1
        popj    17,

; D6SET MEMBERS is one fixed 8-character line per configured member:
; DSK0:<unit> CR LF.  Membership is frozen by DEVICEFS MINIT in one word.
devicefs_members_readchar:
        move    6,2
        lsh     6,-3                   ; member index = off / 8
        move    5,devicefs_d6set_members
        move    0,5
        andi    0,7                    ; member count
        caml    6,0
        jrst    pdp10_ret_zero
        andi    2,7                    ; column = off % 8
        caile   2,3
        jrst    devicefs_members_tail
        move    0,[0446353200000]      ; DSK0
        move    5,2
        imuli   5,6
        subi    5,036
        lsh     0,0(5)
        andi    0,077
        addi    0,040
        jrst    devicefs_members_store
devicefs_members_tail:
        caie    2,4
        jrst    devicefs_members_unit
        movei   0,072                  ; ':'
        jrst    devicefs_members_store
devicefs_members_unit:
        caie    2,5
        jrst    devicefs_members_cr
        move    5,6
        imuli   5,3
        addi    5,3
        movns   5
        move    6,devicefs_d6set_members
        lsh     6,0(5)
        andi    6,7
        movei   0,060(6)
        jrst    devicefs_members_store
devicefs_members_cr:
        caie    2,6
        jrst    devicefs_members_lf
        movei   0,015
        jrst    devicefs_members_store
devicefs_members_lf:
        movei   0,012
devicefs_members_store:
        movem   0,(3)
        movei   1,1
        popj    17,
