; devicefs_pdp10.s -- compact resident DEVICEFS primitives.
        .text
        .globl  pdp10_ret_busy
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  devicefs_io_in
        .globl  devicefs_io_out
        .globl  devicefs_storage_errors
        .globl  devicefs_mtc_words_read
        .globl  devicefs_mtc_words_written
        .globl  devicefs_drm_reads
        .globl  devicefs_drm_writes
        .globl  devicefs_d6set_reads
        .globl  devicefs_d6set_writes
        .globl  devicefs_d6set_blocks_read
        .globl  devicefs_d6set_blocks_written
        .globl  devicefs_log_reads
        .globl  devicefs_log_writes
        .globl  devicefs_log_blocks_read
        .globl  devicefs_log_blocks_written
        .globl  devicefs_log_errors
        .globl  devicefs_d6set_members
        .globl  proc_swap_blocks_used
        .globl  diskset_runtime_reg_enter

; Full DEVICEFS runtime operations.  MINIT freezes detected device names
; into devicefs_names; a zero slot means that device is absent.
        .globl  devicefs_names

; Capability masks indexed by device id.  TTY0 has output accounting at the
; logical terminal layer in addition to the physical backend accounting.
; IN:  CTY0 PTR0 CR0 DCS0 GE0 WCNSLS OCNSLS DTC0 MTC0 DSK0 DRM0
; OUT: CTY0 PTP0 CP0 DCS0 GE0 DPY0 TTY0 WCNSLS DTC0 MTC0 DSK0 DRM0

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

; Validate AC4 as a present device id.  Return 0/-1 in AC1.
devicefs_validate_id:
        cail    4,022
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
        cail    4,022
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
        jrst    pdp10_ret_one

; Set AC7 to the VFS dirent type for an IO endpoint.  AC0=device id.
devicefs_io_type:
        caige   0,014
        jrst    devicefs_io_type_char
        caile   0,016
        jrst    devicefs_io_type_maybe_mount
        movei   7,4                    ; block
        popj    17,
devicefs_io_type_maybe_mount:
        cain    0,021                  ; DRM0
        jrst    devicefs_io_type_block
        caie    0,020
        jrst    devicefs_io_type_char
        movei   7,5                    ; mount source
        popj    17,
devicefs_io_type_block:
        movei   7,4                    ; block
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
        cail    5,022
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
        cain    4,021                  ; DRM0
        jrst    devicefs_stat_block
        caie    4,020
        jrst    devicefs_stat_char
        movei   5,5
        jrst    devicefs_stat_device_mode
devicefs_stat_block:
        movei   5,4
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
        cain    6,020002
        jrst    pdp10_ret_busy          ; VFS_DEVICE_IO

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
        movei   5,devicefs_log_reads
        jrst    devicefs_stats_readchar

devicefs_stats_swap:
        ; SWAP reports live allocation state, not lifetime I/O accounting.
        ; Lines are TOTAL, USED, FREE blocks.
        setz    6,
devicefs_swap_line_loop:
        caig    2,015
        jrst    devicefs_swap_select
        subi    2,016
        aoja    6,devicefs_swap_line_loop
devicefs_swap_select:
        cail    6,3
        jrst    pdp10_ret_zero
        movei   5,6                   ; DISKSET_MRES_OP_SWAP_BLOCKS
        pushj   17,diskset_runtime_reg_enter
        jumpe   6,devicefs_stats_emit
        cain    6,1
        jrst    devicefs_swap_used
        sub     1,proc_swap_blocks_used
        jrst    devicefs_stats_emit
devicefs_swap_used:
        move    1,proc_swap_blocks_used
        jrst    devicefs_stats_emit

devicefs_stats_device:
        setz    5,                     ; zero means ordinary device stats

; Fixed-width bare octal values keep readchar offset handling compact.
; AC4=device id, AC5=0 for device stats or base of five subsystem counters.
devicefs_stats_readchar:
        setz    6,                     ; line number
devicefs_stats_line_loop:
        caig    2,015                  ; select once offset is inside a line
        jrst    devicefs_stats_select
        subi    2,016
        aoja    6,devicefs_stats_line_loop

devicefs_stats_select:
        jumpe   5,devicefs_stats_select_device
        cail    6,5
        jrst    pdp10_ret_zero
        add     5,6
        move    1,(5)
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
        cain    4,021                  ; DRM0 is one native block/request
        jrst    devicefs_stats_device_drm
        caie    4,020
        jrst    devicefs_stats_device_simple_error
        movei   0,3
devicefs_stats_device_drm_done:
        jrst    devicefs_stats_device_native
devicefs_stats_device_drm:
        movei   0,4
        jrst    devicefs_stats_device_drm_done
devicefs_stats_device_native:
        caie    6,4
        jrst    devicefs_stats_device_native_value
        move    1,devicefs_storage_errors-014(4)
        jrst    devicefs_stats_emit
devicefs_stats_device_native_value:
        caige   6,2
        jrst    pdp10_ret_zero
        caile   6,3
        jrst    pdp10_ret_zero
        move    5,6
        subi    5,2                    ; read/write selector
        cain    0,1                    ; MTC has separate word volume
        jrst    devicefs_stats_native_mtc
        cain    0,3                    ; D6SET has aggregate block volume
        jrst    devicefs_stats_native_d6
        cain    0,4                    ; DRM has sparse block counters
        jrst    devicefs_stats_native_drm
        ; DTC and DSK are one native unit per request.
        jumpe   5,devicefs_stats_device_reads
        jrst    devicefs_stats_device_writes
devicefs_stats_native_mtc:
        move    1,devicefs_mtc_words_read(5)
        jrst    devicefs_stats_emit
devicefs_stats_native_d6:
        move    1,devicefs_d6set_blocks_read(5)
        jrst    devicefs_stats_emit
devicefs_stats_native_drm:
        jumpe   5,devicefs_stats_drm_reads
        move    1,devicefs_drm_writes
        jrst    devicefs_stats_emit

devicefs_stats_device_simple_error:
        caie    6,2
        jrst    pdp10_ret_zero
        setz    1,
        caige   4,014
        jrst    devicefs_stats_emit
        move    1,devicefs_storage_errors-014(4)
        jrst    devicefs_stats_emit

devicefs_stats_device_reads:
        cain    4,021
        jrst    devicefs_stats_drm_reads
        cain    4,020
        jrst    devicefs_stats_d6_reads
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[076325]
        jrst    devicefs_stats_zero
        move    1,devicefs_io_in(4)
        jrst    devicefs_stats_emit
devicefs_stats_drm_reads:
        move    1,devicefs_drm_reads
        jrst    devicefs_stats_emit
devicefs_stats_d6_reads:
        move    1,devicefs_d6set_reads
        jrst    devicefs_stats_emit

devicefs_stats_device_writes:
        cain    4,021
        jrst    devicefs_stats_drm_writes
        cain    4,020
        jrst    devicefs_stats_d6_writes
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[073751]
        jrst    devicefs_stats_zero
        move    1,devicefs_io_out(4)
        jrst    devicefs_stats_emit
devicefs_stats_drm_writes:
        move    1,devicefs_drm_writes
        jrst    devicefs_stats_emit
devicefs_stats_d6_writes:
        move    1,devicefs_d6set_writes
        jrst    devicefs_stats_emit

devicefs_stats_zero:
        setz    1,

; Emit one fixed-width bare value: 12 octal digits CR LF = 016 chars.
; Native octal formatting avoids a decimal formatter in resident KCORE.
devicefs_stats_emit:
        caile   2,013                  ; columns 0..11 are octal digits
        jrst    devicefs_stats_eol
        move    6,2
        imuli   6,-3
        addi    6,041                  ; bit shift 33..0
        move    0,1
        lsh     0,0(6)
        andi    0,7
        addi    0,060
        jrst    devicefs_stats_store
devicefs_stats_eol:
        movei   0,015                  ; normalized offset is either CR or LF
        caie    2,014
        movei   0,012
devicefs_stats_store:
        movem   0,(3)
        jrst    pdp10_ret_one

; D6SET MEMBERS is one fixed four-character line per configured member:
; two octal unit digits plus CR LF.  D6SET currently contains only DSK units,
; so repeating the DSK prefix in every line would waste resident formatter code.
devicefs_members_readchar:
        move    6,2
        lsh     6,-2                   ; member index = off / 4
        move    5,devicefs_d6set_members
        move    0,5
        andi    0,7                    ; member count
        caml    6,0
        jrst    pdp10_ret_zero
        andi    2,3                    ; column = off % 4
        jumpe   2,devicefs_members_zero
        caie    2,1
        jrst    devicefs_members_eol
        imuli   6,-3
        subi    6,3
        lsh     5,0(6)
        andi    5,7
        movei   0,060(5)
        jrst    devicefs_members_store
devicefs_members_zero:
        movei   0,060
        jrst    devicefs_members_store
devicefs_members_eol:
        movei   0,015
        caie    2,2
        movei   0,012
devicefs_members_store:
        jrst    devicefs_stats_store
