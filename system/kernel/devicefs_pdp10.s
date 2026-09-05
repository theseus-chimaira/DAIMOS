; devicefs_pdp10.s -- compact resident DEVICEFS primitives.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  devicefs_io_in
        .globl  devicefs_io_out
        .globl  kfmt_u36_decimal_readchar

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
        jrst    devicefs_lookup_out
        move    5,1(2)
        camn    5,[0515700000000]      ; IO
        jrst    devicefs_lookup_io
        came    5,[0515600000000]      ; IN
        jrst    pdp10_ret_neg1
        pushj   17,devicefs_skip_if_in
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020004
        jrst    devicefs_lookup_store

devicefs_lookup_out:
        caie    5,3
        jrst    pdp10_ret_neg1
        move    5,1(2)
        came    5,[0576564000000]      ; OUT
        jrst    pdp10_ret_neg1
        pushj   17,devicefs_skip_if_out
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020005
        jrst    devicefs_lookup_store

devicefs_lookup_io:
        move    5,4
        tlo     5,020002

devicefs_lookup_store:
        movem   5,(3)
        jrst    pdp10_ret_zero


; Store ent name/type and return 1. AC4=ent, AC5=word, AC6=chars, AC7=type.
devicefs_readdir_store:
        movem   6,(4)
        movem   5,1(4)
        setzm   2(4)
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
        move    5,0
        move    4,5
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        move    4,3
        jumpe   2,devicefs_readdir_io
        move    4,0
        pushj   17,devicefs_skip_if_in
        jrst    devicefs_readdir_no_in
        caie    2,1
        jrst    devicefs_readdir_maybe_out2
        move    5,[0515600000000]      ; IN
        movei   6,2
        movei   7,2                    ; regular
        move    4,3
        jrst    devicefs_readdir_store

devicefs_readdir_maybe_out2:
        caie    2,2
        jrst    pdp10_ret_zero
        jrst    devicefs_readdir_out

devicefs_readdir_no_in:
        caie    2,1
        jrst    pdp10_ret_zero

devicefs_readdir_out:
        move    4,0
        pushj   17,devicefs_skip_if_out
        jrst    pdp10_ret_zero
        move    5,[0576564000000]      ; OUT
        movei   6,3
        movei   7,2
        move    4,3
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
        cain    3,020004
        jrst    devicefs_stat_in
        cain    3,020005
        jrst    devicefs_stat_out
        caie    3,020002
        jrst    pdp10_ret_neg1
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

devicefs_stat_dir:
        movei   5,1
        movei   6,0555
        jrst    devicefs_stat_store

devicefs_stat_in:
        pushj   17,devicefs_skip_if_in
        jrst    pdp10_ret_neg1
        jrst    devicefs_stat_counter

devicefs_stat_out:
        pushj   17,devicefs_skip_if_out
        jrst    pdp10_ret_neg1
devicefs_stat_counter:
        movei   5,2
        movei   6,0444

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
        hlrz    5,1
        hrrz    4,1
        pushj   17,devicefs_validate_id
        jumpn   1,pdp10_ret_neg1
        caie    5,020002
        jrst    devicefs_readchar_not_io
        move    1,[-3]                 ; VFS_DEVICE_IO
        popj    17,
devicefs_readchar_not_io:
        cain    5,020004
        jrst    devicefs_readchar_in
        caie    5,020005
        jrst    pdp10_ret_neg1
        pushj   17,devicefs_skip_if_out
        jrst    pdp10_ret_neg1
        move    1,devicefs_io_out(4)
        jrst    devicefs_readchar_tail
devicefs_readchar_in:
        pushj   17,devicefs_skip_if_in
        jrst    pdp10_ret_neg1
        move    1,devicefs_io_in(4)
devicefs_readchar_tail:
        jrst    kfmt_u36_decimal_readchar
