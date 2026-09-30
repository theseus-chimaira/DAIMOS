; blockset_map.s -- reentrant generic BLOCKSET descriptor mapper.
;
; This is deliberately separate from the root BLOCKSET MRES.  D6FS roots use
; homogeneous equal-size INTERLEAVE and do not need CONCAT code resident.
; Consumers that need arbitrary descriptors (notably future TSFS extent I/O)
; link this object with their own provider.

        .text
        .globl blockset_map_descriptor
        .globl kret_neg1

; Reentrant mapper.
;   AC1 = packed descriptor pointer
;   AC2 = logical block
; Returns AC1=physical unit, AC2=physical block, or AC1=-1.
; The descriptor is read-only and no global mapping state is used.
blockset_map_descriptor:
        jumpe 1,kret_neg1
        jumpl 2,kret_neg1
        move 4,1                         ; descriptor
        move 1,2                         ; logical working value
        caml 1,1(4)                      ; total
        jrst kret_neg1
        hlrz 0,(4)                       ; flags
        move 5,0
        andi 5,07                        ; member count
        jumpe 5,kret_neg1
        andi 0,070                       ; policy field
        jumpe 0,blockset_map_interleave
        caie 0,010                       ; CONCAT == 1 << 3
        jrst kret_neg1

; CONCAT maps consecutive logical windows to consecutive members.
blockset_map_concat:
        setz 7,
blockset_map_concat_loop:
        caml 7,5
        jrst kret_neg1
        move 6,4
        add 6,7
        hrrz 0,011(6)                    ; member length
        caml 1,0
        jrst blockset_map_concat_next
        move 2,1
        move 1,7
        jrst blockset_map_finish
blockset_map_concat_next:
        sub 1,0
        aoja 7,blockset_map_concat_loop

; INTERLEAVE is deliberately restricted to equal-sized homogeneous members.
; DSK270 and DRM236 sets have fixed geometry, so this replaces the much larger
; unequal-zone mapper.  Unequal member windows use CONCAT instead.
blockset_map_interleave:
        setz 0,
        div 0,5                          ; quotient AC0, member AC1
        move 2,0                         ; member-relative block

; AC1=member index, AC2=member-relative block, AC4=descriptor.
blockset_map_finish:
        move 7,1                         ; preserve member index
        move 6,4
        add 6,7
        move 1,2(6)                      ; physical unit
        hlrz 3,011(6)                    ; member base
        add 2,3
        popj 17,

