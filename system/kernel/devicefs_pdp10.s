; devicefs_pdp10.s -- compact resident DEVICEFS primitives.
        .text

; Full DEVICEFS runtime operations.  Names remain packed SIXBIT words.
        .data
devicefs_v1_names:
        .word   0436471200000          ; CTY0
        .word   0435453200000          ; CLK0
        .word   0606462200000          ; PTR0
        .word   0606460200000          ; PTP0
        .word   0436220000000          ; CR0
        .word   0436020000000          ; CP0
        .word   0444363200000          ; DCS0
        .word   0474520000000          ; GE0
        .word   0446071200000          ; DPY0
        .word   0646471200000          ; TTY0
        .word   0674356635463          ; WCNSLS
        .word   0574356635463          ; OCNSLS
        .word   0446443200000          ; DTC0
        .word   0556443200000          ; MTC0
        .word   0446353200000          ; DSK0
        .word   0635466200000          ; SLV0
        .word   0442663456420          ; D6SET0
        .text

; Derive 3/4/6-character device name length from trailing SIXBIT blanks.
; input AC5=name word, output AC6=chars.
devicefs_name_length:
        move    6,5
        andi    6,0777777
        jumpe   6,devicefs_name_len3
        move    6,5
        andi    6,07777
        jumpe   6,devicefs_name_len4
        movei   6,6
        popj    17,
devicefs_name_len3:
        movei   6,3
        popj    17,
devicefs_name_len4:
        movei   6,4
        popj    17,

; int devicefs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
;     vnode_v1_t *nodep)
        .globl  devicefs_v1_lookup
devicefs_v1_lookup:
        jumpe   2,devicefs_lookup_fail
        jumpe   3,devicefs_lookup_fail
        camn    1,[020003000000]       ; /DEVICE/CTY0 directory
        jrst    devicefs_lookup_cty
        hlrz    4,1
        caie    4,020001               ; DEVICEFS root
        jrst    devicefs_lookup_fail
        movei   4,0
devicefs_lookup_scan:
        cail    4,021                  ; 17 devices
        jrst    devicefs_lookup_fail
        movei   5,1
        lsh     5,0(4)
        tdnn    5,devicefs_v1_present
        jrst    devicefs_lookup_next
        move    5,devicefs_v1_names(4)
        pushj   17,devicefs_name_length
        came    6,(2)
        jrst    devicefs_lookup_next
        came    5,1(2)
        jrst    devicefs_lookup_next
        move    5,4
        jumpe   4,devicefs_lookup_ctydir_node
        tlo     5,020002               ; normal device
        jrst    devicefs_lookup_store
devicefs_lookup_ctydir_node:
        tlo     5,020003
devicefs_lookup_store:
        movem   5,(3)
        movei   1,0
        popj    17,
devicefs_lookup_next:
        addi    4,1
        jrst    devicefs_lookup_scan

devicefs_lookup_cty:
        move    5,devicefs_v1_present
        trnn    5,1
        jrst    devicefs_lookup_fail
        move    4,(2)
        caie    4,2
        jrst    devicefs_lookup_cty_out
        move    4,1(2)
        camn    4,[0515700000000]      ; IO
        jrst    devicefs_lookup_io
        camn    4,[0515600000000]      ; IN
        jrst    devicefs_lookup_in
        jrst    devicefs_lookup_fail
devicefs_lookup_cty_out:
        caie    4,3
        jrst    devicefs_lookup_fail
        move    4,1(2)
        came    4,[0576564000000]      ; OUT
        jrst    devicefs_lookup_fail
        move    4,[020005000000]
        jrst    devicefs_lookup_cty_store
devicefs_lookup_io:
        move    4,[020002000000]
        jrst    devicefs_lookup_cty_store
devicefs_lookup_in:
        move    4,[020004000000]
devicefs_lookup_cty_store:
        movem   4,(3)
        movei   1,0
        popj    17,
devicefs_lookup_fail:
        seto    1,
        popj    17,

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

; int devicefs_v1_readdir(vnode_v1_t dir, unsigned int off,
;     struct vfs_v1_dirent *ent)
        .globl  devicefs_v1_readdir
devicefs_v1_readdir:
        jumpe   3,devicefs_readdir_fail
        move    4,3                     ; ent
        camn    1,[020003000000]
        jrst    devicefs_readdir_cty
        hlrz    5,1
        caie    5,020001
        jrst    devicefs_readdir_fail
        movei   5,0                     ; id
        movei   7,0                     ; visible ordinal
devicefs_readdir_scan:
        cail    5,021
        jrst    devicefs_readdir_eof
        movei   6,1
        lsh     6,0(5)
        tdnn    6,devicefs_v1_present
        jrst    devicefs_readdir_next
        camn    7,2
        jrst    devicefs_readdir_found
        addi    7,1
devicefs_readdir_next:
        addi    5,1
        jrst    devicefs_readdir_scan
devicefs_readdir_found:
        move    0,5                     ; preserve id in AC0
        move    5,devicefs_v1_names(5)
        pushj   17,devicefs_name_length
        jumpe   0,devicefs_readdir_root_cty_type
        caige   0,014
        jrst    devicefs_readdir_char_type
        caile   0,016
        jrst    devicefs_readdir_maybe_mount
        movei   7,4                    ; block
        jrst    devicefs_readdir_store
devicefs_readdir_maybe_mount:
        caie    0,020
        jrst    devicefs_readdir_char_type
        movei   7,5                    ; mount source
        jrst    devicefs_readdir_store
devicefs_readdir_char_type:
        movei   7,3
        jrst    devicefs_readdir_store
devicefs_readdir_root_cty_type:
        movei   7,1                    ; directory
        jrst    devicefs_readdir_store

devicefs_readdir_cty:
        move    5,devicefs_v1_present
        trnn    5,1
        jrst    devicefs_readdir_fail
        cail    2,3
        jrst    devicefs_readdir_eof
        jumpe   2,devicefs_readdir_cty_io
        caie    2,1
        jrst    devicefs_readdir_cty_out
        move    5,[0515600000000]      ; IN
        movei   6,2
        movei   7,2                    ; regular
        jrst    devicefs_readdir_store
devicefs_readdir_cty_io:
        move    5,[0515700000000]      ; IO
        movei   6,2
        movei   7,3                    ; char
        jrst    devicefs_readdir_store
devicefs_readdir_cty_out:
        move    5,[0576564000000]      ; OUT
        movei   6,3
        movei   7,2
        jrst    devicefs_readdir_store
devicefs_readdir_eof:
        movei   1,0
        popj    17,
devicefs_readdir_fail:
        seto    1,
        popj    17,

; int devicefs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st)
        .globl  devicefs_v1_stat
devicefs_v1_stat:
        jumpe   2,devicefs_stat_fail
        hlrz    3,1
        hrrz    4,1
        caie    3,020001
        jrst    devicefs_stat_not_root
        movei   5,1                    ; dir
        movei   6,0555
        jrst    devicefs_stat_store
devicefs_stat_not_root:
        cain    3,020003               ; CTY directory
        jrst    devicefs_stat_ctydir
        cain    3,020004
        jrst    devicefs_stat_ctyfile
        cain    3,020005
        jrst    devicefs_stat_ctyfile
        caie    3,020002
        jrst    devicefs_stat_fail
        cail    4,021
        jrst    devicefs_stat_fail
        movei   5,1
        lsh     5,0(4)
        tdnn    5,devicefs_v1_present
        jrst    devicefs_stat_fail
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
devicefs_stat_ctydir:
        jumpn   4,devicefs_stat_fail
        move    5,devicefs_v1_present
        trnn    5,1
        jrst    devicefs_stat_fail
        movei   5,1
        movei   6,0555
        jrst    devicefs_stat_store
devicefs_stat_ctyfile:
        jumpn   4,devicefs_stat_fail
        move    5,devicefs_v1_present
        trnn    5,1
        jrst    devicefs_stat_fail
        movei   5,2
        movei   6,0444
devicefs_stat_store:
        movem   5,(2)
        movem   6,1(2)
        setzm   2(2)
        setzm   3(2)
        movei   1,0
        popj    17,
devicefs_stat_fail:
        seto    1,
        popj    17,

; int devicefs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp)
        .globl  devicefs_v1_readchar
devicefs_v1_readchar:
        jumpe   3,devicefs_readchar_fail
        hrrz    4,1
        jumpn   4,devicefs_readchar_fail
        move    5,devicefs_v1_present
        trnn    5,1
        jrst    devicefs_readchar_fail
        hlrz    4,1
        caie    4,020002
        jrst    devicefs_readchar_not_device
        move    1,[-3]
        popj    17,
devicefs_readchar_not_device:
        cain    4,020004
        jrst    devicefs_readchar_in
        caie    4,020005
        jrst    devicefs_readchar_fail
        move    1,devicefs_v1_io_out
        jrst    devicefs_readchar_tail
devicefs_readchar_in:
        move    1,devicefs_v1_io_in
devicefs_readchar_tail:
        ; AC2 already holds off, AC3 already holds chp.
        jrst    kfmt_u36_decimal_readchar
devicefs_readchar_fail:
        seto    1,
        popj    17,
