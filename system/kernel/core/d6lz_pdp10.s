; Allocation-free D6LZ36 VFS streaming decoder for PDP-6/PDP-10.
;
; The memory decoder d6lz36_decode is the common fixed low-core routine from
; system/stand/pdp6/common/decompressor.inc.  It is linked first in KCORE at
; 000060 and installed at the same address by every PDP-6 Stage1.
;
; VFS entry:
;   AC1=vnode, AC2=file_offset, AC3=compressed_words,
;   AC4=dst_words,,dst_address.
;
; VFS input uses one stack scratch word, so compressed EXEC loading allocates
; no compressed-input buffer.

        .text
        .globl d6lz36_decode_vfs
        .globl vfs_read_words

d6lz36_decode_vfs:
        push    17,10
        push    17,11
        push    17,13
        push    17,15
        push    17,1                    ; local -4: vnode
        push    17,[0]                  ; local -3: one-word input scratch
        push    17,2                    ; local -2: current file offset
        push    17,[0]                  ; local -1: control word
        hlrz    11,4                   ; output words remaining
        hrrz    10,4                   ; current output address
        push    17,10                   ; local  0: output base
        move    13,3                   ; compressed words remaining
        jumpe   13,d6lz_error
        jumpe   11,d6lz_error
        jumpe   10,d6lz_error
        setz    15,                    ; zero means load control

d6lz_vfs_token_loop:
        jumpn   15,d6lz_vfs_have_control
        pushj   17,d6lz_vfs_getword
        movem   5,-1(17)               ; control word
        movsi   15,0400000             ; control bit 35

d6lz_vfs_have_control:
        pushj   17,d6lz_vfs_getword

        move    0,-1(17)               ; control word
        and     0,15
        jumpe   0,d6lz_literal

        move    0,5                    ; descriptor bits 14..35 must be zero
        lsh     0,-016
        jumpn   0,d6lz_error

        move    0,5                    ; length = next7 + 3
        lsh     0,-7
        andi    0,0177
        addi    0,3

        andi    5,0177                 ; match source = dst - distance
        addi    5,1
        movn    5,5
        add     5,10
        camge   5,0(17)                ; output base
        jrst    d6lz_error
        camge   11,0
        jrst    d6lz_error
        sub     11,0

d6lz_match_loop:
        move    4,0(5)
        movem   4,0(10)
        addi    5,1
        addi    10,1
        sojg    0,d6lz_match_loop
        jrst    d6lz_token_done

d6lz_literal:
        movem   5,0(10)
        addi    10,1
        subi    11,1

d6lz_token_done:
        lsh     15,-1
        jumpn   11,d6lz_vfs_token_loop
        jumpn   13,d6lz_error          ; exact compressed payload required

d6lz_success:
        setz    1,
        jrst    d6lz_return

; Return next compressed VFS word in AC5.  Helper return is at (17), so the
; output-base/control/offset/scratch/vnode locals are -1..-5(17).
d6lz_vfs_getword:
        sojl    13,d6lz_vfs_getword_fail
        move    1,-5(17)               ; vnode
        move    2,-3(17)               ; file offset
        movei   3,-4(17)               ; scratch word
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    d6lz_vfs_getword_fail
        move    5,-4(17)
        aos     -3(17)                  ; file offset++
        popj    17,
d6lz_vfs_getword_fail:
        pop     17,0                   ; discard helper return PC
        jrst    d6lz_error

d6lz_error:
        seto    1,
d6lz_return:
        sub     17,[5,,5]              ; discard five locals
        pop     17,15
        pop     17,13
        pop     17,11
        pop     17,10
        popj    17,
