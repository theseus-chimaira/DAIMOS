; pipe_pdp10.s -- compact PDP-10 implementation of DAIMOS pipes/FIFOs.
;
; This replaces the larger compiler output after measurement showed a
; substantial permanent-KCORE win.  The representation and semantics remain
; those documented in pipe.h and the pipe/FIFO implementation design.
;
; struct pipe word offsets:
;   0 next_fifo, 1 fifo_node, 2 state, 3 refs,
;   4 read_event, 5 write_event, 6..045 packed 9-bit data.
; state packs head in bits 0..6 and count in bits 7..14.  Tail is derived as
; (head + count) & 0177.  refs is readers,,writers in two 18-bit halves.
        .text
        .globl  file_table
        .globl  mm_alloc_aligned
        .globl  mm_free
        .globl  fs_zero_words
        .globl  proc_wait_event
        .globl  proc_wait_event_intr
        .globl  proc_wakeup_event
        .globl  proc_event_apply
        .globl  proc_current_slot

; Allocate and zero a 046-word pipe object.  AC1 is fifo_node, zero for an
; anonymous pipe.  Return the stable low-18-bit physical base or zero.
pipe_alloc:
        push    17,010
        move    010,1
        add     17,[3,,3]
        setzm   -2(17)                  ; returned base
        movei   1,-2(17)
        movem   1,-1(17)               ; arg 6: basep
        setzm   (17)                    ; arg 5: MM_ALLOC_LOW
        movei   1,046                   ; words
        movei   2,1                     ; alignment
        movei   3,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   4,5                     ; PIPE_MM_OWNER
        pushj   17,mm_alloc_aligned
        jumpn   1,pipe_alloc_fail
        move    1,-2(17)
        jumpe   1,pipe_alloc_fail
        ; MM extent bases are 18-bit physical addresses by construction.
        movei   2,046
        pushj   17,fs_zero_words
        move    1,-2(17)
        movem   010,1(1)
        jrst    pipe_alloc_done
pipe_alloc_fail:
        setz    1,
pipe_alloc_done:
        sub     17,[3,,3]
        pop     17,010
        popj    17,


; Signal a pipe event in AC1 and wake every waiter.  A nonzero event value is
; sufficient; using SETOM avoids materializing/storing a separate constant.
pipe_signal_event:
        setom   (1)
        jrst    proc_wakeup_event

; Arm and sleep interruptibly on a user-visible pipe/FIFO event in AC1.
; Executive code is non-preemptible between clearing the event and publishing
; the wait.  ALRM returns -1; internal kernel waits continue to use the
; noninterruptible proc_wait_event entry directly.
pipe_wait_event:
        setzm   (1)
        jrst    proc_wait_event_intr

; Find the active named-FIFO object for AC1 vnode; return zero if absent.
pipe_fifo_find:
        move    2,1
        move    3,pipe_fifo_head
pipe_fifo_find_loop:
        jumpe   3,pipe_fifo_find_none
        came    2,1(3)
        jrst    pipe_fifo_find_next
        move    1,3
        popj    17,
pipe_fifo_find_next:
        move    3,(3)
        jrst    pipe_fifo_find_loop
pipe_fifo_find_none:
        setz    1,
        popj    17,

; Remove AC1 object from the active FIFO list.  The object itself stays live.
        .globl  pipe_fifo_unlink_object
pipe_fifo_unlink_object:
        movei   2,pipe_fifo_head
        move    3,pipe_fifo_head
pipe_fifo_unlink_loop:
        jumpe   3,pipe_fifo_unlink_done
        came    3,1
        jrst    pipe_fifo_unlink_next
        move    4,(3)
        movem   4,(2)
        setzm   (3)
        popj    17,
pipe_fifo_unlink_next:
        move    2,3
        move    3,(3)
        jrst    pipe_fifo_unlink_loop
pipe_fifo_unlink_done:
        popj    17,

; kword_t pipe_create(void)
        .globl  pipe_create
pipe_create:
        push    17,010
        skipn   2,file_table
        jrst    pipe_create_fail
        movei   010,020                 ; first free fd sentinel
        movei   3,0
pipe_create_scan:
        skipe   (2)
        jrst    pipe_create_next
        caie    010,020
        jrst    pipe_create_second
        move    010,3
        jrst    pipe_create_next
pipe_create_second:
        hrl     010,010                 ; first fd into LH
        hrr     010,3                   ; second fd into RH
        jrst    pipe_create_alloc
pipe_create_next:
        addi    2,2
        addi    3,1
        caige   3,020
        jrst    pipe_create_scan
        jrst    pipe_create_fail
pipe_create_alloc:
        setz    1,
        pushj   17,pipe_alloc
        jumpe   1,pipe_create_fail
        move    4,[01000001]            ; one reader, one writer
        movem   4,3(1)
        movei   4,1
        movem   4,5(1)                  ; writers may initially proceed
        tlo     1,070001                ; PIPE_PROVIDER, PIPE_KIND_STREAM

        move    2,file_table
        hlrz    3,010
        lsh     3,1
        add     3,2
        move    4,1
        tlo     4,0400000               ; FILE_META_READ
        movem   4,(3)
        setzm   1(3)
        hrrz    3,010
        lsh     3,1
        add     3,2
        tlo     1,0200000               ; FILE_META_WRITE
        movem   1,(3)
        setzm   1(3)

        move    1,010                 ; read fd,,write fd
        jrst    pipe_create_done
pipe_create_fail:
        seto    1,
pipe_create_done:
        pop     17,010
        popj    17,

; vnode_t pipe_fifo_open(vnode_t fifo_node, kword_t node_meta)
        .globl  pipe_fifo_open
pipe_fifo_open:
        push    17,010
        push    17,011
        move    7,1                     ; persistent FIFO vnode
        move    011,2                   ; requested descriptor metadata
        jumpe   7,pipe_fifo_open_bad
        jumpge  011,pipe_fifo_open_no_read
        jrst    pipe_fifo_open_access_ok
pipe_fifo_open_no_read:
        tlnn    011,0200000
        jrst    pipe_fifo_open_bad
pipe_fifo_open_access_ok:
        move    1,7
        pushj   17,pipe_fifo_find
        move    010,1
        jumpn   010,pipe_fifo_open_refs
        move    1,7
        pushj   17,pipe_alloc
        move    010,1
        jumpe   010,pipe_fifo_open_bad
        move    3,pipe_fifo_head
        movem   3,(010)
        movem   010,pipe_fifo_head

pipe_fifo_open_refs:
        jumpge  011,pipe_fifo_open_writer_ref
        movsi   3,1
        addm    3,3(010)
        movei   1,5(010)
        pushj   17,pipe_signal_event
pipe_fifo_open_writer_ref:
        tlnn    011,0200000
        jrst    pipe_fifo_open_rendezvous
        aos     3(010)
        movei   1,4(010)
        pushj   17,pipe_signal_event

pipe_fifo_open_rendezvous:
        jumpge  011,pipe_fifo_open_write_only
        tlne    011,0200000              ; O_RDWR never waits
        jrst    pipe_fifo_open_done
        hrrz    3,3(010)
        jumpn   3,pipe_fifo_open_done
        movei   1,4(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_fifo_open_intr
        jrst    pipe_fifo_open_done
pipe_fifo_open_write_only:
        hlrz    3,3(010)
        jumpn   3,pipe_fifo_open_done
        movei   1,5(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_fifo_open_intr
        jrst    pipe_fifo_open_done
pipe_fifo_open_intr:
        move    1,010
        move    2,011
        pushj   17,pipe_close_ref
        jrst    pipe_fifo_open_bad
pipe_fifo_open_done:
        move    1,010
        tlo     1,070001
        jrst    pipe_fifo_open_exit
pipe_fifo_open_bad:
        setz    1,
pipe_fifo_open_exit:
        pop     17,011
        pop     17,010
        popj    17,

        .globl  pipe_fifo_detach
pipe_fifo_detach:
        pushj   17,pipe_fifo_find
        jumpe   1,pipe_fifo_detach_done
        pushj   17,pipe_fifo_unlink_object
        setzm   1(1)
pipe_fifo_detach_done:
        popj    17,

        .globl  pipe_fifo_mount_busy
pipe_fifo_mount_busy:
        move    2,1
        move    3,pipe_fifo_head
pipe_fifo_mount_busy_loop:
        jumpe   3,pipe_fifo_mount_free
        ldb     4,[POINT 6,1(3),11]
        camn    4,2
        jrst    pipe_fifo_mount_yes
        move    3,(3)
        jrst    pipe_fifo_mount_busy_loop
pipe_fifo_mount_yes:
        movei   1,1
        popj    17,
pipe_fifo_mount_free:
        setz    1,
        popj    17,

; int pipe_readchar(vnode_t node)
        .globl  pipe_readchar
pipe_readchar:
        push    17,010
        hrrz    010,1
        jumpe   010,pipe_read_bad
pipe_read_retry:
        ldb     2,[POINT 8,2(010),28]
        jumpe   2,pipe_read_empty
        move    3,2(010)
        andi    3,0177
        movei   4,3
        andcm   4,3
        move    5,4
        lsh     5,3
        add     4,5
        move    5,3
        lsh     5,-2
        add     5,010
        move    1,6(5)
        movn    4,4
        lsh     1,0(4)
        andi    1,0777
        addi    3,1
        andi    3,0177
        subi    2,1
        lsh     2,7
        ior     3,2
        movem   3,2(010)
        push    17,1
        movei   1,5(010)
        pushj   17,pipe_signal_event
        pop     17,1
        pop     17,010
        popj    17,
pipe_read_empty:
        hrrz    2,3(010)
        jumpe   2,pipe_read_eof
        movei   1,4(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_read_bad
        jrst    pipe_read_retry
pipe_read_eof:
        hrroi   1,0777776
        jrst    pipe_read_error_done
pipe_read_bad:
        seto    1,
pipe_read_error_done:
        pop     17,010
        popj    17,

; int pipe_writechar(vnode_t node, unsigned int ch, unsigned int reserve)
        .globl  pipe_writechar
pipe_writechar:
        push    17,010
        push    17,011
        push    17,012
        hrrz    010,1
        move    011,2
        move    012,3
        jumpe   010,pipe_write_bad
        jumpe   012,pipe_write_bad
        caile   012,0200
        jrst    pipe_write_bad
pipe_write_retry:
        hlrz    4,3(010)
        jumpe   4,pipe_write_broken
        ldb     5,[POINT 8,2(010),28]
        movei   6,0200
        sub     6,5
        sub     6,012
        jumpl   6,pipe_write_wait
        move    4,2(010)
        andi    4,0177
        move    2,4
        add     2,5
        andi    2,0177
        movei   6,3
        andcm   6,2
        move    7,6
        lsh     7,3
        add     6,7
        movei   7,0777
        lsh     7,0(6)
        move    3,2
        lsh     3,-2
        add     3,010
        andca   7,6(3)
        move    1,011
        andi    1,0777
        lsh     1,0(6)
        ior     7,1
        movem   7,6(3)
        addi    5,1
        lsh     5,7
        ior     4,5
        movem   4,2(010)
        movei   1,4(010)
        pushj   17,pipe_signal_event
        setz    1,
        jrst    pipe_write_done
pipe_write_wait:
        movei   1,5(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_write_bad
        jrst    pipe_write_retry
pipe_write_broken:
        move    1,proc_current_slot
        movei   2,7                     ; SYS_EVENT_PIPE
        pushj   17,proc_event_apply
pipe_write_bad:
        seto    1,
pipe_write_done:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  pipe_add_ref
pipe_add_ref:
        move    6,1
        tlz     6,0707070               ; strip descriptor metadata
        lsh     6,-036
        caie    6,7                     ; PIPE_PROVIDER
        popj    17,
        hrrz    6,1
        jumpge  1,pipe_add_ref_write
        movsi   7,1
        addm    7,3(6)
pipe_add_ref_write:
        tlne    1,0200000
        aos     3(6)
        popj    17,

        .globl  pipe_add_refs
pipe_add_refs:
        jumpe   1,pipe_add_refs_done
        move    2,1
        movei   3,020
pipe_add_refs_loop:
        move    1,(2)
        pushj   17,pipe_add_ref
        addi    2,2
        sojg    3,pipe_add_refs_loop
pipe_add_refs_done:
        popj    17,

        .globl  pipe_close_ref
pipe_close_ref:
        push    17,010
        push    17,011
        hrrz    010,1
        jumpe   010,pipe_close_bad
        move    011,3(010)
        jumpge  2,pipe_close_writer
        hlrz    3,011
        jumpe   3,pipe_close_bad
        add     011,[-01000000]
pipe_close_writer:
        tlnn    2,0200000
        jrst    pipe_close_store
        hrrz    3,011
        jumpe   3,pipe_close_bad
        subi    011,1
pipe_close_store:
        movem   011,3(010)
        hlrz    3,011
        jumpn   3,pipe_close_writers
        movei   1,5(010)
        pushj   17,pipe_signal_event
pipe_close_writers:
        hrrz    3,011
        jumpn   3,pipe_close_live
        movei   1,4(010)
        pushj   17,pipe_signal_event
pipe_close_live:
        jumpn   011,pipe_close_ok
        skipn   1(010)
        jrst    pipe_close_free
        move    1,010
        pushj   17,pipe_fifo_unlink_object
pipe_close_free:
        move    1,010
        movei   2,3
        movei   3,5
        pushj   17,mm_free
        jumpn   1,pipe_close_bad
pipe_close_ok:
        setz    1,
        jrst    pipe_close_done
pipe_close_bad:
        seto    1,
pipe_close_done:
        pop     17,011
        pop     17,010
        popj    17,

        .bss
pipe_fifo_head:
        .block  1
