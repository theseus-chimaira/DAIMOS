; D6FS FCB decode/validation for PDP-6/PDP-10.

        .text
; Validate and decode one D6FS FCB in the sole resident implementation.
        .globl  d6fs_fcb_decode_valid
; int d6fs_fcb_decode_valid(fcb, fs_blocks, fcb_count, info)
d6fs_fcb_decode_valid:
        push    17,010
        push    17,011                   ; loop index; preserve C ABI
        move    5,(1)                   ; META
        move    6,5
        lsh     6,-041
        movem   6,0(4)                  ; type
        move    7,5
        lsh     7,-030
        andi    7,0777
        movem   7,1(4)                  ; flags
        move    7,5
        lsh     7,-014
        andi    7,07777
        movem   7,2(4)                  ; mode
        move    7,5
        lsh     7,-010
        andi    7,017
        movem   7,3(4)                  ; tail
        move    7,5
        lsh     7,-4
        andi    7,017
        movem   7,4(4)                  ; extent_count
        move    7,1(1)
        hlrzm   7,5(4)                  ; uid
        hrrzm   7,6(4)                  ; gid
        move    7,2(1)
        movem   7,7(4)                  ; size_words
        move    7,3(1)
        movem   7,010(4)                ; mtime
        move    7,4(1)
        hlrzm   7,011(4)                ; parent_fcb

        caile   6,4                     ; highest defined type is FIFO
        jrst    d6fs_fcb_invalid
        move    7,4(4)
        caile   7,7                     ; at most seven extents
        jrst    d6fs_fcb_invalid
        skipn   7,3(4)                  ; former tail field is reserved zero
        jrst    d6fs_fcb_reserved
        jrst    d6fs_fcb_invalid

        ; Reserved representation bits and words must remain zero.
d6fs_fcb_reserved:
        move    7,(1)
        andi    7,017
        hrrz    5,4(1)
        ior     7,5
        ior     7,015(1)
        ior     7,016(1)
        ior     7,017(1)
        jumpn   7,d6fs_fcb_invalid

        ; FREE FCBs and FIFO nodes carry no extents and no data size.
        jumpn   6,d6fs_fcb_live
d6fs_fcb_nodata:
        skipe   4(4)
        jrst    d6fs_fcb_invalid
        skipe   7(4)
        jrst    d6fs_fcb_invalid
        movei   1,1
        jrst    d6fs_fcb_done

        ; The superblock caps parent indices and extent block numbers to
        ; positive ranges, so ordinary signed compares are sufficient here.
d6fs_fcb_live:
        move    7,011(4)
        caml    7,3
        jrst    d6fs_fcb_invalid
        cain    6,4                     ; FIFO has no payload allocation
        jrst    d6fs_fcb_nodata
        move    3,5(1)                  ; packed 5-bit extent highs
        addi    1,6                     ; extent-word cursor
        movei   011,0                   ; extent index
        movei   010,0                   ; accumulated capacity in words

d6fs_fcb_extent_loop:
        move    7,3
        andi    7,037                   ; high length bits
        lsh     3,-5
        move    6,(1)                   ; encoded extent
        addi    1,1
        caml    011,4(4)
        jrst    d6fs_fcb_unused_extent

        lsh     7,014
        ; low twelve bits plus packed high length, then encoded +1.
        move    5,6
        andi    5,07777
        ior     7,5
        addi    7,1
        lsh     6,-014                  ; start block
        add     6,7
        camle   6,2
        jrst    d6fs_fcb_invalid
        lsh     7,7                     ; blocks * 128 words
        add     010,7
        jrst    d6fs_fcb_extent_next

d6fs_fcb_unused_extent:
        jumpn   6,d6fs_fcb_invalid
        jumpn   7,d6fs_fcb_invalid

d6fs_fcb_extent_next:
        addi    011,1
        caige   011,7
        jrst    d6fs_fcb_extent_loop
        move    7,7(4)
        jumpl   7,d6fs_fcb_invalid      ; capacity is always non-negative
        camle   7,010
        jrst    d6fs_fcb_invalid
        movei   1,1
        jrst    d6fs_fcb_done

d6fs_fcb_invalid:
        setz    1,

d6fs_fcb_done:
        pop     17,011
        pop     17,010
        popj    17,
