; Allocation-free D6LZ36 decoder for PDP-6/PDP-10.
;
; Memory entry:
;   AC1=dst, AC2=dst_words, AC3=src, AC4=src_words,
;   fifth C argument src_usedp is at -1(17) on entry.
;
; VFS entry:
;   AC1=vnode, AC2=file_offset, AC3=compressed_words,
;   AC4=dst_words,,dst_address.
;
; Both entries share descriptor validation and overlap-copy code.  VFS input
; uses one stack scratch word, so compressed EXEC loading allocates no buffer.

        .text
        .globl d6lz36_decode
        .globl d6lz36_decode_vfs
        .globl vfs_read_words

d6lz36_decode:
        move    0,-1(17)               ; stack-passed src_usedp
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        push    17,3                    ; local -1: immutable input base
        push    17,0                    ; local  0: src_usedp
        move    10,1                   ; current output
        move    11,2                   ; output words remaining
        move    12,3                   ; current input
        move    13,4                   ; input words remaining
        move    16,1                   ; output base

        jumpe   10,d6lz_mem_null_dst
        jrst    d6lz_mem_check_src
d6lz_mem_null_dst:
        jumpn   11,d6lz_error
d6lz_mem_check_src:
        jumpe   12,d6lz_mem_null_src
        jrst    d6lz_mem_start
d6lz_mem_null_src:
        jumpn   13,d6lz_error
d6lz_mem_start:
        setz    14,                    ; control word
        setz    15,                    ; zero means load control
        jumpe   11,d6lz_mem_success

d6lz_mem_token_loop:
        jumpn   15,d6lz_mem_have_control
        jumpe   13,d6lz_error
        move    14,0(12)
        addi    12,1
        subi    13,1
        movei   15,1
        lsh     15,043                 ; bit 35
d6lz_mem_have_control:
        jumpe   13,d6lz_error
        move    5,0(12)                ; token
        addi    12,1
        subi    13,1
        setz    6,                     ; memory frontend
        jrst    d6lz_process_token

d6lz_mem_continue:
        lsh     15,-1
        jumpn   11,d6lz_mem_token_loop

d6lz_mem_success:
        move    6,(17)                 ; src_usedp
        jumpe   6,d6lz_success
        move    0,12
        sub     0,-1(17)               ; immutable input base
        movem   0,0(6)
        jrst    d6lz_success

; int d6lz36_decode_vfs(node, file_offset, compressed_words,
;     dst_words,,dst_address)
d6lz36_decode_vfs:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        push    17,1                    ; local -1: vnode
        push    17,[0]                 ; local  0: one-word input scratch
        move    12,2                   ; current file offset
        move    13,3                   ; compressed words remaining
        hlrz    11,4                   ; output words remaining
        hrrz    10,4                   ; current output address
        move    16,10                  ; output base
        jumpe   13,d6lz_error
        jumpe   11,d6lz_error
        jumpe   10,d6lz_error
        setz    14,
        setz    15,

d6lz_vfs_token_loop:
        jumpn   15,d6lz_vfs_have_control
        pushj   17,d6lz_vfs_getword
        move    14,5
        movei   15,1
        lsh     15,043
d6lz_vfs_have_control:
        pushj   17,d6lz_vfs_getword
        movei   6,1                    ; VFS frontend
        jrst    d6lz_process_token

d6lz_vfs_continue:
        lsh     15,-1
        jumpn   11,d6lz_vfs_token_loop
        jumpn   13,d6lz_error           ; exact compressed payload required
        jrst    d6lz_success

; Return next compressed VFS word in AC5.  Helper return is at (17), so the
; shared scratch and vnode locals are at -1(17) and -2(17).
d6lz_vfs_getword:
        jumpe   13,d6lz_vfs_getword_fail
        move    1,-2(17)               ; vnode
        move    2,12
        movei   3,-1(17)               ; scratch word
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    d6lz_vfs_getword_fail
        move    5,-1(17)
        addi    12,1
        subi    13,1
        popj    17,
d6lz_vfs_getword_fail:
        pop     17,0                    ; discard helper return PC
        jrst    d6lz_error

; Shared token decoder. AC5=token; AC6=0 for memory, nonzero for VFS.
d6lz_process_token:
        move    0,14
        and     0,15
        jumpe   0,d6lz_literal

        move    0,5                    ; bits 14..35 must be zero
        lsh     0,-016
        jumpn   0,d6lz_error

        move    3,5                    ; distance = low7 + 1
        andi    3,0177
        addi    3,1
        move    0,10
        sub     0,16
        sub     0,3
        jumpl   0,d6lz_error

        move    4,5                    ; length = next7 + 3
        lsh     4,-7
        andi    4,0177
        addi    4,3
        move    0,11
        sub     0,4
        jumpl   0,d6lz_error

        move    5,10                   ; overlap-safe forward copy
        sub     5,3
d6lz_match_loop:
        move    0,0(5)
        movem   0,0(10)
        addi    5,1
        addi    10,1
        subi    11,1
        sojg    4,d6lz_match_loop
        jrst    d6lz_token_done

d6lz_literal:
        movem   5,0(10)
        addi    10,1
        subi    11,1
d6lz_token_done:
        jumpn   6,d6lz_vfs_continue
        jrst    d6lz_mem_continue

d6lz_success:
        setz    1,
        jrst    d6lz_return
d6lz_error:
        seto    1,
d6lz_return:
        sub     17,[2,,2]              ; discard two local words
        pop     17,16
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
